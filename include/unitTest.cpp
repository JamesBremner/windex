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

    thePlot.CalcScale(500, 200);

    CHECK_CLOSE(100, thePlot.pixel2Xuser(50), 0.5);
    CHECK_CLOSE(140, thePlot.pixel2Xuser(386), 0.5);
}

TEST(plot_Yscale)
{
    wex::plot::YScale Y;
    Y.YVrange(-50, 50);
    Y.YPrange(190, 20);

    // pixels run from 0 at top of window
    CHECK_EQUAL(190, Y.YV2YP(-50));
    CHECK_EQUAL(105, Y.YV2YP(0));
    CHECK_EQUAL(20, Y.YV2YP(50));

    CHECK_EQUAL(-50, Y.YP2YV(190));
    CHECK_EQUAL(0, Y.YP2YV(105));
    CHECK_EQUAL(50, Y.YP2YV(20));
}
TEST(zoom)
{
    wex::plot::XScale X;
    X.ximax_set(8);
    X.XPrange(0, 400);
    X.XUValues(100,5);

    CHECK_CLOSE(140,X.XUmax(),0.5);
    CHECK_CLOSE(110, X.XP2XU(100),0.5);
    X.zoom(110,120);
    CHECK_CLOSE(112.5, X.XP2XU(100),0.5);
    CHECK_CLOSE(115, X.XP2XU(200),0.5);
    CHECK_CLOSE(120, X.XP2XU(400),0.5);
}
main()
{
    return raven::set::UnitTest::RunAllTests();
}