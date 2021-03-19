
// include the header only windex gui framework
#include "wex.h"
#include "propertygrid.h"
#include "inputbox.h"
#include "plot2d.h"
#include "window2file.h"



using namespace wex;

void helloworld()
{
    // construct top level window
    gui& form = maker::make();
    form.move({ 50,50,400,400});
    form.fontHeight( 25 );
    form.text("Label and Editbox demo");

    // display labels
    label& lbA = maker::make<label>( form );
    lbA.move( {20, 20, 100, 30 } );
    lbA.text("A:");
    label& lbB = maker::make<label>( form );
    lbB.move( {20, 60, 100, 30 } );
    lbB.text("B:");

    // display textboxes
    editbox& edit1 = maker::make<editbox>( form );
    edit1.move( {80, 20, 100, 50 } );
    edit1.text( "type value");
    edit1.events().change( edit1.id(),[]
    {
        std::cout << "edit A changed\n" ;
    });
    editbox& edit2 = maker::make<editbox>( form );
    edit2.move( {80, 60, 100, 50 } );
    edit2.text( "type value");

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 100, 200, 30 } );
    btn.text( "Show values entered" );
    btn.tooltip("tooltip explaining button function");


    // popup a message box when button is clicked
    // showing the value entered in textbox
    btn.events().click([&]
    {
        std::string m =
        "A is " + edit1.text() +
        ", B is " + edit2.text();
        msgbox mb(
            m );
    });

    form.show();
}

void choiceDemo()
{
    // construct top level  window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("A windex combobox");

    // display combobox
    choice& cb = wex::maker::make<choice>( form );
    cb.move(20, 50, 150, 130 );
    cb.add("Alpha");
    cb.add("Beta");
    cb.select(-1);
    cb.events().select( cb.id(), [&]
    {
        msgbox mb(
            cb.selectedText() );
    });

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Show values selected" );
    btn.events().click([&]
    {
        msgbox mb(
            cb.selectedText() );
    });

    list& listbox = wex::maker::make<list>( form );
    listbox.move( 20,200,150,150 );
    listbox.add("Alpha");
    listbox.add("Beta");

    form.show();
}

void drawDemo()
{
    // construct top level  window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("A windex draw demo");

    form.events().draw([]( PAINTSTRUCT& ps )
    {
        shapes S( ps );
        S.color( 255, 0, 0 );
        S.line( { 10,10, 50,50 } );
        S.color( 255,255,255 );
        S.rectangle( { 20,20,20,20});
        S.color( 255,255,0 );
        S.text( "test", {50,50,50,25} );
        S.textVertical();
        S.text( "vertical", {200,200,50,100});
        S.textVertical( false );
        S.text( "horizontal", {225, 225,100,50});
        S.color(0,0,255);
        S.circle( 100,100,40);
        S.arc( 100,100,30, 0, 90 );

        S.fill();
        S.rectangle(  { 200,20,20,20});
        S.polygon( { 200,60, 220,60, 210,80 });


        S.textCenterHz("this is", {20,250,150,30});
        S.textCenterHz("some centered", {20,280,150,30});
        S.textCenterHz("text", {20,310,150,30});
    });

    form.show();
}

void PGDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("A windex property grid");

    // construct propertygrid
    propertyGrid& pg = wex::maker::make<propertyGrid>( form );
    pg.move( { 10,10, 200, 200});
    pg.labelWidth( 50 );
    pg.bgcolor( 0xFFA0A0 );

    // add properties
    //pg.category("Strings");
    pg.string( "A", "72" );
    pg.string( "B", "4600" );
    pg.string( "C", "72" );
//    pg.string( "D", "4600" );
//    pg.string( "E", "4600" );
//    pg.string( "F", "4600" );
//    pg.string( "G", "4600" );
//    pg.string( "H", "4600" );
//    pg.string( "I", "4600" );
//    pg.string( "J", "4600" );
//    pg.string( "K", "4600" );
//    pg.string( "L", "4600" );
//    pg.string( "M", "4600" );
//    pg.string( "N", "4600" );
//    pg.string( "O", "4600" );
    pg.expand("Strings",false);
    pg.category("Others");
    pg.choice( "Choose", { "X", "Y", "Z"} );
    pg.check( "Enable", false );
    pg.tabList();


    propertyGrid& pg2 = wex::maker::make<propertyGrid>( form );
    pg2.move( { 10,300, 200, 200});
    pg2.labelWidth( 50 );
    pg2.bgcolor( 0xFFA0A0 );

//    // add properties
//    //pg.category("Strings");
//    pg2.string( "x", "72" );
//    pg2.string( "y", "4600" );

    std::string json = "{\"strings\": { \"x\": 72, \"y\": 4600 }}";
    pg2.addjson( json );

    form.events().resize([&](int w, int h)
    {
        // window containing propertyGrid has been resized
        // ensure that every visible label is completely redrawn
        pg.update();
        form.update();
    });

    pg.change( []
    {
        std::cout << "property value changed\n";
    });

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 250, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the values entered
    btn.events().click([&]
    {
        std::string msg =
        "A is " + pg.value("A") +
        ", B is " + pg.value("B") +
        ", choice is " + pg.value("Choose") +
        ", enabled is ";
        if( pg.value("Enable") == "1" )
            msg += "true";
        else
            msg += "false";
        msgbox mb(
            msg );
    });

    pg.events().click([&]
    {
        msgbox mb( "pg click");
    });

    form.showModal();
}

void InputboxDemo( gui& form )
{
    wex::inputbox ib;
    ib.gridWidth( 800 );
    ib.add("A","72");
    ib.add("B","4600");
    ib.choice("Choose", { "X", "Y"} );
    std::cout << "showing modal " << ib.id() << "\n";
    ib.showModal();

    std::string msg =
        "A is " + ib.value("A") +
        ", B is " + ib.value("B") +
        ", choice is " + ib.value("Choose");
    msgbox mb(
        msg );
}
void RBDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("A windex radiobutton");

    wex::groupbox& P = wex::maker::make<wex::groupbox>( form );
    P.move( 5, 5, 350,200 );

    // use laypout to atomatically arrange buttons in columns
    wex::layout& L = wex::maker::make<wex::layout>(P  );
    L.move( 50, 50,300,190);
    L.grid( 2 );                // specify 2 columns
    L.colfirst();               // specify column first order

    // first group of radiobuttons
    static std::vector<std::string> group0labels { "Alpha", "Beta", "Gamma" };
    radiobutton& rb1 = wex::maker::make<radiobutton>(L);
    rb1.first();                // first in group of interacting buttons
    rb1.move( {20,20,100,30} );
    rb1.text(group0labels[0]);
    radiobutton& rb2 = wex::maker::make<radiobutton>(L);
    rb2.move( {20,60,100,30} );
    rb2.text(group0labels[1]);
    radiobutton& rb3 = wex::maker::make<radiobutton>(L);
    rb3.move( {20,100,100,30} );
    rb3.text(group0labels[2]);

    // second group of radio buttons
    static std::vector<std::string> group1labels { "X", "Y", "Z" };
    radiobutton& rb4 = wex::maker::make<radiobutton>(L);
    rb4.first();                // first in group of interacting buttons
    rb4.size( 80,30 );
    rb4.text(group1labels[0]);
    radiobutton& rb5 = wex::maker::make<radiobutton>(L);
    rb5.size( 80,30 );
    rb5.text(group1labels[1]);
    radiobutton& rb6 = wex::maker::make<radiobutton>(L);
    rb6.size( 80,30 );
    rb6.text(group1labels[2]);

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 250, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the values entered
    btn.events().click([&]
    {
        std::string msg;
        int coff = rb1.checkedOffset();
        if( coff >= 0 )
            msg = group0labels[ coff ];
        else
            msg = "nothing";
        coff = rb4.checkedOffset();
        if( coff >= 0 )
            msg += " and " + group1labels[ coff ];
        else
            msg = " and nothing";

        msgbox mb(
            msg );
    });

    // show the application
    form.show();
}

void CBDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("A windex checkbox");

    checkbox& rb1 = wex::maker::make<checkbox>(form);
    rb1.move( {20,20,100,20} );
    rb1.text("Alpha");
    rb1.events().click([&]
    {
        if( rb1.isChecked() )
            msgbox mb( "Alpha clicked true") ;
        else
            msgbox mb( "Alpha clicked false") ;
    });
    checkbox& rb2 = wex::maker::make<checkbox>(form);
    rb2.plus();
    rb2.move( {20,60,100,30} );
    rb2.text("Beta");
    checkbox& rb3 = wex::maker::make<checkbox>(form);
    rb3.move( {20,100,100,30} );
    rb3.text("Gamma");

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the values entered
    btn.events().click([&]
    {
        std::string msg;
        if( rb1.isChecked() )
            msg += "Alpha ";
        if( rb2.isChecked() )
            msg += " Beta ";
        if( rb3.isChecked() )
            msg += " Gamma ";
        msg += " are checked";
        msgbox mb(
            msg );
    });

    // show the application
    form.show();

}

void PanelDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("Panel demo");

    // construct panel
    groupbox& pnl = wex::maker::make<groupbox>( form );
    pnl.move({ 100,100,200,200} );
    pnl.text("test");

    // display labels
    label& lbA = wex::maker::make<label>( pnl );
    lbA.move( {20, 20, 50, 30 } );
    lbA.text("A:");
    label& lbB = wex::maker::make<label>( pnl );
    lbB.move( {20, 60, 50, 30 } );
    lbB.text("B:");

    form.show();
}

void ScrollDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.scroll();
    form.move({ 50,50,400,400});
    form.scrollRange(600,600);
    form.text("Scroll demo");

    // display labels
    label& lbA = wex::maker::make<label>( form );
    lbA.move( {20, 20, 500, 30 } );
    lbA.text("AXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    label& lbB = wex::maker::make<label>( form );
    lbB.move( {20, 460, 500, 30 } );
    lbB.text("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");;

    form.show();
}

void SliderDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,500,700});
    form.text("Slider demo");

    // construct labels to display values when sliders are moved
    wex::label& label = wex::maker::make<wex::label>(form);
    label.move( 200, 110, 100,30 );
    label.text("");
    wex::label& vlabel = wex::maker::make<wex::label>(form);
    vlabel.move( 320, 110, 100,30 );
    vlabel.text("");

    wex::gauge& hg = wex::maker::make<wex::gauge>(form);
    hg.move(200,150, 200,200 );
    hg.range(100);
    wex::gauge& hv = wex::maker::make<wex::gauge>(form);
    hv.move(200,350, 200,200 );

    // construct horizontal slider
    wex::slider& S = wex::maker::make<wex::slider>( form );
    S.move({ 50,50,400,50});
    S.range( 0, 100 );
    S.text("horiz slider");
    S.events().slid([&](int pos)
    {
        label.text("horiz value: " + std::to_string( pos ));
        label.update();
        hg.value( pos );
        hg.update();
    });

    // construct vertical slider
    wex::slider& V = wex::maker::make<wex::slider>( form );
    V.move({ 50,100,50,400});
    V.range( 0, 10 );
    V.vertical();
    V.bgcolor( 0x0000FF );
    V.events().slid([&](int pos)
    {
        vlabel.text("vert value: " + std::to_string( pos ));
        vlabel.update();
        hv.value( pos );
        hv.update();
    });

    form.show();
}

void DropDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,500,400});
    form.text("Drop files demo");

    // widget for receiving dropped files
    drop& dropper = wex::maker::make<wex::drop>( form );
    dropper.move( 10,10,490,390 );
    label& instructions = wex::maker::make<wex::label>( dropper );
    instructions.move(30,30,400,200);
    instructions.text("Drop files here");

    // dropped files event handler
    dropper.events().drop( [&](const std::vector<std::string>& files )
    {
        // display list of dropped files
        std::string msg;
        msg = "Files dropped:\n";
        for( auto& f : files )
            msg += f + "\n ";
        instructions.text( msg );
        instructions.update();
    });

    form.show();
}

void MenuDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("Menu demo");

    menubar mb( form );

    menu f( form );
    f.append("open",[&](const std::string& title)
    {
        msgbox mb("File open");
    });
    f.append("save");
    mb.append("File", f );
    menu e( form );
    e.append("change");
    mb.append("Edit", e );


//    int clicked = -1;
//
//    menu m;
//    m.append("test",[&]
//    {
//        clicked = 1;
//    });
//    m.append("second",[&]
//    {
//        clicked = 2;
//    });
//    m.append("third",[&]
//    {
//        clicked = 3;
//    });
//    m.popup( form, 200,200 );
//
//    msgbox( form,std::string("item ") + std::to_string(clicked) + " clicked");

    form.show();
}

void PlotDemo()
{
    wex::gui& fm = wex::maker::make();
    fm.move( 50,50,1200,600 );

    // construct plot to be drawn on form
    wex::plot::plot& thePlot = wex::maker::make<wex::plot::plot>( fm );
    thePlot.bgcolor( 0 );
    //thePlot.Grid( true );
    // resize plot when form resizes
    fm.events().resize([&](int w, int h )
    {
        thePlot.size( w-100, h-200 );
        thePlot.move( 30, 100 );
        thePlot.update();
    });

    wex::button& btnStatic = wex::maker::make<wex::button>(fm);
    btnStatic.move(100,10,50,20);
    btnStatic.text("Static");
    btnStatic.tooltip("Display line ( single valued ) plot");
    btnStatic.events().click([&]
    {
        // construct plot traces
        wex::plot::trace& t1 = thePlot.AddStaticTrace();
        wex::plot::trace& t2 = thePlot.AddStaticTrace();

        // provide some data for first trace
        std::vector< double > d1 { 10, 15, 20, 25, 30, 25, 20, 15, 10 };
//        for( double& d : d1 )
//            d = d / 1000;
        t1.set( d1 );

        // plot in blue
        t1.color( 0x0000FF );

        // provide data for second trace
        std::vector< double > d2 { 20, 59, 60, 59, 20 };
//        for( double& d : d2 )
//            d = d / 1000;
        t2.set( d2 );

        // plot in red
        t2.color( 0xFF0000 );

        thePlot.update();
    });

    wex::button& btnScatter = wex::maker::make<wex::button>(fm);
    btnScatter.move(200,10,50,20);
    btnScatter.text("Scatter");
    btnScatter.tooltip("Display scatter ( individual, perhaps multiple y values for each x ) plot");
    btnScatter.events().click([&]
    {
        std::vector< double > x { 0, 1, 2, 3, 4 };
        std::vector< double > y {  100, 50, -100, 50, 200 };
        plot::trace& t2 = thePlot.AddScatterTrace();
        t2.color( 0xFF0000 );
        for( int k = 0; k < (int)x.size(); k++ )
        {
            t2.add( 10 * x[k], y[k] );
        }
        thePlot.update();
    });

    wex::button& btnTime = wex::maker::make<wex::button>(fm);
    btnTime.move(300,10,100,20);
    btnTime.text("Real Time");
    btnTime.tooltip("Display moving line plot that updates in real time");
    btnTime.events().click([&]
    {
        // construct plot trace
        // displaying 100 points before they scroll off the plot
        plot::trace& t1 = thePlot.AddRealTimeTrace( 100 );

        // plot in blue
        t1.color( 0xFF0000 );

        // create timer handler to provide new data regularly
        fm.events().timer([&](int id)
        {
            static int p = 0;
            t1.add( 10 * sin( p++ / 10.0 ) );
            thePlot.update();
        });

        // construct timer as static so the destructor is not called
        // when it goes out of scope
        static timer t( fm, 10 );
    });

    fm.show();
}

void TabDemo()
{
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("Tabbed Panel demo");

    // construct tabbed panel
    tabbed& tabs = maker::make<tabbed>( form );
    tabs.move( 50,50, 300, 200 );

    // add some demo panels
    panel& cam1panel = maker::make<panel>( tabs );
    label& cam1label = maker::make<label>( cam1panel );
    cam1label.move(30,100, 100,20 );
    cam1label.text("CAM1 panel");
    tabs.add( "CAM1", cam1panel );

    panel& cam2panel = maker::make<panel>( tabs );
    label& cam2label = maker::make<label>( cam2panel );
    cam2label.move(30,100, 100,20 );
    cam2label.text("CAM2 panel");
    tabs.add( "CAM2", cam2panel );

    panel& cam3panel = maker::make<panel>( tabs );
    label& cam3label = maker::make<label>( cam3panel );
    cam3label.move(30,100, 100,20 );
    cam3label.text("CAM3 panel");
    tabs.add( "CAM3", cam3panel );

    tabs.tabChanged( [&]( int index )
    {
        msgbox mb(
            "tab changed to" + std::to_string( index )) ;
    });

    form.show();

    // initially show the first panel
    // must be donw after call to show, which displays the last panel added
    tabs.select( 0 );
}



int main()
{

    // construct top level application window
    gui& form = maker::make();
    form.move({ 50,50,400,500});
    form.text("Windex demos");
    form.fontHeight( 20 );

    // construct layout to arrange buttons in a grid
    // layout& l = wex::make<layout>( form );
    layout& l = maker::make<layout>( form );
    l.move( {20,20,400,400} );
    l.grid( 2 );

    // handle resize
    form.events().resize([&]( int w, int h )
    {
        l.size( w-40,h-40 );
        l.move( 20,20 );
        l.update();
    });

    // display a button
    button& btnhello = wex::maker::make<button>( l );
    btnhello.size( 150, 30 );
    btnhello.text( "Label and Editbox" );
    btnhello.events().click([]
    {
        helloworld();
    });
    button& btnchoice = wex::maker::make<button>( l );
    btnchoice.size(  150, 30 );
    btnchoice.text( "Choice" );
    btnchoice.events().click([]
    {
        choiceDemo();
    });
    button& btnpg = wex::maker::make<button>( l );
    btnpg.size(  150, 30 );
    btnpg.text( "Property Grid" );
    btnpg.events().click([]
    {
        PGDemo();
    });
    button& btnib = wex::maker::make<button>( l );
    btnib.size(  150, 30 );
    btnib.text( "Inputbox" );
    btnib.events().click([&]
    {
        InputboxDemo( form );
    });

    button& btnfb = wex::maker::make<button>( l );
    btnfb.size(  150, 30 );
    btnfb.text( "Filebox" );
    btnfb.events().click([&]
    {
        filebox fb( form );
        msgbox mb(
                fb.path() );
    });

    button& btnrb = wex::maker::make<button>( l );
    btnrb.size( 150, 30 );
    btnrb.text( "Radiobutton" );
    btnrb.events().click([&]
    {
        RBDemo();
    });

    button& btncb = wex::maker::make<button>( l );
    btncb.size( 150, 30 );
    btncb.text( "Checkbox" );
    btncb.events().click([&]
    {
        CBDemo();
    });

    button& btnpanel =wex::maker::make<button>( l );
    btnpanel.size( 150, 30 );
    btnpanel.text( "Panel" );
    btnpanel.events().click([&]
    {
        PanelDemo();
    });

    button& btndraw = wex::maker::make<button>( l );
    btndraw.size( 150, 30 );
    btndraw.text( "Draw" );
    btndraw.events().click([&]
    {
        drawDemo();
    });

    button& btnscroll = wex::maker::make<button>( l );
    btnscroll.size( 150, 30 );
    btnscroll.text( "Scroll" );
    btnscroll.events().click([&]
    {
        ScrollDemo();
    });

    button& btnmenu = wex::maker::make<button>( l );
    btnmenu.size(  150, 30 );
    btnmenu.text( "Menu" );
    btnmenu.events().click([&]
    {
        MenuDemo();
    });

    button& btnplot = wex::maker::make<button>( l );
    btnplot.size(  150, 30 );
    btnplot.text( "Plot" );
    btnplot.events().click([&]
    {
        PlotDemo();
    });

    button& btnslider = wex::maker::make<button>( l );
    btnslider.size(  150, 30 );
    btnslider.text( "Slider Guage" );
    btnslider.events().click([&]
    {
        SliderDemo();
    });

    button& btnpng = wex::maker::make<button>( l );
    btnpng.size(  150, 30 );
    btnpng.text( "Save to demo.png" );
    btnpng.events().click([&]
    {
        window2file w2f;
        w2f.save( form, "demo.png" );
    });

    button& btndrop = wex::maker::make<button>( l );
    btndrop.size(  150, 30 );
    btndrop.text( "Drop files" );
    btndrop.events().click([&]
    {
        DropDemo();
    });

    button& btntabs = wex::maker::make<button>( l );
    btntabs.size(  150, 30 );
    btntabs.text( "Tabbed Panel" );
    btntabs.events().click([&]
    {
        TabDemo();
    });

    // show the application
    form.show();




    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    form.run();

    return 0;
}
