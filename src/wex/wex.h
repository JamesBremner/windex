#pragma once
#include <iostream>
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
    eventhandler()
    {
        // initialize functions with no-ops
        click([] {});
    }
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
    void Create( char* window_class, HWND parent,
                 DWORD style, DWORD exstyle = 0, int id=0 )
    {
        myHandle = CreateWindowEx(
                       exstyle,                // Optional window styles.
                       window_class,     // Window class
                       "widget",         // Window text
                       style,            // Window style

                       // Size and position
                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                       parent,       // Parent window
                       (HMENU)id,       // Menu or control id
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
    widget(
        HWND parent,
        children_t& children,
        char* window_class = "windex",
        unsigned long style = WS_CHILD,
        unsigned long exstyle = 0 )
        : myParent( parent )
    {
        myID = NewID();
        Create( window_class, parent, style, exstyle, myID );
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

            case WM_COMMAND:
                std::cout << "command\n";
                return true;
            }
        }
        return false;
    }


    void show()
    {
        ShowWindow(myHandle, SW_SHOWDEFAULT);
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
    int id()
    {
        return myID;
    }

    virtual void command( WORD cmd )
    {
    }

protected:

    int myID;
    HWND myParent;

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



    int NewID()
    {
        static int lastID = 0;
        lastID++;
        return lastID;
    }
};


/// A top level window
class window : public gui
{
public:
    window()
        : myfApp( false )
    {
        Create( "windex", NULL, WS_OVERLAPPEDWINDOW );

        // Assume that the first window created is the application window
        // quit the application of it is destroyed
        static bool appdone = false;
        if( ! appdone )
        {
            myfApp = true;
            appdone = true;
        }
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

    children_t& children()
    {
        return myWidget;
    }

    widget* find( int id )
    {
        for( auto w : myWidget )
        {
            if ( w->id() == id )
                return w;
        }
        return nullptr;
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

            case WM_COMMAND:
                //std::cout << "window <- command "<< LOWORD(wParam) <<" " << HIWORD(wParam)<< "\n";

                // send notification to widget with ID
                auto w = find( LOWORD(wParam) );
                if( w )
                    w->command( HIWORD( wParam ));
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
        : widget( parent, children, "windex" )
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
/// User can enter a string
class editbox : public widget
{
public:
    editbox( HWND parent, children_t& children )
        : widget( parent, children, "Edit",
                  WS_CHILD | ES_LEFT | WS_BORDER | WS_VISIBLE,
                  WS_EX_CLIENTEDGE )
    {

    }
    /// editbox generated a notification - nop
    void notification( WORD ntf )
    {
        //std::cout << "editbox notification " << ntf << "\n";
        if( ntf == EN_KILLFOCUS )
        {
            //std::cout << "done\n";
        }
    }

    // change text in textbox
    void text( const std::string& t )
    {
        SetDlgItemText(
                       myParent,
                       myID,
                       t.c_str() );
    }

    // get text in textbox
    std::string text()
    {
        char buf[1000];
        GetDlgItemText(
            myParent,
            myID,
            buf,
            999
        );
        return std::string( buf );
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
        return *w;
    }

    /** get reference to new widget of type T
        @param[in] parent reference to parent window
    */
    template <class T>
    T& make( window& parent )
    {
        T* w = new T( parent.handle(), parent.children() );
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
