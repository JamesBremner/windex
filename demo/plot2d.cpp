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

