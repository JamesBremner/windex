#include <iostream>
#include "tcp.h"
#include "wex.h"

using namespace std;

int main()
{
    cout << "Hello world!" << endl;

    wex::tcp server;
    server.type( wex::tcp::eType::server );
    try
    {
        server.socket();
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
