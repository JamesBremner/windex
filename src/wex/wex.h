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
typedef std::vector< widget* > children_t;

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
    }
    virtual ~gui()
    {
        myDeleteList->push_back( myHandle );
    }

    /// No messages are handled by the base class
    virtual bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return false;
    }

    /// Get event handler
    eventhandler& events()
    {
        return myEvents;
    }

    /// get window handle
    HWND handle()
    {
        return myHandle;
    }


    /// set delete list for when gui is detroyed
    void delete_list( std::vector< HWND >* list )
    {
        myDeleteList = list;
    }

protected:
    HWND myHandle;
    eventhandler myEvents;
    std::vector< HWND >* myDeleteList;

    /** Create the managed window */
    void Create( HWND parent, DWORD style )
    {
        myHandle = CreateWindowEx(
                       0,                              // Optional window styles.
                       "windex",                     // Window class
                       "widget",    // Window text
                       style,            // Window style

                       // Size and position
                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                       parent,       // Parent window
                       NULL,       // Menu
                       NULL,  // Instance handle
                       NULL        // Additional application data
                   );
    }
};


/// A widget placed inside a window
class widget : public gui
{
public:

    /** CTOR
        @param[in] parent window handle
        @param[in] children parent's list to add to
    */
    widget( HWND parent, children_t& children )
    {
        Create( parent, WS_CHILD );
        children.push_back( this );
    }

    /// Handle windows messages
    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //std::cout << " widget WindowMessageHandler " << uMsg << "\n";
        if( hwnd == myHandle )
        {
            switch (uMsg)
            {
            case WM_DESTROY:
                return true;

            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(myHandle, &ps);

                draw(ps);

                EndPaint(myHandle, &ps);
            }
            return true;

            case WM_LBUTTONDOWN:
                myEvents.onLeftdown();
                return true;
            }
        }
        return false;
    }


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


/// A top level window
class window : public gui
{
public:
    window()
        : myfApp( false )
    {
        Create( NULL, WS_OVERLAPPEDWINDOW );
    }

    void show()
    {
        ShowWindow(myHandle,  SW_SHOWDEFAULT);

        // display the children
        for( auto w : myWidget )
            w->show();
    }

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

    /// Set so that application quits if window is destoryed
    void quit()
    {
        myfApp = true;
    }

    children_t& children()
    {
        return myWidget;
    }

    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
//    std::cout << "window::WindowMessageHandler "
//              << myHandle <<" "<< uMsg << "\n";
        if( hwnd == myHandle )
        {
            switch (uMsg)
            {
            case WM_DESTROY:
                if( myfApp )
                    PostQuitMessage(0);
                return true;

            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(myHandle, &ps);

                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
                draw( ps );

                EndPaint(myHandle, &ps);
            }
            return true;

            }
        }
        else
        {
            for( auto w : myWidget )
            {
                if( w->WindowMessageHandler( hwnd, uMsg, wParam, lParam ))
                    return true;
            }
        }
        return false;

    }

protected:
    std::vector< widget* > myWidget;
    bool myfApp;                        /// true if app should quit when window destroyed

    virtual void draw( PAINTSTRUCT& ps ) {}
};

/// A button
class button : public widget
{
public:
    button( HWND parent, children_t& children )
        : widget( parent, children )
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
class msgbox
{
public:
    msgbox( gui& parent, const std::string& msg )
    {
        MessageBox(parent.handle(),
                   msg.c_str(),
                   "Message",
                   MB_OK);
    }
};

/// label
class label : public widget
{
public:
    label( HWND parent, children_t& children )
        : widget( parent, children )
    {

    }
};

class windex
{
public:

    /** get reference to windex gui framework ( singleton )

    This creates a singleton instance of the windex class
    and returns as many references to it as needed.

    The windex ctor should never be called by app code - it is private!

    */
    static windex& get()
    {
        static windex theInstance;
        return theInstance;
    }

    /// get reference to new top level window
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

    /// get reference to new label to be displayed in parent window
    label& MakeLabel( window& parent )
    {
        label* w = new label( parent.handle(), parent.children() );
        Add( w );
        return *w;
    }

    /// get reference to new button to be displayed in parent window
    button& MakeButton( window& parent )
    {
        button* w = new button( parent.handle(), parent.children() );
        Add( w );
        return *w;
    }

    /// poll the windows message queue
    void exec()
    {
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    /// get map of existing gui elements
    mgui_t * mgui()
    {
        return &myGui;
    }

    /// handle windows messages
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //find gui element that generated message
        mgui_t* mgui = get().mgui();
        auto w = mgui->find( hwnd );
        if( w != mgui->end() )
        {
            if( w->second )
            {
                // handle message
                if( w->second->WindowMessageHandler( hwnd, uMsg, wParam, lParam ) )
                    return 0;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

private:
    mgui_t myGui;                       ///< map of existing gui elements
    std::vector< HWND > myDeleteList;   ///< gui elements that have been deleted but not yet removed from map

    windex()
    {
        // register a callback function
        // to be invoked every time a windex gui element receives a windows message
        WNDCLASS wc = { };
        wc.lpfnWndProc   = &windex::WindowProc;
        wc.hInstance     = NULL;
        wc.lpszClassName = "windex";
        RegisterClass(&wc);
    }

    /// Add new gui element
    void Add( gui * g )
    {
        // delete any destroyed elements
        Delete();

        // provide reference to delete list, so new gui element can be removed after destruction
        g->delete_list( &myDeleteList );

        // add to existing gui elements
        myGui.insert( std::make_pair( g->handle(), g ));
    }

    /// remove destroyed gui elements
    void Delete()
    {
        for( auto h : myDeleteList )
        {
            auto i = myGui.find( h );
            if( i != myGui.end() )
                myGui.erase( i );
        }
        myDeleteList.clear();
    }

};

}
