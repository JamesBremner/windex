#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <windows.h>

namespace wex
{

class widget;

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
    {
        registerWindowClass();
    }
    virtual bool WindowMessageHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    eventhandler& events()
    {
        return myEvents;
    }
protected:
    HWND myHandle;
    eventhandler myEvents;
    void registerGui();
private:
    void registerWindowClass();
};



/// A top level window
class window : public gui
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

protected:
    std::vector< widget* > myWidget;

    virtual void draw( PAINTSTRUCT& ps ) {}
};

/// A widget placed inside a window
class widget : public gui
{
public:
    widget( window& parent );

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
    virtual void draw( PAINTSTRUCT& ps );
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
    virtual void draw( PAINTSTRUCT& ps );
};

/// A popup with a message
class msgbox : public window
{
public:
    msgbox( const std::string& msg );
    ~msgbox();

private:
    std::string myText;
};

void exec();

}
