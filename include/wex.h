#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <windows.h>
#include <CommCtrl.h>

namespace wex
{

class gui;

typedef std::map< HWND, gui* > mgui_t;
typedef std::vector< gui* > children_t;

/// A structure containing the mouse status for event handlers
struct  sMouse
{
    int x;
    int y;
    bool left;
    bool right;
    bool shift;
};


/// A class where application code can register functions to be called when an event occurs
class eventhandler
{
public:
    eventhandler()
        : myfClickPropogate( false )
    {
        // initialize functions with no-ops
        click([] {});
        clickWex([] {});
        draw([](PAINTSTRUCT& ps) {});
        resize([](int w, int h) {});
        scrollH([](int c) {});
        scrollV([](int c) {});
        mouseMove([](sMouse& m) {});
        mouseWheel([](int dist) {});
        mouseUp([] {});
        timer([] {});
        slid([](int pos) {});
    }
    bool onLeftdown()
    {
        myClickFunWex();
        myClickFunctionApp();
        return ! myfClickPropogate;
    }
    void onMouseUp()
    {
        myMouseUpFunction();
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
    void onMenuCommand( int id )
    {
        auto fp = myMapMenuFunction.find( id );
        if( fp != myMapMenuFunction.end() )
        {
            fp->second();
        }
    }
    void onMouseMove( WPARAM wParam, LPARAM lParam )
    {
        sMouse m;

//        m.x = GET_X_LPARAM(lParam);
//        m.y = GET_Y_LPARAM(lParam);
        m.x = LOWORD(lParam);
        m.y = HIWORD(lParam);
        m.left = ( wParam == MK_LBUTTON );

        myMouseMoveFunction( m );
    }
    void onMouseWheel( int dist )
    {
        myMouseWheelFunction( dist );
    }
    void onTimer()
    {
        myTimerFunction();
    }
    bool onSelect(
        unsigned short id )
    {
        auto it = mapControlFunction().find( std::make_pair(id,CBN_SELCHANGE));
        if( it == mapControlFunction().end() )
            return true;
        it->second();
        return true;
    }
    void onSlid(unsigned short id )
    {
        mySlidFunction( (int) id );
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
        myClickFunctionApp = f;
        myfClickPropogate = propogate;
    }
    void clickWex( std::function<void(void)> f )
    {
        myClickFunWex = f;
    }
    /// specify that click event should propogate to parent window after currently registered click event handler runs
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
    /** Register function to run when menu item clicked
        @param[in] id
        @param[in] f function to run when menu item with id is clicked
    */
    void menuCommand(
        int id,
        std::function<void(void)> f )
    {
        myMapMenuFunction.insert( std::make_pair( id, f ));
    }
    void select(
        int id,
        std::function<void(void)> f )
    {
        mapControlFunction().insert(
            std::make_pair( std::make_pair( id, CBN_SELCHANGE), f ));
    }
    void mouseMove( std::function<void(sMouse& m)> f )
    {
        myMouseMoveFunction = f;
    }
    void mouseWheel( std::function<void(int dist)> f )
    {
        myMouseWheelFunction = f;
    }
    void mouseUp( std::function<void(void)> f )
    {
        myMouseUpFunction = f;
    }
    void timer( std::function<void(void)> f )
    {
        myTimerFunction = f;
    }
    void slid( std::function<void(int pos)> f )
    {
        mySlidFunction = f;
    }
private:
    bool myfClickPropogate;

    // event handlers registered by application code
    std::function<void(void)> myClickFunctionApp;
    std::function<void(PAINTSTRUCT& ps)> myDrawFunction;
    std::function<void(int w, int h)> myResizeFunction;
    std::function<void(int code)> myScrollHFunction;
    std::function<void(int code)> myScrollVFunction;
    std::map< int, std::function<void(void)> > myMapMenuFunction;
    std::function<void(sMouse& m)> myMouseMoveFunction;
    std::function<void(int dist)> myMouseWheelFunction;
    std::function<void(void)> myTimerFunction;
    std::function<void(void)> myMouseUpFunction;
    std::function<void(int pos)> mySlidFunction;

    // event handlers registered by windex class
    std::function<void(void)> myClickFunWex;

    /// reference to application wide list of registered event handlers
    /// mapped to control ids and notification code
    std::map< std::pair<int,unsigned short>, std::function<void(void)> >&
    mapControlFunction()
    {
        static std::map< std::pair<int,unsigned short>, std::function<void(void)> >
        myMapControlFunction;
        return myMapControlFunction;
    }
};

/** @brief A class that offers application code methods to draw on a window.

<pre>
    // construct top level  window
    gui& form = wex::windex::topWindow();
    form.move({ 50,50,400,400});
    form.text("A windex draw demo");

    form.events().draw([]( PAINTSTRUCT& ps )
    {
        shapes S( ps );
        S.color( 255, 0, 0 );
        S.line( { 10,10, 50,50 } );
        S.color( 255,255,255 );
        S.rectangle( { 20,20,20,20});
        S.color( 255,255,0 );
        S.text( "test", {50,50,50,25} );
        S.color(0,0,255);
        S.circle( 100,100,40);
        S.arc( 100,100,30, 0, 90 );
    });

    form.show();
</pre>
*/
class shapes
{
public:
    shapes( PAINTSTRUCT& ps )
        : myHDC( ps.hdc )
        , myPenThick( 1 )
        , myFill( false )
    {
        hPen = CreatePen(
                   PS_SOLID,
                   myPenThick,
                   RGB(0,0,0));
        hPenOld =  SelectObject(myHDC, hPen);

        myLogfont = {0};
        HANDLE hFont;
        ZeroMemory(&myLogfont, sizeof(LOGFONT));
        myLogfont.lfWeight = FW_NORMAL;
        strcpy(myLogfont.lfFaceName, "Tahoma");
        myLogfont.lfHeight = 20;
        hFont = CreateFontIndirect (&myLogfont);
        hFont = (HFONT)SelectObject (myHDC, hFont);
        DeleteObject( hFont );

    }
    ~shapes()
    {
        HGDIOBJ pen = SelectObject(myHDC, hPenOld);
        DeleteObject( pen );
    }
    /** Set color for drawings
    @param[in] r red 0-255
    @param[in] g green 0-255
    @param[in] b blue 0-255
    */
    void color( int r, int g, int b )
    {
        color( RGB(r,g,b) );
    }
    void color( int c )
    {
        myColor = c;
        hPen = CreatePen(
                   PS_SOLID,
                   myPenThick,
                   c);
        HGDIOBJ old = SelectObject(myHDC, hPen);
        DeleteObject( old );
        SetTextColor( myHDC,  c);
        HBRUSH brush = CreateSolidBrush( c );
        old = SelectObject(myHDC, brush );
        DeleteObject( old );
    }
    /// set background color
    void bgcolor( int c )
    {
        SetBkColor(
            myHDC,
            c );
    }
    /// enable/disable transparent background
    void transparent( bool f = true )
    {
        SetBkMode(
            myHDC,
            TRANSPARENT);
    }

    /// Set pen thickness in pixels
    void penThick( int t )
    {
        myPenThick = t;
    }
    /// Set filling option
    void fill( bool f = true )
    {
        myFill = f;
    }
    /// Color a pixel
    void pixel( int x, int y )
    {
        SetPixel( myHDC, x, y, myColor );
    }
    /** Draw line between two points
        @param[in] v vector with x1, y1, x2, y2
    */
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
    /** Draw rectangle
        @param[in] v vector with left, top, width height
    */
    void rectangle( const std::vector<int>& v )
    {
        if( ! myFill )
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
        else
        {
            Rectangle(
                myHDC,
                v[0], v[1], v[0]+v[2], v[1]+v[3] );
        }
    }
    /** Draw Arc of circle

    @param[in] x for center, pixels 0 at left of window
    @param[in] y for center, pixels 0 at top of window
    @param[in] r radius, pixels
    @param[in] sa start angle degrees anti-clockwise from 3 o'clock
    @param[in] se end angle degrees anti-clockwise from 3 o'clock
    */
    void arc(
        int x, int y, double r,
        double sa, double ea )
    {
        int xl =round( x-r );
        int yt =round( y-r );
        int xr =round( x+r );
        int yb =round( y+r );
        int xs =round( x + r * cos(sa * M_PI/180) );
        int ys =round( y + r * sin(sa * M_PI/180) );
        int xe =round( x + r * cos(ea * M_PI/180) );
        int ye =round( y + r * sin(ea * M_PI/180) );
        Arc(
            myHDC,
            xl,yt,xr,yb,xs,ys,xe,ye );
    }
    /** Draw circle
    @param[in] x0 x for center, pixels 0 at left of window
    @param[in] y0 y for center, pixels 0 at left of window
    @param[in] r radius, pixels
    */
    void circle( int x0, int y0, double r )
    {
        arc( x0, y0, r, 0, 0 );
    }
    /** Draw text.
    @param[in] t the text
    @param[in] v vector of left, top, width, height
    */
    void text(
        const std::string& t,
        const std::vector<int>& v )
    {
        RECT rc;
        rc.left = v[0];
        rc.top  = v[1];
        rc.right = v[0]+ v[2];
        rc.bottom = v[1]+v[3];
        DrawText(
            myHDC,
            t.c_str(),
            -1,
            &rc,
            0 );
    }
    /// Enable / disable drawing text in vertical orientation
    void textVertical( bool f = true )
    {
        if( f )
            myLogfont.lfEscapement = 900;
        else
            myLogfont.lfEscapement = 0;
        HANDLE hFont = CreateFontIndirect (&myLogfont);
        hFont = (HFONT)SelectObject (myHDC, hFont);
        DeleteObject( hFont );
    }

    /// Set text height
    void textHeight( int h )
    {
        myLogfont.lfHeight = h;
        HANDLE hFont = CreateFontIndirect (&myLogfont);
        hFont = (HFONT)SelectObject (myHDC, hFont);
        DeleteObject( hFont );
    }

private:
    HDC myHDC;
    int myPenThick;
    HGDIOBJ hPen;
    HGDIOBJ hPenOld;
    bool myFill;
    LOGFONT myLogfont;
    int myColor;
};

/// The base class for all windex gui elements
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

        /*  default resize event handler
            simply forces a refresh so partially visible widgets are correctly drawn
            Application code, if it needs to move child windows around,
            should overwrite this event handler.  Remember to call update() at end of event handler.
        */
        events().resize( [this](int w, int h)
        {
            update();
        });
    }
/// Construct child of a parent
    gui(
        gui* parent,
        const char* window_class = "windex",
        unsigned long style = WS_CHILD,
        unsigned long exstyle = 0 )
        : myParent( parent )
        , myDeleteList( 0 )
    {
        myID = NewID();
        Create( parent->handle(), window_class, style, exstyle, myID );
        parent->child( this );
        text("???"+std::to_string(myID));
    }
    virtual ~gui()
    {
        std::cout << "deleting " << myText << "\n";
        if( myDeleteList )
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

    /** Change icon
        @param[in] iconfilename

        Use to set the application icon in the taskbar
    */
    void icon( const std::string& iconfilename )
    {
        HICON hIcon = ExtractIconA(
                          NULL,
                          iconfilename.c_str(),
                          0 );
        SetClassLongPtr(
            myHandle,
            GCLP_HICON,
            (LONG_PTR) hIcon );

        SendMessage(myHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(myHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(GetWindow(myHandle, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(GetWindow(myHandle, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
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

    /** Get mouse status
        @return sMouse structure containing x and y positions, etx
    */
    sMouse getMouseStatus()
    {
        sMouse m;
        POINT p;
        GetCursorPos( &p );
        if( ! ScreenToClient( myHandle, &p ) )
        {
            m.x = -1;
            m.y = -1;
        }
        m.x = p.x;
        m.y = p.y;
        m.left = (GetKeyState(VK_LBUTTON) < 0);
        m.right = (GetKeyState(VK_RBUTTON) < 0);
        m.shift = (GetKeyState(VK_SHIFT) < 0);
        return m;
    }
    /** \brief Run the windows message loop

    This waits for events to occur
    and runs the requested event handler when they do.

    It does not return until the application window
    ( that is the first top level window created )
    is closed

    */
    void run()
    {
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    /// Add tooltip that pops up helpfully when mouse cursor hovers ober widget
    void tooltip( const std::string& text )
    {
        // Create the tooltip. g_hInst is the global instance handle.
        HWND hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
                                      WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      myHandle, NULL, NULL, NULL);

        // Associate the tooltip with the tool.
        TOOLINFO toolInfo = { 0 };
        toolInfo.cbSize = sizeof(toolInfo);
        toolInfo.hwnd = myHandle;
        toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        toolInfo.uId = (UINT_PTR)myHandle;
        toolInfo.lpszText = (char*)text.c_str();
        SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
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
            case WM_RBUTTONDOWN:
                //std::cout << "click on " << myText << "\n";
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

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
                myEvents.onMouseUp();
                return true;

            case WM_MOUSEMOVE:
                myEvents.onMouseMove( wParam, lParam );
                break;

            case WM_MOUSEWHEEL:
            {
                int d = HIWORD(wParam);
                if( d > 0xEFFF)
                    d = -120;
                myEvents.onMouseWheel( d );
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
                if( lParam )
                    trackbarMessageHandler( (HWND) lParam );
                else
                    myEvents.onScrollH( LOWORD (wParam) );
                return true;

            case WM_VSCROLL:
                if( lParam )
                    trackbarMessageHandler( (HWND) lParam );
                else
                    myEvents.onScrollV( LOWORD (wParam) );
                return true;



            case WM_COMMAND:
                if( lParam )
                {
                    if( HIWORD(wParam) == CBN_SELCHANGE )
                    {
                        return events().onSelect( LOWORD(wParam) );
                    }
                    return true;
                }
                events().onMenuCommand( wParam );
                return true;

            case WM_TIMER:
                events().onTimer();
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

/// Show window and all children
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

/// Show this window and suspend all other windows inteactions until this is closed
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

    /** force widget to redraw completely

    Windex makes no effort to auto update the screen display.
    If application code alters something and the change should be seen immediatly
    then update() should be called, either on the widget that has changed
    or the top level window that contains the changed widgets.

    */
    void update()
    {
        InvalidateRect(myHandle,NULL,true);
        UpdateWindow(myHandle);
    }

    /** Move the window
        @param[in] r specify location and size
        r contains 4 values, { x, y, width, height }
    */
    void move( const std::vector<int>& r )
    {
        if( r.size() != 4 )
            return;
        MoveWindow( myHandle,
                    r[0],r[1],r[2],r[3],false);
    }
    /** Change size without moving top left corner
        @param[in] w width
        @param[in] h height
    */
    void size( int w, int h )
    {
        RECT rect;
        GetClientRect( myHandle, &rect );
        MoveWindow( myHandle,
                    rect.left, rect.top, w, h,
                    false );
    }
    /** Change position without changing size
        @param[in] x left
        @param[in] y top
    */
    void move( int x, int y )
    {
        RECT rect;
        GetClientRect( myHandle, &rect );
        MoveWindow( myHandle,
                    x,y,rect.right-rect.left,rect.bottom-rect.top,
                    false );
    }
    void move( int x, int y, int w, int h )
    {
        MoveWindow( myHandle,
                    x, y, w, h, false);
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
    gui* myParent;
    eventhandler myEvents;
    int myBGColor;
    HBRUSH myBGBrush;
    std::vector< HWND >* myDeleteList;
    std::string myText;
    int myID;
    std::vector< gui* > myChild;            ///< gui elements to be displayed in this window
    bool myfModal;                          ///< true if element is being shown as modal


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
private:
    void trackbarMessageHandler( HWND hwnd )
    {
        // trackbar notifications are sent to trackbar's parent window
        // find the child that generated this notification
        // get the tackbar position and call the slid event handler
        for( auto c : myChild )
        {
            if( c->handle() == hwnd )
            {
                c->events().onSlid(
                    SendMessage(
                        hwnd,
                        TBM_GETPOS,
                        (WPARAM) 0, (LPARAM) 0 ));
            }
        }
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

/// \brief A panel which arranges the widgets it contains in a grid.
class layout : public panel
{
public:
    layout( gui* parent )
        : panel( parent )
        , myColCount( 2 )
        , myfWidthsSpecified( false )
        , myfColFirst( false )
    {

    }
    /** Specify number of cols to use for layout.

    The child windows will be laid out in the specified number of columns
    with the required number of rows to show them all.

    The space between columns and rows will be adjusted so that
    all child windows will be visible.

    The size of the child windows will not be altered.

    */
    void grid( int cols )
    {
        myColCount = cols;
    }
    /** Specify column widths.
        @param[in] vw vector of widths for each column
    */
    void colWidths( const std::vector<int>& vw )
    {
        myWidths = vw;
        myfWidthsSpecified = true;
    }
    /** Specify that widgets should be added to fill columns first
        @param[in] f column first flag, defaault true

        By default ( if this method is not called ) rows are filled first

        <pre>
         1    2
         3    4
         5    6
         </pre>

         In column first, the widgets are added like this

         <pre>
         1    4
         2    5
         3    6
         </pre>
    */
    void colfirst( bool f = true )
    {
        myfColFirst = f;
    }
    void draw( PAINTSTRUCT& ps )
    {
        RECT r;
        GetClientRect(myHandle, &r );
        if( ! myfWidthsSpecified )
        {
            // col widths not specified, default to all the same width to fill panel
            int colwidth = (r.right - r.left) / myColCount;
            myWidths.clear();
            for( int k = 0; k < myColCount; k++ )
            {
                myWidths.push_back( colwidth );
            }
        }
        int rowheight;
        if( ! myfColFirst )
            rowheight = ( r.bottom - r.top ) / ( ( myChild.size() + 1 ) / myColCount );
        else
            rowheight = 50;

        // display the children laid out in a grid
        int colcount = 0;
        int rowcount = 0;
        int x = 0;

        if( ! myfColFirst )
        {
            for( auto w : myChild )
            {
                w->move( x, rowcount*rowheight );
                w->update();

                x += myWidths[colcount];
                colcount++;
                if( colcount >= myColCount )
                {
                    colcount = 0;
                    x = 0;
                    rowcount++;
                }
            }
        }
        else
        {
            for( auto w : myChild )
            {
                w->move( x, rowcount * rowheight );
                w->update();
                rowcount++;
                if( rowcount >= (int)myChild.size() / myColCount )
                {
                    rowcount = 0;
                    x += myWidths[colcount];
                    colcount++;
                }
            }
        }
    }

private:
    int myColCount;
    std::vector<int> myWidths;
    bool myfWidthsSpecified;        // true if app code specified column widths
    bool myfColFirst;               // true if columns should be filled first
};

/// A widget that user can click to start an action.
class button : public gui
{
public:
    button( gui* parent )
        : gui( parent )
        , myBitmap( NULL )
    {

    }
    /// Specify image to be used for button
    void image( const std::string& fname )
    {
        myBitmap  = (HBITMAP)LoadImage(
                        NULL, fname.c_str(), IMAGE_BITMAP,
                        0, 0, LR_LOADFROMFILE);
    }
protected:
    HBITMAP myBitmap;

    /// draw
    virtual void draw( PAINTSTRUCT& ps )
    {
        if( ! myBitmap )
        {
            // button with text

            SetBkColor(
                ps.hdc,
                myBGColor );

            RECT r( ps.rcPaint );
            r.left += 1;
            r.top  += 1;
            DrawText(
                ps.hdc,
                myText.c_str(),
                -1,
                &r,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER );

            DrawEdge(
                ps.hdc,
                &ps.rcPaint,
                EDGE_RAISED,
                BF_RECT
            );
        }
        else
        {
            // button with bitmap

            HDC hLocalDC = CreateCompatibleDC(ps.hdc);
            BITMAP qBitmap;
            GetObject(reinterpret_cast<HGDIOBJ>(myBitmap), sizeof(BITMAP),
                      reinterpret_cast<LPVOID>(&qBitmap));
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hLocalDC, myBitmap);
            BitBlt(
                ps.hdc, 0, 0, qBitmap.bmWidth, qBitmap.bmHeight,
                hLocalDC, 0, 0, SRCCOPY);
            SelectObject(hLocalDC, hOldBmp);
            DeleteDC(hLocalDC);
        }
    }
};
/** A widget that user can click to select one of an exclusive set of options

<pre>
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("A windex radiobutton");

    wex::groupbox& P = wex::maker::make<wex::groupbox>( form );
    P.move( 5, 5, 350,200 );

    // use laypout to atomatically arrange buttons in columns
    wex::layout& L = wex::maker::make<wex::layout>(P  );
    L.move( 50, 50,300,190);
    L.grid( 2 );                // specify 2 columns
    L.colfirst();               // specify column first order

    // first group of radiobuttons
    radiobutton& rb1 = wex::maker::make<radiobutton>(L);
    rb1.first();                // first in group of interacting buttons
    rb1.move( {20,20,100,30} );
    rb1.text("Alpha");
    radiobutton& rb2 = wex::maker::make<radiobutton>(L);
    rb2.move( {20,60,100,30} );
    rb2.text("Beta");
    radiobutton& rb3 = wex::maker::make<radiobutton>(L);
    rb3.move( {20,100,100,30} );
    rb3.text("Gamma");

    // second group of radio buttons
    radiobutton& rb4 = wex::maker::make<radiobutton>(L);
    rb4.first();                // first in group of interacting buttons
    rb4.size( 80,30 );
    rb4.text("X");
    radiobutton& rb5 = wex::maker::make<radiobutton>(L);
    rb5.size( 80,30 );
    rb5.text("Y");
    radiobutton& rb6 = wex::maker::make<radiobutton>(L);
    rb6.size( 80,30 );
    rb6.text("Z");

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 250, 150, 30 } );
    btn.text( "Show values entered" );

    // popup a message box when button is clicked
    // showing the values entered
    btn.events().click([&]
    {
        std::string msg;
        if( rb1.isChecked() )
            msg = "Alpha";
        else if( rb2.isChecked() )
            msg = "Beta";
        else if( rb3.isChecked() )
            msg = "Gamma";
        else
            msg = "Nothing";
        msg += " is checked";
        msgbox(
            form,
            msg );
    });

    // show the application
    form.show();
</pre>
*/
class radiobutton : public gui
{
public:
    radiobutton( gui* parent )
        : gui( parent )
        , myValue( false )
    {
        // Add to current group
        group().back().push_back( this );
        myGroup = group().size()-1;

        // set the boolean value when clicked
        events().clickWex([this]
        {
            // set all buttons in group false
            for( auto b : group()[ myGroup ] )
            {
                b->myValue = false;
                b->update();
            }
            // set this button true
            myValue = true;
            update();
        });
    }
    /** Make this button first of a new group

    The other buttons in a group will become false when one is clicked
    but buttons in different groups will not be changed.

    All succeeding buttons,
    those that were constructed after this button and remain in the same group,
    are also moved to the new group.

    */
    void first()
    {
        // find button in its group
        auto this_it = std::find(
                           group()[myGroup].begin(),
                           group()[myGroup].end(),
                           this );

        if( this_it == group()[myGroup].end() )
            throw std::runtime_error("wex::radiobutton::first error in group");

        // if button is first in group, nothing is needed
        if( this_it == group()[myGroup].begin() )
            return;

        // construct new group
        std::vector< radiobutton * > g;
        group().push_back( g );

        // copy button and following buttons in same group to new group
        for(
            auto it = this_it;
            it != group()[myGroup].end();
            it++ )
        {
            group().back().push_back( *it );
        }

        // erase from old group
        group()[myGroup].erase(
            this_it,
            group()[myGroup].end() );

        // tell buttons that were moved about their new group
        for( auto b : group().back() )
            b->myGroup = group().size() - 1;

//        std::cout << "< first\n";
//        for( int kg=0; kg< group().size(); kg++ )
//        {
//            for( auto b : group()[kg] )
//                std::cout << b->id() << " , " << b->text() << " ";
//            std::cout << "\n";
//        }
    }

    /// true if checked
    bool isChecked()
    {
        return myValue;
    }

    /** Which button in group is checked
        @return zero-based offset of checked button in group this button belongs to, -1 if none checked
    */
    int checkedOffset()
    {
        int off = 0;
        for( auto b : group()[myGroup] )
        {
            if( b->isChecked() )
                break;
            off++;
        }
        if( off < (int)group()[myGroup].size() )
            return off;
        return -1;
    }

    /// set value true( default ) or false
    void check( bool f = true )
    {
        if( f )
        {
            // set all buttons in group false
            for( auto b : group()[ myGroup ] )
            {
                b->myValue = false;
                b->update();
            }
        }
        myValue = f;
        update();
    }

    virtual void draw( PAINTSTRUCT& ps )
    {
        SetBkColor(
            ps.hdc,
            myBGColor );
        RECT r( ps.rcPaint );
        r.left += 20;
        shapes S( ps );

        DrawText(
            ps.hdc,
            myText.c_str(),
            -1,
            &r,
            0);
        if( ! myValue )
        {
            S.circle( 10, 10, 5 );
        }
        else
        {
            SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
            Ellipse( ps.hdc, 5, 5, 15, 15 );
        }
    }
private:
    bool myValue;
    int myGroup;            /// index of group button belongs to

    /// get reference to radiobutton groups
    std::vector< std::vector< radiobutton * > > & group()
    {
        static std::vector< std::vector< radiobutton * > > theGroups;
        static bool fGroupInit = false;
        if( ! fGroupInit )
        {
            // create first group
            fGroupInit = true;
            std::vector< radiobutton * > g;
            theGroups.push_back( g );
        }
        return theGroups;
    }
};
/** @brief A widget that user can click to toggle a true/false value

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
        events().clickWex([this]
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
        r.left += cbg+5;
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
        S.color( 0 );
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

/// A widget that displays a string.
class label : public gui
{
public:
    label( gui* parent )
        : gui( parent )
    {

    }
};
/// A widget where user can enter a string.
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

    /// change text in textbox
    void text( const std::string& t )
    {
        SetDlgItemText(
            myParent->handle(),
            myID,
            t.c_str() );
    }

    /// get text in textbox
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
    /// disable ( or enable ) user editing
    void readonly( bool f = true )
    {
        SendMessage(
            handle(),
            EM_SETREADONLY,
            (WPARAM)f, (LPARAM)0);
    }
};

/// A widget where user can choose from a dropdown list of strings
class choice : public gui
{
public:
    choice( gui* parent )
        : gui( parent, "Combobox",
               CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE )
    {
    }
    /// Override move to ensure height is sufficient to allow dropdown to apprear
    void move( int x, int y, int w, int h )
    {
        if( h < 100 )
            h = 100;
        gui::move( x, y, w, h );
    }
    /// Add an option
    void add( const std::string& s )
    {
        SendMessageA(
            handle(),
            (UINT) CB_ADDSTRING,
            (WPARAM) 0,
            (LPARAM) s.c_str());
    }
    /// Clear all options
    void clear()
    {
        SendMessage(
            handle(),
            CB_RESETCONTENT,
            (WPARAM)0, (LPARAM)0);
    }
    /** Select by index
        @param[in] i index of item to selecct, -1 clears selection
    */
    void select( int i )
    {
        SendMessage(
            handle(),
            CB_SETCURSEL,
            (WPARAM)i, (LPARAM)0);
    }
    /** Select by string
        @param[in] s the string to select
    */
    void select( const std::string& s )
    {
        SendMessage(
            handle(),
            CB_SELECTSTRING,
            (WPARAM)-1, (LPARAM)s.c_str());
    }
    /// get index of selected item
    int SelectedIndex()
    {
        return SendMessage(
                   handle(),
                   (UINT) CB_GETCURSEL,
                   (WPARAM) 0, (LPARAM) 0);
    }
    /// get text of selected item
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
    /// get count of items
    int count()
    {
        return SendMessage(
                   handle(),
                   (UINT)CB_GETCOUNT,
                   (WPARAM) 0, (LPARAM) 0);
    }
};
/// A class containing a database of the current gui elements
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

    /* handle window messages

    All messages to windows created by windex come here.
    The messages are passed on to be handled by the gui element for the window they are directed to
    */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        auto w =  get().myGui.find( hwnd );
        if( w !=  get().myGui.end() )
        {
            if( w->second->WindowMessageHandler( hwnd, uMsg, wParam, lParam ) )
                return 0;
        }

        // run default message processing
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

        //std::cout << "windexAdd " << myGui.size() <<" in "<< this << "\n";
    }

    mgui_t myGui;                       ///< map of existing gui elements
private:

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

/// A popup window where used can browse folders and select a file
class filebox
{
public:
    filebox( gui& parent )
    {

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
    }

    /** \brief prompt user for file to open
        @return path to file to be opened, "" if cancelled
    */
    std::string open()
    {
        if (GetOpenFileName(&ofn)==TRUE)
        {
            myfname = ofn.lpstrFile;
        }
        else
            myfname = "";
        return myfname;
    }
    /** \brief prompt user for folder and filename to save
        @return path to file to be saved, "" if cancelled
    */
    std::string save()
    {
        if( GetSaveFileName(&ofn) == TRUE)
            myfname = ofn.lpstrFile;
        else
            myfname = "";
        return myfname;
    }
    /// get filename entered by user
    std::string path() const
    {
        return myfname;
    }
private:
    OPENFILENAME ofn;       // common dialog box structure
    char szFile[260];       // buffer for file name
    std::string myfname;
};

/** \brief A drop down list of options that user can click to start an action.

<pre>
    // construct top level window
    gui& form = wex::windex::topWindow();
    form.move({ 50,50,400,400});
    form.text("Menu demo");

     int clicked = -1;

    menu m;
    m.append("test",[&]
    {
        clicked = 1;
    });
    m.append("second",[&]
    {
        clicked = 2;
    });
    m.append("third",[&]
    {
        clicked = 3;
    });
    m.popup( form, 200,200 );

    msgbox( form,std::string("item ") + std::to_string(clicked) + " clicked");

    form.show();
</pre>

*/
class menu
{
public:
    menu( gui& parent )
        : myM( CreatePopupMenu() )
        , myParent( parent )
    {

    }
    /** Append menu item.
        @param[in] title
        @param[in] f function to be run when menu item clicked
    */
    void append(
        const std::string& title,
        const std::function<void(void)>& f = [] {})
    {
        // add item to menu
        auto mi = CommandHandlers().size();
        AppendMenu(
            myM,
            0,
            mi,
            title.c_str());

        // store function to run when menu item clicked in popup
        CommandHandlers().push_back( f );

        // store function to run when menu item click in menubar
        myParent.events().menuCommand( mi, f );
    }
    /** Append submenu
        @param[in] title
        @param[in] submenu
    */
    void append(
        const std::string& title,
        menu& submenu )
    {
        AppendMenu(
            myM,
            MF_POPUP,
            (UINT_PTR)submenu.handle(),
            title.c_str());
    }
    /** Popup menu and run user selection.
        @param[in] x location
        @param[in] y location
    */
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
        if( 0 <= i && i < (int)CommandHandlers().size() )
            CommandHandlers()[i]();
    }
    HMENU handle()
    {
        return myM;
    }
private:
    HMENU myM;
    gui& myParent;

    std::vector< std::function<void(void)> >& CommandHandlers()
    {
        static std::vector< std::function<void(void)> > myf;
        return myf;
    }
};

/// A widget that displays across top of a window and contains a number of dropdown menues.
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
        @param[in] title that appears in the menubar
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
/** \brief Generate events at regularly timed intervals.

<pre>
    myDriveTimer = new wex::timer( fm, 50 );
    fm.events().timer([this]
    {
        ... code to run when timer event occurs ...
    });
</pre>
*/
class timer
{
public:
    /** CTOR
        @param[in] g gui element that will receive the events
        @param[in] intervalmsecs time between events

        The events will begin immediatly
    */
    timer( gui& g, int intervalmsecs )
    {
        SetTimer(
            g.handle(),             // handle to  window
            1,            // timer identifier
            intervalmsecs,                 //  interval ms
            (TIMERPROC) NULL);     // no timer callback
    }
};
/** \brief A widget which user can drag to change a value.

<pre>
    // construct top level window
    gui& form = wex::windex::topWindow();
    form.move({ 50,50,500,400});
    form.text("Slider demo");

    // construct labels to display values when sliders are moved
    wex::label& label = wex::make<wex::label>(form);
    label.move( 200, 200, 100,30 );
    label.text("");
    wex::label& vlabel = wex::make<wex::label>(form);
    vlabel.move( 200, 240, 100,30 );
    vlabel.text("");

    // construct horizontal slider
    wex::slider& S = wex::make<wex::slider>( form );
    S.move({ 50,50,400,50});
    S.range( 0, 100 );
    S.text("horiz slider");
    S.events().slid([&](int pos)
    {
        label.text("horiz value: " + std::to_string( pos ));
        label.update();
    });

    // construct vertical slider
    wex::slider& V = wex::make<wex::slider>( form );
    V.move({ 50,100,50,400});
    V.range( 0, 10 );
    V.vertical();
    V.events().slid([&](int pos)
    {
        vlabel.text("vert value: " + std::to_string( pos ));
        vlabel.update();
    });

    form.show();
</pre>
*/

class slider : public gui
{
public:
    slider( gui* parent )
        : gui( parent, "msctls_trackbar32",
               WS_CHILD | WS_OVERLAPPED | WS_VISIBLE |
               TBS_AUTOTICKS | TBS_TRANSPARENTBKGND )
    {
    }
    /** Specify the range values used
        @param[in] min
        @param[in] max

        The values must all be positive,
        otherwise a runtime_error exception is thrown
    */
    void range( int min, int max )
    {
        if( min < 0 )
            throw std::runtime_error("wex::slider positions must be positive");

        SendMessage(
            myHandle,
            TBM_SETRANGE,
            (WPARAM) TRUE,                   // redraw flag
            (LPARAM) MAKELONG(min, max));  // min. & max. positions
    }
    /// Change the orientation to be vertical
    void vertical()
    {
        SetWindowLongPtr(
            myHandle,
            GWL_STYLE,
            GetWindowLongPtr( myHandle, GWL_STYLE) |  TBS_VERT );
    }

    /// Position of the slider thumb in range
    int position()
    {
        return SendMessage(
                   myHandle,
                   TBM_GETPOS,
                   (WPARAM) 0, (LPARAM) 0 );
    }
    void position( int pos )
    {
        SendMessage(
            myHandle,
            TBM_SETPOS,
            (WPARAM) true, (LPARAM) pos );
    }
};

/// \brief A class for making windex objects.
class maker {
    public:

/** Construct widget
        @param[in] parent reference to parent window or widget
*/
template < class W, class P >
static W& make( P& parent )
{
    W* w = new W( (gui*)&parent );

    // inherit background color from parent
    w->bgcolor( parent.bgcolor() );

    windex::get().Add( w );
    return *w;
}

/// Construct a top level window ( first call constructs application window )
static gui&  make()
{
    windex::get().myGui.size();

    gui* w = new gui();
    windex::get().Add( w );
    return *w;
}
};

}

