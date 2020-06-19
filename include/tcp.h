#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <future>
#include "wex.h"

namespace wex
{
/** \brief Read/Write to TCP/IP socket, client or server */
class tcp  : public gui
{
public:
    enum class eType
    {
        client,
        server,
    };
    /** CTOR
        @param[in] parent windows that will receive event messages
    */
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
    /** Create client socket
        @param[in] ipaddr IP address of server, defaults to same computer
        @param[in] port defaults to 27654
    */
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
    /** Create server socket
        @param[in] port, defaults to 27654

        Starts listening for client connection.
        Returns immediatly
        throws runtime_error exception on error
        sends eventMsgID::tcpServerAccept message to parent window when new client accepted
    */
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

        // start waiting for accept completion in own thread
        myThread = new std::thread(accept_wait, this);
    }

    /// true if valid connection
    bool isConnected()
    {
        return mySocket != INVALID_SOCKET;
    }

    /** send message to server
        @param[in] msg
    */
    void send(const std::string& msg )
    {
        if( mySocket == INVALID_SOCKET )
            return;
        ::send(mySocket, msg.c_str(), (int) msg.length(), 0);
    }
    /// get socket connected to client
    SOCKET& clientSocket()
    {
        return myClientSocket;
    }
    /** asynchronous read message from client
        @param[in] s socket connected to client
    */
    void read( SOCKET& s )
    {
        if( s == INVALID_SOCKET )
            return;
        myFuture = std::async(
                       std::launch::async,              // insist on starting immediatly
                       &tcp::read_block,
                       this,
                       std::ref( s ) );
        myThread = new std::thread(read_wait, this);
    }
    /// get pointer to receive buffer as null terminated character string
    char * rcvbuf()
    {
        return (char*) myRecvbuf;
    }

private:
    eType myType;
    std::string myPort;
    SOCKET mySocket;
    SOCKET myClientSocket;
    std::future< void > myFuture;
    std::thread*        myThread;
    unsigned char       myRecvbuf[1024];


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

    void read_block( SOCKET& s )
    {
        ZeroMemory( myRecvbuf, 1024 );
        ::recv( s, (char*)myRecvbuf, 1024, 0);
    }
    void read_wait()
    {
        const int check_interval_msecs = 50;
        while (myFuture.wait_for(std::chrono::milliseconds(check_interval_msecs))==std::future_status::timeout)
        {

        }
        PostMessageA(
            myParent->handle(),
            WM_APP+3,
            myID,
            0 );
    }
};
}
