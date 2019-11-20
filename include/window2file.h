#pragma once
#include <sstream>
#include <windows.h>
#include <gdiplus.h>
#include "wex.h"
namespace wex
{
class window2file
{
public:
    window2file()
    {
        using namespace Gdiplus;

        GdiplusStartupInput gdiplusStartupInput;

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        UINT  num = 0;
        UINT  size = 0;

        ImageCodecInfo* pImageCodecInfo = NULL;

        GetImageEncodersSize(&num, &size);
        if(size == 0)
            throw std::runtime_error("window2file");

        pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
        if(pImageCodecInfo == NULL)
            throw std::runtime_error("window2file");

        GetImageEncoders(num, size, pImageCodecInfo);
        for(UINT j = 0; j < num; ++j)
        {
            if( wcscmp(pImageCodecInfo[j].MimeType, L"image/png" ) == 0 )
            {
                myPngclsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return;
            }
        }
        free(pImageCodecInfo);
        throw std::runtime_error("window2file cannot find encoder");
    }
    ~window2file()
    {
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }
    void save( gui& w, const std::string& filename )
    {
        using namespace Gdiplus;

        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        HDC  memdc;
        HBITMAP membit;
        HDC scrdc = ::GetDC( w.handle() );
        RECT rcClient;
        GetClientRect( w.handle(), &rcClient);
        int Height = rcClient.bottom-rcClient.top;
        int Width = rcClient.right-rcClient.left;
        memdc = CreateCompatibleDC(scrdc);
        membit = CreateCompatibleBitmap(scrdc, Width, Height);
        HBITMAP hOldBitmap =(HBITMAP) SelectObject(memdc, membit);
        BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

        Gdiplus::Bitmap bitmap(membit, NULL);
        std::wstringstream wss;
        wss << filename.c_str();

        Status stat = bitmap.Save(
                          wss.str().c_str(),
                          &myPngclsid,
                          NULL);
        //std::cout << "status " << stat << "\n";

        DeleteObject(memdc);
        DeleteObject(membit);
        ReleaseDC(0,scrdc);
    }
private:
    CLSID myPngclsid;
    ULONG_PTR gdiplusToken;
};
}
