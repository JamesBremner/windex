#include <iostream>
#include "tcp.h"

class cGUI
{
public:
    cGUI();
    void run();
private:
    wex::gui& myForm;
    wex::radiobutton& myClientrb;
    wex::radiobutton& myServerrb;
    wex::button& myConnectbn;
    wex::button& mySendbn;
    wex::label& myStatus;
    wex::tcp& myTCP;
    SOCKET * myClientSocket;

    void status(const std::string& msg );
    void connect();
};

cGUI::cGUI()
    : myForm(wex::maker::make())
    , myClientrb( wex::maker::make<wex::radiobutton>( myForm ) )
    , myServerrb( wex::maker::make<wex::radiobutton>( myForm ) )
    , myConnectbn( wex::maker::make<wex::button>( myForm ) )
    , mySendbn( wex::maker::make<wex::button>( myForm ) )
    , myStatus( wex::maker::make<wex::label>( myForm ) )
    , myTCP( wex::maker::make<wex::tcp>( myForm ) )
{
    myForm.move(50,50,300,400);
    myForm.text("tcpDemo");

    myClientrb.move( 30, 20, 100,30 );
    myClientrb.text( "Client" );
    myServerrb.move( 200,20, 100,30 );
    myServerrb.text("Server");

    myConnectbn.move({50,50,100,30});
    myConnectbn.text("Connect");

    myStatus.move(50,100,300,30);
    myStatus.text("Not connected");

    myConnectbn.events().click([this]
    {
        connect();
    });
    myForm.events().tcpServerAccept([this]
    {
        status("Client connected");
        myClientSocket = &myTCP.clientSocket();
        myTCP.read( *myClientSocket );
    });
    myForm.events().tcpServerReadComplete([this]
    {
        // display mesage
        status(std::string("Msg read: ") + myTCP.rcvbuf() );

        // setup for next message
        if( myTCP.isServer() )
            myTCP.read( *myClientSocket );
        else
            myTCP.read();
    });

    mySendbn.move(50,150,100,30);
    mySendbn.text("Send hello msg");
    mySendbn.events().click([&]
    {
        if( myServerrb.isChecked() )
            myTCP.send( myTCP.clientSocket(), "Hello" );
        else
            myTCP.send("Hello");
    });


    myForm.show();

}
void cGUI::connect()
{
    if( myServerrb.isChecked() )
    {
        try
        {
            myTCP.server();
            status("Waiting for client to connect");
        }
        catch( std::runtime_error& e )
        {
            status(std::string("Cannot start server ") + e.what() );
        }
    }
    else
    {
        try
        {
            myTCP.client();
            status("Connected to server ");
            myTCP.read();
        }
        catch( std::runtime_error& e )
        {
            status(std::string("Cannot connect to server ") + e.what() );

        }
    }
}
void cGUI::status(const std::string& msg )
{
    myStatus.text(msg);
    myStatus.update();
}
void cGUI::run()
{
    myForm.run();
}
int main()
{

    cGUI gui;
    gui.run();
    return 0;
}
