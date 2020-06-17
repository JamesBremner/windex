#include <iostream>
#include "tcp.h"
#include "wex.h"


int main()
{
    wex::tcp client;
    try {
    client.socket();
    }
    catch( std::runtime_error& e )
    {
        std::cout << "Cannot connect to server: " << e.what() << "\n";
        return 1;
    }

    wex::gui& form = wex::maker::make();
    form.show();
    form.run();
    return 0;
}
