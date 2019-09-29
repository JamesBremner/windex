
// include the header only windex gui framework
#include "wex.h"
#include "propertygrid.h"

int main()
{
    using namespace wex;

    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level application window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex property grid");

    wex::propertyGrid pg( form );
    pg.move( { 50,50, 200, 0});
    pg.labelWidth( 50 );
    pg.bgcolor( 0xFFA0A0 );
    pg.string( "A", "72" );
    pg.string( "B", "4600" );

    // display a button
    button& btn = W.make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the value entered in textbox
    btn.events().click([&]
    {
        std::string msg =
            "A is " + pg.value("A") +
            ", B is " + pg.value("B");
        msgbox(
            form,
            msg );
    });

    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}

