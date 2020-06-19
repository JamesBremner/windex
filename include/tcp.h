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
        }
    }
    void client(
        const std::string& ipaddr = "localhost",
        const std::string& port = "27654" )
    {
        myType = eType::client;
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

        if( ::connect(
                    mySocket,
                    result->ai_addr,
                    (int)result->ai_addrlen)
                == SOCKET_ERROR )
        {
            closesocket(mySocket);
            mySocket = INVALID_SOCKET;
            throw std::runtime_error("connect failed "+std::to_string(WSAGetLastError()) );
        }
    }
    void server( const std::string& port = "27654" )
    {
        myType = eType::server;
        struct addrinfo *result = NULL,
                             hints;

        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        if ( getaddrinfo(
                    NULL, port.c_str(),
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

        if( ::bind( mySocket,
                    result->ai_addr,
                    (int)result->ai_addrlen)
                == SOCKET_ERROR )
        {
            closesocket(mySocket);
            mySocket = INVALID_SOCKET;
            throw std::runtime_error("bind failed" );
        }

        if ( ::listen(
                    mySocket,
                    SOMAXCONN )
                == SOCKET_ERROR )
        {
            closesocket(mySocket);
            mySocket = INVALID_SOCKET;
            throw std::runtime_error("listen failed" );
        }
        std::cout << "listening for client on port " << port << "\n";

        SOCKET ClientSocket = accept(mySocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
             throw std::runtime_error("accept  failed" );
        }
        std::cout << "clent accepted\n";
    }

private:
    eType myType;
    SOCKET mySocket;
};
}
