#include "commctrl.h"

// Enable common controls v6
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace wex
{
    class datebox : public gui
    {
    public:
        datebox(gui *parent)
            : gui(parent, DATETIMEPICK_CLASS,
            WS_BORDER | WS_CHILD | WS_VISIBLE | DTS_SHOWNONE)
        {
            int dbg = 0;
        }
        static void init()
        {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_DATE_CLASSES;
            if( ! InitCommonControlsEx(&icex) )
                throw std::runtime_error("InitCommonControlsEx failed");
        }
    };
}