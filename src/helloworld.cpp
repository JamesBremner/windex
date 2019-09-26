#include <iostream>

#include "wex.h"

using namespace std;

wex::windex W;

int main()
{
    wex::window& form = W.MakeWindow();
    form.move({ 50,50,800,800});
    form.text("Hello world from windex");

    wex::label& label = W.MakeLabel( form );
    label.move( {20, 20, 100, 30 } );
    label.text("tst label");

    wex::button& btn = W.MakeButton( form );
    btn.move( {20, 60, 70, 30 } );
    btn.text( "OK" );
    btn.events().click([]
    {
        wex::msgbox& m = W.MakeMsgBox("button clicked");
        //m.show();
    });
    form.show();
    wex::exec();
    return 0;
}
