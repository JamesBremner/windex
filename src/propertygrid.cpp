
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
    pg.labelWidth( 50 );
    pg.string( "test1", "72" );
    pg.string( "test2", "4600" );

    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}

