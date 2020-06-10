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
    clock.value( 1 );

    wex::editbox& e = wex::maker::make<wex::editbox>( form );
    e.move(100,500,50,30);
    e.text("1.0");
    e.events().change( e.id(),[&]
    {
        clock.value( atof(e.text().c_str()));
        clock.update();
    });
    form.events().resize([&](int w, int h)
    {
        clock.move( 30,30,w-120,h-220 );
        e.move(100,h-100,50,30);
        form.update();
    });

    form.show();
    form.run();
    return 0;
}
