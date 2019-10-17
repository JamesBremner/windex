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

void MessageLoop()
{
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

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
        : myBGColor( 0xC8C8C8 )
        , myBGBrush( CreateSolidBrush( myBGColor ))
        , myDeleteList( 0 )
    {
    }
    virtual ~gui()
    {
        myDeleteList->push_back( myHandle );
    }

    /** Change background color
    @param[in] color eg 0x0000FF
    */
    void bgcolor( int color )
    {
        myBGColor = color;
        DeleteObject( myBGBrush);
        myBGBrush = CreateSolidBrush( color );
    }
    int bgcolor() const
    {
        return myBGColor;
    }
    void text( const std::string& text )
    {
        myText = text;
    }
    std::string text()
    {
        return myText;
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
    int myBGColor;
    HBRUSH myBGBrush;
    std::vector< HWND >* myDeleteList;
    std::string myText;

    /** Create the managed window
        @param[in] parent handle of parent window
        @param[in] window_class controls which callback function handles window messages
        @param[in] style
        @param[in] exstyle
        @param[in] id identifies which control generated notification
     */
    void Create(
        HWND parent,
        const char* window_class,
        DWORD style, DWORD exstyle = 0,
        int id=0 )
    {
        myHandle = CreateWindowEx(
                       exstyle,          // Optional window styles.
                       window_class,     // Window class
                       "widget",         // Window text
                       style,            // Window style

                       // Size and position
                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                       parent,       // Parent window
                       reinterpret_cast<HMENU>( id ),       // Menu or control id
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
        const char* window_class = "windex",
        unsigned long style = WS_CHILD,
        unsigned long exstyle = 0 )
        : myParent( parent )
    {
        myID = NewID();
        Create( parent, window_class, style, exstyle, myID );
        children.push_back( this );
        text("not set");
    }

    /// Handle windows messages
    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //std::cout << " widget " << myText << " WindowMessageHandler " << uMsg << "\n";
        if( hwnd == myHandle )
        {
            switch (uMsg)
            {
            case WM_DESTROY:
                return true;

            case WM_ERASEBKGND:
            {
                RECT rc;
                GetWindowRect(hwnd, &rc);
                GetClientRect(hwnd,&rc );
                //FillRect((HDC)wParam, &rc, myBGBrush );
                return true;
            }

            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(myHandle, &ps);
                FillRect(ps.hdc, &ps.rcPaint, myBGBrush );
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
        SetBkColor(
            ps.hdc,
            myBGColor );
        DrawText(
            ps.hdc,
            myText.c_str(),
            -1,
            &ps.rcPaint,
            0);
    }

private:

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
        , myfModal( false )
    {
        Create( NULL, "windex", WS_OVERLAPPEDWINDOW );

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

    void showModal()
    {
        //std::cout << "modal " << myHandle << "\n";
        myfModal = true;
        show();
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if( ! myfModal )
                break;
        }
    }

    void text( const std::string& txt )
    {
        gui::text( txt );
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
//              << myText <<" "<< myHandle <<" "<< uMsg << "\n";
        if( hwnd == myHandle )
        {
            switch (uMsg)
            {
            case WM_DESTROY:
                if( myfApp )
                    PostQuitMessage(0);
                return true;

            case WM_NCDESTROY:
                myfModal = false;
                return false;

            case WM_ERASEBKGND:
            {
                RECT rc;
                GetWindowRect(hwnd, &rc);
                FillRect((HDC)wParam, &rc, myBGBrush );
                return true;
            }

            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(myHandle, &ps);

                FillRect(hdc, &ps.rcPaint, myBGBrush );
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
    bool myfModal;

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
        std::cout << "editbox notification " << ntf << "\n";
        if( ntf == EN_KILLFOCUS )
        {
            std::cout << "done\n";
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
        buf[0] = '\0';
        GetDlgItemText(
            myParent,
            myID,
            buf,
            999
        );
        return std::string( buf );
    }
};

class combobox : public widget
{
public:
    combobox( HWND parent, children_t& children )
        : widget( parent, children, "Combobox",
                  CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE )
    {
    }
    void add( const std::string& s )
    {
        SendMessageA(
            handle(),
            (UINT) CB_ADDSTRING,
            (WPARAM) 0,
            (LPARAM) s.c_str());
    }
    int SelectedIndex()
    {
        return SendMessage(
                   handle(),
                   (UINT) CB_GETCURSEL,
                   (WPARAM) 0, (LPARAM) 0);
    }
    std::string SelectedText()
    {
        int i = SelectedIndex();
        if( i < 0 )
            return std::string("");
        char buf[256];
        SendMessage(
            handle(),
            (UINT) CB_GETLBTEXT,
            (WPARAM) i,
            (LPARAM) buf);
        return std::string( buf );
    }
    int count()
    {
        return SendMessage(
                   handle(),
                   (UINT)CB_GETCOUNT,
                   (WPARAM) 0, (LPARAM) 0);
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

        // inherit background color from parent
        w->bgcolor( parent.bgcolor() );

        Add( w );
        return *w;
    }

    /// poll the windows message queue
    void exec()
    {
        MessageLoop();
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
        wc.hbrBackground = CreateSolidBrush(0xc8c8c8);
        RegisterClass(&wc);
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
