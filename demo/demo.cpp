
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

void PGDemo()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex property grid");

    propertyGrid pg( form );
    pg.move( { 50,50, 200, 0});
    pg.labelWidth( 50 );
    pg.bgcolor( 0xFFA0A0 );
    pg.string( "A", "72" );
    pg.string( "B", "4600" );
    pg.choice( "Choose", { "X", "Y", "Z"} );

    // display a button
    button& btn = W.make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the values entered
    btn.events().click([&]
    {
        std::string msg =
        "A is " + pg.value("A") +
        ", B is " + pg.value("B") +
        ", choice is " + pg.value("Choose");
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

    radiobutton& rb1 = W.make<radiobutton>(form);
    rb1.move( {20,20,100,30} );
    rb1.text("Alpha");
    radiobutton& rb2 = W.make<radiobutton>(form);
    rb2.move( {20,60,100,30} );
    rb2.text("Beta");
    radiobutton& rb3 = W.make<radiobutton>(form);
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
    form.text("A windex radiobutton");

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
int main()
{
    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level application window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,500});
    form.text("Windex demos");

    // display a button
    button& btnhello = W.make<button>( form );
    btnhello.move( {20, 30, 150, 30 } );
    btnhello.text( "Label and Editbox" );
    btnhello.events().click([]
    {
        helloworld();
    });
    button& btnchoice = W.make<button>( form );
    btnchoice.move( {20, 80, 150, 30 } );
    btnchoice.text( "Choice" );
    btnchoice.events().click([]
    {
        choiceDemo();
    });
    button& btnpg = W.make<button>( form );
    btnpg.move( {20, 130, 150, 30 } );
    btnpg.text( "Property Grid" );
    btnpg.events().click([]
    {
        PGDemo();
    });
    button& btnib = W.make<button>( form );
    btnib.move( {20, 180, 150, 30 } );
    btnib.text( "Inputbox" );
    btnib.events().click([]
    {
        InputboxDemo();
    });

    button& btnfb = W.make<button>( form );
    btnfb.move( {20, 230, 150, 30 } );
    btnfb.text( "Filebox" );
    btnfb.events().click([&]
    {
        filebox fb( form );
        msgbox( form,
                fb.path() );
    });

    button& btnrb = W.make<button>( form );
    btnrb.move( {20, 280, 150, 30 } );
    btnrb.text( "Radiobutton" );
    btnrb.events().click([&]
    {
        RBDemo();
    });

    button& btncb = W.make<button>( form );
    btncb.move( {20, 330, 150, 30 } );
    btncb.text( "Checkbox" );
    btncb.events().click([&]
    {
        CBDemo();
    });

    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}
