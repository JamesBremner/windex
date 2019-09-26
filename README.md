# windex
 
A header only modern c++ wrapper for the windows API.

Here is the hello world code:

```
// include the header only windex gui framework
#include "wex.h"

int main()
{
    // reference the windex gui framework
    wex::windex& W = wex::windex::get();

    // construct top level application window
    wex::window& form = W.MakeWindow();
    form.move({ 50,50,800,800});
    form.text("Hello world from windex");

    // display a label
    wex::label& label = W.MakeLabel( form );
    label.move( {20, 20, 100, 30 } );
    label.text("tst label");

    // display a button
    wex::button& btn = W.MakeButton( form );
    btn.move( {20, 60, 70, 30 } );
    btn.text( "OK" );

    // popup a message box when button is clicked
    btn.events().click([&W]
    {
        W.MakeMsgBox("button clicked");
    });

    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}
```
