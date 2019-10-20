
// include the header only windex gui framework
#include "wex.h"
#include "propertygrid.h"
#include "inputbox.h"
using namespace wex;

void helloworld()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("Label and Editbox demo");

    // display labels
    label& lbA = W.make<label>( form );
    lbA.move( {20, 20, 100, 30 } );
    lbA.text("A:");
    label& lbB = W.make<label>( form );
    lbB.move( {20, 60, 100, 30 } );
    lbB.text("B:");

    // display textboxes
    editbox& edit1 = W.make<editbox>( form );
    edit1.move( {80, 20, 100, 30 } );
    edit1.text( "type value");
    editbox& edit2 = W.make<editbox>( form );
    edit2.move( {80, 60, 100, 30 } );
    edit2.text( "type value");

    // display a button
    button& btn = W.make<button>( form );
    btn.move( {20, 100, 150, 30 } );
    btn.text( "Show values entered" );


    // popup a message box when button is clicked
    // showing the value entered in textbox
    btn.events().click([&]
    {
        std::string msg =
        "A is " + edit1.text() +
        ", B is " + edit2.text();
        msgbox(
            form,
            msg );
    });

    form.show();
}

void choiceDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level  window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex combobox");

    // display combobox
    choice& cb = W.make<choice>( form );
    cb.move({20, 50, 150, 130 });
    cb.add("Alpha");
    cb.add("Beta");

    // display a button
    button& btn = W.make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Show values selected" );
    btn.events().click([&]
    {
        msgbox(
            form,
            cb.SelectedText() );
    });

    form.show();
}

void drawDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level  window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex draw demo");

    form.events().draw([]( PAINTSTRUCT& ps )
    {
        shapes S( ps.hdc );
        S.color( 255, 0, 0 );
        S.line( { 10,10, 50,50 } );
        S.color( 255,255,255 );
        S.rectangle( { 20,20,20,20});
    });

    form.show();
}

void PGDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex property grid");

    // construct propertygrid
    propertyGrid pg( form );
    pg.move( { 50,50, 200, 0});
    pg.labelWidth( 50 );
    pg.bgcolor( 0xFFA0A0 );

    // add properties
    pg.category("Strings");
    pg.string( "A", "72" );
    pg.string( "B", "4600" );
    pg.category("Others");
    pg.choice( "Choose", { "X", "Y", "Z"} );
    pg.check( "Enable", false );

    // display a button
    button& btn = W.make<button>( form );
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
        msgbox(
            form,
            msg );
    });

    form.showModal();
}

void InputboxDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex inputbox");

    wex::inputbox ib( form );
    ib.add("A","72");
    ib.add("B","4600");
    ib.choice("Choose", { "X", "Y"} );
    ib.showModal();

    std::string msg =
        "A is " + ib.value("A") +
        ", B is " + ib.value("B") +
        ", choice is " + ib.value("Choose");
    msgbox(
        form,
        msg );

    // show the application
    form.show();

}
void RBDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex radiobutton");

    // first group of radiobuttons
    radiobutton& rb1 = W.make<radiobutton>(form);
    rb1.first();
    rb1.move( {20,20,100,30} );
    rb1.text("Alpha");
    radiobutton& rb2 = W.make<radiobutton>(form);
    rb2.move( {20,60,100,30} );
    rb2.text("Beta");
    radiobutton& rb3 = W.make<radiobutton>(form);
    rb3.move( {20,100,100,30} );
    rb3.text("Gamma");

    // second group of radio buttons
    radiobutton& rb4 = W.make<radiobutton>(form);
    rb4.first();
    rb4.move( {150,20,100,30} );
    rb4.text("X");
    radiobutton& rb5 = W.make<radiobutton>(form);
    rb5.move( {150,60,100,30} );
    rb5.text("Y");
    radiobutton& rb6 = W.make<radiobutton>(form);
    rb6.move( {150,100,100,30} );
    rb6.text("Z");

    // display a button
    button& btn = W.make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the values entered
    btn.events().click([&]
    {
        std::string msg;
        if( rb1.isChecked() )
            msg = "Alpha";
        else if( rb2.isChecked() )
            msg = "Beta";
        else if( rb3.isChecked() )
            msg = "Gamma";
        else
            msg = "Nothing";
        msg += " is checked";
        msgbox(
            form,
            msg );
    });

    // show the application
    form.show();

}

void CBDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex checkbox");

    checkbox& rb1 = W.make<checkbox>(form);
    rb1.move( {20,20,100,30} );
    rb1.text("Alpha");
    checkbox& rb2 = W.make<checkbox>(form);
    rb2.move( {20,60,100,30} );
    rb2.text("Beta");
    checkbox& rb3 = W.make<checkbox>(form);
    rb3.move( {20,100,100,30} );
    rb3.text("Gamma");

    // display a button
    button& btn = W.make<button>( form );
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
        msgbox(
            form,
            msg );
    });

    // show the application
    form.show();

}

void PanelDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("Panel demo");

    // construct panel
    groupbox& pnl = W.make<groupbox>( form );
    pnl.move({ 100,100,200,200} );
    pnl.text("test");

    // display labels
    label& lbA = W.make<label>( pnl );
    lbA.move( {20, 20, 50, 30 } );
    lbA.text("A:");
    label& lbB = W.make<label>( pnl );
    lbB.move( {20, 60, 50, 30 } );
    lbB.text("B:");

    form.show();
}

int main()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level application window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,500});
    form.text("Windex demos");

    // construct layout to arrange buttons in a grid
    layout& l = W.make<layout>( form );
    l.move( {20,20,400,400} );
    l.grid( 2 );

    // display a button
    button& btnhello = W.make<button>( l );
    btnhello.move( { 150, 30 }, false );
    btnhello.text( "Label and Editbox" );
    btnhello.events().click([]
    {
        helloworld();
    });
    button& btnchoice = W.make<button>( l );
    btnchoice.move(  { 150, 30 }, false );
    btnchoice.text( "Choice" );
    btnchoice.events().click([]
    {
        choiceDemo();
    });
    button& btnpg = W.make<button>( l );
    btnpg.move(  { 150, 30 }, false );
    btnpg.text( "Property Grid" );
    btnpg.events().click([]
    {
        PGDemo();
    });
    button& btnib = W.make<button>( l );
    btnib.move(  { 150, 30 }, false );
    btnib.text( "Inputbox" );
    btnib.events().click([]
    {
        InputboxDemo();
    });

    button& btnfb = W.make<button>( l );
    btnfb.move(  { 150, 30 }, false );
    btnfb.text( "Filebox" );
    btnfb.events().click([&]
    {
        filebox fb( form );
        msgbox( form,
                fb.path() );
    });

    button& btnrb = W.make<button>( l );
    btnrb.move(  { 150, 30 }, false );
    btnrb.text( "Radiobutton" );
    btnrb.events().click([&]
    {
        RBDemo();
    });

    button& btncb = W.make<button>( l );
    btncb.move( {20, 330, 150, 30 } );
    btncb.text( "Checkbox" );
    btncb.events().click([&]
    {
        CBDemo();
    });

    button& btnpanel = W.make<button>( l );
    btnpanel.move( {20, 380, 150, 30 } );
    btnpanel.text( "Panel" );
    btnpanel.events().click([&]
    {
        PanelDemo();
    });

    button& btndraw = W.make<button>( l );
    btndraw.move(  { 150, 30 }, false );
    btndraw.text( "Draw" );
    btndraw.events().click([&]
    {
        drawDemo();
    });


    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}
