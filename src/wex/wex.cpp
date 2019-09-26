#include <iostream>
#include <map>
#include "wex.h"

namespace wex
{
void exec()
{
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

std::map< HWND, gui * > mGui;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //std::cout << "WindowProc " << hwnd <<" "<< uMsg << "\n";
    auto w = mGui.find( hwnd );
    if( w != mGui.end() )
    {
        if( w->second )
        {
            if( w->second->WindowMessageHandler( hwnd, uMsg, wParam, lParam ) )
                return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void gui::registerWindowClass()
{
    static bool done = false;
    if( ! done )
    {
        WNDCLASS wc = { };
        wc.lpfnWndProc   = &WindowProc;
        wc.hInstance     = NULL;
        wc.lpszClassName = "windex";
        RegisterClass(&wc);
        done = true;
    }
}

void gui::registerGui()
{
    mGui.insert(std::pair<HWND,gui*>(myHandle,this));

//    std::cout << "registerGui " << mGui.size() <<" "<< myHandle << "\n";
//    for( auto p : mGui )
//        std::cout << p.first << " ";
//    std::cout << "\n";
}

window::window()
{
    myHandle = CreateWindowEx(
                   0,                              // Optional window styles.
                   "windex",                     // Window class
                   "windex",    // Window text
                   WS_OVERLAPPEDWINDOW,            // Window style

                   // Size and position
                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                   NULL,       // Parent window
                   NULL,       // Menu
                   NULL,  // Instance handle
                   NULL        // Additional application data
               );
    registerGui();
    std::cout << "window created " << myHandle << "\n";
}

bool window::WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//    std::cout << "window::WindowMessageHandler "
//              << myHandle <<" "<< uMsg << "\n";
    if( hwnd == myHandle )
    {
        switch (uMsg)
        {
        case WM_DESTROY:
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


void window::show()
{
    ShowWindow(myHandle,  SW_SHOWDEFAULT);
    for( auto w : myWidget )
        w->show();
}

widget::widget( window& parent )
{
    myHandle = CreateWindowEx(
                   0,                              // Optional window styles.
                   "windex",                     // Window class
                   "widget",    // Window text
                   WS_CHILD,            // Window style

                   // Size and position
                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                   parent.handle(),       // Parent window
                   NULL,       // Menu
                   NULL,  // Instance handle
                   NULL        // Additional application data
               );
    registerGui();
    parent.child( this );
}

bool widget::WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //std::cout << " widget WindowMessageHandler " << uMsg << "\n";
    if( hwnd == myHandle )
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            //PostQuitMessage(0);
            return true;

        case WM_PAINT:
        {
            std::cout << "widget paint\n";
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
void widget::draw( PAINTSTRUCT& ps )
{
    DrawText(
        ps.hdc,
        myText.c_str(),
        -1,
        &ps.rcPaint,
        0);
}
void button::draw( PAINTSTRUCT& ps )
{
    widget::draw( ps );
    DrawEdge(
        ps.hdc,
        &ps.rcPaint,
        EDGE_RAISED,
        BF_RECT
    );
}

msgbox::msgbox( const std::string& msg )
    : myText( msg )
{
    MoveWindow( myHandle,
                100,100,400,100,false);
    ShowWindow(myHandle,  SW_SHOWDEFAULT);
    RECT rect;
    rect.left = 5;
    rect.top = 5;
    rect.bottom = 50;
    rect.right = 100;
    DrawText(
        GetDC( myHandle ),
        myText.c_str(),
        -1,
        &rect,
        DT_TOP|DT_RIGHT);
}
msgbox::~msgbox()
{
    auto i = mGui.find( myHandle );
    if( i != mGui.end() )
        mGui.erase( i );
}

}
