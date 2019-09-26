#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <windows.h>

namespace wex
{

class widget;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

class window
{
public:
    window();

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
    HWND handle()
    {
        return myHandle;
    }
    void child( widget* w )
    {
        myWidget.push_back( w );
    }

    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND myHandle;
    std::vector< widget* > myWidget;
};

class widget
{
public:
    widget( window& parent );

    bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void show()
    {
        ShowWindow(myHandle,  SW_SHOWDEFAULT);
    }

    eventhandler& events()
    {
        return myEvents;
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
private:
    HWND myHandle;
    std::string myText;
    eventhandler myEvents;
};

class button : public widget
{
    public:
        button( window& parent )
        : widget( parent )
        {

        }
};

void exec();

}
