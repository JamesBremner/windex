#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <future>
#include "wex.h"

namespace wex
{
class tcp  : public gui
{
public:
    enum class eType
    {
        client,
        server,
    };
    tcp( gui* parent )
        : gui( parent )
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
        const std::string& ipaddr = "127.0.0.1",
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

        std::cout <<"try connect to " << ipaddr <<":"<< port << "\n";
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
        myPort = port;
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
        std::cout << "=>accept\n";

        // start blocking accept in own thread
        myFuture = std::async(
                       std::launch::async,              // insist on starting immediatly
                       &tcp::accept,
                       this );

        // start waiting for read completion in own thread
        myThread = new std::thread(accept_wait, this);
    }

private:
    eType myType;
    std::string myPort;
    SOCKET mySocket;
    SOCKET myClientSocket;
    std::future< void > myFuture;
    std::thread*        myThread;



    void accept()
    {
        std::cout << "listening for client on port " << myPort << "\n";

        myClientSocket = ::accept(mySocket, NULL, NULL);
        if (myClientSocket != INVALID_SOCKET)
        {
            std::cout << "clent accepted\n";
        }
    }
    void accept_wait()
    {
        const int check_interval_msecs = 50;
        while (myFuture.wait_for(std::chrono::milliseconds(check_interval_msecs))==std::future_status::timeout)
        {

        }
        PostMessageA(
            myParent->handle(),
            WM_APP+2,
            myID,
            0 );
    }
};
}
