# windex
 
A header only modern c++ wrapper for the windows API.

Implements simple versions of toplevel window, label, button, textbox, messagebox and propertyGrid.

Here is the hello world code:

```

// include the header only windex gui framework
#include "wex.h"

int main()
{
    using namespace wex;

    // reference the windex gui framework
    windex& W = windex::get();

    // construct top level application window
    window& form = W.MakeWindow();
    form.move({ 50,50,400,400});
    form.text("Hello world from windex");

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

    // show the application
    form.show();

    //Pass the control of the application to the windows message queue.
    //Blocks execution for dispatching user
    //input until the form is closed.

    W.exec();

    return 0;
}

```

# Build

No separate build step is required. since this is a header only library.  

Add `"include "wex.h";` to application source files that use windex and add the `include` folder to the compiler search directories.

This avoids all the challenging configuration management problems that can result from trying to maintain a separate library build, when the library build and application build use different compilers or compiler options.
