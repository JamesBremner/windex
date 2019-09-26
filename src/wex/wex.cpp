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

mgui_t windex::myGui;


void gui::registerWindowClass()
{
    static bool done = false;
    if( ! done )
    {
        WNDCLASS wc = { };
        wc.lpfnWndProc   = &windex::WindowProc;
        wc.hInstance     = NULL;
        wc.lpszClassName = "windex";
        RegisterClass(&wc);
        done = true;
    }
}

void gui::Create( HWND parent, DWORD style )
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
//    registerGui();
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

bool widget::WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void window::show()
{
    ShowWindow(myHandle,  SW_SHOWDEFAULT);
    for( auto w : myWidget )
        w->show();
}

}
