#include <algorithm>

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

    static scale& get()
    {
        static scale theInstance;
        return theInstance;
    }
    void set( int xo, double xs, int yo, double ys )
    {
        myXOffset = xo;
        myXScale  = xs;
        myYOffset = yo;
        myYScale  = ys;
    }
    void bounds( int XMin, int XMax, int YMin, int YMax )
    {
        myXMin = XMin;
        myXMax = XMax;
        myYMin = YMin;
        myYMax = YMax;
    }
    int X2Pixel( double x ) const
    {
        return myXOffset + myXScale * x;
    }
    int Y2Pixel( double y ) const
    {
        return myYOffset - myYScale * y;
    }
    int minX() const
    {
        return myXMin;
    }
    int maxX() const
    {
        return myXMax;
    }
    int minY() const
    {
        return myYMin;
    }
    int maxY() const
    {
        return myYMax;
    }
private:
    double myXScale, myYScale;
    int myXOffset;
    int myYOffset;
    int myXMin, myXMax, myYMin, myYMax;
};
/// @endcond
/** \brief Single trace to be plotted

    Application code shouild not attempt to construct a trace
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
        for a trace that is not plot type
    */
    void set( const std::vector< double >& y )
    {
        if( myType != eType::plot )
            throw std::runtime_error("nanaplot error: plot data added to non plot trace");

        myY = y;
    }

    /** \brief add new value to real time data
        @param[in] y the new data point

        An exception is thrown when this is called
        for a trace that is not real time type.
    */
    void add( double y )
    {
        if( myType != eType::realtime )
            throw std::runtime_error("nanaplot error: realtime data added to non realtime trace");
        myY[ myRealTimeNext++ ] = y;
        if( myRealTimeNext >= (int)myY.size() )
            myRealTimeNext = 0;
    }

    /** \brief add point to scatter trace
        @param[in] x location
        @param[in] y location

        An exception is thrown when this is called
        for a trace that is not scatter type
    */

    void add( double x, double y )
    {
        if( myType != eType::scatter )
            throw std::runtime_error("nanaplot error: point data added to non scatter type trace");
        myX.push_back( x );
        myY.push_back( y );
    }
    /// set color
    void color( int clr )
    {
        myColor = clr;
    }

    int size()
    {
        return (int) myY.size();
    }

private:

    friend plot;

    plot * myPlot;
    std::vector< double > myX;
    std::vector< double > myY;
    int myColor;
    int myRealTimeNext;
    enum class eType
    {
        plot,
        realtime,
        scatter
    } myType;

    /** CTOR
    Application code should not call this constructor
    Rather call one of plot::AddPointTrace, plot::AddRealTimeTrace or plot::AddStaticTrace

    */
    trace()
        : myType( eType::plot )
    {

    }

    /// set plot where this trace will appear
    void Plot( plot * p )
    {
        myPlot = p;
    }

    /** \brief Convert trace to real time operation
    @param[in] w number of data points to display

    Data points older than w scroll off the left edge of the plot and are lost
    */
    void realTime( int w )
    {
        myType = eType::realtime;
        myRealTimeNext = 0;
        myY.clear();
        myY.resize( w );
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
        double& txmin, double& txmax,
        double& tymin, double& tymax )
    {
        if( myY.size() )
        {
            if( myType == eType::realtime || myX.size() == 0 )
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
            auto result = std::minmax_element(
                              myY.begin(),
                              myY.end());
            tymin = *result.first;
            tymax = *result.second;
        }
    }

    /// draw
    void update( PAINTSTRUCT& ps )
    {
        shapes S( ps );
        S.color( myColor );

        bool first = true;
        int xi    = 0;
//        float xinc = myPlot->xinc();
        double prevX, prev;


        switch( myType )
        {
        case eType::plot:

            // loop over data points
            for( auto y : myY )
            {
                // scale
                double x  = scale::get().X2Pixel( xi );
                double ys = scale::get().Y2Pixel( y );

                if( first )
                {
                    first = false;
                    prevX = x;
                    prev = ys;
                    xi++;
                    continue;
                }

                // draw line from previous to this data point
                S.line(
                {
                    (int)prevX, (int)prev,
                    (int)x, (int)ys
                });
//                   myColor);

                prevX = x;
                prev = ys;
                xi++;
            }
            break;

        case eType::scatter:

            for( int k = 0; k < (int)myX.size(); k++ )
            {
                S.rectangle(
                {
                    scale::get().X2Pixel( myX[ k ] )-5,  scale::get().Y2Pixel( myY[ k ] )-5,
                    10, 10
                } );
            }
            break;

        case eType::realtime:

        {
            // loop over data points

            // they are stored in a circular buffer
            // so we have to start with the oldest data point
            int yidx = myRealTimeNext;
            do
            {
                double x = scale::get().X2Pixel( yidx );
                double y = scale::get().Y2Pixel( myY[ yidx ] );

                // the next data point
                // with wrap-around if the end of the vector is reached
                yidx++;
                if( yidx >= (int)myY.size() )
                    yidx = 0;

                if( first )
                {
                    first = false;
                    prevX = x;
                    prev = y;
                    continue;
                }
                // draw line from previous to this data point
                S.line(
                { (int)prevX, (int)prev, (int)x, (int)y } );

                prevX = x;;
                prev = y;

            }

            // check for end of circular buffer
            // ( most recent point )
            while( yidx != myRealTimeNext );

        }
        break;
        }
    }


};
/// @cond
/** \brief Draw decorated vertical line on LHS of plot for Y-axis

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
    axis( gui& p, bool xaxis = false )
        : myfGrid( false )
        , myfX( xaxis )
        , myParent( p )
    {
    }

    /// draw
    void update( PAINTSTRUCT& ps )
    {
        const int tick_length = 10;

        shapes S( ps );
        S.color( 0xFFFFFF - myParent.bgcolor() );
        if( ! myfX )
        {
            double mn = 10 * ( scale::get().minY() / 10 );
            double mx = 10 * ( scale::get().maxY() / 10 );
            if( mx-mn < 2 )
            {
                mn = scale::get().minY();
                mx = scale::get().maxY();
            }
            int ymn_px = scale::get().Y2Pixel( mn );
            S.text( std::to_string((int)mn), { 5,ymn_px,50,15});

            int ymx_px = scale::get().Y2Pixel( mx );
            S.text( std::to_string((int)mx), { 5,ymx_px,50,15});

            S.line( { 2, ymn_px,
                      2, ymx_px
                    });

            if( mn * mx < 0 )
            {
                S.line( {2, ymx_px,
                         5, ymx_px
                        });
                int y0_px = scale::get().Y2Pixel( 0 );
                S.text( "0", { 5, y0_px - 15, 50,15 } );
                S.line( {2, y0_px,
                         tick_length, y0_px
                        });
                S.line( {2, ymn_px,
                         tick_length, ymn_px
                        });

//            if( myfGrid )
//                for( int k=5; k<graph.width(); k=k+10 )
//                {
//                    graph.set_pixel(k, y0_px, colors::blue );
//                    graph.set_pixel(k+1, y0_px, colors::blue );
//                }
            }
            else
            {
                int yinc = ( ymn_px - ymx_px ) / 4;
                for( int ky = 0; ky < 4; ky++ )
                {
                    int y = ymx_px + ky * yinc;
                    S.line( {2, y,
                             tick_length, y
                            });
//                if( myfGrid )
//                    for( int k=5; k<graph.width(); k=k+10 )
//                    {
//                        graph.set_pixel(k, y, colors::blue );
//                        graph.set_pixel(k+1, y, colors::blue );
//                    }
                }
            }
        }
        else
        {
            // x-axis
            int ypos = ps.rcPaint.bottom - 20;

            double mn = 10 * (scale::get().minX() / 10 );
            double mx = 10 * ( scale::get().maxX() / 10 );
            if( mx-mn < 2 )
            {
                mn = scale::get().minX();
                mx = scale::get().maxX();
            }
            int xmn_px = scale::get().X2Pixel( mn );
            S.text( std::to_string((int)mn), {xmn_px, ypos+3, 50,15});

            int xmx_px = scale::get().X2Pixel( mx );
            S.text( std::to_string((int)mx), {xmx_px, ypos+3, 50,15});

            S.line( { xmn_px, ypos,
                      xmx_px, ypos
                    });
        }

    }



    void Grid( bool f )
    {
        myfGrid = f;
    }

private:
    bool myfGrid;
    bool myfX;              // true for x-axis
    gui& myParent;
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

<pre>
#include "wex.h"
#include "plot2d.h"

int main()
{
        wex::gui& fm = wex::windex::topWindow();
        fm.move( {50,50,1200,600} );

        // construct plot to be drawn on form
        wex::plot::plot& thePlot = wex::make<wex::plot::plot>( fm );
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
    plot( gui* parent )
        : gui( parent )
    {
        text("Plot");

        events().draw([this](PAINTSTRUCT& ps)
        {
            // check there are traces that need to be drawn
            if( ! myTrace.size() )
                return;

            // calculate scaling factors
            // so plot will fit
            CalcScale(
                ps.rcPaint.right,
                ps.rcPaint.bottom );

            // draw axis
            myAxis->update( ps );
            myAxisX->update( ps );

            // loop over traces
            for( auto t : myTrace )
            {
                // draw a trace
                t->update( ps );
            }
        });

        myAxis = new axis( *this );
        myAxisX = new axis( *this, true );
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
    trace& AddStaticTrace()
    {
        trace * t = new trace();
        t->Plot( this );
        myTrace.push_back( t );
        return *t;
    }

    /** \brief Add real time trace
        @param[in] w number of recent data points to display
        @return reference to new trace

        The data in a real time trace receives new values from time to time
        The display shows w recent values.  Older values scroll off the
        left hand side of the plot and disappear.
    */
    trace& AddRealTimeTrace( int w )
    {
        trace * t = new trace();
        t->Plot( this );
        t->realTime( w );
        myTrace.push_back( t );
        return *t;
    }

    /** \brief Add scatter trace
        @return reference to new trace

        A static trace for scatter plots
        No line between points,
          box around each point.
        Specify x AND y locations for each point.
    */
    trace& AddScatterTrace()
    {
        trace * t = new trace();
        t->Plot( this );
        t->scatter();
        myTrace.push_back( t );
        return *t;
    }


    /** \brief Enable display of grid markings */
    void Grid( bool enable )
    {
        myAxis->Grid( enable );
    }

    float xinc()
    {
        return myXinc;
    }

    void debug()
    {
        for( auto t : myTrace )
        {
            std::cout << "debugtsize " << t->size() << "\n";
        }
    }

private:

    ///window where plot will be drawn
//    window myParent;

    axis * myAxis;
    axis * myAxisX;

    /// plot traces
    std::vector< trace* > myTrace;

    float myXinc;
    double myMinX, myMaxX;
    double myMinY, myMaxY;
    double myXScale, myYScale;
    int myXOffset;
    int myYOffset;

    /** calculate scaling factors so plot will fit in window client area
        @param[in] w width
        @param[in] h height
    */
    void CalcScale( int w, int h )
    {
        //std::cout << "Plot::CalcScale " << w << " " << h << "\n";
        w *= 0.9;
        h *= 0.95;

        int maxCount = 0;
        myTrace[0]->bounds(
            myMinX, myMaxX,
            myMinY, myMaxY );
        for( auto& t : myTrace )
        {
            if( t->size() > maxCount )
                maxCount = t->size();
            double txmin, txmax, tymin, tymax;
            t->bounds( txmin, txmax, tymin, tymax );
            if( txmin < myMinX )
                myMinX = txmin;
            if( txmax > myMaxX )
                myMaxX = txmax;
            if( tymin < myMinY )
                myMinY = tymin;
            if( tymax > myMaxY )
                myMaxY = tymax;
        }
        if( ! maxCount )
            return;
        if( fabs( myMaxX - myMinX) < 0.0001 )
            myXScale = 1;
        else
            myXScale = 0.9 * w / ( myMaxX - myMinX );
        if( fabs( myMaxY - myMinY ) < 0.0001 )
            myYScale = 1;
        else
            myYScale = 0.9 * h / ( myMaxY - myMinY );

        myXOffset = 0.05 * w;
        myYOffset = h - 10 + myYScale * myMinY;

        scale::get().set( myXOffset, myXScale, myYOffset, myYScale );
        scale::get().bounds( myMinX, myMaxX, myMinY, myMaxY );

        //std::cout << myMinY <<" "<< myMaxY <<" "<< myScale;
    }
};

}
}
