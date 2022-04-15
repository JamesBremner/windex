#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <windows.h>
#include <CommCtrl.h>
#include <Shellapi.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace wex
{

    class gui;

    typedef std::map<HWND, gui *> mgui_t;
    typedef std::vector<gui *> children_t;

    /// A structure containing the mouse status for event handlers
    struct sMouse
    {
        int x;
        int y;
        bool left;
        bool right;
        bool shift;
    };

    enum eventMsgID
    {
        asyncReadComplete = WM_APP + 1,
        tcpServerAccept,
        tcpServerReadComplete,
    };

    class modalMgr
    {
    public:
        /// get reference to singleton modal manager
        static modalMgr &get()
        {
            static modalMgr theModalMgr;
            return theModalMgr;
        }

        /** Set modal running
    @param[in] id of modal window starting to run
    @return true if successful, false if other modal winow running

    If other modal window running, it gets focus

    */
        bool set(int id, HWND h)
        {
            if (!id)
            {
                myModalID = 0;
                return true;
            }
            if (myModalID)
            {
                std::cout << "App tried to show two modal windows\n";
                SetFocus(myModalHandle);
                return false;
            }

            myModalID = id;
            myModalHandle = h;
            return true;
        }

        /** Can a window be closed
        @param[in] is of window requesting to be closed
        @return true if window can be closed

        Window can be closed if no modal running
        or if window to be closed is the the running modal
    */
        bool canClose(int id)
        {
            if (!myModalID)
                return true; // no modal window is running

            if (myModalID == id)
            {
                // the modal window is being closed
                myModalID = 0;
                return true;
            }

            // attempt to close other window while modal window is running
            // give the modal window focus
            SetFocus(myModalHandle);
            return false;
        }

    private:
        int myModalID;
        HWND myModalHandle;

        modalMgr()
            : myModalID(0)
        {
        }
    };

    /// A class where application code can register functions to be called when an event occurs
    class eventhandler
    {
    public:
        eventhandler()
            : myfClickPropogate(false), myfMouseTracking(false)
        {
            // initialize functions with no-ops
            click([] {});
            clickWex([] {});
            clickRight([] {});
            draw([](PAINTSTRUCT &ps) {});
            resize([](int w, int h) {});
            scrollH([](int c) {});
            scrollV([](int c) {});
            keydown([](int c) {});
            mouseEnter([] {});
            mouseLeave([] {});
            mouseMove([](sMouse &m) {});
            mouseWheel([](int dist) {});
            mouseUp([] {});
            timer([](int id) {});
            slid([](int pos) {});
            dropStart([](HDROP hDrop) {});
            drop([](const std::vector<std::string> &files) {});
            asyncReadComplete([](int id) {});
            tcpServerAccept([] {});
            tcpRead([] {});
            quitApp([]
                    { return true; });
        }
        bool onLeftdown()
        {
            myClickFunWex();
            myClickFunctionApp();
            return !myfClickPropogate;
        }
        void onRightDown()
        {
            myClickRightFunction();
        }
        void onMouseUp()
        {
            myMouseUpFunction();
        }
        void onDraw(PAINTSTRUCT &ps)
        {
            myDrawFunction(ps);
        }
        void onResize(int w, int h)
        {
            myResizeFunction(w, h);
        }
        void onScrollH(int code)
        {
            myScrollHFunction(code);
        }
        void onScrollV(int code)
        {
            myScrollVFunction(code);
        }
        void onMenuCommand(int id)
        {
            if (0 > id || id >= (int)myVectorMenuFunction.size())
                return;
            myVectorMenuFunction[id](myVectorMenuTitle[id]);
        }
        void onKeydown(int keycode)
        {
            myKeydownFunction(keycode);
        }
        void onMouseMove(WPARAM wParam, LPARAM lParam)
        {
            sMouse m;
            m.x = LOWORD(lParam);
            m.y = HIWORD(lParam);
            m.left = (wParam == MK_LBUTTON);

            myMouseMoveFunction(m);
        }
        void onMouseEnter()
        {
            myMouseEnterFunction();
        }
        void onMouseWheel(int dist)
        {
            myMouseWheelFunction(dist);
        }
        void onMouseLeave()
        {
            myMouseLeaveFunction();
        }
        void onTimer(int id)
        {
            myTimerFunction(id);
        }
        bool onSelect(
            unsigned short id)
        {
            auto it = mapControlFunction().find(std::make_pair(id, CBN_SELCHANGE));
            if (it == mapControlFunction().end())
                return true;
            it->second();
            return true;
        }
        bool onChange(
            unsigned short id)
        {
            auto it = mapControlFunction().find(std::make_pair(id, EN_CHANGE));
            if (it == mapControlFunction().end())
                return true;
            it->second();
            return true;
        }
        void onSlid(unsigned short id)
        {
            mySlidFunction((int)id);
        }
        void onDropStart(HDROP hDrop)
        {
            myDropStartFunction(hDrop);
        }
        void onDrop(const std::vector<std::string> &files)
        {
            myDropFunction(files);
        }
        void onAsyncReadComplete(int id)
        {
            myAsyncReadCompleteFunction(id);
        }
        void onTcpServerAccept()
        {
            myTcpServerAcceptFunction();
        }
        void onTcpServerReadComplete()
        {
            myTcpServerReadCompleteFunction();
        }
        bool onQuitApp()
        {
            return myQuitAppFunction();
        }
        /////////////////////////// register event handlers /////////////////////

        /** register click event handler
        @param[in] f the function to call when user clicks on gui in order to process event
        @param[in] propogate specify that event should propogate to parent window after processing, default is false

        A click occurs when the left mouse button is pressed
    */
        void click(
            std::function<void(void)> f,
            bool propogate = false)
        {
            myClickFunctionApp = f;
            myfClickPropogate = propogate;
        }
        /** register a function to do some housekeeping when clicked, before calling handler registered by application code
        @param[in] f the function to call

        This should NOT be called by application code.

        For example, this looks after the check mark in checkboxes
        or the removal of the filled in dot of the other radio buttons in a group
    */
        void clickWex(std::function<void(void)> f)
        {
            myClickFunWex = f;
        }
        /// specify that click event should propogate to parent window after currently registered click event handler runs
        void clickPropogate(bool f = true)
        {
            myfClickPropogate = f;
        }

        void clickRight(std::function<void(void)> f)
        {
            myClickRightFunction = f;
        }

        void draw(std::function<void(PAINTSTRUCT &ps)> f)
        {
            myDrawFunction = f;
        }
        void resize(std::function<void(int w, int h)> f)
        {
            myResizeFunction = f;
        }
        void scrollH(std::function<void(int code)> f)
        {
            myScrollHFunction = f;
        }
        void scrollV(std::function<void(int code)> f)
        {
            myScrollVFunction = f;
        }
        /** Register function to run when menu item clicked
        @param[in] f function to run when menu item with id is clicked
        @param[in] title of menu item, passed as parameter to event handler
        @return event handler index to be used as menu item index
    */
        int menuCommand(
            std::function<void(const std::string &title)> f,
            const std::string &title)
        {
            int id = (int)myVectorMenuFunction.size();
            myVectorMenuFunction.push_back(f);
            myVectorMenuTitle.push_back(title);
            return id;
        }
        void select(
            int id,
            std::function<void(void)> f)
        {
            mapControlFunction().insert(
                std::make_pair(std::make_pair(id, CBN_SELCHANGE), f));
        }
        /** register function to call when control changes
        @param[in] id of control
    */
        void change(
            int id,
            std::function<void(void)> f)
        {
            mapControlFunction().insert(
                std::make_pair(std::make_pair(id, EN_CHANGE), f));
        }
        /// register function to call when key pressed. Function is passed key code.
        void keydown(std::function<void(int keydown)> f)
        {
            myKeydownFunction = f;
        }
        void mouseEnter(std::function<void(void)> f)
        {
            myMouseEnterFunction = f;
        }
        void mouseMove(std::function<void(sMouse &m)> f)
        {
            myMouseMoveFunction = f;
        }
        void mouseWheel(std::function<void(int dist)> f)
        {
            myMouseWheelFunction = f;
        }
        void mouseUp(std::function<void(void)> f)
        {
            myMouseUpFunction = f;
        }
        void mouseLeave(std::function<void(void)> f)
        {
            myMouseLeaveFunction = f;
        }
        void timer(std::function<void(int id)> f)
        {
            myTimerFunction = f;
        }
        void slid(std::function<void(int pos)> f)
        {
            mySlidFunction = f;
        }
        /// register function to call when user drops files.  App code should NOT call this!
        void dropStart(std::function<void(HDROP hDrop)> f)
        {
            myDropStartFunction = f;
        }
        /// register function to call when files dropped by user have been extracted.  App code use this!
        void drop(std::function<void(const std::vector<std::string> &files)> f)
        {
            myDropFunction = f;
        }
        /** register function to call when an asynchronous read completes.
        The function parameter identifies the com class that completed the read
    */
        void asyncReadComplete(std::function<void(int id)> f)
        {
            myAsyncReadCompleteFunction = f;
        }
        void tcpServerAccept(std::function<void(void)> f)
        {
            myTcpServerAcceptFunction = f;
        }
        /// register function to call when tcp read accurs
        void tcpRead(std::function<void(void)> f)
        {
            myTcpServerReadCompleteFunction = f;
        }
        /** register function to call when application is about to quit
        The function should return true to allow the quit to proceed.
    */
        void quitApp(std::function<bool(void)> f)
        {
            myQuitAppFunction = f;
        }

    private:
        bool myfClickPropogate;
        bool myfMouseTracking;

        // event handlers registered by application code
        std::function<void(void)> myClickFunctionApp;
        std::function<void(void)> myClickRightFunction;
        std::function<void(PAINTSTRUCT &ps)> myDrawFunction;
        std::function<void(int w, int h)> myResizeFunction;
        std::function<void(int code)> myScrollHFunction;
        std::function<void(int code)> myScrollVFunction;
        std::vector<std::function<void(const std::string &title)>> myVectorMenuFunction;
        std::vector<std::string> myVectorMenuTitle;
        std::function<void(int keycode)> myKeydownFunction;
        std::function<void(sMouse &m)> myMouseMoveFunction;
        std::function<void(void)> myMouseEnterFunction;
        std::function<void(int dist)> myMouseWheelFunction;
        std::function<void(int id)> myTimerFunction;
        std::function<void(void)> myMouseUpFunction;
        std::function<void(void)> myMouseLeaveFunction;
        std::function<void(int pos)> mySlidFunction;
        std::function<void(HDROP hDrop)> myDropStartFunction;
        std::function<void(const std::vector<std::string> &files)> myDropFunction;
        std::function<void(int id)> myAsyncReadCompleteFunction;
        std::function<void(void)> myTcpServerAcceptFunction;
        std::function<void(void)> myTcpServerReadCompleteFunction;
        std::function<bool(void)> myQuitAppFunction;

        // event handlers registered by windex class
        std::function<void(void)> myClickFunWex;

        /// reference to application wide list of registered event handlers
        /// mapped to control ids and notification code
        std::map<std::pair<int, unsigned short>, std::function<void(void)>> &
        mapControlFunction()
        {
            static std::map<std::pair<int, unsigned short>, std::function<void(void)>>
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
        /** Constructor
        @param[in] ps The PAINTSTRUCT passed as parameter into the draw event handler
    */
        shapes(PAINTSTRUCT &ps)
            : myHDC(ps.hdc), myPenThick(1), myFill(false)
        {
            hPen = CreatePen(
                PS_SOLID,
                myPenThick,
                RGB(0, 0, 0));
            hPenOld = SelectObject(myHDC, hPen);

            myLogfont = {0};
            HANDLE hFont;
            ZeroMemory(&myLogfont, sizeof(LOGFONT));
            myLogfont.lfWeight = FW_NORMAL;
            strcpy(myLogfont.lfFaceName, "Tahoma");
            myLogfont.lfHeight = 20;
            hFont = CreateFontIndirect(&myLogfont);
            hFont = (HFONT)SelectObject(myHDC, hFont);
            DeleteObject(hFont);
        }
        ~shapes()
        {
            HGDIOBJ pen = SelectObject(myHDC, hPenOld);
            DeleteObject(pen);
        }
        /** Set color for drawings
    @param[in] r red 0-255
    @param[in] g green 0-255
    @param[in] b blue 0-255
    */
        void color(int r, int g, int b)
        {
            color(RGB(r, g, b));
        }
        void color(int c)
        {
            myColor = c;
            hPen = CreatePen(
                PS_SOLID,
                myPenThick,
                c);
            HGDIOBJ old = SelectObject(myHDC, hPen);
            DeleteObject(old);
            SetTextColor(myHDC, c);
            HBRUSH brush = CreateSolidBrush(c);
            old = SelectObject(myHDC, brush);
            DeleteObject(old);
        }
        /// set background color
        void bgcolor(int c)
        {
            SetBkColor(
                myHDC,
                c);
        }
        void bgcolor(int r, int g, int b)
        {
            bgcolor(RGB(r, g, b));
        }
        /// enable/disable transparent background
        void transparent(bool f = true)
        {
            SetBkMode(
                myHDC,
                TRANSPARENT);
        }

        /// Set pen thickness in pixels
        void penThick(int t)
        {
            myPenThick = t;
            color(myColor);
        }

        /// Set filling option
        void fill(bool f = true)
        {
            myFill = f;
        }
        /// Color a pixel
        void pixel(int x, int y)
        {
            SetPixel(myHDC, x, y, myColor);
        }
        /** Draw line between two points
        @param[in] v vector with x1, y1, x2, y2
    */
        void line(const std::vector<int> &v)
        {
            MoveToEx(
                myHDC,
                v[0],
                v[1],
                NULL);
            LineTo(
                myHDC,
                v[2],
                v[3]);
        }

        void polyLine(POINT *pp, int n)
        {
            Polyline(
                myHDC,
                pp,
                n);
        }
        /** Draw rectangle
        @param[in] v vector with left, top, width, height
    */
        void rectangle(const std::vector<int> &v)
        {
            if (!myFill)
            {
                MoveToEx(
                    myHDC,
                    v[0],
                    v[1],
                    NULL);
                LineTo(
                    myHDC,
                    v[0] + v[2],
                    v[1]);
                LineTo(
                    myHDC,
                    v[0] + v[2],
                    v[1] + v[3]);
                LineTo(
                    myHDC,
                    v[0],
                    v[1] + v[3]);
                LineTo(
                    myHDC,
                    v[0],
                    v[1]);
            }
            else
            {
                //            std::cout << "wex rectangle fill "  << v[0]<<" "<< v[1]<<" "<<  v[0]+v[2]<<" "<<  v[1]+v[3] << "\n";
                Rectangle(
                    myHDC,
                    v[0], v[1], v[0] + v[2], v[1] + v[3]);
            }
        }

        /** Draw Polygon

    @param[in] point array of points, such as x0,y0,x1,y1,x2,y2,x3,y3...
    */
        void polygon(const std::vector<int> &v)
        {
            Polygon(myHDC, (const POINT *)&(v[0]), v.size() / 2);
        }

        /** Draw Arc of circle

    @param[in] x for center, pixels 0 at left of window
    @param[in] y for center, pixels 0 at top of window
    @param[in] r radius, pixels
    @param[in] sa start angle degrees anti-clockwise from 3 o'clock
    @param[in] se end angle degrees anti-clockwise from 3 o'clock

    The arc is drawn from sa to se in the anti-clockwise direction.
    */
        void arc(
            int x, int y, double r,
            double sa, double ea)
        {
            int xl = round(x - r);
            int yt = round(y - r);
            int xr = round(x + r);
            int yb = round(y + r);
            int xs = round(x + r * cos(sa * M_PI / 180));
            int ys = round(y - r * sin(sa * M_PI / 180));
            int xe = round(x + r * cos(ea * M_PI / 180));
            int ye = round(y - r * sin(ea * M_PI / 180));
            Arc(
                myHDC,
                xl, yt, xr, yb, xs, ys, xe, ye);
        }
        /** Draw circle
    @param[in] x0 x for center, pixels 0 at left of window
    @param[in] y0 y for center, pixels 0 at left of window
    @param[in] r radius, pixels
    */
        void circle(int x0, int y0, double r)
        {
            int ir = r;
            Ellipse(
                myHDC,
                x0 - ir, y0 - ir,
                x0 + ir, y0 + ir);
        }
        /** Draw text.
    @param[in] t the text
    @param[in] v vector of left, top
    */
        void text(
            const std::string &t,
            const std::vector<int> &v)
        {
            if ((int)v.size() < 2)
                return;
            TextOut(
                myHDC,
                v[0],
                v[1],
                t.c_str(),
                t.length());
        }

        void textCenterHz(
            const std::string &t,
            const std::vector<int> &v)
        {
            int ws = textWidthPixels(t);
            int pad = (v[2] - ws) / 2;
            if (pad < 0)
                pad = 0;
            std::vector<int> vc = v;
            vc[0] += pad;
            text(t, vc);
        }
        /// Enable / disable drawing text in vertical orientation
        void textVertical(bool f = true)
        {
            if (f)
                myLogfont.lfEscapement = 2700;
            else
                myLogfont.lfEscapement = 0;
            myLogfont.lfOrientation = myLogfont.lfEscapement;
            HANDLE hFont = CreateFontIndirect(&myLogfont);
            hFont = (HFONT)SelectObject(myHDC, hFont);
            DeleteObject(hFont);
        }

        /// Set text height
        void textHeight(int h)
        {
            myLogfont.lfHeight = h;
            HANDLE hFont = CreateFontIndirect(&myLogfont);
            hFont = (HFONT)SelectObject(myHDC, hFont);
            DeleteObject(hFont);
        }
        /// set text font name
        void textFontName(const std::string &fn)
        {
            strcpy(myLogfont.lfFaceName, fn.c_str());
            HANDLE hFont = CreateFontIndirect(&myLogfont);
            hFont = (HFONT)SelectObject(myHDC, hFont);
            DeleteObject(hFont);
        }
        int textWidthPixels(const std::string &t)
        {
            SIZE sz;
            GetTextExtentPoint32A(
                myHDC,
                t.c_str(),
                t.length(),
                &sz);
            return sz.cx;
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
        /** Construct top level window with no parent

    Application code should NOT use this constructor directly,
    nor that of any specilaization classes.  Intead use maker::make().
    */
        gui()
            : myParent(NULL), myBGColor(0xC8C8C8), myBGBrush(CreateSolidBrush(myBGColor)), myDeleteList(0), myfModal(false), myfEnabled(true), myfnobgerase(false), myToolTip(NULL), myAsyncReadCompleteMsgID(0), myCursorID(0)
        {
            myID = NewID();
            Create(
                NULL,
                "windex",
                WS_OVERLAPPEDWINDOW, WS_EX_CONTROLPARENT,
                0);

            /*  default resize event handler
            simply forces a refresh so partially visible widgets are correctly drawn
            Application code, if it needs to move child windows around,
            should overwrite this event handler.  Remember to call update() at end of event handler.
        */
            events().resize([this](int w, int h)
                            { update(); });

            /*  Construct font, initialized with default GUI font

            Each top level window keeps a font and associated logfont
            so that the font can be changed and inherited by all child windows
            */
            myLogFont = {0};
            GetObject(
                GetStockObject(DEFAULT_GUI_FONT),
                sizeof(myLogFont), &myLogFont);
            
            // default font clips descenders ( p, q, y) so increase height
            myLogFont.lfHeight = 18;

            myFont = CreateFontIndirectA(&myLogFont);
        }
        /** Construct child of a parent

        Application code should NOT use this constructor directly,
        nor that of any sepecilaization classes.  Intead use maker::make().

    */
        gui(
            gui *parent,
            const char *window_class = "windex",
            unsigned long style = WS_CHILD,
            unsigned long exstyle = WS_EX_CONTROLPARENT) : myParent(parent),
                                                           myDeleteList(0),
                                                           myfEnabled(true),
                                                           myToolTip(NULL),
                                                           myCursorID(IDC_ARROW)
        {
            // get a new unique ID
            myID = NewID();

            // create the window as requested
            Create(parent->handle(), window_class, style, exstyle, myID);

            // tell the parent that it has a new child
            parent->child(this);

            // by default show text with ID ( helps debugging )
            text("???" + std::to_string(myID));

            // inherit background color from parent
            bgcolor(parent->bgcolor());

            // inherit font from parent
            parent->font(myLogFont, myFont);
            SendMessage(
                myHandle,
                WM_SETFONT,
                (WPARAM)myFont,
                0);
        }
        virtual ~gui()
        {
            //std::cout << "deleting " << myText << "\n";
            DestroyWindow(myHandle);
            if (myDeleteList)
                myDeleteList->push_back(myHandle);
        }

        /// register child on this window
        void child(gui *w)
        {
            myChild.push_back(w);
        }

        /// get vector of children
        children_t &children()
        {
            return myChild;
        }

        gui * parent()
        {
            return myParent;
        }

        /// find child window with specified id
        gui *find(int id)
        {
            for (auto w : myChild)
            {
                if (w->id() == id)
                    return (gui *)w;
            }
            return nullptr;
        }

        void focus()
        {
            SetFocus(myHandle);
        }
        /** Change background color
        * @param[in] color in BGR format eg 0x0000FF for red
        */
        void bgcolor(int color)
        {
            myBGColor = color;
            DeleteObject(myBGBrush);
            myBGBrush = CreateSolidBrush(color);
            for (auto w : myChild)
                w->bgcolor(color);
        }

        void nobgerase()
        {
            myfnobgerase = true;
        }
        /// Enable/Disable, default enable
        void enable(bool f = true)
        {
            myfEnabled = f;
        }
        bool isEnabled() const
        {
            return myfEnabled;
        }

        /// Change font height for this and all child windows
        void fontHeight(int h)
        {
            myLogFont.lfHeight = h;
            createNewFont();
            setfont(myLogFont, myFont);
        }
        void fontName(const std::string &name)
        {
            strcpy(myLogFont.lfFaceName, name.c_str());
            createNewFont();
            setfont(myLogFont, myFont);
        }

        /** Change icon
        @param[in] iconfilename

        Use to set the application icon in the taskbar
    */
        void icon(const std::string &iconfilename)
        {
            HICON hIcon = ExtractIconA(
                NULL,
                iconfilename.c_str(),
                0);
            SetClassLongPtr(
                myHandle,
                GCLP_HICON,
                (LONG_PTR)hIcon);

            SendMessage(myHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessage(myHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(GetWindow(myHandle, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessage(GetWindow(myHandle, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        }
        void cursor(char *cursorID)
        {
            myCursorID = cursorID;
        }
        int id()
        {
            return myID;
        }
        int bgcolor() const
        {
            return myBGColor;
        }
        void text(const std::string &text)
        {
            myText = text;
            SetWindowText(myHandle, myText.c_str());
        }

        std::string text() const
        {
            return myText;
        }
        /** Add scrollbars
         * 
         * @param[in] fHoriz true if horizontal scroll reuired, default true
         * 
         */
        void scroll(bool fHoriz = true)
        {
            myfScrollHoriz = fHoriz;

            // Add scrollbars to window style
            LONG_PTR extra;
            if (fHoriz)
                extra = WS_HSCROLL | WS_VSCROLL;
            else
                extra = WS_VSCROLL;
            SetWindowLongPtr(
                myHandle,
                GWL_STYLE,
                GetWindowLongPtr(myHandle, GWL_STYLE) | extra);

            // Set the scrolling range and page size to defaaults
            scrollRange(100, 100);

            // horizontal scroll handler
            events().scrollH([this](int code)
                             {
                                 SCROLLINFO si;
                                 si.cbSize = sizeof(si);
                                 si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE;
                                 if (!GetScrollInfo(myHandle, SB_HORZ, &si))
                                     return;

                                 int oldPos = scrollMove(si, code);

                                 si.fMask = SIF_POS;
                                 SetScrollInfo(myHandle, SB_HORZ, &si, TRUE);
                                 GetScrollInfo(myHandle, SB_CTL, &si);

                                 RECT rect;
                                 GetClientRect(myHandle, &rect);
                                 int xs = oldPos - si.nPos;
                                 //std::cout << "scrollH " << xs <<" "<< oldPos <<" "<< si.nPos << "\n";
                                 ScrollWindow(
                                     myHandle,
                                     xs,
                                     0, NULL, NULL);

                                 for (auto &w : myChild)
                                     w->update();
                             });

            // vertical scroll handler
            events().scrollV([this](int code)
                             {
                                 SCROLLINFO si;
                                 si.cbSize = sizeof(si);
                                 si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE;
                                 if (!GetScrollInfo(myHandle, SB_VERT, &si))
                                     return;

                                 int oldPos = scrollMove(si, code);

                                 si.fMask = SIF_POS;
                                 SetScrollInfo(myHandle, SB_VERT, &si, TRUE);
                                 GetScrollInfo(myHandle, SB_VERT, &si);
                                 RECT rect;
                                 GetClientRect(myHandle, &rect);
                                 int ys = oldPos - si.nPos;
                                 ScrollWindow(
                                     myHandle,
                                     0,
                                     ys, // amount to scroll
                                     NULL, NULL);

                                 // update entire window and all children
                                 // this prevents visual artefacts on fast scrolling
                                 // but creates an unpleasant flicker
                                 // so it is commented out
                                 //update();

                                 // update any child windows
                                 // this has a fast and smooth appearance
                                 // but sometimes leaves fragments littering the window
                                 for (auto &w : myChild)
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
        void scrollRange(int width, int height)
        {

            /* maximum scroll position

            We want the max scroll position 
            which is the top of the visible portion
            to be placed where the bottom if the underlying window is just visible
            */

            RECT r;
            GetClientRect(myHandle, &r);
            int xmax = width - r.right;
            if (xmax < 0)
                xmax = 0;
            int ymax = height - (r.bottom - r.top) + 60;
            if (ymax < 0)
                ymax = 0;
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = ymax;
            si.nPage = ymax / 10;
            SetScrollInfo(myHandle, SB_VERT, &si, TRUE);
            if (myfScrollHoriz)
            {
                si.nMax = xmax;
                si.nPage = xmax / 10;
                SetScrollInfo(myHandle, SB_HORZ, &si, TRUE);
            }
        }

        /** Get mouse status
        @return sMouse structure containing x and y positions, etc
    */
        sMouse getMouseStatus()
        {
            sMouse m;
            POINT p;
            GetCursorPos(&p);
            if (!ScreenToClient(myHandle, &p))
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
            MSG msg = {};
            while (GetMessage(&msg, NULL, 0, 0))
            {
                //            std::cout << "gui::run " << msg.message << "\n";
                //            if( msg.message == 256 )
                //            {
                //                std::cout << "widget text: " << myText << "\n";
                //                int dbg = 0;
                //                continue;
                //            }
                if (!IsDialogMessage(myHandle, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        /** Add tooltip that pops up helpfully when mouse cursor hovers over widget
        @param[in] text of tooltip
        @param[in] width of multiline tooltip, default single line
    */
        void tooltip(const std::string &text, int width = 0)
        {
            TOOLINFO toolInfo = {0};
            toolInfo.cbSize = sizeof(toolInfo);
            toolInfo.hwnd = myHandle;
            toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            toolInfo.uId = (UINT_PTR)myHandle;
            toolInfo.lpszText = (char *)text.c_str();

            // check for existing tooltip
            if (!myToolTip)
            {
                // Create the tooltip.
                myToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
                                           WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           myHandle, NULL, NULL, NULL);
                SendMessage(myToolTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
            }

            else

                // change tooltip
                SendMessage(myToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolInfo);

            //std::cout << "tooltip: " << width << "\n" << text << "\n";

            if (width > 0)
                SendMessage(myToolTip, TTM_SETMAXTIPWIDTH, 0, width);
        }

        virtual LRESULT WindowMessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            //        if( uMsg != 132  && uMsg != 275 )
            //            std::cout << " widget " << myText << " WindowMessageHandler " << uMsg << "\n";
            if (hwnd == myHandle)
            {
                switch (uMsg)
                {
                case WM_CLOSE:

                    // Premission to close window requested

                    if (!modalMgr::get().canClose(myID))
                    {
                        // Cannot close when modal window running
                        return true;
                    }
                    if (myID == 1)
                    {
                        // check with registered QuitApp function
                        if (!myEvents.onQuitApp())
                        {
                            return true;
                        }
                    }
                    // close permission granted
                    DestroyWindow(myHandle);

                    return true;

                case WM_DESTROY:

                    // if this is the appliction window
                    // then quit application when destroyed
                    if (myID == 1)
                    {
                        PostQuitMessage(0);
                    }
                    return true;

                case WM_NCDESTROY:

                    // all the children are gone
                    // so a modal display can close
                    myfModal = false;
                    return false;

                    //            case WM_SETFOCUS:
                    //                std::cout << myText << "  got focus\n";
                    //                return true;

                case WM_ERASEBKGND:
                {
                    if (myfnobgerase)
                        return true;
                    RECT rc;
                    GetWindowRect(hwnd, &rc);
                    FillRect((HDC)wParam, &rc, myBGBrush);
                    return true;
                }

                case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(myHandle, &ps);
                    if (!myfnobgerase)
                        FillRect(ps.hdc, &ps.rcPaint, myBGBrush);
                    draw(ps);

                    EndPaint(myHandle, &ps);
                }
                    return true;

                case WM_CTLCOLORSTATIC:
                {
                    //                SetBkColor((HDC)wParam, myBGColor);
                    //                return (INT_PTR)myBGBrush;
                    RECT r;
                    GetBoundsRect(GetDC(myHandle), &r, 0);
                    FillRect(GetDC(myHandle), &r, myBGBrush);
                    SetBkMode((HDC)wParam, TRANSPARENT);

                    return (INT_PTR)GetStockObject(NULL_BRUSH);
                }

                case WM_LBUTTONDOWN:
                    //std::cout << "click on " << myText << "\n";
                    if (!myfEnabled)
                        return true;
                    if (myEvents.onLeftdown())
                        return true;
                    // the event was not completely handled, maybe the parent can look after it
                    if (myParent)
                    {
                        if (myParent->WindowMessageHandler(
                                myParent->handle(),
                                uMsg, wParam, lParam))
                            return true;
                    }
                    break;

                case WM_RBUTTONDOWN:
                    myEvents.onRightDown();
                    break;

                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                    myEvents.onMouseUp();
                    return true;

                case WM_MOUSEMOVE:
                    myEvents.onMouseMove(wParam, lParam);
                    break;

                case WM_MOUSEWHEEL:
                {
                    int d = HIWORD(wParam);
                    if (d > 0xEFFF)
                        d = -120;
                    myEvents.onMouseWheel(d);
                }
                break;

                case WM_MOUSELEAVE:
                    myEvents.onMouseLeave();
                    break;

                case WM_SIZE:
                    myEvents.onResize(LOWORD(lParam), HIWORD(lParam));
                    return true;

                case WM_HSCROLL:
                    if (lParam)
                        trackbarMessageHandler((HWND)lParam);
                    else
                        myEvents.onScrollH(LOWORD(wParam));
                    return true;

                case WM_VSCROLL:
                    std::cout << "VSCROLL\n";
                    if (lParam)
                        trackbarMessageHandler((HWND)lParam);
                    else
                        myEvents.onScrollV(LOWORD(wParam));
                    return true;

                case WM_COMMAND:
                {
                    // https://docs.microsoft.com/en-us/windows/win32/menurc/wm-command

                    auto wp_hi = HIWORD(wParam);
                    if (!wp_hi)
                    {
                        events().onMenuCommand(wParam);
                        return true;
                    }

                    if (wp_hi == CBN_SELCHANGE || wp_hi == LBN_SELCHANGE)
                    {
                        return events().onSelect(LOWORD(wParam));
                    }

                    if (wp_hi == EN_CHANGE)
                    {
                        return events().onChange(LOWORD(wParam));
                    }
                    return true;
                }

                case WM_TIMER:
                    events().onTimer((int)wParam);
                    return true;

                case WM_DROPFILES:
                    events().onDropStart((HDROP)wParam);
                    return true;

                case WM_GETDLGCODE:
                    events().onKeydown((int)wParam);
                    return DLGC_WANTARROWS;

                case WM_KEYDOWN:
                    events().onKeydown((int)wParam);
                    return true;

                case WM_SETCURSOR:
                    if (myCursorID)
                    {
                        SetCursor(LoadCursor(NULL, (LPCSTR)myCursorID));
                        return true;
                    }
                    return false;

                case eventMsgID::asyncReadComplete:
                    events().onAsyncReadComplete(wParam);
                    return true;

                case eventMsgID::tcpServerAccept:
                    events().onTcpServerAccept();
                    return true;

                case eventMsgID::tcpServerReadComplete:
                    events().onTcpServerReadComplete();
                    return true;
                }
            }
            else
            {
                for (auto w : myChild)
                {
                    if (w->WindowMessageHandler(hwnd, uMsg, wParam, lParam))
                        return true;
                }
            }

            return false;
        }

        /// Show window and all children
        virtual void show(bool f = true)
        {
            int cmd = SW_SHOWDEFAULT;
            if (!f)
                cmd = SW_HIDE;
            //std::cout << "show " << myText <<" "<< myHandle <<" "<< myChild.size() << "\n"; ;
            ShowWindow(myHandle, cmd);
            // display any children
            for (auto w : myChild)
                w->show(f);
        }

        /// Show this window and suspend all other windows interactions until this is closed
        void showModal()
        {
            myfModal = true;
            if (!modalMgr::get().set(myID, myHandle))
                return;
            show();

            // prevent other windows from interaction
            // by running our own message loop
            //std::cout << "-> modal msg loop\n";
            MSG msg = {};
            while (GetMessage(&msg, NULL, 0, 0))
            {
                if (!IsDialogMessage(myHandle, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    switch (msg.message)
                    {
                    case WM_CLOSE:
                        std::cout << myText << " WM_CLOSE";
                        break;

                    case WM_DESTROY:
                        std::cout << myText << " WM_DESTROY";
                        myfModal = false;
                        break;
                    }
                }
                // window no longer modal
                // so break out of our own message loop
                if (!myfModal)
                    break;
            }
            //std::cout << "<- modal msg loop\n";
        }
        /// Stop modal interaction and close window
        void endModal()
        {
            std::cout << myText << " endModal\n";
            myfModal = false;
            modalMgr::get().set(0, 0);
            DestroyWindow(myHandle);
            if (myDeleteList)
                myDeleteList->push_back(myHandle);
        }

        /** force widget to redraw completely

    Windex makes no effort to auto update the screen display.
    If application code alters something and the change should be seen immediatly
    then update() should be called, either on the widget that has changed
    or the top level window that contains the changed widgets.

    */
        void update()
        {
            InvalidateRect(myHandle, NULL, true);
            UpdateWindow(myHandle);
        }

        /** Move the window
        @param[in] r specify location and size
        r contains 4 values, { x, y, width, height }
    */
        void move(const std::vector<int> &r)
        {
            if (r.size() != 4)
                return;
            MoveWindow(myHandle,
                       r[0], r[1], r[2], r[3], false);
        }
        /** Change size without moving top left corner
        @param[in] w width
        @param[in] h height
    */
        void size(int w, int h)
        {
            RECT rect;
            GetWindowRect(myHandle, &rect);
            MoveWindow(myHandle,
                       rect.left, rect.top, w, h,
                       false);
        }
        /** Change position without changing size
        @param[in] x left
        @param[in] y top
    */
        void move(int x, int y)
        {
            RECT rect;
            GetClientRect(myHandle, &rect);
            MoveWindow(myHandle,
                       x, y, rect.right - rect.left, rect.bottom - rect.top,
                       false);
        }
        void move(int x, int y, int w, int h)
        {
            MoveWindow(myHandle,
                       x, y, w, h, false);
        }
        /** Size of window client area
        @return vector [0] width of window in pixels [1] height of window in pixels
    */
        std::vector<int> size()
        {
            RECT r;
            GetClientRect(myHandle, &r);
            std::vector<int> ret{
                r.right - r.left, r.bottom - r.top};
            return ret;
        }
        std::vector<int> lefttop()
        {
            RECT rp;
            GetWindowRect(myParent->handle(), &rp);
            RECT r;
            GetWindowRect(myHandle, &r);
            //        std::cout << "parent " << rp.left <<" "<< rp.top
            //                  << " child " << r.left <<" "<< r.top << "\n";
            static std::vector<int> ret(2);
            ret[0] = r.left - rp.left;
            ret[1] = r.top - rp.top;
            return ret;
        }

        /// Get event handler
        eventhandler &events()
        {
            return myEvents;
        }

        /// get window handle
        HWND handle()
        {
            return myHandle;
        }

        /// set delete list for when gui is detroyed
        void delete_list(std::vector<HWND> *list)
        {
            myDeleteList = list;
        }

        /// change font for this and all child windows
        void setfont(LOGFONT &logfont, HFONT &font)
        {
            myLogFont = logfont;
            myFont = font;
            SendMessage(
                myHandle,
                WM_SETFONT,
                (WPARAM)myFont,
                0);
            for (auto w : myChild)
                w->setfont(myLogFont, myFont);
        }
        void setAsyncReadCompleteMsgID(int id)
        {
            myAsyncReadCompleteMsgID = id;
        }

    protected:
        HWND myHandle;
        gui *myParent;
        eventhandler myEvents;
        int myBGColor;
        HBRUSH myBGBrush;
        LOGFONT myLogFont;
        HFONT myFont;
        std::vector<HWND> *myDeleteList;
        std::string myText;
        int myID;
        std::vector<gui *> myChild; ///< gui elements to be displayed in this window
        bool myfModal;              ///< true if element is being shown as modal
        bool myfEnabled;            ///< true if not disabled
        bool myfnobgerase;
        HWND myToolTip; /// handle to tooltip control for this gui element
        unsigned int myAsyncReadCompleteMsgID;
        char *myCursorID;
        bool myfScrollHoriz;

        /** Create the managed window
        @param[in] parent handle of parent window
        @param[in] window_class controls which callback function handles window messages
        @param[in] style
        @param[in] exstyle
        @param[in] id identifies which control generated notification
     */
        void Create(
            HWND parent,
            const char *window_class,
            DWORD style, DWORD exstyle = 0,
            int id = 0)
        {
            myHandle = CreateWindowEx(
                exstyle,      // Optional window styles.
                window_class, // Window class
                "widget",     // Window text
                style,        // Window style

                // Size and position
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                parent,                      // Parent window
                reinterpret_cast<HMENU>(id), // Menu or control id
                NULL,                        // Instance handle
                NULL                         // Additional application data
            );
        }

        /** get font details
        @param[out] logfont logical font
        @param[out] font

       Used by child window constructor
       to inherit font from parent.
    */
        void font(LOGFONT &logfont, HFONT &font)
        {
            logfont = myLogFont;
            font = myFont;
        }

        /// Replace font used by this and child windows from logfont
        void createNewFont()
        {
            DeleteObject(myFont);
            myFont = CreateFontIndirectA(&myLogFont);
        }

        virtual void draw(PAINTSTRUCT &ps)
        {
            SetBkColor(
                ps.hdc,
                myBGColor);
            int color = 0x000000;
            if (!myfEnabled)
                color = 0xAAAAAA;
            SetTextColor(
                ps.hdc,
                color);
            if (myParent)
            {
                SelectObject(ps.hdc, myFont);

                RECT r(ps.rcPaint);
                auto hbrBkgnd =CreateSolidBrush(myBGColor); 
                FillRect(
                    ps.hdc,
                    &r,
                    hbrBkgnd );
                DeleteObject(hbrBkgnd); 
                r.left += 1;
                r.top += 1;
                DrawText(
                    ps.hdc,
                    myText.c_str(),
                    -1,
                    &r,
                    0);
            }
            myEvents.onDraw(ps);
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
        int scrollMove(SCROLLINFO &si, int code)
        {
            int oldPos = si.nPos;
            switch (code)
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
        void trackbarMessageHandler(HWND hwnd)
        {
            std::cout << "trackbarMessageHandler\n";
            // trackbar notifications are sent to trackbar's parent window
            // find the child that generated this notification
            // get the tackbar position and call the slid event handler
            for (auto c : myChild)
            {
                if (c->handle() == hwnd)
                {
                    c->events().onSlid(
                        SendMessage(
                            hwnd,
                            TBM_GETPOS,
                            (WPARAM)0, (LPARAM)0));
                }
            }
        }
    };

    /// A child window that can contain widgets
    class panel : public gui
    {
    public:
        panel(gui *parent)
            : gui(parent)
        {
            text("");
        }
    };
    /** \brief A widget where users can drop files dragged from windows explorer.

<pre>
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,500,400});
    form.text("Drop files demo");

    // widget for receiving dropped files
    drop& dropper = wex::maker::make<wex::drop>( form );
    dropper.move( 10,10,490,390 );
    label& instructions = wex::maker::make<wex::label>( dropper );
    instructions.move(30,30,400,200);
    instructions.text("Drop files here");

    // dropped files event handler
    dropper.events().drop( [&](const std::vector<std::string>& files )
    {
        // display list of dropped files
        std::string msg;
        msg = "Files dropped:\n";
        for( auto& f : files )
            msg += f + "\n ";
        instructions.text( msg );
        instructions.update();
    });

    form.show();
</pre>
*/
    class drop : public gui
    {
    public:
        drop(gui *parent)
            : gui(parent)
        {
            text("");

            // register as drop recipient
            DragAcceptFiles(myHandle, true);

            // handle drop event
            myEvents.dropStart([this](HDROP hDrop)
                               {
                                   int count = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
                                   if (count)
                                   {
                                       // extract files from drop structure
                                       std::vector<std::string> files;
                                       char fname[MAX_PATH];
                                       for (int k = 0; k < count; k++)
                                       {
                                           DragQueryFileA(hDrop, k, fname, MAX_PATH);
                                           files.push_back(fname);
                                       }
                                       // call app code's event handler
                                       myEvents.onDrop(files);
                                   }
                                   DragFinish(hDrop);
                               });
        }
    };

    /// Displaying a title and a box
    class groupbox : public panel
    {
    public:
        groupbox(gui *parent)
            : panel(parent)
        {
        }
        /** Set size and location of group box
        @param[in] r x, y, width, heeight of groupbox
    */
        void move(const std::vector<int> &r)
        {
            if (r.size() != 4)
                return;

            // store location and size of groupbox
            myRect.left = r[0];
            myRect.top = r[1];
            myRect.right = r[0] + r[2];
            myRect.bottom = r[1] + r[3];

            // set location and size of groupbox label
            MoveWindow(myHandle,
                       r[0] + 5, r[1] + 2, 60, 25, false);
        }
        virtual void draw(PAINTSTRUCT &ps)
        {
            //Draw group box on parent window
            HDC hdc = GetDC(myParent->handle());
            DrawEdge(
                hdc,
                &myRect,
                EDGE_BUMP,
                BF_RECT);
            ReleaseDC(myParent->handle(), hdc);

            // Draw label
            SetBkColor(
                ps.hdc,
                myBGColor);
            SelectObject(ps.hdc, myFont);
            DrawText(
                ps.hdc,
                myText.c_str(),
                myText.length(),
                &ps.rcPaint,
                0);
        }

    private:
        RECT myRect;
    };

    /// \brief A panel which arranges the widgets it contains in a grid.
    class layout : public panel
    {
    public:
        layout(gui *parent)
            : panel(parent), myColCount(2), myfWidthsSpecified(false), myfColFirst(false)
        {
        }
        /** Specify number of cols to use for layout.

    The child windows will be laid out in the specified number of columns
    with the required number of rows to show them all.

    The space between columns and rows will be adjusted so that
    all child windows will be visible.

    The size of the child windows will not be altered.

    */
        void grid(int cols)
        {
            myColCount = cols;
        }
        /** Specify column widths.
        @param[in] vw vector of widths for each column
    */
        void colWidths(const std::vector<int> &vw)
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
        void colfirst(bool f = true)
        {
            myfColFirst = f;
        }
        void draw(PAINTSTRUCT &ps)
        {
            if (!myChild.size())
                return;
            RECT r;
            GetClientRect(myHandle, &r);
            if (!myfWidthsSpecified)
            {
                // col widths not specified, default to all the same width to fill panel
                int colwidth = (r.right - r.left) / myColCount;
                myWidths.clear();
                for (int k = 0; k < myColCount; k++)
                {
                    myWidths.push_back(colwidth);
                }
            }
            int rowheight;
            if (!myfColFirst)
                rowheight = (r.bottom - r.top) / ((myChild.size() + 1) / myColCount);
            else
                rowheight = 50;

            // display the children laid out in a grid
            int colcount = 0;
            int rowcount = 0;
            int x = 0;

            if (!myfColFirst)
            {
                for (auto w : myChild)
                {
                    w->move(x, rowcount * rowheight);
                    w->update();

                    x += myWidths[colcount];
                    colcount++;
                    if (colcount >= myColCount)
                    {
                        colcount = 0;
                        x = 0;
                        rowcount++;
                    }
                }
            }
            else
            {
                for (auto w : myChild)
                {
                    w->move(x, rowcount * rowheight);
                    w->update();
                    rowcount++;
                    if (rowcount >= (int)myChild.size() / myColCount)
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
        bool myfWidthsSpecified; // true if app code specified column widths
        bool myfColFirst;        // true if columns should be filled first
    };

    /// A widget that user can click to start an action.
    class button : public gui
    {
    public:
        button(gui *parent)
            : gui(parent), myBitmap(NULL)
        {
            myBGColor = 0xC8C8C8;
        }

        /** Specify bitmap image to be used for button, read from file
        @param[in] name of file
    */
        void imageFile(const std::string &name)
        {
            myBitmap = (HBITMAP)LoadImage(
                NULL, name.c_str(), IMAGE_BITMAP,
                0, 0, LR_LOADFROMFILE);
        }
        /** Specify bitmap image to be used for button, read from resource
        @param[in] name of resource
        @return 0 for no error

        The image is stored in the executable.

        Specify images to be built into the executable in a .rc file

    <pre>
        MAINICON ICON "app.ico"
        ZOOM_IN_BLACK BITMAP "zoom_in_black.bmp"
        ZOOM_IN_RED BITMAP "zoom_in_red.bmp"
    </pre>

        On error, will set button to blank.

    */
        int imageResource(const std::string &name)
        {
            int ret = 0;
            auto h = GetModuleHandleA(NULL);
            if (!h)
                ret = 1;
            myBitmap = LoadBitmap(
                h, name.c_str());
            if (!myBitmap)
                ret = 2;
            if (ret)
                text("");
            return ret;
        }

    protected:
        HBITMAP myBitmap;

        /// draw
        virtual void draw(PAINTSTRUCT &ps)
        {
            if (!myBitmap)
            {
                // button with text

                int color = 0x000000;
                if (!myfEnabled)
                    color = 0xAAAAAA;
                SetTextColor(
                    ps.hdc,
                    color);
                SetBkColor(
                    ps.hdc,
                    myBGColor);

                SelectObject(ps.hdc, myFont);

                RECT r(ps.rcPaint);
                auto hbrBkgnd =CreateSolidBrush(myBGColor); 
                FillRect(
                    ps.hdc,
                    &r,
                    hbrBkgnd );
                DeleteObject(hbrBkgnd); 

                r.left += 1;
                r.top += 1;
                DrawText(
                    ps.hdc,
                    myText.c_str(),
                    -1,
                    &r,
                    DT_SINGLELINE | DT_CENTER | DT_VCENTER);

                DrawEdge(
                    ps.hdc,
                    &ps.rcPaint,
                    EDGE_RAISED,
                    BF_RECT);
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
        radiobutton(gui *parent)
            : gui(parent), myValue(false)
        {
            // Add to current group
            group().back().push_back(this);
            myGroup = group().size() - 1;

            // set the boolean value when clicked
            events().clickWex([this]
                              {
                                  if (!myfEnabled)
                                      return;
                                  // set all buttons in group false
                                  for (auto b : group()[myGroup])
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
                this);

            if (this_it == group()[myGroup].end())
                throw std::runtime_error("wex::radiobutton::first error in group");

            // if button is first in group, nothing is needed
            if (this_it == group()[myGroup].begin())
                return;

            // construct new group
            std::vector<radiobutton *> g;
            group().push_back(g);

            // copy button and following buttons in same group to new group
            for (
                auto it = this_it;
                it != group()[myGroup].end();
                it++)
            {
                group().back().push_back(*it);
            }

            // erase from old group
            group()[myGroup].erase(
                this_it,
                group()[myGroup].end());

            // tell buttons that were moved about their new group
            for (auto b : group().back())
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
            for (auto b : group()[myGroup])
            {
                if (b->isChecked())
                    break;
                off++;
            }
            if (off < (int)group()[myGroup].size())
                return off;
            return -1;
        }

        /// set value true( default ) or false
        void check(bool f = true)
        {
            if (f)
            {
                // set all buttons in group false
                for (auto b : group()[myGroup])
                {
                    b->myValue = false;
                    b->update();
                }
            }
            myValue = f;
            update();
        }

        virtual void draw(PAINTSTRUCT &ps)
        {
            SelectObject(ps.hdc, myFont);
            int color = 0x000000;
            if (!myfEnabled)
                color = 0xAAAAAA;
            SetTextColor(
                ps.hdc,
                color);
            SetBkColor(
                ps.hdc,
                myBGColor);
            RECT r(ps.rcPaint);
            r.left += 20;
            shapes S(ps);

            DrawText(
                ps.hdc,
                myText.c_str(),
                -1,
                &r,
                0);
            if (!myValue)
            {
                S.circle(10, 10, 5);
            }
            else
            {
                SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
                Ellipse(ps.hdc, 5, 5, 15, 15);
            }
        }

    private:
        bool myValue;
        int myGroup; /// index of group button belongs to

        /// get reference to radiobutton groups
        std::vector<std::vector<radiobutton *>> &group()
        {
            static std::vector<std::vector<radiobutton *>> theGroups;
            static bool fGroupInit = false;
            if (!fGroupInit)
            {
                // create first group
                fGroupInit = true;
                std::vector<radiobutton *> g;
                theGroups.push_back(g);
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
        checkbox(gui *parent)
            : gui(parent), myType(eType::check), myValue(false)
        {
            // toggle the boolean value when clicked
            events().clickWex([this]
                              {
                                  if (!myfEnabled)
                                      return;
                                  myValue = !myValue;
                                  update();
                              });
        }
        /// set type to plus, useful to indicate expanded or collapsed property categories
        void plus(bool f = true)
        {
            if (f)
                myType = eType::plus;
            else
                myType = eType::check;
        }
        void check(bool f = true)
        {
            myValue = f;
        }
        bool isChecked()
        {
            return (myValue);
        }
        virtual void draw(PAINTSTRUCT &ps)
        {
            SetBkColor(
                ps.hdc,
                myBGColor);
            RECT r(ps.rcPaint);
            int cbg = r.bottom - r.top - 10;
            r.left += cbg + 5;
            r.top -= 2;

            shapes S(ps);
            S.textHeight(myLogFont.lfHeight);
            S.textFontName( myLogFont.lfFaceName );
            S.text(myText, {r.left, r.top, r.right, r.bottom});
            S.rectangle({0, 0, cbg, cbg});
            S.fill();
            S.penThick(2);
            S.color(0);
            switch (myType)
            {
            case eType::check:
                if (myValue)
                {
                    S.line({2, cbg / 2, cbg / 2 - 1, cbg - 2});
                    S.line({cbg / 2, cbg - 3, cbg - 4, 3});
                }
                break;
            case eType::plus:
                S.line({2, cbg / 2, cbg - 2, cbg / 2});
                if (myValue)
                    S.line({1 + cbg / 2, 2, 1 + cbg / 2, cbg - 2});
                break;
            }
            S.penThick(1);
        }
        void clickFunction(std::function<void(void)> f)
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
        /// CTOR for simple message box with OK button
        msgbox(
            const std::string &msg)
        {
            myReturn = MessageBox(NULL,
                                  msg.c_str(),
                                  "Message",
                                  MB_OK);
        }
        /// CTOR for message box with title and configurable buttons
        msgbox(
            gui &parent,
            const std::string &msg,
            const std::string &title,
            unsigned int type)
        {
            myReturn = MessageBox(parent.handle(),
                                  msg.c_str(),
                                  title.c_str(),
                                  type);
        }
        int myReturn; ///< Button id clicked by user
    };

    /// A widget that displays a string.
    class label : public gui
    {
    public:
        label(gui *parent)
            : gui(parent)
        {
        }
    };
    /** \brief A widget where user can enter a single line string.
<pre>
    // construct top level window
    gui& form = maker::make();
    form.move({ 50,50,400,400});
    form.text("Label and Editbox demo");

    // display labels
    label& lbA = maker::make<label>( form );
    lbA.move( {20, 20, 100, 30 } );
    lbA.text("A:");
    label& lbB = maker::make<label>( form );
    lbB.move( {20, 60, 100, 30 } );
    lbB.text("B:");

    // display textboxes
    editbox& edit1 = maker::make<editbox>( form );
    edit1.move( {80, 20, 100, 30 } );
    edit1.text( "type value");
    editbox& edit2 = maker::make<editbox>( form );
    edit2.move( {80, 60, 100, 30 } );
    edit2.text( "type value");

    // display a button
    button& btn = wex::maker::make<button>( form );
    btn.move( {20, 100, 150, 30 } );
    btn.text( "Show values entered" );
    btn.tooltip("tooltip explaining button function");

    // popup a message box when button is clicked
    // showing the value entered in textbox
    btn.events().click([&]
    {
        std::string msg =
        "A is " + edit1.text() +
        ", B is " + edit2.text();
        msgbox(
            form,
            msg );
    });

    form.show();
</pre>
*/
    class editbox : public gui
    {
    public:
        editbox(gui *parent)
            : gui(parent, "Edit",
                  WS_CHILD | ES_LEFT | WS_BORDER | WS_VISIBLE,
                  WS_EX_CLIENTEDGE)
        {
        }
        /// editbox generated a notification - nop
        void notification(WORD ntf)
        {
            std::cout << "editbox notification " << ntf << "\n";
            if (ntf == EN_KILLFOCUS)
            {
                std::cout << "done\n";
            }
        }
        void text(const std::string &t)
        {
            SetDlgItemText(
                myParent->handle(),
                myID,
                t.c_str());
        }
        /// get text in textbox
        std::string text()
        {
            char buf[1000];
            buf[0] = '\0';
            GetWindowText(
                handle(),
                buf,
                999);
            return std::string(buf);
        }
        /// disable ( or enable ) user editing
        void readonly(bool f = true)
        {
            SendMessage(
                handle(),
                EM_SETREADONLY,
                (WPARAM)f, (LPARAM)0);
        }
    };

    /// A mutiline editbox
    class multiline : public gui
    {
    public:
        multiline(gui *parent)
            : gui(parent, "Edit",
                  WS_CHILD | ES_LEFT | WS_BORDER | WS_VISIBLE | ES_MULTILINE | ES_WANTRETURN,
                  WS_EX_CLIENTEDGE)
        {
        }
        /** Set text
        @param[in] t the text
        Line breaks must be specified by "\r\n"
    */
        void text(const std::string &t)
        {
            SetDlgItemText(
                myParent->handle(),
                myID,
                t.c_str());
        }
        /// get text in textbox
        std::string text()
        {
            char buf[1000];
            buf[0] = '\0';
            GetWindowText(
                handle(),
                buf,
                999);
            return std::string(buf);
        }
    };
    /// A widget where user can choose from a dropdown list of strings
    class choice : public gui
    {
    public:
        choice(gui *parent)
            : gui(parent, "Combobox",
                  CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE)
        {
        }
        /// Override move to ensure height is sufficient to allow dropdown to apprear
        void move(int x, int y, int w, int h)
        {
            if (h < 200)
                h = 200;
            gui::move(x, y, w, h);
        }
        /// set item height in drop doown list
        void itemHeight(int h)
        {
            SendMessage(
                handle(),
                CB_SETITEMHEIGHT,
                (WPARAM)0, (LPARAM)20);

            /* WPARAM = 1 is supposed to set the "selection box" height
        but it appears to do nothing */
            //        SendMessage(
            //            handle(),
            //            CB_SETITEMHEIGHT,
            //            (WPARAM)1, (LPARAM)40);
        }

        /// Add an option
        void add(const std::string &s)
        {
            SendMessageA(
                handle(),
                (UINT)CB_ADDSTRING,
                (WPARAM)0,
                (LPARAM)s.c_str());
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
        void select(int i)
        {
            SendMessage(
                handle(),
                CB_SETCURSEL,
                (WPARAM)i, (LPARAM)0);
        }
        /** Select by string
        @param[in] s the string to select
    */
        void select(const std::string &s)
        {
            SendMessage(
                handle(),
                CB_SELECTSTRING,
                (WPARAM)-1, (LPARAM)s.c_str());
        }
        /// get index of selected item
        int selectedIndex()
        {
            return SendMessage(
                handle(),
                (UINT)CB_GETCURSEL,
                (WPARAM)0, (LPARAM)0);
        }
        /// get text of selected item
        std::string selectedText()
        {
            int i = selectedIndex();
            if (i < 0)
                return std::string("");
            return text(i);
        }
        /// get text by index
        std::string text(int i)
        {
            char buf[256];
            SendMessage(
                handle(),
                (UINT)CB_GETLBTEXT,
                (WPARAM)i,
                (LPARAM)buf);
            return std::string(buf);
        }
        /// get count of items
        int count()
        {
            return SendMessage(
                handle(),
                (UINT)CB_GETCOUNT,
                (WPARAM)0, (LPARAM)0);
        }
    };

    /** A widget where user can choose from a list of strings
     * 
     * Event: select handler
     * 
   <pre>
        list.events().select(
        list.id(), [this,&list]
        { 
            auto s = list.selectedText();
        });
    </pre>
    */
    class list : public gui
    {
    public:
        list(gui *parent)
            : gui(parent, "listbox",
                  LBS_NOTIFY | WS_VSCROLL | WS_BORDER |
                      WS_CHILD | WS_OVERLAPPED | WS_VISIBLE)
        {
        }
        /// Override move to ensure column width is sufficient
        void move(int x, int y, int w, int h)
        {
            gui::move(x, y, w, h);
            SendMessageA(
                handle(),
                (UINT)LB_SETCOLUMNWIDTH,
                (WPARAM)w,
                (LPARAM)0);
        }
        /// Add an option
        void add(const std::string &s)
        {
            SendMessageA(
                handle(),
                (UINT)LB_ADDSTRING,
                (WPARAM)0,
                (LPARAM)s.c_str());
        }
        /// Clear all options
        void clear()
        {
            SendMessage(
                handle(),
                LB_RESETCONTENT,
                (WPARAM)0, (LPARAM)0);
        }
        /** Select by index
        @param[in] i index of item to selecct, -1 clears selection
    */
        void select(int i)
        {
            SendMessage(
                handle(),
                LB_SETCURSEL,
                (WPARAM)i, (LPARAM)0);
        }
        /** Select by string
        @param[in] s the string to select
    */
        void select(const std::string &s)
        {
            SendMessage(
                handle(),
                LB_SELECTSTRING,
                (WPARAM)-1, (LPARAM)s.c_str());
        }
        /** get index of selected item
        @return 0-based index, or -1 if no selection
    */
        int selectedIndex()
        {
            return SendMessage(
                handle(),
                (UINT)LB_GETCURSEL,
                (WPARAM)0, (LPARAM)0);
        }

        /// get text of selected item
        std::string selectedText()
        {
            int i = selectedIndex();
            if (i < 0)
                return std::string("");
            char buf[256];
            SendMessage(
                handle(),
                (UINT)LB_GETTEXT,
                (WPARAM)i,
                (LPARAM)buf);
            return std::string(buf);
        }
        /// get count of items
        int count()
        {
            return SendMessage(
                handle(),
                (UINT)LB_GETCOUNT,
                (WPARAM)0, (LPARAM)0);
        }
    };

    /** A class containing a database of the current gui elements

This looks after directing messages to their intended gui element.

It should NOT be used by application code.

*/
    class windex
    {
    public:
        /** get reference to windex gui framework ( singleton )

    This creates a singleton instance of the windex class
    and returns as many references to it as needed.

    The windex ctor should never be called by app code - it is private!

    */
        static windex &get()
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
            auto w = get().myGui.find(hwnd);
            if (w != get().myGui.end())
            {
                if (uMsg == WM_GETDLGCODE)
                    return w->second->WindowMessageHandler(hwnd, uMsg, wParam, lParam);

                if (w->second->WindowMessageHandler(hwnd, uMsg, wParam, lParam))
                    return 0;
            }

            // run default message processing
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        /// Add new gui element
        gui *Add(gui *g)
        {
            // delete any destroyed elements
            Delete();

            // provide reference to delete list, so new gui element can be removed after destruction
            g->delete_list(&myDeleteList);

            // add to existing gui elements
            myGui.insert(std::make_pair(g->handle(), g));

            //std::cout << "windexAdd " << myGui.size() <<" in "<< this << "\n";

            return g;
        }

        mgui_t myGui; ///< map of existing gui elements
    private:
        std::vector<HWND> myDeleteList; ///< gui elements that have been deleted but not yet removed from map

        windex()
        {
            // register a callback function
            // to be invoked every time a windex gui element receives a windows message
            WNDCLASS wc = {};
            wc.lpfnWndProc = &windex::WindowProc;
            wc.hInstance = NULL;
            wc.lpszClassName = "windex";
            wc.hbrBackground = CreateSolidBrush(0xc8c8c8);
            RegisterClass(&wc);
        }

        /// remove destroyed gui elements
        void Delete()
        {
            //std::cout << "windex::Delete " << myDeleteList.size() << "\n";
            for (auto h : myDeleteList)
            {
                auto i = myGui.find(h);
                if (i != myGui.end())
                    myGui.erase(i);
            }
            myDeleteList.clear();
        }
    };

    /** \brief A drop down list of options that user can click to start an action.

<pre>
    // construct top level window
    gui& form = wex::windex::topWindow();
    form.move({ 50,50,400,400});
    form.text("Menu demo");

     int clicked = -1;

    menu m( form );
    m.append("test",[&](const std::string &title)
    {
        clicked = 1;
    });
    m.append("second",[&](const std::string &title)
    {
        clicked = 2;
    });
    m.append("third",[&](const std::string &title)
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
        menu(gui &parent)
            : myM(CreatePopupMenu()), myParent(parent)
        {
        }
        ~menu()
        {
            DestroyMenu(myM);
        }
        /** Append menu item.
        @param[in] title
        @param[in] f function to be run when menu item clicked

        The function signature is void f( const std::string& title ).

        The title can be used by the function
        if it needs to know the menue item title that caused execution
        i.e when several items invoke the same function
    */
        void append(
            const std::string &title,
            const std::function<void(const std::string &)> &f = [](const std::string &title) {})
        {
            AppendMenu(
                myM,
                0,
                myParent.events().menuCommand(f, title),
                title.c_str());
        }

        /** Append submenu
        @param[in] title
        @param[in] submenu
    */
        void append(
            const std::string &title,
            menu &submenu)
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
            int x, int y)
        {
            TrackPopupMenu(
                myM,
                0,
                x, y,
                0,
                myParent.handle(),
                NULL);
        }

        HMENU handle()
        {
            return myM;
        }
        /** Set or unset check mark beside menu item
        @param[in] index 0-based index of menu item
        @param[in] f true if menu item is to be checked, default true
        @return true if the menu item was previously checked
    */
        bool check(int index, bool f = true)
        {
            unsigned int uCheck;
            if (f)
                uCheck = MF_BYPOSITION | MF_CHECKED;
            else
                uCheck = MF_BYPOSITION | MF_UNCHECKED;
            return MF_CHECKED == CheckMenuItem(
                                     myM,
                                     index,
                                     uCheck);
        }
        /// Number of items in menu
        int size()
        {
            return GetMenuItemCount(myM);
        }

    private:
        HMENU myM;
        gui &myParent;
    };

    /// A widget that displays across top of a window and contains a number of dropdown menues.
    class menubar
    {
    public:
        menubar(gui &parent)
            : myParent(parent), myM(CreateMenu())
        {
            // attach menu to window
            SetMenu(parent.handle(), myM);
        }
        /** Append menu to menubar
        @param[in] title that appears in the menubar
        @param[in] m menu that drops down when title clicked
    */
        void append(
            const std::string &title,
            menu &m)
        {
            AppendMenu(
                myM,
                MF_POPUP,
                (UINT_PTR)m.handle(),
                title.c_str());
            DrawMenuBar(myParent.handle());
        }

    private:
        gui &myParent;
        HMENU myM;
    };
    /** \brief Generate events at regularly timed intervals.

<pre>
    myDriveTimer = new wex::timer( fm, 50 );
    fm.events().timer([this](int id)
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
        @param[in] id number

        The events will begin immediatly on construction
        and stop on destruction - don't let the timer go out of scope!

        The id number will be passed as a parameter to the event handler
    */
        timer(gui &g, int intervalmsecs, int id = 1)
            : myGUI(g), myID(id)
        {
            SetTimer(
                myGUI.handle(),   // handle to  window
                myID,             // timer identifier
                intervalmsecs,    //  interval ms
                (TIMERPROC)NULL); // no timer callback
        }
        ~timer()
        {
            KillTimer(
                myGUI.handle(),
                myID);
        }

    private:
        gui &myGUI;
        int myID;
    };

}

#include "widgets.h"

namespace wex
{

    /** A class for making windex objects.

Use the methods of this class to create windex objects,
NOT the constructors of the objects.

*/
    class maker
    {
    public:
        /** Construct widget
            @param[in] parent reference to parent window or widget
            @return reference to new widget
    */
        template <class W, class P>
        static W &make(P &parent)
        {
            return *((W *)windex::get().Add(new W((gui *)&parent)));
        }

        /** Construct a top level window ( first call constructs application window )
        @return reference to new window
    */
        static gui &make()
        {
            return *windex::get().Add(new gui());
        }
    };

    /** \brief A widget where user can select which panel to display by clicking a tab button

Usage:
<pre>
    // construct top level window
    gui& form = wex::maker::make();
    form.move({ 50,50,400,400});
    form.text("Tabbed Panel demo");

    // construct tabbed panel
    tabbed& tabs = maker::make<tabbed>( form );
    tabs.move( 50,50, 300, 200 );

    // add some demo panels
    panel& cam1panel = maker::make<panel>( tabs );
    label& cam1label = maker::make<label>( cam1panel );
    cam1label.move(30,100, 100,20 );
    cam1label.text("CAM1 panel");
    tabs.add( "CAM1", cam1panel );

    panel& cam2panel = maker::make<panel>( tabs );
    label& cam2label = maker::make<label>( cam2panel );
    cam2label.move(30,100, 100,20 );
    cam2label.text("CAM2 panel");
    tabs.add( "CAM2", cam2panel );

    panel& cam3panel = maker::make<panel>( tabs );
    label& cam3label = maker::make<label>( cam3panel );
    cam3label.move(30,100, 100,20 );
    cam3label.text("CAM3 panel");
    tabs.add( "CAM3", cam3panel );

    form.show();

    // initially show the first panel
    // must be donw after call to show, which displays the last panel added
    tabs.select( 0 );
</pre>
*/
    class tabbed : public panel
    {
    public:
        tabbed(gui *parent)
            : panel(parent), myTabWidth(50)
        {
            tabChanging([](int tabIndex) {});
            tabChanged([](int tabIndex) {});
        }
        /** add panel that can be displayed
        @param[in] tabname text for button that brings up the panel
        @param[in] panel to be displayed when tab selected.

        Panel will be resized to fit neatly
    */
        void add(
            const std::string &tabname,
            gui &panel)
        {
            //resize the child panel so it fits neatly under the tab buttons
            RECT rect;
            GetClientRect(myHandle, &rect);
            panel.move(0, 31, rect.right - rect.left, rect.bottom - rect.top - 30);

            button &btn = maker::make<button>(*this);
            btn.text(tabname);
            btn.move(myButton.size() * myTabWidth,
                     0, myTabWidth, 30);
            myButton.push_back(&btn);
            myPanel.push_back(&panel);
            int tabIndex = myButton.size() - 1;

            btn.events().click([this, tabIndex]()
                               {
                                   myTabChangingFn(tabIndex);
                                   select(tabIndex);
                                   myTabChangeFn(tabIndex);
                               });
        }
        /// select panel to displayed
        void select(int i)
        {
            std::cout << "select " << i << "\n";

            if (0 > i || i >= (int)myButton.size())
                return;

            for (auto b : myButton)
            {
                b->bgcolor(0xC8C8C8);
                b->update();
            }
            for (auto p : myPanel)
                p->show(false);

            myButton[i]->bgcolor(0xFFFFFF);
            myPanel[i]->show();
            update();
            mySelect = i;
        }
        /// zero-based index of panel currently selected
        int select() const
        {
            return mySelect;
        }
        /// set width of tab buttons
        void tabWidth(int w)
        {
            myTabWidth = w;
        }
        /** register function to call when tab is about to change
    This is only called when user changes the tab, not when app code call select() function
    Zero-based new tab index passed to registered function when called
    */
        void tabChanging(std::function<void(int tabIndex)> f)
        {
            myTabChangingFn = f;
        }
        /** register function to call when tab has changed
        This is only called when user changes the tab, not when app code call select() function
        Zero-based tab index passed to registered function when called
    */
        void tabChanged(std::function<void(int tabIndex)> f)
        {
            myTabChangeFn = f;
        }

    private:
        std::vector<button *> myButton;
        std::vector<gui *> myPanel;
        int myTabWidth;
        int mySelect;
        std::function<void(int tabIndex)> myTabChangingFn;
        std::function<void(int tabIndex)> myTabChangeFn;
    };

    /** Widget to layout a group of radio buttons

Usage:
<pre>
    wex::radiobuttonLayout & myGroup = wex::maker::make<wex::radiobuttonLayout>( form );
    myGroup.move( {50,50,200,400} );
    myGroup.grid(1);        // layout in one column
    wex::radiobutton& rb = myGroup.add();
    rb.text("Heart");
    rb.size(60,20);
    rb.events().click([this]
    {
        Change( 1 );
    });
    rb = myGroup.add();
    rb.text("EMI 1");
    rb.size(60,20);
    rb.events().click([this]
    {
        Change(2 );
    });
    rb = myGroup.add();
    rb.text("EMI 2");
    rb.size(60,20);
    rb.events().click([this]
    {
        Change( 3 );
    });
</pre>
*/
    class radiobuttonLayout : public layout
    {
    public:
        radiobuttonLayout(gui *parent)
            : layout(parent), myFirst(true)
        {
        }
        /** add a radio button
        @return reference to radibutton
    */
        radiobutton &add()
        {
            wex::radiobutton &rb = wex::maker::make<wex::radiobutton>(*this);
            if (myFirst)
            {
                myFirst = false;
                rb.first();
            }
            return rb;
        }
        /// 0-based index of checked radio button
        int checked()
        {
            if (myFirst)
                return -1;
            return ((radiobutton *)children()[0])->checkedOffset();
        }
        /** set status of radio button
        @param[in] i zero-based index of radio button
        @param[in] f true or false, default true
    */
        void check(int i, bool f = true)
        {
            if (0 > i || i >= (int)children().size())
                return;
            ((radiobutton *)children()[i])->check();
        }
        /** Enable/disable all radio buttons
        @param[in] f status required, true for enabled, default enabled
    */
        void enable(bool f = true)
        {
            for (auto rb : children())
                ((radiobutton *)rb)->enable(f);
        }

    private:
        bool myFirst;
    };
    /// Print a text document
    class printDoc
    {
    public:
        /** CTOR
         * @param[in] title that asppears in print spooler
         */
        printDoc(const std::string &title = "printDoc")
        {
            // https://www.equestionanswers.com/vcpp/screen-dc-printer-dc.php
            PRINTDLG pdlg;

            /* Initialize the PRINTDLG structure. */
            memset(&pdlg, 0, sizeof(PRINTDLG));
            pdlg.lStructSize = sizeof(PRINTDLG);
            /* Set the flag to return printer DC. */
            pdlg.Flags = PD_RETURNDC;

            /* Invoke the printer dialog box. */
            PrintDlg(&pdlg);

            /* hDC member of the PRINTDLG structure contains the printer DC. */
            dc = pdlg.hDC;
            if (!dc)
                return;

            DOCINFO di;
            memset(&di, 0, sizeof(DOCINFO));
            /* Fill in the required members. */
            di.cbSize = sizeof(DOCINFO);
            di.lpszDocName = title.c_str();

            StartDoc(dc, &di);
        }
        /// Finalize and send to printer
        ~printDoc()
        {
            EndDoc(dc);
            DeleteDC(dc);
        }
        /// True if CTOR was successful
        bool isOpen()
        {
            return (bool)dc;
        }
        void pageStart()
        {
            StartPage(dc);
        }
        void pageEnd()
        {
            EndPage(dc);
        }
        /** Add some text
         * @param[in]  x, y locatioon
         * @param[in] s the text
         * 
         * Each character needs about 100 by 100 location units
         */
        void text(
            int x, int y,
            const std::string &s)
        {
            TextOut(
                dc,
                x, y,
                s.c_str(), s.length());
        }

    private:
        HDC dc;
    };

    struct free
    {
        /** Start a command in its own process
     * @param[in] command line, same as would be used from a command window running in working directory
     * @param[out] error details, if any
     * @return 0 if no errors
     */
        static int startProcess(
            const std::string &command,
            std::string &error)
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Retain keyboard focus, minimize module2 window
            si.wShowWindow = SW_SHOWNOACTIVATE | SW_MINIMIZE;
            si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
            si.dwX = 600;
            si.dwY = 200;

            if (!CreateProcessA(
                    NULL,                   // No module name (use command line)
                    (LPSTR)command.c_str(), // Command line
                    NULL,                   // Process handle not inheritable
                    NULL,                   // Thread handle not inheritable
                    FALSE,                  // Set handle inheritance to FALSE
                    CREATE_NEW_CONSOLE,     //  creation flags
                    NULL,                   // Use parent's environment block
                    NULL,                   // Use parent's starting directory
                    &si,                    // Pointer to STARTUPINFO structure
                    &pi)                    // Pointer to PROCESS_INFORMATION structure
            )
            {
                int syserrno = GetLastError();
                if (syserrno == 2)
                {
                    error = "Cannot find executable file";
                    return 2;
                }
                char *lpMsgBuf;
                FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    (DWORD)syserrno,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPSTR)&lpMsgBuf,
                    0, NULL);
                error = lpMsgBuf;
                LocalFree(lpMsgBuf);
                return 1;
            }

            // Close process and thread handles.
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            error = "";
            return 0;
        }
    };
}
