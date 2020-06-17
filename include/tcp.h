#include <winsock2.h>
#include <ws2tcpip.h>

namespace wex
{
class tcp
{
public:
    enum class eType
    {
        client,
        server,
    };
    tcp()
        : myType( eType::client )
        ,mySocket( INVALID_SOCKET )

    {
        // Initialize Winsock
        WSADATA wsaData;
        if( WSAStartup(MAKEWORD(2,2), &wsaData) )
        {
            throw std::runtime_error("Winsock init failed");
        };
    }
    void type( eType t )
    {
        myType = t;
    }
    void socket(
        const std::string& ipaddr = "localhost",
        const std::string& port = "27654" )
    {
        struct addrinfo *result = NULL,
                             hints;

        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if ( getaddrinfo(
                    ipaddr.c_str(), port.c_str(),
                    &hints, &result) )
        {
            throw std::runtime_error("getaddrinfo failed" );
        }

        mySocket = ::socket(
                       result->ai_family,
                       result->ai_socktype,
                       result->ai_protocol);
        if( mySocket == INVALID_SOCKET )
        {
            throw std::runtime_error("socket failed" );
        }

        switch( myType )
        {
        case eType::client:
            if( ::connect(
                        mySocket,
                        result->ai_addr,
                        (int)result->ai_addrlen)
                    == SOCKET_ERROR )
            {
                closesocket(mySocket);
                mySocket = INVALID_SOCKET;
                throw std::runtime_error("connect failed" );
            }
            break;
        case eType::server:
            if( ::bind( mySocket,
                        result->ai_addr,
                        (int)result->ai_addrlen)
                    == SOCKET_ERROR )
            {
                closesocket(mySocket);
                mySocket = INVALID_SOCKET;
                throw std::runtime_error("bind failed" );
            }
            break;
        }
    }
private:
    eType myType;
    SOCKET mySocket;
};
}
