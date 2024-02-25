#pragma once

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <cfloat>

// minimum data range that will produce sensible plots
#define minDataRange 0.000001

namespace wex
{
    namespace plot
    {
        class plot;
        /// @cond
        /// Plot dimensions
        class scale
        {
        public:
            static scale &get()
            {
                static scale theInstance;
                return theInstance;
            }

            void set(int xo, double xs, int yo, double ys)
            {
                myXOffset = xo;
                myXScale = xs;
                myYOffset = yo;
                myYScale = ys;
            }
            void bounds(double XMin, double XMax, double YMin, double YMax)
            {
                myXMin = XMin;
                myXIMax = XMax;
                myYMin = YMin;
                myYMax = YMax;
            }

            // xvalue conversion factors
            void xpstart_set(int start)
            {
                xpstart = start;
            }
            void xustart_set(double start)
            {
                xustart = start;
            }
            double XUStart(int xustart) const
            {
                return xustart;
            }
            void sxi2xp_set( double s )
            {
                xpstart = 50;
                sxi2xp = s;
            }
            void sxi2xu_set( double s)
            {
                sxi2xu = s;
            }

            // Convert between x values

            int XI2XP(int xi) const
            {
                return xi * sxi2xp + xpstart;
            }
            int XU2XP( double xu ) const
            {
                return XI2XP( XU2XI( xu ));
            }
            int XU2XI( double xu ) const
            {
                return (xu - xustart) / sxi2xu;
            }
            int XI2XU(int xi) const
            {
                return xi * sxi2xu + xustart;
            }
            double XP2XU( int pixel ) const
            {
                return XI2XU( XP2XI( pixel ));
            }
            double XI2XU( double xi ) const
            {
                return xi * sxi2xu + xustart;
            }
            double XP2XI( int pixel ) const
            {
                return (pixel - xpstart) / sxi2xp;
            }
            int XPstart() const
            {
                return xpstart;
            }
            int X2Pixel(double x) const
            {
                return (int)round(myXOffset + myXScale * x);
            }
            int Y2Pixel(double y) const
            {
                return myYOffset - myYScale * y;
            }
            /// @brief get X index value from x pixel
            /// @param x pixel
            /// @return 0 if there is a problem
            double Pixel2XI(int x) const
            {
                if (myXScale < 0.0001)
                {
                    /* Probably means there is no data in the plot
                    So simply return 0
                    */
                    return 0;
                }
                return ((double)(x - myXOffset)) / myXScale;
            }
            // double Pixel2XU(int xp) const
            // {
            //     return myXUStart + Pixel2XI(xp) * myScalexi2xu;
            // }
            // pixel from x user
            // int XI2Pixel(double xu) const
            // {
            //     return (XU2XI(xu));
            // }
            // // x index from x user
            // int XU2XI(int xu) const
            // {
            //     return (xu - myXUStart) / myScalexi2xu;
            // }
            double Pixel2Y(int y) const
            {
                return ((double)(myYOffset - y)) / myYScale;
            }
            int minX() const
            {
                return myXMin;
            }
            int maxXI() const
            {
                return myXIMax;
            }
            double minY() const
            {
                return myYMin;
            }
            double maxY() const
            {
                return myYMax;
            }

        private:
            double myXScale, myYScale;
            int myXOffset;
            int myYOffset;
            double myXMin, myXIMax, myYMin, myYMax;

            int xpstart;     // pixel location of xi = 0
            double xustart;  // x user value of xi = 0
            double sxi2xp;    // scale conversion x index to pixel
            double sxi2xu;    // scale conversion from x index to x user


        };
        /// @endcond

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
            /// set trace thickness in pixels
            void thick(int t)
            {
                myThick = t;
            }
            /// get number of points
            int size()
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

        private:
            friend plot;

            plot *myPlot;               // plot where this trace is displayed
            std::vector<double> myX;    // X value of each data point
            std::vector<double> myY;    // Y value of each data point
            cCircularBuffer myCircular; // maintain indices of circular buffer used by real time trace
            int myColor;                // trace color
            int myThick;                // trace thickness
            enum class eType
            {
                plot,
                realtime,
                scatter
            } myType; // trace type

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
                double &txmin, double &txmax,
                double &tymin, double &tymax)
            {
                if (myY.size())
                {

                    // find the x limits

                    if (myType == eType::realtime || myX.size() == 0)
                    {
                        txmin = 0;
                        txmax = myY.size();
                    }
                    else
                    {
                        auto result = std::minmax_element(
                            myX.begin(),
                            myX.end());
                        txmin = *result.first;
                        txmax = *result.second;
                    }

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

            /// draw
            void update(PAINTSTRUCT &ps)
            {

                shapes S(ps);
                S.penThick(myThick);
                S.color(myColor);

                bool first = true;
                int xi = 0;
                //        float xinc = myPlot->xinc();
                double prevX, prev;

                switch (myType)
                {
                case eType::plot:
                {
                    POINT p;
                    std::vector<POINT> vp;
                    for (auto y : myY)
                    {
                        // scale
                        p.x = scale::get().X2Pixel(xi);
                        ;
                        p.y = scale::get().Y2Pixel(y);

                        vp.push_back(p);
                        xi++;
                    }

                    S.polyLine(vp.data(), myY.size());
                }

                break;

                case eType::scatter:

                    for (int k = 0; k < (int)myX.size(); k++)
                    {
                        S.rectangle(
                            {scale::get().X2Pixel(myX[k]) - 5, scale::get().Y2Pixel(myY[k]) - 5,
                             5, 5});
                    }
                    break;

                case eType::realtime:
                {
                    // loop over data in circular buffer
                    int xidx = 0;
                    for (int yidx = myCircular.first();
                         yidx >= 0;
                         yidx = myCircular.next())
                    {

                        // scale data point to pixels
                        double x = scale::get().X2Pixel(xidx);
                        double y = scale::get().Y2Pixel(myY[yidx]);

                        if (first)
                        {
                            first = false;
                        }
                        else
                        {
                            // draw line from previous to this data point
                            S.line(
                                {(int)prevX, (int)prev, (int)x, (int)y});
                        }

                        prevX = x;
                        prev = y;

                        xidx++;
                    }
                }
                break;
                }
            }
        };
        /// @cond
        /** \brief Draw decorated axis line

            This class is internal and none of its methods should be
            called by the application code
        */
        class axis
        {
        public:
            /** CTOR
                @param[in] p pointer to plot panel
                @param[in} xaxis true for x ( horixonatl ) axis, defaults to false ( vertical )
            */
            axis(gui &p, bool xaxis = false)
                : myfGrid(false), myfX(xaxis), myParent(p), myfXset(false),
                  myXStartValue(FLT_MAX)
            {
            }

            /// draw
            void update(PAINTSTRUCT &ps)
            {

                shapes S(ps);
                S.color(0xFFFFFF - myParent.bgcolor());
                S.textHeight(15);
                if (!myfX)
                {
                    // Y-axis

                    double mn = scale::get().minY();
                    double mx = scale::get().maxY();
                    S.line({50, scale::get().Y2Pixel(mn),
                            50, scale::get().Y2Pixel(mx)});
                    for (double y : tickValues(mn, mx))
                    {
                        int yp = scale::get().Y2Pixel(y);
                        S.text(numberformat(y),
                               {0, yp - 8, 50, 15});
                        S.line({50, yp,
                                60, yp});
                        if (myfGrid)
                            for (int kp = 65;
                                 kp < scale::get().X2Pixel(scale::get().maxXI());
                                 kp += 25)
                            {
                                S.pixel(kp, yp);
                                S.pixel(kp + 1, yp);
                            }
                    }
                    int yp = scale::get().Y2Pixel(mx);
                    S.text(myMaxYLabel,
                           {0, yp + 10, 50, 15});
                }
                else
                {
                    // x-axis
                    int ypos = ps.rcPaint.bottom - 20;

                    double mn = scale::get().minX();
                    double mx = scale::get().maxXI();

                    int xmn_px = scale::get().XPstart();
                    int xmx_px = scale::get().XI2XP(mx);

                    S.line({xmn_px, ypos,
                            xmx_px, ypos});

                    if (myfGrid)
                    {
                        int tickCount = 8;
                        int xtickinc = (xmx_px - xmn_px) / tickCount;
                        for (int kxtick = 0; kxtick <= tickCount; kxtick++)
                        {
                            int xPixel = xmn_px + xtickinc * kxtick;
                            float tick_label_value = scale::get().XP2XU( xPixel );


                            // std::cout << "tick " << kxtick << " " << tick_label_value << " " << myXStartValue << " " << myXScaleValue
                            //           << " " << xPixel << " pixel2X " << scale::get().Pixel2X(xPixel) << "\n";

                            S.text(
                                std::to_string(tick_label_value).substr(0, 4),
                                {xPixel, ypos + 1, 50, 15});

                            for (
                                int k = scale::get().Y2Pixel(scale::get().maxY());
                                k < scale::get().Y2Pixel(scale::get().minY());
                                k = k + 25)
                            {
                                S.pixel(xPixel, k);
                                S.pixel(xPixel, k + 1);
                            }
                        }
                    }
                    else
                    {
                        // there is no grid
                        // so just label the minimum, maximum x points
                        float xmin_label_value = 0;
                        float xmax_label_value = 100;
                        if (myfXset)
                        {
                            xmin_label_value = myXStartValue;
                            xmax_label_value = myXStartValue + myXScaleValue * (mx - mn);
                        }
                        S.text(std::to_string((int)xmin_label_value), {xmn_px, ypos + 3, 50, 15});
                        S.text(std::to_string((int)xmax_label_value), {xmx_px - 25, ypos + 3, 50, 15});
                        S.text(myMaxXLabel,
                               {xmx_px - 50, ypos + 3,
                                50, 15});
                    }
                }
            }

            void grid(bool f)
            {
                myfGrid = f;
            }

            void XLabels(
                const std::string min,
                const std::string max)
            {
                myMinXLabel = min;
                myMaxXLabel = max;
            }
            void YLabels(
                const std::string min,
                const std::string max)
            {
                myMinYLabel = min;
                myMaxYLabel = max;
            }

            /** Set conversion from y value index to x user units
                @param[in] start x user value of first y-value
                @param[in] scale to convert from index to user value

                Used to label the x-axis
            */
            void XValues(
                float start,
                float scale)
            {
                myXStartValue = start;
                myXScaleValue = scale;
                myfXset = true;
            }

            bool isStartValue() const
            {
                return (myXStartValue < FLT_MAX);
            }

            // get conversion from X index in trace to  X user units
            float xScaleValue() const
            {
                return myXScaleValue;
            }
            float xStartValue() const
            {
                return myXStartValue;
            }

        private:
            bool myfGrid;
            bool myfX; // true for x-axis
            gui &myParent;
            std::string myMinXLabel;
            std::string myMaxXLabel;
            std::string myMinYLabel;
            std::string myMaxYLabel;
            bool myfXset;
            int myMinXValue;
            int myMaxXValue;
            float myXStartValue;
            float myXScaleValue;

            std::vector<double> tickValues(
                double mn, double mx)
            {
                std::vector<double> vl;
                double range = mx - mn;
                if (range < minDataRange)
                {
                    // plot is single valued
                    // display just one tick
                    vl.push_back(mn);
                    return vl;
                }
                double inc = range / 4;
                double tick;
                if (inc > 1)
                {
                    inc = (int)inc;
                    tick = (int)mn;
                }
                else
                {
                    tick = mn;
                }
                // if (tick < 0)
                //     return vl;

                while (true)
                {
                    double v = tick;
                    if (v > 100)
                        v = ((int)v / 100) * 100;
                    else if (v > 10)
                        v = ((int)v / 10) * 10;
                    vl.push_back(v);
                    tick += inc;
                    if (tick >= mx)
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
        };
        /// @endcond

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
                : gui(parent), myfFit(true), myfDrag(false), myfZoom(false)
            {
                text("Plot");

                myAxis = new axis(*this);
                myAxisX = new axis(*this, true);

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

                        // draw axis
                        myAxis->update(ps);
                        myAxisX->update(ps);

                        // loop over traces
                        for (auto t : myTrace)
                        {
                            // draw a trace
                            t->update(ps);
                        }

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
                            myZoomXMin = scale::get()
                                             .XP2XU(myStartDragX);
                            myZoomXMax = scale::get().XP2XU(myStopDragX);
                            myZoomYMax = scale::get().Pixel2Y(myStartDragY);
                            myZoomYMin = scale::get().Pixel2Y(myStopDragY);
                            myfZoom = true;
                            // std::cout << myStartDragX <<" "<< myStopDragX <<" "<< myStartDragY <<" "<< myStopDragY << "\n";
                            // std::cout << myZoomXMin <<" "<< myZoomXMax <<" "<< myZoomYMin <<" "<< myZoomYMax << "\n";
                        }
                        myfDrag = false;
                        update();
                    });
                events().clickRight(
                    [&]
                    {
                        // restore autofit
                        autoFit();
                    });
            }

            ~plot()
            {
                delete myAxis;
                delete myAxisX;
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
                myAxis->grid(enable);
                myAxisX->grid(enable);
            }

            void setFixedScale(
                double minX, double minY, double maxX, double maxY)
            {
                myZoomXMin = minX;
                myZoomYMin = minY;
                myZoomXMax = maxX;
                myZoomYMax = maxY;
                myfFit = false;
            }

            int traceCount() const
            {
                return (int)myTrace.size();
            }

            /** get step size along x-axis */
            float xinc() const
            {
                return myXinc;
            }

            /* get data bounds
                @return vector of doubles { minX, minY, maxX, maxY
            */
            std::vector<double> bounds() const
            {
                std::vector<double> ret;
                ret.push_back(myMinXU);
                ret.push_back(myMinY);
                ret.push_back(myMaxX);
                ret.push_back(myMaxY);
                return ret;
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
            void axisYminmax(double min, double max)
            {
                myfFit = false;
                myMinY = min;
                myMaxY = max;
            }

            /// Enable auto-fit scaling and remove any zoom setting
            void autoFit()
            {
                myfFit = true;
                myfDrag = false;
                myfZoom = false;
                update();
            }

            void XLabels(
                const std::string min,
                const std::string max)
            {
                myAxisX->XLabels(min, max);
            }
            void YLabels(
                const std::string min,
                const std::string max)
            {
                myAxis->YLabels(min, max);
            }
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
                scale::get().xustart_set( start_xu );
                scale::get().sxi2xu_set( scale_xi2xu );
                myAxisX->XValues(start_xu, scale_xi2xu);
            }

            /// @brief calculate scaling factors so plot will fit in window client area
            /// @return true if succesful
            bool CalcScale(int w, int h)
            {
                // std::cout << "Plot::CalcScale " << w << " " << h << "\n";

                // check there are traces that need to be drawn
                if (!myTrace.size())
                    return false;

                // check traces contain data
                bool OK = false;
                for (auto t : myTrace)
                    if (t->size())
                    {
                        OK = true;
                        break;
                    }
                if (!OK)
                    return false;

                if (myfFit)
                {
                    CalulateDataBounds();
                }

                if (fabs(myMaxX - myMinXU) < 0.0001)
                    myXScale = 1;
                else
                    myXScale = (w - 70) / (myMaxX - myMinXU);
                if (fabs(myMaxY - myMinY) < minDataRange)
                    myYScale = 1;
                else
                    myYScale = (h - 70) / (myMaxY - myMinY);

                myXOffset = 50 - myXScale * myMinXU;
                myYOffset = h - 20 + myYScale * myMinY;

                scale::get().sxi2xp_set( myXScale );
                scale::get().set(myXOffset, myXScale, myYOffset, myYScale);
                scale::get().bounds(myMinXU, myMaxX, myMinY, myMaxY);

                // std::cout << "X " << myMinX <<" "<< myMaxX <<" "<< myXScale << "\n";
                // std::cout << "Y " << myMinY <<" "<< myMaxY <<" "<< myYScale << "\n";

                // If user has not called XValues(), set X-axis scale to 1
                if (!myAxisX->isStartValue())
                    XUValues(0, 1);

                return true;
            }

            std::vector<trace *> &traces()
            {
                return myTrace;
            }

            bool isZoomed() const
            {
                return myfZoom;
            }

            /// get X user value from x pixel
            double pixel2Xuser(int xpixel) const
            {
                return scale::get().XP2XU(xpixel);
            }
            int xuser2pixel(double xu) const
            {
                return scale::get().XU2XP(xu);
            }
            /// get Y user value from y pixel
            double pixel2Yuser(int ypixel) const
            {
                return scale::get().Pixel2Y(ypixel);
            }

        private:
            /// window where plot will be drawn
            //    window myParent;

            axis *myAxis;
            axis *myAxisX;

            /// plot traces
            std::vector<trace *> myTrace;

            float myXinc;
            double myMinXU; // minimum user X value
            double myMaxX;
            double myMinY, myMaxY;
            double myXScale, myYScale;
            int myXOffset;
            int myYOffset;

            bool myfFit; /// true if scale should fit plot to window

            bool myfDrag;
            bool myfZoom;
            int myStartDragX;
            int myStartDragY;
            int myStopDragX;
            int myStopDragY;
            double myZoomXMin;
            double myZoomXMax;
            double myZoomYMin;
            double myZoomYMax;
            // std::function<void(void)> myClickRightFunction;

            void CalulateDataBounds()
            {
                if (myfZoom)
                {
                    myMinXU = myZoomXMin;
                    myMaxX = myZoomXMax;
                    myMinY = myZoomYMin;
                    myMaxY = myZoomYMax;
                }
                else
                {
                    myTrace[0]->bounds(
                        myMinXU, myMaxX,
                        myMinY, myMaxY);
                    for (auto &t : myTrace)
                    {
                        double txmin, txmax, tymin, tymax;
                        txmin = txmax = tymax = 0;
                        tymin = std::numeric_limits<double>::max();
                        t->bounds(txmin, txmax, tymin, tymax);
                        if (txmin < myMinXU)
                            myMinXU = txmin;
                        if (txmax > myMaxX)
                            myMaxX = txmax;
                        if (tymin < myMinY)
                            myMinY = tymin;
                        if (tymax > myMaxY)
                            myMaxY = tymax;
                    }
                }
            }
            bool isGoodDrag()
            {
                return (myfDrag && myStopDragX > 0 && myStopDragX > myStartDragX && myStopDragY > myStartDragY);
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
