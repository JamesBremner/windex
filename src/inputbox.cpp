
// include the header only windex gui framework
#include "wex.h"
#include "propertygrid.h"
#include "inputbox.h"

int main()
{
    using namespace wex;

    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level application window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("A windex inputbox");

    // display a button
    button& btn = W.make<button>( form );
    btn.move( {20, 150, 150, 30 } );
    btn.text( "Launch inputbox" );

    // popup a input box when button is clicked
    btn.events().click([&]
    {
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
    });

    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}

