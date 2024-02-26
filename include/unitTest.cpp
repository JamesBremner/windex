#include <string>
#include <iostream>
#include "cutest.h"
#include "wex.h"
#include "plot2d.h"

TEST(plot_Xscale)
{
    wex::gui &fm = wex::maker::make();
    fm.move(50, 50, 1200, 600);
    wex::plot::plot &thePlot = wex::maker::make<wex::plot::plot>(fm);
    thePlot.bgcolor(0);
    thePlot.XUValues(100, 5);
    thePlot.grid(true);
    thePlot.size(500, 200);
    thePlot.move(30, 100);

    wex::plot::trace &t1 = thePlot.AddStaticTrace();
    std::vector<double> d1{10, 15, 20, 25, 30, 25, 20, 15, 10};
    t1.set(d1);

    thePlot.CalcScale(500,200);

    auto& scale = thePlot.XScale_get();
    //scale.text();

    CHECK_EQUAL(50,scale.XI2XP( 0 ));

    CHECK_EQUAL(432,scale.XI2XP( 8 ));
    CHECK_CLOSE(100,scale.XP2XU(50),0.5);
    CHECK_CLOSE(140,scale.XP2XU(432),0.5);

    CHECK_CLOSE(100,thePlot.pixel2Xuser(50),0.5);
    CHECK_CLOSE(140,thePlot.pixel2Xuser(432),0.5);

    }

TEST( plot_Yscale)
{
    wex::plot::YScale Y;
    Y.YVrange( -50,50);
    Y.YPrange( 190, 20 );
    Y.text();

    // pixels run from 0 at top of window
    CHECK_EQUAL( 190, Y.YV2YP(-50));
    CHECK_EQUAL( 105, Y.YV2YP(0));
    CHECK_EQUAL( 20, Y.YV2YP(50));

    CHECK_EQUAL( -50, Y.YP2YV( 190));
    CHECK_EQUAL( 0, Y.YP2YV( 105));
    CHECK_EQUAL( 50, Y.YP2YV( 20));
}
main()
{
    return raven::set::UnitTest::RunAllTests();
}