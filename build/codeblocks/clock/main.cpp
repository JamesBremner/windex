#include <iostream>

#include "wex.h"

using namespace std;

int main()
{
    wex::gui& form = wex::maker::make();
    form.move({ 50,50,500,600});
    form.text("Clock guage");

    wex::clock& clock = wex::maker::make<wex::clock>( form );
    clock.move( 30,30,380,380 );

    form.show();
    form.run();
    return 0;
}
