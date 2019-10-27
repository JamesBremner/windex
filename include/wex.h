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
typedef std::vector< gui* > children_t;

void run()
{
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/** Functions to be called when event occurs

*/
class eventhandler
{
public:
    eventhandler()
        : myfClickPropogate( false )
    {
        // initialize functions with no-ops
        click([] {});
        draw([](PAINTSTRUCT& ps) {});
        resize([](int w, int h) {});
        scrollH([](int c) {});
        scrollV([](int c) {});
    }
    bool onLeftdown()
    {
        myClickFunction();
        return ! myfClickPropogate;
    }
    void onDraw( PAINTSTRUCT& ps )
    {
        myDrawFunction( ps );
    }
    void onResize( int w, int h )
    {
        myResizeFunction( w, h );
    }
    void onScrollH( int code )
    {
        myScrollHFunction( code );
    }
    void onScrollV( int code )
    {
        myScrollVFunction( code );
    }
    /////////////////////////// register event handlers /////////////////////

    /** register click event handler
        @param[in] f the function to call when user clicks on gui in order to process event
        @param[in] propogate specify that event should propogate to parent window after processing, default is false
    */
    void click(
        std::function<void(void)> f,
        bool propogate = false )
    {
        myClickFunction = f;
        myfClickPropogate = propogate;
    }
    /// specify that event should propogate to parent window after currently registered click event handler runs
    void clickPropogate( bool f = true)
    {
        myfClickPropogate = f;
    }

    void draw( std::function<void(PAINTSTRUCT& ps)> f )
    {
        myDrawFunction = f;
    }
    void resize( std::function<void(int w, int h)> f )
    {
        myResizeFunction = f;
    }
    void scrollH( std::function<void(int code)> f )
    {
        myScrollHFunction = f;
    }
    void scrollV( std::function<void(int code)> f )
    {
        myScrollVFunction = f;
    }
private:
    bool myfClickPropogate;
    std::function<void(void)> myClickFunction;
    std::function<void(PAINTSTRUCT& ps)> myDrawFunction;
    std::function<void(int w, int h)> myResizeFunction;
    std::function<void(int code)> myScrollHFunction;
    std::function<void(int code)> myScrollVFunction;

};

class shapes
{
public:
    shapes( PAINTSTRUCT& ps )
        : myHDC( ps.hdc )
        , myPenThick( 1 )
    {
        hPen = CreatePen(
                   PS_SOLID,
                   myPenThick,
                   RGB(0,0,0));
        hPenOld =  SelectObject(myHDC, hPen);

    }
    ~shapes()
    {
        HGDIOBJ pen = SelectObject(myHDC, hPenOld);
        DeleteObject( pen );
    }
    /** Set color for drawings
    @param[in] color
    */
    void color( int r, int g, int b )
    {
        hPen = CreatePen(
                   PS_SOLID,
                   myPenThick,
                   RGB(r,g,b));
        HGDIOBJ pen = SelectObject(myHDC, hPen);
        DeleteObject( pen );
    }
    void penThick( int t )
    {
        myPenThick = t;
        color( 0, 0, 0 );
    }
    void line( const std::vector<int>& v )
    {
        MoveToEx(
            myHDC,
            v[0],
            v[1],
            NULL
        );
        LineTo(
            myHDC,
            v[2],
            v[3] );
    }
    void rectangle( const std::vector<int>& v )
    {
        MoveToEx(
            myHDC,
            v[0],
            v[1],
            NULL
        );
        LineTo(
            myHDC,
            v[0]+v[2],
            v[1] );
        LineTo(
            myHDC,
            v[0]+v[2],
            v[1]+v[3] );
        LineTo(
            myHDC,
            v[0],
            v[1]+v[3] );
        LineTo(
            myHDC,
            v[0],
            v[1] );
    }
    void text(
        const std::string& t,
        RECT& r )
    {
        DrawText(
            myHDC,
            t.c_str(),
            -1,
            &r,
            DT_NOCLIP);
    }
private:
    HDC myHDC;
    int myPenThick;
    HGDIOBJ hPen;
    HGDIOBJ hPenOld;
};

/// Base class for all gui elements
class gui
{
public:
    /// Construct top level with no parent
    gui()
        : myParent( NULL )
        , myBGColor( 0xC8C8C8 )
        , myBGBrush( CreateSolidBrush( myBGColor ))
        , myDeleteList( 0 )
        , myfModal( false )
    {
        myID = NewID();
        Create(NULL,"windex",WS_OVERLAPPEDWINDOW);
    }
    /// Construct child of a parent
    gui(
        gui* parent,
        const char* window_class = "windex",
        unsigned long style = WS_CHILD,
        unsigned long exstyle = 0 )
        : myParent( parent )
    {
        myID = NewID();
        Create( parent->handle(), window_class, style, exstyle, myID );
        parent->child( this );
        text("???"+std::to_string(myID));
    }
    virtual ~gui()
    {
        myDeleteList->push_back( myHandle );
    }

    // register child on this window
    void child( gui* w )
    {
        myChild.push_back( w );
    }
    children_t& children()
    {
        return myChild;
    }

    gui* find( int id )
    {
        for( auto w : myChild )
        {
            if ( w->id() == id )
                return (gui*)w;
        }
        return nullptr;
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
    int id()
    {
        return myID;
    }
    int bgcolor() const
    {
        return myBGColor;
    }
    void text( const std::string& text )
    {
        myText = text;
        SetWindowText( myHandle, text.c_str() );
    }
    std::string text() const
    {
        return myText;
    }
    /// Add scrollbars
    void scroll()
    {
        // Add scrollbars to window style
        SetWindowLongPtr(
            myHandle,
            GWL_STYLE,
            GetWindowLongPtr( myHandle, GWL_STYLE) | WS_HSCROLL | WS_VSCROLL );

        // Set the scrolling range and page size to defaaults
        scrollRange( 100, 100 );

        // horizontal scroll handler
        events().scrollH([this](int code)
        {
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask  = SIF_POS | SIF_TRACKPOS | SIF_PAGE;
            if( ! GetScrollInfo (myHandle, SB_HORZ, &si) )
                return;

            int oldPos = scrollMove( si, code );

            si.fMask = SIF_POS;
            SetScrollInfo (myHandle, SB_HORZ, &si, TRUE);
            GetScrollInfo (myHandle, SB_CTL, &si);

            RECT rect;
            GetClientRect(myHandle, &rect);
            int xs = oldPos - si.nPos;
            //std::cout << "scrollH " << xs <<" "<< oldPos <<" "<< si.nPos << "\n";
            ScrollWindow(
                myHandle,
                xs,
                0, NULL, NULL);

            for( auto& w : myChild )
                w->update();
        });

        // vertical scroll handler
        events().scrollV([this](int code)
        {
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask  = SIF_POS | SIF_TRACKPOS | SIF_PAGE;
            if( ! GetScrollInfo (myHandle, SB_VERT, &si) )
                return;

            int oldPos = scrollMove( si, code );

            si.fMask = SIF_POS;
            SetScrollInfo (myHandle, SB_VERT, &si, TRUE);
            GetScrollInfo (myHandle, SB_VERT, &si);
            RECT rect;
            GetClientRect(myHandle, &rect);
            int ys = oldPos - si.nPos;
            //std::cout << "scroll " << ys <<" "<< oldPos <<" "<< si.nPos << "\n";
            ScrollWindow(
                myHandle,
                0,
                ys, NULL, NULL);

            for( auto& w : myChild )
                w->update();
        });

    }
    /** Set the scrolling range
        @param[in] width of the underlying window to be scrolled over
        @param[in] height of the underlying window to be scrolled over

        This sets how far scrolling can move horixonatally and vertically.
        The width and height should be set to the maximum locations
        to be shown when scrolled to the limits.

        This should be called again if the window size changes.
    */
    void scrollRange( int width, int height )
    {
        RECT r;
        GetClientRect(myHandle,&r);
        int xmax = width - r.right;
        if( xmax < 0 )
            xmax = 0;
        int ymax = height - r.bottom;
        if( ymax < 0 )
            ymax = 0;
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask  = SIF_RANGE | SIF_PAGE;
        si.nMin   = 0;
        si.nMax   = ymax;
        si.nPage  = ymax/10;
        SetScrollInfo(myHandle, SB_VERT, &si, TRUE);
        si.nMax   = xmax;
        si.nPage  = xmax/10;
        SetScrollInfo(myHandle, SB_HORZ, &si, TRUE);
    }

    virtual bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //std::cout << " widget " << myText << " WindowMessageHandler " << uMsg << "\n";
        if( hwnd == myHandle )
        {
            switch (uMsg)
            {
            case WM_DESTROY:
                // if this is the appliction window
                // then quit application when destroyed
                if( myID == 1 )
                    PostQuitMessage(0);
                return true;
                return true;

            case WM_NCDESTROY:
                // all the children are gone
                // so a modal display can close
                myfModal = false;
                return false;

            case WM_ERASEBKGND:
            {
//                if( ! myParent )
//                {
                RECT rc;
                GetWindowRect(hwnd, &rc);
                FillRect((HDC)wParam, &rc, myBGBrush );
                return true;
//               }
//                RECT rc;
//                GetWindowRect(hwnd, &rc);
//                GetClientRect(hwnd,&rc );
//                //FillRect((HDC)wParam, &rc, myBGBrush );
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
                if( myEvents.onLeftdown() )
                    return true;
                // the event was not completely handled, maybe the parent can look after it
                if( myParent )
                {
                    if( myParent->WindowMessageHandler(
                                myParent->handle(),
                                uMsg, wParam, lParam ))
                        return true;
                }
                break;

            case WM_SIZE:
                if( wParam == SIZE_RESTORED)
                {
                    myEvents.onResize( LOWORD(lParam), HIWORD(lParam) );
                    return true;
                }
                return false;

            case WM_HSCROLL:
                myEvents.onScrollH( LOWORD (wParam) );
                return true;
            case WM_VSCROLL:
                myEvents.onScrollV( LOWORD (wParam) );
                return true;

            case WM_COMMAND:
                std::cout << "command " << wParam << "\n";
                MenuFunctionRun( wParam );
                return true;
            }

        }
        else
        {
            for( auto w : myChild )
            {
                if( w->WindowMessageHandler( hwnd, uMsg, wParam, lParam ))
                    return true;
            }
        }

        return false;
    }

    virtual void show( bool f = true )
    {
        int cmd = SW_SHOWDEFAULT;
        if( ! f )
            cmd = SW_HIDE;
        //std::cout << "show " << myText <<" "<< myHandle <<" "<< myChild.size() << "\n"; ;
        ShowWindow(myHandle, cmd);
        // display any children
        for( auto w : myChild )
            w->show( f );
    }

    void showModal()
    {
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

    /// force widget to redraw completely
    void update()
    {
        InvalidateRect(myHandle,NULL,true);
        UpdateWindow(myHandle);
    }

    /** Move the window
        @param[in] r specify location and size
        if r contains 4 values, then { x, y, width, height }
        if r contains 2 values, then if move true ( x, y } keep current size
        if r contains 2 values, then if move false ( width height } keep current location
    */
    void move( const std::vector<int>& r, bool move = true )
    {
        switch( r.size() )
        {
        case 4:
            MoveWindow( myHandle,
                        r[0],r[1],r[2],r[3],false);
            break;
        case 2:
        {
            RECT rect;
            GetClientRect( myHandle, &rect );
            if( move )
                MoveWindow( myHandle,
                            r[0],r[1],rect.right-rect.left,rect.bottom-rect.top,
                            false );
            else
                MoveWindow( myHandle,
                            rect.left, rect.top, r[0], r[1],
                            false );
            break;
        }
        default:
            throw std::runtime_error( "windex move bad parameter" );
        }
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
    void MenuFunctionInsert(
        int id,
        std::function<void(void)> f
    )
    {
        myMapMenuFunction.insert( std::make_pair( id, f ));
    }
    void MenuFunctionRun( int id )
    {
        auto fp = myMapMenuFunction.find( id );
        if( fp != myMapMenuFunction.end() )
        {
            fp->second();
        }
    }

protected:
    HWND myHandle;
    gui* myParent;
    eventhandler myEvents;
    int myBGColor;
    HBRUSH myBGBrush;
    std::vector< HWND >* myDeleteList;
    std::string myText;
    int myID;
    std::vector< gui* > myChild;            ///< gui elements to be displayed in this window
    bool myfModal;                          ///< true if element is being shown as modal
    std::map< int, std::function<void(void)> > myMapMenuFunction;

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
    virtual void draw( PAINTSTRUCT& ps )
    {
        SetBkColor(
            ps.hdc,
            myBGColor );
        if( myParent )
        {
            RECT r( ps.rcPaint );
            r.left += 1;
            r.top  += 1;
            DrawText(
                ps.hdc,
                myText.c_str(),
                -1,
                &r,
                0);
        }
        myEvents.onDraw( ps );
    }

    /** Create new, unique ID for gui element

    The first ID will be 1, and is assumed to be the application window
    which causes the application to quit when destroyed
    */
    int NewID()
    {
        static int lastID = 0;
        lastID++;
        return lastID;
    }
    int scrollMove( SCROLLINFO& si, int code )
    {
        int oldPos = si.nPos;
        switch( code )
        {
        // User clicked the left arrow.
        case SB_LINELEFT:
            si.nPos -= 1;
            break;

        // User clicked the right arrow.
        case SB_LINERIGHT:
            si.nPos += 1;
            break;

        // User clicked the scroll bar shaft left of the scroll box.
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;

        // User clicked the scroll bar shaft right of the scroll box.
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;

        // User dragged the scroll box.
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        }
        return oldPos;
    }

};


/// A child window that can contain widgets
class panel : public gui
{
public:
    panel( gui* parent )
        : gui( parent )
    {
        text("");
    }
};

/// A panel displaying a title and box around contents
class groupbox : public panel
{
public:
    groupbox( gui* parent )
        : panel( parent )
    {

    }
    virtual void draw( PAINTSTRUCT& ps )
    {
        gui::draw( ps );
        DrawEdge(
            ps.hdc,
            &ps.rcPaint,
            EDGE_BUMP,
            BF_RECT
        );
        RECT r { 0, 0, 50, 25 };
        DrawText(
            ps.hdc,
            myText.c_str(),
            myText.length(),
            &r,
            0
        );
    }
};

class layout : public panel
{
public:
    layout( gui* parent )
        : panel( parent )
    {

    }
    void grid( int cols )
    {
        myColCount = cols;
    }
    void show( bool f = true )
    {
        ShowWindow(myHandle,  SW_SHOWDEFAULT);

        RECT r;
        GetClientRect(myHandle, &r );
        int colwidth = (r.right - r.left) / myColCount;
        int rowheight = ( r.bottom - r.top ) / ( ( myChild.size() + 1 ) / myColCount );

        // display the children laid out in a grid
        int colcount = 0;
        int rowcount = 0;
        for( auto w : myChild )
        {
            w->move( { colcount*colwidth, rowcount*rowheight } );
            w->show();

            colcount++;
            if( colcount >= myColCount )
            {
                colcount = 0;
                rowcount++;
            }
        }
    }
private:
    int myColCount;
};

/// A button
class button : public gui
{
public:
    button( gui* parent )
        : gui( parent )
    {

    }
protected:
    /// draw - label inside rectangle
    virtual void draw( PAINTSTRUCT& ps )
    {
        gui::draw( ps );
        DrawEdge(
            ps.hdc,
            &ps.rcPaint,
            EDGE_RAISED,
            BF_RECT
        );
    }
};

class radiobutton : public gui
{
public:
    radiobutton( gui* parent )
        : gui( parent, "button",
               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON )
    {
    }
    /** Make first radiobutton in a group

    Clicking any radiobutton in a group will set that button
    and clear the other buttons in the group
    without effecting buttons in other groups.

    This must be called for the first radiobutton of the first group
    if there will be more than one group.

    This must be called before constructing the other buttons in the group.
    */
    void first()
    {
        SetWindowLongPtr(
            myHandle,
            GWL_STYLE,
            (LONG_PTR)WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP );
    }

    bool isChecked()
    {
        return ( IsDlgButtonChecked(
                     myParent->handle(),
                     myID ) == BST_CHECKED );
    }
};
/** User can toggle true/false value by clicking

This draws a custom checkbox that expands with the height of the widget ( set by move() )
( The native checkbox is very small and its size cannot be changed )

*/
class checkbox : public gui
{
    enum class eType
    {
        check,
        plus
    } myType;
public:
    checkbox( gui* parent )
        : gui( parent )
        , myType( eType::check )
        , myValue( false )
    {
        // toggle the boolean value when clicked
        events().click([this]
        {
            myValue = ! myValue;
            update();
        });
    }
    /// set type to plus, useful to indicate expanded or collapsed property categories
    void plus( bool f = true )
    {
        if( f )
            myType = eType::plus;
        else
            myType = eType::check;
    }
    void check( bool f = true )
    {
        myValue = f;
    }
    bool isChecked()
    {
        return ( myValue );
    }
    virtual void draw( PAINTSTRUCT& ps )
    {
        SetBkColor(
            ps.hdc,
            myBGColor );
        RECT r( ps.rcPaint );
        int cbg = r.bottom-r.top-2;
        r.left += cbg+2;
        r.top  += 1;
        DrawText(
            ps.hdc,
            myText.c_str(),
            -1,
            &r,
            0);
        shapes S( ps );
        S.rectangle( { 0,0, cbg, cbg} );
        S.penThick( 3 );
        switch( myType )
        {
        case eType::check:
            if( myValue )
            {
                S.line( {2,cbg/2,cbg/2-2,cbg-2} );
                S.line( {cbg/2,cbg-4,cbg-4,4} );
            }
            break;
        case eType::plus:
            S.line( { 2,cbg/2, cbg-2,cbg/2} );
            if( myValue )
                S.line( { cbg/2,2,cbg/2,cbg-2} );
            break;
        }
        S.penThick( 1 );
    }
    void clickFunction( std::function<void(void)> f )
    {
        myClickFunction = f;
    }
private:
    bool myValue;
    std::function<void(void)> myClickFunction;
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
class label : public gui
{
public:
    label( gui* parent )
        : gui( parent )
    {

    }
};
/// User can enter a string
class editbox : public gui
{
public:
    editbox( gui* parent )
        : gui( parent, "Edit",
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
            myParent->handle(),
            myID,
            t.c_str() );
    }

    // get text in textbox
    std::string text()
    {
        char buf[1000];
        buf[0] = '\0';
        GetDlgItemText(
            myParent->handle(),
            myID,
            buf,
            999
        );
        return std::string( buf );
    }
};

class choice : public gui
{
public:
    choice( gui* parent )
        : gui( parent, "Combobox",
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
    gui& MakeWindow()
    {
        gui* w = new gui();
        Add( w );
        return *w;
    }

    /** get reference to new widget or window of type T
        @param[in] parent reference to parent window or widget
    */
    template <class T, class W>
    T& make( W& parent )
    {
        T* w = new T( (gui*)&parent );

        // inherit background color from parent
        w->bgcolor( parent.bgcolor() );

        Add( w );
        return *w;
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

class filebox
{
public:
    filebox( gui& parent )
    {
        OPENFILENAME ofn;       // common dialog box structure
        char szFile[260];       // buffer for file name

// Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = parent.handle();
        ofn.lpstrFile = szFile;
// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
// use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

// Display the Open dialog box.

        if (GetOpenFileName(&ofn)==TRUE)
        {
            myfname = ofn.lpstrFile;
        }
    }
    std::string path() const
    {
        return myfname;
    }
private:
    std::string myfname;
};

class menu
{
public:
    menu( gui& parent )
        : myM( CreatePopupMenu() )
        , myParent( parent )
    {

    }
    /** Append menu item
        @param[in] title
        @param[in] f function to be run when menu item clicked
    */
    void append(
        const std::string& title,
        const std::function<void(void)>& f = []{})
    {
        // calculate unique id for menu item
        int itemID = NewID();

        // add item to menu
        AppendMenu(
            myM,
            0,
            itemID,
            title.c_str());

        // store function to run when menu item clicked in popup
        myf.push_back( f );

        // store function to run when menu item click in menubar
        myParent.MenuFunctionInsert( itemID, f );
    }
    void popup(
        int x, int y
    )
    {
        // display menu
        int i = TrackPopupMenu(
                    myM,
                    TPM_RETURNCMD,
                    x, y,
                    0,
                    myParent.handle(),
                    NULL    );
        // if user clicked item, execute associated function
        if( i )
            myf[i-1]();
    }
    HMENU handle()
    {
        return myM;
    }
private:
    HMENU myM;
    std::vector< std::function<void(void)> > myf;
    gui& myParent;

    int NewID()
    {
        static int lastID = 0;
        lastID++;
        return lastID;
    }
};

class menubar
{
public:
    menubar( gui& parent )
        : myParent( parent )
        , myM( CreateMenu() )
    {
        // attach menu to window
        SetMenu( parent.handle(), myM );
    }
    /** Append menu to menubar
        @param[in] title
        @param[in] m menu that drops down when title clicked
    */
    void append(
        const std::string& title,
        menu& m )
    {
        AppendMenu(
            myM,
            MF_POPUP,
            (UINT_PTR)m.handle(),
            title.c_str());
        DrawMenuBar( myParent.handle() );
    }
private:
    gui& myParent;
    HMENU myM;
};

/// Construct a top level window ( first call constructs application window )
gui & topWindow()
{
    return windex::get().MakeWindow();
}

/** Construct widget
        @param[in] parent reference to parent window or widget
*/
template <class T, class W>
T& make( W& parent )
{
    return windex::get().make<T>(parent);
}

}

