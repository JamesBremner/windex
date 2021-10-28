#pragma once
#include <winsock2.h>
#include <windows.h>
#include "wex.h"
#include "ctcp.h"
#include "await.h"

namespace wex
{
    /** @brief Read/Write to TCP/IP socket, client or server

For sample code, see https://github.com/JamesBremner/windex/blob/master/demo/tcpdemo.cpp
*/
    class tcp : public gui
    {
    public:

        /** CTOR
        @param[in] parent window that will receive event messages
    */
        tcp(gui *parent) : gui(parent)
        {
        }

        /** Create client socket connected to server
        @param[in] ipaddr IP address of server, defaults to same computer
        @param[in] port defaults to 27654
    */
        void client(
            const std::string &ipaddr = "127.0.0.1",
            const std::string &port = "27654")
        {
            myTCP.server(ipaddr, port);

            // wait for connection to server
            myWaiter(
                [&]
                { myTCP.serverWait(); },
                [this]
                {
                    std::cout << "wex::tcp connected" << std::endl;
                    if (!PostMessageA(
                            myParent->handle(),
                            WM_APP + 2,
                            myID,
                            0))
                    {
                        std::cout << "Post Message Error\n";
                    }
                });
        }

        /// Run asynchronous wait handler in its own thread
        void run()
        {
            std::thread t(
                raven::await::cAwait::run,
                &myWaiter);
            t.detach();
        }

        /** Create server socket waiting for connection requests
        @param[in] port, defaults to 27654

        Starts listening for client connection.
        Returns immediatly
        throws runtime_error exception on error
        sends eventMsgID::tcpServerAccept message to parent window when new client accepted

        One connection will be accepted.  This can be called again if the connection is closed
        to wait for another client.
    */
        void server(const std::string &port = "27654")
        {
            myTCP.server("", port);

            myWaiter(
                [&]
                { myTCP.acceptClient(); },
                [this]
                {
                    std::cout << "connected" << std::endl;
                    PostMessageA(
                        myParent->handle(),
                        WM_APP + 2,
                        myID,
                        0);
                });
        }

        /// true if valid connection
        bool isConnected()
        {
            return myTCP.isConnected();
        }

        /** send message to peer
        * @param[in] msg
        */
        void send(const std::string &msg)
        {
            std::cout << "wex::tcp::send\n";
            myTCP.send(msg);
        }

        /** asynchronous read message on tcp connection
         * 
         * Throws exception if no tcp connection
         * 
         * Returns immediatly.
         * 
         * When message is received, the parent window will receive tcpRead event
         * 
         * If the connection is closed, or suffers any error
         * the same event will be invoked, so the isConnected()
         * method should be checked.
    */
        void read()
        {
            myWaiter(
                [this]
                { myTCP.read(); },
                [this]
                {
                    // post read complete message
                    PostMessageA(
                        myParent->handle(),
                        WM_APP + 3,
                        myID,
                        0);
                });
        }

        /// Get last message from peer
        std::string readMsg() const
        {
            return myTCP.readMsg();
        }

    private:

        std::string myRemoteAddress;

        cTCP myTCP;
        raven::await::cAwait myWaiter;

 
    };
}
