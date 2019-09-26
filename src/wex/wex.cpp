#include <iostream>
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

window * the_top_window;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if( the_top_window )
    {
        if( the_top_window->WindowMessageHandler( hwnd, uMsg, wParam, lParam ) )
            return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

window::window()
{
    WNDCLASS wc = { };
    wc.lpfnWndProc   = &WindowProc;
    wc.hInstance     = NULL;
    wc.lpszClassName = "windex";
    RegisterClass(&wc);

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
    the_top_window = this;
}

bool window::WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //std::cout << "WindowMessageHandler\n";
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
            //std::cout << "widget paint\n";
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(myHandle, &ps);

            DrawText(hdc,myText.c_str(),-1,&ps.rcPaint,0);

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

}
