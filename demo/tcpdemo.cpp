#include <iostream>
#include "tcp.h"
#include "wex.h"


int main( int argc, char* argv[] )
{
    if( argc != 2 )
    {
        std::cout << "Usage: tcp client|server\n";
        return 1;
    }
    wex::tcp tcp;
    try
    {
        if( *argv[1] == 'c' )
            tcp.client();
        else if( *argv[1] == 's' )
            tcp.server();
        else
        {
            std::cout << "Usage: tcp client|server\n";
            return 1;
        }

    }
    catch( std::runtime_error& e )
    {
        std::cout << "Cannot connect " << e.what() << "\n";
        return 1;
    }

    wex::gui& form = wex::maker::make();
    form.show();
    form.run();
    return 0;
}
