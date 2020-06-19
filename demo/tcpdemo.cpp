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
    wex::label& myStatus;
    wex::tcp& myTCP;

    void status(const std::string& msg );
};

cGUI::cGUI()
    : myForm(wex::maker::make())
    , myClientrb( wex::maker::make<wex::radiobutton>( myForm ) )
    , myServerrb( wex::maker::make<wex::radiobutton>( myForm ) )
    , myConnectbn( wex::maker::make<wex::button>( myForm ) )
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
        if( myServerrb.isChecked() )
        {
            try
            {
                myTCP.server();
                status("Waiting for client to connedct");
            }
            catch( std::runtime_error& e )
            {
                status(std::string("Cannot start server ") + e.what() );
            }
        }
        else {
                try
                {
                    myTCP.client();
                    status("Connected to server ");
                }
                catch( std::runtime_error& e )
                {
                    status(std::string("Cannot connect to server ") + e.what() );

                }
            }
        });
        myForm.events().tcpServerAccept([this]
        {
            status("Client connected");
        });


        myForm.show();

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
