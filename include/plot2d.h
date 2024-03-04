#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>
#include <cfloat>

#include <wex.h>

// minimum data range that will produce sensible plots
#define minDataRange 0.000001

namespace wex
{
    namespace plot
    {
        class plot;

        /// @cond

        /** @brief Maintain indices of circular buffer
         *
         * When the buffer is full, new data over-writes the oldest ( wraps around )
         * The data is stored in an external buffer
         * Wrap around is done by manipulating the indices, rather than actually moving the buffer contents
         */
        class cCircularBuffer
        {
            int myCurrentID;     // buffer index for current iteration
            int myLastValid;     // index of most recent data point
            int mySize;          // largest buffer index
            bool myfWrapped;     // true if buffer is full aand wrap around has occurred
            bool myfCurrentLast; // true if current iteration is at most recent data point
            bool myfIterating;   // an iteration is in progress

        public:
            cCircularBuffer()
                : myCurrentID(-1), myLastValid(-1), mySize(-1),
                  myfWrapped(false), myfCurrentLast(false), myfIterating(false)
            {
            }

            /// @brief set largest buffer index
            /// @param s size of buffer
            /// Must be called before any data is added
            void set(int s)
            {
                mySize = s;
            }

            /// true if buffer has some data
            bool isValidData() const
            {
                return myLastValid >= 0;
            }

            /// true if buffer is full of data
            bool isFull()
            {
                return myfWrapped;
            }

            /// register an addition of a data point to the buffer
            /// throws exception if buffer size has not been set
            /// throws exception if called while an iteration is ongoing
            int add()
            {
                if (mySize < 0)
                    throw std::logic_error(
                        "cCircularBuffer::add() called without size buffer");

                if (myfIterating)
                    throw std::logic_error(
                        "cCircularBuffer::add() called during iteration");

                myLastValid++;
                if (myLastValid > mySize)
                {
                    myLastValid = 0;
                    myfWrapped = true;
                }
                return myLastValid;
            }

            /// @brief start new iteration through buffer from oldest to most recent
            /// @return first index, -1 if buffer empty
            /// iteration must run to completion without adding any new data
            int first()
            {
                if (myLastValid < 0)
                    return -1;
                myfIterating = true;
                if (!myfWrapped)
                {
                    myCurrentID = 0;
                    myfCurrentLast = false;
                    if (myLastValid == 0)
                        myfCurrentLast = true;
                }
                else
                {
                    myCurrentID = myLastValid + 1;
                    if (myCurrentID > mySize)
                        myCurrentID = 0;
                    myfCurrentLast = false;
                }
                return myCurrentID;
            }

            /// @brief next buffer index in current iteration
            /// @return buffer index, -1 if iteration complete or problem
            int next()
            {
                // check for no data in buffer
                if (myLastValid < 0)
                    return -1;

                // check for end of data
                if (myfCurrentLast)
                {
                    myfIterating = false;
                    return -1;
                }

                myCurrentID++;

                if (myCurrentID > mySize)
                    myCurrentID = 0;

                if (myCurrentID == myLastValid)
                    myfCurrentLast = true;

                return myCurrentID;
            }
        };

        /**
         * @brief Scale state machine
         *
         * https://github.com/JamesBremner/windex/issues/30#issuecomment-1971379013
         */
        class scaleStateMachine
        {
        public:
            enum class eState
            {
                none,
                fit,
                fix,
                fitzoom,
                fixzoom,
            };
            enum class eEvent
            {
                start,
                zoom,
                unzoom,
                fix,
                fit,
            };

            eState myState;

            scaleStateMachine()
                : myState(eState::fit)
            {
            }

            /// @brief Handle an event
            /// @param event
            /// @return new state, or eState::none if event to be ignored
            eState event(eEvent event)
            {
                switch (event)
                {

                case eEvent::start:
                    // handled by constructor
                    return eState::fit;

                case eEvent::zoom:
                    switch (myState)
                    {
                    case eState::fit:
                        myState = eState::fitzoom;
                        break;
                    case eState::fix:
                        myState = eState::fixzoom;
                        break;
                    default:
                        return eState::none;
                    }
                    break;

                case eEvent::unzoom:
                    switch (myState)
                    {
                    case eState::fitzoom:
                        myState = eState::fit;
                        break;
                    case eState::fixzoom:
                        myState = eState::fix;
                        break;
                    default:
                        return eState::none;
                    }
                    break;

                case eEvent::fix:
                    switch (myState)
                    {
                    case eState::fit:
                        myState = eState::fix;
                        break;
                    default:
                        return eState::none;
                    }
                    break;

                case eEvent::fit:
                    switch (myState)
                    {
                    case eState::fit:
                        break;
                    case eState::fix:
                        myState = eState::fit;
                        break;
                    default:
                        return eState::none;
                    }
                    break;

                default:
                    throw std::runtime_error(
                        "plot scaleStateMachine unrecognized event");
                }

                // std::cout << "scaleStateMachine state change " << (int)myState << "\n";

                return myState;
            }
        };

        /**
         * @brief Manage X value
         *
         * Each point along the x axis is convertable to
         *
         * XP  the pixel where iy is displayed
         * XI  the index into the data buffer where the data value is stored
         * XU  the user value ascribed to the point
         *
         */
        class XScale
        {
            scaleStateMachine::eState &theState;

            int xpmin; // min pixel
            int xpmax; // max pixel

            int ximin; // min data index
            int ximax; // max data index

            double xumin;   // min displayed x user value
            double xuximin; // user x value for ximin
            double xumax;   // max user x value displayed
            double xixumin; // data index for user x min

            double xuminfix;
            double xumaxfix;
            double xuminZoom;
            double xumaxZoom;

            double sxi2xu; // scale from data index to x user
            double sxi2xp; // scale from data index to pixel
            double sxu2xp; // scale from x user to pixel

        public:
            XScale(scaleStateMachine &machine)
                : theState(machine.myState)
            {
            }
            void xiSet(int min, int max)
            {
                ximin = min;
                ximax = max;
            }
            void xpSet(int min, int max)
            {
                xpmin = min;
                xpmax = max;
            }

            /// @brief set data index to user x conversion parameters
            /// @param u0   // user x at start of data buffer
            /// @param sc   // scale from data buffer inex to user x

            void xi2xuSet(double u0, double sc)
            {
                xuximin = u0;
                sxi2xu = sc;
            }
            void fixSet(double min, double max)
            {
                xuminfix = min;
                xumaxfix = max;
            }

            /// @brief switch on zooming into a subset of the data
            /// @param umin minimum user x to display
            /// @param umax maximum user x to display

            void zoom(double umin, double umax)
            {
                xuminZoom = umin;
                xumaxZoom = umax;
            }
            void zoomExit()
            {
            }

            void calculate()
            {
                switch (theState)
                {
                case scaleStateMachine::eState::fit:
                    xumin = xuximin;
                    xumax = xumin + sxi2xu * ximax;
                    xixumin = 0;
                    sxi2xp = (double)(xpmax - xpmin) / (ximax - ximin);
                    sxu2xp = (xpmax - xpmin) / (xumax - xumin);
                    break;

                case scaleStateMachine::eState::fix:
                {
                    xumin = xuminfix;
                    xumax = xumaxfix;
                    xixumin = (xumin - xuximin) / sxi2xu;
                    double xixumax = (xumax - xuximin) / sxi2xu;
                    sxi2xp = (xpmax - xpmin) / (xixumax - xixumin);
                    sxu2xp = (xpmax - xpmin) / (xumax - xumin);
                }
                break;

                case scaleStateMachine::eState::fitzoom:
                case scaleStateMachine::eState::fixzoom:
                {
                    xumin = xuminZoom;
                    xumax = xumaxZoom;
                    xixumin = (xumin - xuximin) / sxi2xu;
                    double xixumax = (xumax - xuximin) / sxi2xu;
                    sxi2xp = (xpmax - xpmin) / (xixumax - xixumin);
                    sxu2xp = (xpmax - xpmin) / (xumax - xumin);
                }
                break;
                }
            }

            int XI2XP(double xi) const
            {
                // std::cout << "XI2XP " << xi
                //     << " xpmin " << xpmin
                //     << " sxi2xp " << sxi2xp
                //     << " xixumin " << xixumin
                //     << "\n";

                return round(xpmin + sxi2xp * (xi - xixumin));
            }
            double XP2XU(int pixel) const
            {
                return xumin + (pixel - xpmin) / sxu2xp;
            }
            int XU2XP(double xu) const
            {
                return round(xpmin + sxu2xp * (xu - xumin));
            }

            int XUmin() const
            {
                return xumin;
            }
            int XUmax() const
            {
                return xumax;
            }
            int XPmin() const
            {
                return xpmin;
            }
            int XPmax() const
            {
                return xpmax;
            }

            void text() const
            {
                std::cout
                    << "state " << (int)theState
                    << " xpstart " << xpmin << " xpmax " << xpmax
                    << " xistart " << ximin << " ximax " << ximax
                    << " xustart " << xumin << " xumax " << xumax
                    << " sxi2xp " << sxi2xp
                    << " sxi2xu " << sxi2xu
                    << "\n";
            }
        };

        /// @brief Manage connversions between data values and y pixels
        ///
        /// Note: pixels run from 0 at top of window towards bottom
        class YScale
        {
            scaleStateMachine::eState &theState;
            double yvmin;     // smallest value in data currently displayed
            double yvmax;     // largest value in data currently displayed
            int ypmin;        // y pixel showing smallest data value
            int ypmax;        // y pixel showing largest data value
            double syv2yp;    // scale from data value to y pixel
            double yvminZoom; // smallest value in data when zoomed
            double yvmaxZoom; // largest value in data when zoomed
            double yvminFit;  // smallest value in data when fitted
            double yvmaxFit;  // largest value in data when fitted
            double yvminFix;  // smallest value in data when fixed
            double yvmaxFix;  // largest value in data when fixed

        public:
            YScale(scaleStateMachine &scaleMachine)
                : theState(scaleMachine.myState)
            {
            }

            void YVrange(double min, double max)
            {
                yvminFit = min;
                yvmaxFit = max;
            }

            double YVrange() const
            {
                return yvmax - yvmin;
            }

            /// @brief  set range of pixels
            /// @param min  pixel that will represent the lowest value
            /// @param max pixel that will represent the largest value
            /// Since the pixel indices run from the top of the window, min wil be greater than max

            void YPrange(int min, int max)
            {
                ypmin = min;
                ypmax = max;
                calculate();
            }

            void zoom(double min, double max)
            {
                yvminZoom = min;
                yvmaxZoom = max;
            }

            void fixSet(double min, double max)
            {
                yvminFix = min;
                yvmaxFix = max;
            }

            double YP2YV(int pixel) const
            {
                return yvmin - (ypmin - pixel) / syv2yp;
            }
            int YV2YP(double v) const
            {
                return ypmin + syv2yp * (v - yvmin);
            }
            int YPmin() const
            {
                return ypmin;
            }
            int YPmax() const
            {
                return ypmax;
            }
            void text() const
            {
                std::cout << "yv " << yvmin << " " << yvmax
                          << " xp " << ypmin << " " << ypmax
                          << " " << syv2yp
                          << "\n";
            }

            void calculate()
            {
                switch (theState)
                {
                case scaleStateMachine::eState::fit:
                    yvmin = yvminFit;
                    yvmax = yvmaxFit;
                    break;

                case scaleStateMachine::eState::fix:
                    yvmin = yvminFix;
                    yvmax = yvmaxFix;
                    break;

                case scaleStateMachine::eState::fitzoom:
                case scaleStateMachine::eState::fixzoom:
                    yvmin = yvminZoom;
                    yvmax = yvmaxZoom;
                    break;
                }
                double yvrange = yvmax - yvmin;
                if (fabs(yvrange) < 0.00001)
                {
                    // seems like there are no meaningful data
                    syv2yp = 1;
                    return;
                }

                syv2yp = -(ypmin - ypmax) / yvrange;
            }
        };

        // @endcond

        /** \brief Single trace to be plotted

            Application code should not attempt to construct a trace
            Rather call one of plot::AddPointTrace, plot::AddRealTimeTrace or plot::AddStaticTrace
            which return a reference to the trace which can be configured
            and be populated with data.

            <pre>
                form fm;

                 construct plot to be drawn on form

                plot::plot thePlot( fm );

                 construct plot trace

                auto t1 = thePlot.AddStaticTrace();

                 provide some data for  trace

                std::vector< double > d1 { 10, 15, 20, 25, 30, 25, 20, 15, 10 };
                t1.set( d1 );

                 plot in blue

                t1.color( colors::blue );

                 show and run

                fm.show();
                exec();
            </pre>
        */
        class trace
        {
        public:
            enum class eType
            {
                plot,
                realtime,
                scatter
            } myType; // trace type

            /** \brief set plot data
                @param[in] y vector of data points to display

                Replaces any existing data.  Plot is NOT refreshed.
                An exception is thrown when this is called
                for a trace that is not plot or scatter type
            */
            void set(const std::vector<double> &y)
            {
                if ((myType != eType::plot) && (myType != eType::scatter))
                    throw std::runtime_error("plot2d error: plot data added to non plot/scatter trace");

                myY = y;
            }
            void setScatterX(const std::vector<double> &x)
            {
                if (myType != eType::scatter)
                    throw std::runtime_error("plot2d error: plot X added to non scatter trace");

                myX = x;
            }
            std::vector<double> get() const
            {
                return myY;
            }

            /** \brief add new value to real time data
                @param[in] y the new data point

                An exception is thrown when this is called
                for a trace that is not real time type.
            */
            void add(double y)
            {
                if (myType != eType::realtime)
                    throw std::runtime_error("plot2d error: realtime data added to non realtime trace");
                myY[myCircular.add()] = y;
            }

            /** \brief add point to scatter trace
                @param[in] x location
                @param[in] y location

                An exception is thrown when this is called
                for a trace that is not scatter type
            */

            void add(double x, double y)
            {
                if (myType != eType::scatter)
                    throw std::runtime_error("plot2d error: point data added to non scatter type trace");
                myX.push_back(x);
                myY.push_back(y);
            }

            /// @brief clear data from trace
            void clear()
            {
                myX.clear();
                myY.clear();
            }

            /// set color
            void color(int clr)
            {
                myColor = clr;
            }
            int color() const
            {
                return myColor;
            }

            /// set trace thickness in pixels
            void thick(int t)
            {
                myThick = t;
            }
            int thick() const
            {
                return myThick;
            }

            /// get number of points
            int size() const
            {
                return (int)myY.size();
            }

            /** y value at fractional position along x-axis
                @param[in] xfraction x-axis position 0 to 1
                @return y value at position, 0 if xfraction outside range 0 to 1
            */
            double value(double xfraction)
            {
                if (0 > xfraction || xfraction > 1)
                    return 0;
                return myY[(int)(xfraction * myY.size())];
            }

            const std::vector<double> &getY()
            {
                if (myType != eType::realtime)
                    return myY;

                static std::vector<double> ret;
                ret.clear();
                for (int yidx = myCircular.first();
                     yidx >= 0;
                     yidx = myCircular.next())
                    ret.push_back(myY[yidx]);
                return ret;
            }

        private:
            friend plot;

            plot *myPlot;               // plot where this trace is displayed
            std::vector<double> myX;    // X value of each data point
            std::vector<double> myY;    // Y value of each data point
            cCircularBuffer myCircular; // maintain indices of circular buffer used by real time trace
            int myColor;                // trace color
            int myThick;                // trace thickness

            /** CTOR
            Application code should not call this constructor
            Rather call one of plot::AddPointTrace, plot::AddRealTimeTrace or plot::AddStaticTrace

            */
            trace()
                : myThick(1), myType(eType::plot)
            {
            }

            /// set plot where this trace will appear
            void Plot(plot *p)
            {
                myPlot = p;
            }

            /** \brief Convert trace to real time operation
            @param[in] w number of data points to display

            Data points older than w scroll off the left edge of the plot and are lost

            X-axis represents time, with 'present' at right end.
            Assumes data points are evenly spaced in time
            */
            void realTime(int w)
            {
                myType = eType::realtime;
                myY.clear();
                myY.resize(w);
                myCircular.set(w);
            }

            /** \brief Convert trace to point operation for scatter plots */
            void scatter()
            {
                myType = eType::scatter;
                myY.clear();
                myX.clear();
            }

            /// min and max values in trace
            void bounds(
                int &txmin, int &txmax,
                double &tymin, double &tymax)
            {
                if (myY.size())
                {

                    // find the x limits

                    txmin = 0;
                    txmax = myY.size() - 1;

                    // if (myType == eType::realtime || myX.size() == 0)
                    //  {
                    //      txmin = 0;
                    //      txmax = myY.size();
                    //  }
                    //  else
                    //  {
                    //      // auto result = std::minmax_element(
                    //      //     myX.begin(),
                    //      //     myX.end());
                    //      // txmin = *result.first;
                    //      // txmax = *result.second;
                    //      txmin =
                    //  }

                    // find the y limits

                    if (myType == eType::realtime)
                    {
                        if (!myCircular.isValidData())
                        {
                            // no data is buffer
                            tymin = -5;
                            tymax = 5;
                            return;
                        }

                        if (myCircular.isFull())
                        {
                            // buffer is full
                            auto result = std::minmax_element(
                                myY.begin(),
                                myY.end());
                            tymin = *result.first;
                            tymax = *result.second;
                        }

                        // buffer is partially full
                        // set bounds using the data that has been received so far
                        tymin = myY[0];
                        tymax = myY[0];
                        for (int idx = myCircular.first();
                             idx >= 0;
                             idx = myCircular.next())
                        {
                            double y = myY[idx];
                            if (y < tymin)
                                tymin = y;
                            if (y > tymax)
                                tymax = y;
                        }
                    }
                    else
                    {
                        // scatter or static trace
                        auto result = std::minmax_element(
                            myY.begin(),
                            myY.end());
                        tymin = *result.first;
                        tymax = *result.second;
                    }
                }
            }
        };

        /** \brief Draw a 2D plot

        The plot contains one or more traces.

        Each trace can be of one of three types:

        - Plot: succesive y-values with line drawn between them.
        - Scatter: succesive x,y-values with box around each point
        - Realtime: a specified number of the most recent y-values

        Any number of plot and scatter traces can be shown together,
        only one realtime trace may be present in a plot.

        ZOOM

        To zoom into a selected area of the plot
        click on the top left of the area and drag the mouse to the bottom right.
        The plot will draw a box around the area selected.
        When the left button is released, the plot will zoom in to show just the selected area.
        To restore auto-fit, right click on the plot.

        If application code is required to use the right click ( e.g. for a context pop-up menu )
        the event handler must first call the plot method autofit.
        <pre>
            myPlotB.events().clickRight([&]
            {
                myPlotB.autoFit();
                ...
            });
        </pre>

        Sample plot application:

        <pre>
        #include "wex.h"
        #include "plot2d.h"

        int main()
        {
                wex::gui& fm = wex::maker::make();
                fm.move( {50,50,1200,600} );

                // construct plot to be drawn on form
                wex::plot::plot& thePlot = wex::maker::make<wex::plot::plot>(fm);
                thePlot.move( {30,30,1200,600});
                //thePlot.Grid( true );

                // resize plot when form resizes
                fm.events().resize([&](int w, int h )
                {
                    thePlot.move( {30,30,w,h} );
                    thePlot.update();
                });

                // construct plot traces
                wex::plot::trace& t1 = thePlot.AddStaticTrace();
                wex::plot::trace& t2 = thePlot.AddStaticTrace();

                // provide some data for first trace
                std::vector< double > d1 { 10, 15, 20, 25, 30, 25, 20, 15, 10 };
                t1.set( d1 );

                // plot in blue
                t1.color( 0x0000FF );

                // provide data for second trace
                std::vector< double > d2 { 20, 30, 40, 50, 60, 50, 40, 30, 20 };
                t2.set( d2 );

                // plot in red
                t2.color( 0xFF0000 );

                // show and run
                fm.show();
                fm.run();
        }
        </pre>

         */

        class plot : public gui
        {
        public:
            /** \brief CTOR
                @param[in] parent window where plot will be drawn
            */
            plot(gui *parent)
                : gui(parent), myfDrag(false),
                  myXScale(myScaleStateMachine),
                  myYScale(myScaleStateMachine)
            {
                text("Plot");

                events().draw(
                    [this](PAINTSTRUCT &ps)
                    {
                        // calculate scaling factors
                        // so plot will fit
                        if (!CalcScale(ps.rcPaint.right,
                                       ps.rcPaint.bottom))
                        {
                            wex::msgbox("Plot has no data");
                            return;
                        }

                        wex::shapes S(ps);

                        // draw axis
                        drawYAxis(S);
                        drawXAxis(
                            S,
                            ps.rcPaint.bottom - 20);

                        // loop over traces
                        for (auto t : myTrace)
                            drawTrace(t, S);

                        drawSelectedArea(ps);
                    });

                events().click(
                    [&]
                    {
                        // start dragging for selected area
                        auto m = getMouseStatus();
                        myStartDragX = m.x;
                        myStartDragY = m.y;
                        myStopDragX = -1;
                        myfDrag = true;
                    });
                events().mouseMove(
                    [&](wex::sMouse &m)
                    {
                        // extend selected area as mouse is dragged
                        dragExtend(m);
                    });
                events().mouseUp(
                    [&]
                    {
                        // check if user has completed a good drag operation
                        if (isGoodDrag())
                        {
                            // check for valid event
                            if (myScaleStateMachine.event(scaleStateMachine::eEvent::zoom) != scaleStateMachine::eState::none)
                            {
                                double myZoomXMin = myXScale.XP2XU(myStartDragX);
                                double myZoomXMax = myXScale.XP2XU(myStopDragX);
                                double myZoomYMax = myYScale.YP2YV(myStartDragY);
                                double myZoomYMin = myYScale.YP2YV(myStopDragY);

                                myXScale.zoom(myZoomXMin, myZoomXMax);
                                myYScale.zoom(myZoomYMin, myZoomYMax);

                                // myfZoom = true;

                                // std::cout << myStartDragX <<" "<< myStopDragX <<" "<< myStartDragY <<" "<< myStopDragY << "\n";
                                // std::cout << myZoomXMin <<" "<< myZoomXMax <<" "<< myZoomYMin <<" "<< myZoomYMax << "\n";
                            }
                        }
                        myfDrag = false;
                        update();
                    });

                events().clickRight(
                    [&]
                    {
                        myScaleStateMachine.event(scaleStateMachine::eEvent::unzoom);
                        update();
                    });
            }

            ~plot()
            {
            }

            /** \brief Add static trace
                @return reference to new trace

                The data in a static trace does not change
                A line is drawn between successive points
                Specify y location only for each point.
                The points will be evenly distributed along the x-axis
            */
            trace &AddStaticTrace()
            {
                trace *t = new trace();
                t->Plot(this);
                myTrace.push_back(t);
                return *t;
            }

            /** \brief Add real time trace
                @param[in] w number of recent data points to display
                @return reference to new trace

                The data in a real time trace receives new values from time to time
                The display shows w recent values.  Older values scroll off the
                left hand side of the plot and disappear.
            */
            trace &AddRealTimeTrace(int w)
            {
                trace *t = new trace();
                t->Plot(this);
                t->realTime(w);
                myTrace.push_back(t);
                return *t;
            }

            /** \brief Add scatter trace
                @return reference to new trace

                A static trace for scatter plots
                No line between points,
                  box around each point.
                Specify x AND y locations for each point.
            */
            trace &AddScatterTrace()
            {
                trace *t = new trace();
                t->Plot(this);
                t->scatter();
                myTrace.push_back(t);
                return *t;
            }

            /** \brief Enable display of grid markings */
            void grid(bool enable)
            {
                myfGrid = enable;
            }

            /// @brief Set fixed scale
            /// @param minX minimum user x
            /// @param maxX maximum user X
            /// @param minY minimum Y
            /// @param maxY maximum Y

            void setFixedScale(
                double minX, double maxX, double minY, double maxY)
            {
                if (maxX <= minX || maxY <= minY)
                    throw std::runtime_error(
                        "plot::setFixedScale bad params");

                // change scale state
                if (
                    myScaleStateMachine.event(
                        scaleStateMachine::eEvent::fix) == scaleStateMachine::eState::none)
                    return;

                if (!myfXset)
                    XUValues(0, 1);

                myXScale.fixSet(minX, maxX);
                myYScale.fixSet(minY, maxY);

                // myXScale.text();
            }

            void setFitScale()
            {
                if (
                    myScaleStateMachine.event(
                        scaleStateMachine::eEvent::fit) == scaleStateMachine::eState::none)
                    throw std::runtime_error(
                        "wex plot cannot return to fit scale");
            }

            int traceCount() const
            {
                return (int)myTrace.size();
            }

            /// Remove all traces from plot
            void clear()
            {
                myTrace.clear();
            }

            /** Disable auto-fit scaling and set Y minumum, maximum
                @param[in] min enforced min Y
                @param[in] max enforced max Y
            */
            // void fixYVminmax(double min, double max)
            // {
            //     myfFit = false;
            //     myfZoom = false;
            //     myYScale.YVrange(min, max);
            // }

            /// Enable auto-fit scaling and remove any zoom setting
            // void autoFit()
            // {
            //     myfFit = true;
            //     myfDrag = false;
            //     myfZoom = false;
            //     myXScale.zoomExit();
            //     update();
            // }

            void dragExtend(sMouse &m)
            {
                if (!myfDrag)
                    return;
                myStopDragX = m.x;
                myStopDragY = m.y;
                update();
            }
            /** Set conversion from index of x value buffer to x user units
            @param[in] start x user value of first data point
            @param[in] scale to convert from index to user value

            Used to label the x-axis and draw grid lines if enabled
            */
            void XUValues(
                float start_xu,
                float scale_xi2xu)
            {
                myXScale.xi2xuSet(start_xu, scale_xi2xu);
                myfXset = true;
            }

            /// @brief for backward compatability
            /// @param start_xu
            /// @param scale_xi2xu
            void XValues(
                float start_xu,
                float scale_xi2xu)
            {
                XUValues(start_xu, scale_xi2xu);
            }

            /// @brief calculate scaling factors so plot will fit in window client area
            /// @return true if succesful
            bool CalcScale(int w, int h)
            {
                // std::cout << "Plot::CalcScale " << w << " " << h << "\n";

                // If user has not called XValues(), set X-axis scale to 1
                if (!myfXset)
                    XUValues(0, 1);

                // check there are traces that need to be drawn
                if (!myTrace.size())
                    return false;

                // set pixel ranges for the axis
                myYScale.YPrange(h - 40, 10);
                myXScale.xpSet(50, w - 70);

                int ximin, ximax;
                double ymin, ymax;
                switch (myScaleStateMachine.myState)
                {
                case scaleStateMachine::eState::fit:

                    calcDataBounds(ximin, ximax, ymin, ymax);
                    myXScale.xiSet(ximin, ximax);
                    myYScale.YVrange(ymin, ymax);
                    break;

                case scaleStateMachine::eState::fix:
                    calcDataBounds(ximin, ximax, ymin, ymax);
                    myXScale.xiSet(ximin, ximax);
                    break;

                case scaleStateMachine::eState::fitzoom:
                case scaleStateMachine::eState::fixzoom:
                    break;

                default:
                    return false;
                }

                myXScale.calculate();
                myYScale.calculate();

                // myXScale.text();

                return true;
            }

            std::vector<trace *> &traces()
            {
                return myTrace;
            }

            // bool isZoomed() const
            // {
            //     return myfZoom;
            // }

            /// get X user value from x pixel
            double pixel2Xuser(int xpixel) const
            {
                return myXScale.XP2XU(xpixel);
            }
            int xuser2pixel(double xu) const
            {
                return myXScale.XU2XP(xu);
            }
            /// get Y user value from y pixel
            double pixel2Yuser(int ypixel) const
            {
                return myYScale.YP2YV(ypixel);
            }

        private:
            /// plot traces
            std::vector<trace *> myTrace;

            // scales
            scaleStateMachine myScaleStateMachine;
            XScale myXScale;
            YScale myYScale;

            bool myfGrid; // true if tick and grid marks reuired
            bool myfXset; // true if the x user range has been set
            bool myfDrag; // drag in progress

            int myStartDragX;
            int myStartDragY;
            int myStopDragX;
            int myStopDragY;

            void calcDataBounds(
                int &xmin, int &xmax,
                double &ymin, double &ymax)
            {
                myTrace[0]->bounds(
                    xmin, xmax,
                    ymin, ymax);
                for (auto &t : myTrace)
                {
                    int txmin, txmax;
                    double tymin, tymax;
                    txmin = txmax = tymax = 0;
                    tymin = std::numeric_limits<double>::max();
                    t->bounds(txmin, txmax, tymin, tymax);
                    if (txmin < xmin)
                        xmin = txmin;
                    if (txmax > xmax)
                        xmax = txmax;
                    if (tymin < ymin)
                        ymin = tymin;
                    if (tymax > ymax)
                        ymax = tymax;
                }
            }
            bool isGoodDrag()
            {
                return (myfDrag && myStopDragX > 0 && myStopDragX > myStartDragX && myStopDragY > myStartDragY);
            }

            std::vector<double> ytickValues()
            {
                std::vector<double> vl;
                double mn = myYScale.YPmax();
                double mx = myYScale.YPmin();
                double range = mx - mn;
                if (range < minDataRange)
                {
                    // plot is single valued
                    // display just one tick
                    vl.push_back(mn);
                    return vl;
                }
                double inc = myYScale.YVrange() / 4;
                double tickValue;
                if (inc > 1)
                {
                    inc = (int)inc;
                    tickValue = myYScale.YP2YV(myYScale.YPmin());
                }
                else
                {
                    tickValue = mn;
                }
                // if (tick < 0)
                //     return vl;

                while (true)
                {
                    double v = tickValue;
                    if (v > 100)
                        v = ((int)v / 100) * 100;
                    else if (v > 10)
                        v = ((int)v / 10) * 10;
                    vl.push_back(v);
                    tickValue += inc;
                    if (tickValue >= mx)
                        break;
                }
                vl.push_back(mx);
                return vl;
            }
            /** format number with 2 significant digits
            https://stackoverflow.com/a/17211620/16582
            */
            std::string numberformat(double f)
            {
                if (f == 0)
                {
                    return "0";
                }
                int n = 2;                                         // number of significant digits
                int d = (int)::floor(::log10(f < 0 ? -f : f)) + 1; /*digits before decimal point*/
                double order = ::pow(10., n - d);
                std::stringstream ss;
                ss << std::fixed << std::setprecision(std::max(n - d, 0)) << round(f * order) / order;
                return ss.str();
            }
            void drawYAxis(wex::shapes &S)
            {
                S.color(0xFFFFFF - bgcolor());
                S.textHeight(15);

                S.line({50, myYScale.YPmin(),
                        50, myYScale.YPmax()});

                for (double y : ytickValues())
                {
                    int yp = myYScale.YV2YP(y);
                    S.text(numberformat(y),
                           {0, yp - 8, 50, 15});
                    S.line({50, yp,
                            60, yp});
                    if (myfGrid)
                    {
                        auto kpmax = myXScale.XPmax();
                        for (int kp = 65;
                             kp < kpmax;
                             kp += 25)
                        {
                            S.pixel(kp, yp);
                            S.pixel(kp + 1, yp);
                        }
                    }
                }
                // int yp = scale::get().Y2Pixel(mx);
                //  S.text(myMaxYLabel,
                //         {0, yp + 10, 50, 15});
            }
            void drawXAxis(wex::shapes &S, int ypos)
            {
                S.color(0xFFFFFF - bgcolor());
                S.textHeight(15);
                S.line({myXScale.XPmin(), ypos, myXScale.XPmax(), ypos});
                if (!myfGrid)
                {
                    // there is no grid
                    // so just label the minimum, maximum x points
                    float xmin_label_value = 0;
                    float xmax_label_value = 100;
                    if (myfXset)
                    {
                        xmin_label_value = myXScale.XUmin();
                        xmax_label_value = myXScale.XUmax();
                    }
                    S.text(std::to_string((int)xmin_label_value), {myXScale.XPmin(), ypos + 3, 50, 15});
                    S.text(std::to_string((int)xmax_label_value), {myXScale.XPmax() - 25, ypos + 3, 50, 15});
                    return;
                }
                // there is a grid

                int tickCount = 8;
                float xutickinc = (myXScale.XUmax() - myXScale.XUmin()) / tickCount;

                // std::cout << "X ticks ";
                // myXScale.text();
                // std::cout
                //     << " xutickinc " << xutickinc
                //     << "\n";

                // if possible, place tick marks at integer values of x index
                if (xutickinc > 1)
                    xutickinc = floor(xutickinc);

                for (int kxtick = 0; kxtick <= tickCount; kxtick++)
                {
                    float tickXU = myXScale.XUmin() + kxtick * xutickinc;
                    // float tick_label_value = myXScale.XI2XU(tickXU);
                    int xPixel = myXScale.XU2XP(tickXU);

                    // std::cout << "tick " << kxtick << " xu " << tickXU
                    //           << " " << xPixel << "\n";

                    S.text(
                        std::to_string(tickXU).substr(0, 4),
                        {xPixel, ypos + 1, 50, 15});

                    for (
                        int k = myYScale.YPmax();
                        k < myYScale.YPmin();
                        k = k + 25)
                    {
                        S.pixel(xPixel, k);
                        S.pixel(xPixel, k + 1);
                    }
                }
            }
            void drawTrace(trace *t, shapes &S)
            {
                S.penThick(t->thick());
                S.color(t->color());

                bool first = true;
                int xi = 0;
                double prevX, prev;

                switch (t->myType)
                {
                case trace::eType::plot:
                {
                    POINT p;
                    std::vector<POINT> vp;
                    for (auto y : t->getY())
                    {
                        // scale
                        p.x = myXScale.XI2XP(xi++);
                        p.y = myYScale.YV2YP(y);
                        vp.push_back(p);
                    }
                    S.polyLine(vp.data(), t->size());
                }
                break;

                case trace::eType::scatter:

                    for (auto y : t->getY())
                    {
                        S.rectangle(
                            {myXScale.XI2XP(xi++) - 5, myYScale.YV2YP(y) - 5,
                             5, 5});
                    }
                    break;

                case trace::eType::realtime:
                {

                    for (auto y : t->getY())
                    {

                        // scale data point to pixels
                        double x = myXScale.XI2XP(xi++);
                        double yp = myYScale.YV2YP(y);

                        if (first)
                        {
                            first = false;
                        }
                        else
                        {
                            // draw line from previous to this data point
                            S.line(
                                {(int)prevX, (int)prev, (int)x, (int)yp});
                        }

                        prevX = x;
                        prev = yp;
                    }
                }
                break;

                default:
                    throw std::runtime_error(
                        "Trace type NYI");
                }
            }
            void drawSelectedArea(PAINTSTRUCT &ps)
            {
                if (!isGoodDrag())
                    return;

                // display selected area by drawing a box around it
                wex::shapes S(ps);

                // contrast to background color
                S.color(0xFFFFFF ^ bgcolor());

                S.line({myStartDragX, myStartDragY, myStopDragX, myStartDragY});
                S.line({myStopDragX, myStartDragY, myStopDragX, myStopDragY});
                S.line({myStopDragX, myStopDragY, myStartDragX, myStopDragY});
                S.line({myStartDragX, myStopDragY, myStartDragX, myStartDragY});
            }
        };
    }
}
