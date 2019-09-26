#pragma once
#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <windows.h>

namespace wex
{

class gui;
class widget;

typedef std::map< HWND, gui* > mgui_t;

/// Poll the windows message queue ( never returns )
void exec();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/// Functions to be called when event occurs
class eventhandler
{
public:
    void onLeftdown()
    {
        myClickFunction();
    }
    void click( std::function<void(void)> f )
    {
        myClickFunction = f;
    }
private:
    std::function<void(void)> myClickFunction;
};

/// Base class for all gui elements
class gui
{
public:
    gui()
        : myDeleteList( 0 )
    {
        registerWindowClass();
    }
    virtual ~gui()
    {
        myDeleteList->push_back( myHandle );
    }
    virtual bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return false;
    }

    eventhandler& events()
    {
        return myEvents;
    }
    HWND handle()
    {
        return myHandle;
    }
    void delete_list( std::vector< HWND >* list )
    {
        myDeleteList = list;
    }

protected:
    HWND myHandle;
    eventhandler myEvents;
    std::vector< HWND >* myDeleteList;
    void Create( HWND parent, DWORD style );
private:
    void registerWindowClass();
};



/// A top level window
class window : public gui
{
public:
    window()
        : myfApp( false )
    {
        Create( NULL, WS_OVERLAPPEDWINDOW );
    }

    void show();

    void text( const std::string& txt )
    {
        SetWindowText( myHandle, txt.c_str() );
    }
    void move( const std::vector<int>& r )
    {
        MoveWindow( myHandle,
                    r[0],r[1],r[2],r[3],false);
    }
    void child( widget* w )
    {
        myWidget.push_back( w );
    }
    void quit()
    {
        myfApp = true;
    }

    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    std::vector< widget* > myWidget;
    bool myfApp;                        /// true if app should quit when window destroyed

    virtual void draw( PAINTSTRUCT& ps ) {}
};

/// A widget placed inside a window
class widget : public gui
{
public:
    widget( window& parent )
    {
        Create( parent.handle(), WS_CHILD );
        parent.child( this );
    }


    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void show()
    {
        ShowWindow(myHandle,  SW_SHOWDEFAULT);
    }

    void move( const std::vector<int>& r )
    {
        MoveWindow( myHandle,
                    r[0],r[1],r[2],r[3],false);
    }

    void text( const std::string& txt )
    {
        myText = txt;
    }

protected:
    virtual void draw( PAINTSTRUCT& ps )
    {
        DrawText(
            ps.hdc,
            myText.c_str(),
            -1,
            &ps.rcPaint,
            0);
    }
private:
    std::string myText;



};

/// A button
class button : public widget
{
public:
    button( window& parent )
        : widget( parent )
    {

    }
protected:
    /// draw - label inside rectangle
    virtual void draw( PAINTSTRUCT& ps )
    {
        widget::draw( ps );
        DrawEdge(
            ps.hdc,
            &ps.rcPaint,
            EDGE_RAISED,
            BF_RECT
        );
    }
};

/// A popup with a message
class msgbox : public window
{
public:
    msgbox( const std::string& msg )
        : myText( msg )
    {
        SetWindowText( myHandle, "Message" );
        ShowWindow(myHandle,  SW_SHOWDEFAULT);
        MoveWindow( myHandle,
                    100,100,400,100,true);
    }
    void draw( PAINTSTRUCT& ps)
    {
        ps.rcPaint.left += 5;
        ps.rcPaint.top += 5;
        DrawText(
            GetDC( myHandle ),
            myText.c_str(),
            -1,
            &ps.rcPaint,
            0 );
    }

private:
    std::string myText;
};

class label : public widget
{
public:
    label( window& parent )
        : widget( parent )
    {

    }
};

class windex
{
public:
    window& MakeWindow()
    {
        window* w = new window();
        Add( w );

        // Assume that the first window created is the application window
        // quit the application of it is destroyed
        static bool appdone = false;
        if( ! appdone )
        {
            w->quit();
            appdone = true;
        }

        return *w;
    }
    label& MakeLabel( window& parent )
    {
        label* w = new label( parent );
        Add( w );
        return *w;
    }
    button& MakeButton( window& parent )
    {
        button* w = new button( parent );
        Add( w );
        return *w;
    }
    msgbox& MakeMsgBox( const std::string& msg )
    {
        msgbox* w = new msgbox( msg );
        Add( w );
        return *w;
    }
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //std::cout << "WindowProc " << hwnd <<" "<< uMsg << "\n";
        auto w = myGui.find( hwnd );
        if( w != myGui.end() )
        {
            if( w->second )
            {
                if( w->second->WindowMessageHandler( hwnd, uMsg, wParam, lParam ) )
                    return 0;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
private:
    static mgui_t myGui;
    std::vector< HWND > myDeleteList;

    void Add( gui * g )
    {
        Delete();
        g->delete_list( &myDeleteList );
        myGui.insert( std::make_pair( g->handle(), g ));
    }
    void Delete()
    {
        for( auto h : myDeleteList )
        {
            auto i = myGui.find( h );
            if( i != myGui.end() )
                myGui.erase( i );
        }
    }

};

}
