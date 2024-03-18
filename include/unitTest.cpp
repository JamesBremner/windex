#include <string>
#include <iostream>
#include "cutest.h"
#include "wex.h"
#include "plot2d.h"

TEST( ytickValues )
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

    auto vt = thePlot.yscale().tickValues();

    CHECK_EQUAL(4,vt.size());
    CHECK_EQUAL(10,vt[1]);
}
TEST( ytickValues2 )
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
    std::vector<double> d1{.10, .15, .20, .25, .30, .25, .20, .15, .10};
    t1.set(d1);

    thePlot.CalcScale(500, 200);

    auto vt = thePlot.yscale().tickValues();

    CHECK_EQUAL(4,vt.size());
    CHECK_EQUAL(0.15,vt[1]);
}

TEST(setFitScale)
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

    // fit -> fit
    thePlot.setFitScale();
    thePlot.CalcScale(500, 200);
    CHECK_CLOSE(105, thePlot.pixel2Xuser(100),2);
    CHECK_CLOSE(18, thePlot.pixel2Yuser(100),2);

    // fit -> fix
    thePlot.setFixedScale(0, 200, 0, 200);
    thePlot.CalcScale(500, 200);
    CHECK_CLOSE(26, thePlot.pixel2Xuser(100),2);
    CHECK_CLOSE(80, thePlot.pixel2Yuser(100),2);

    // fix -> fit
    thePlot.setFitScale();
    thePlot.CalcScale(500, 200);
    CHECK_CLOSE(105, thePlot.pixel2Xuser(100),2);
    CHECK_CLOSE(18, thePlot.pixel2Yuser(100),2);
}

TEST(zoom)
{
    wex::plot::scaleStateMachine M;
    wex::plot::XScale X(M);
    X.xiSet(0, 8);
    X.xpSet(0, 400);
    X.xi2xuSet(100, 5);
    X.calculate();

    CHECK_CLOSE(140, X.XUmax(), 0.5);
    CHECK_CLOSE(110, X.XP2XU(100), 0.5);

    M.event(wex::plot::scaleStateMachine::eEvent::zoom);
    X.zoom(110, 120);
    X.calculate();
    CHECK_CLOSE(110, X.XP2XU(0), 0.5);
    CHECK_CLOSE(112.5, X.XP2XU(100), 0.5);
    CHECK_CLOSE(115, X.XP2XU(200), 0.5);
    CHECK_CLOSE(120, X.XP2XU(400), 0.5);
}

TEST(plot_XScale2fit)
{
    wex::plot::scaleStateMachine M;
    wex::plot::XScale X(M);

    X.xpSet(50, 1950);
    X.xiSet(0, 8);
    X.xi2xuSet(100, 5);

    X.calculate();

    CHECK_EQUAL(50, X.XI2XP(0));
    CHECK_EQUAL(1000, X.XI2XP(4));
    CHECK_EQUAL(1950, X.XI2XP(8));

    CHECK_EQUAL(100, X.XP2XU(50));
    CHECK_EQUAL(120, X.XP2XU(1000));
    CHECK_EQUAL(140, X.XP2XU(1950));
}
TEST(plot_XScale2fix)
{
    wex::plot::scaleStateMachine M;
    M.event(wex::plot::scaleStateMachine::eEvent::fix);
    wex::plot::XScale X(M);

    X.xpSet(50, 1950);
    X.xiSet(0, 8);
    X.xi2xuSet(100, 5);
    X.fixSet(0, 200);

    X.calculate();

    CHECK_EQUAL(1000, X.XI2XP(0));
    CHECK_EQUAL(1190, X.XI2XP(4));
    CHECK_EQUAL(1380, X.XI2XP(8));

    CHECK_EQUAL(100, X.XP2XU(1000));
    CHECK_EQUAL(140, X.XP2XU(1380));
}

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

    CHECK_CLOSE(100, thePlot.pixel2Xuser(50), 1);
    CHECK_CLOSE(140, thePlot.pixel2Xuser(430), 1);
}

TEST(plot_Yscale)
{
    wex::plot::scaleStateMachine M;
    wex::plot::YScale Y(M);
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

TEST(plot_setFixedScale)
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

    CHECK_CLOSE(24.7, thePlot.pixel2Yuser(50), 0.5);
    CHECK_CLOSE(10, thePlot.pixel2Yuser(150), 0.5);

    thePlot.setFixedScale(0, 200, -100, 100);
    thePlot.CalcScale(500, 200);

    CHECK_CLOSE(100, thePlot.pixel2Yuser(50), 0.5);
    CHECK_CLOSE(140, thePlot.pixel2Yuser(386), 0.5);
}
main()
{
    return raven::set::UnitTest::RunAllTests();
}