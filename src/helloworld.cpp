#include <iostream>

#include "wex.h"

using namespace std;

int main()
{
    wex::window W;
    W.move({ 50,50,800,800});
    W.text("Hello world from windex");
    wex::widget label( W );
    label.move( {20, 20, 100, 30 } );
    label.text("tst label");
    wex::button btn( W );
    btn.move( {20, 60, 70, 30 } );
    btn.text( "OK" );
    btn.events().click([]
    {
        std::cout << "button clicked\n";
    });
    W.show();
    wex::exec();
    return 0;
}
