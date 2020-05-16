#include <functional>
#include <future>
#include "com.h"

int main( int argc, char* argv[] )
{
    if( argc != 2 )
    {
        std::cout << "Usage: com <com port number>\n";
        exit(1);
    }
    wex::com com;
    com.port( argv[1] );
    com.open();
    if( ! com.isOpen() )
    {
        std::cout << "Cannot open port " << argv[1]
                  << "\n";
        exit(1);
    }
    std::cout << "port " << argv[1] << " open\n";
    std::vector<unsigned char> buf;
    com.read( 8 );
    buf = com.readData();
    for( auto c : buf )
        std::cout << (char)c << " ";
    com.read( 5 );
    buf = com.readData();
    std::cout << "read:";
    for( auto c : buf )
        std::cout << (char)c << " ";
    if( buf[0] == 'R' && buf[1] == 'M' && buf[2] == 'A' )
    {
        std::string goodRMAresponse;
        goodRMAresponse = "RMA,12345,789.12,345.67,891.23,456.7,91.23\n\r";
        com.write( goodRMAresponse );
    }
}
