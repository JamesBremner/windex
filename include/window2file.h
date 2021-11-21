#pragma once
#include <sstream>
#include <windows.h>
#include <gdiplus.h>
#include "wex.h"
namespace wex
{
    /** \brief transfer image between window contents and image file in PNG format.

Add library gdiplus to linker library list
*/
    class window2file
    {
    public:
        window2file()
        {
            using namespace Gdiplus;

            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

            UINT num = 0;
            UINT size = 0;

            ImageCodecInfo *pImageCodecInfo = NULL;

            GetImageEncodersSize(&num, &size);
            if (size == 0)
                throw std::runtime_error("window2file");

            pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
            if (pImageCodecInfo == NULL)
                throw std::runtime_error("window2file");

            GetImageEncoders(num, size, pImageCodecInfo);
            for (UINT j = 0; j < num; ++j)
            {
                if (wcscmp(pImageCodecInfo[j].MimeType, L"image/png") == 0)
                {
                    myPngclsid = pImageCodecInfo[j].Clsid;
                    free(pImageCodecInfo);
                    return;
                }
            }
            ::free(pImageCodecInfo);
            throw std::runtime_error("window2file cannot find encoder");
        }
        ~window2file()
        {
            Gdiplus::GdiplusShutdown(gdiplusToken);
        }

        /** Save window contents to image file
        @param[in] w the window to save
        @param[in] filename to save to
    */
        void save(gui &w, const std::string &filename)
        {
            save( w.handle(), filename );
        }
        void save(HWND hw, const std::string &filename)
        {
            HDC memdc;
            HBITMAP membit;
            HDC scrdc = ::GetDC(hw);
            RECT rcClient;
            GetClientRect(hw, &rcClient);
            int Height = rcClient.bottom - rcClient.top;
            int Width = rcClient.right - rcClient.left;
            memdc = CreateCompatibleDC(scrdc);
            membit = CreateCompatibleBitmap(scrdc, Width, Height);
            SelectObject(memdc, membit);
            BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

            Gdiplus::Bitmap bitmap(membit, NULL);
            std::wstringstream wss;
            wss << filename.c_str();

            bitmap.Save(
                wss.str().c_str(),
                &myPngclsid,
                NULL);

            DeleteObject(memdc);
            DeleteObject(membit);
            ReleaseDC(0, scrdc);
        }
        /** Draw png file in window
        @param[in] w the window to draw into
        @param[in] filename to be drawn
        @return true if success
        */
        bool draw(gui &w, const std::string &filename)
        {
            // read image from file
            std::wstringstream wss;
            wss << filename.c_str();
            auto bitmap = new Gdiplus::Bitmap(wss.str().c_str());
            if( ! bitmap )
                return false;
            if( bitmap->GetLastStatus() != S_OK)
                return false;
            // image dimensions
            float xh = bitmap->GetHeight();
            float xw = bitmap->GetWidth();


            // window dimensions
            RECT r;
            GetClientRect(w.handle(), &r);
            int rw = r.right - r.left;
            int rh = r.bottom - r.top;

            // check if shrinking needed
            if (xh > rh || xw > rw)
            {
                // preserve aspect ratio by scaling both dimensions by the largest required by either
                float sh = xh / rh;
                float sw = xw / rw;
                float s = sh;
                if (sw > sh)
                    s = sw;
                xh /= s;
                xw /= s;
            }
            Gdiplus::PointF dst[] =
                {
                    Gdiplus::PointF(0.0f, 0.0f),    // top left
                    Gdiplus::PointF(xw, 0.0f),      // top right
                    Gdiplus::PointF(0.0f, xh),      // bottom left
                };

            // draw
            Gdiplus::Graphics graphics(GetDC(w.handle()));
            graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
            auto ret = graphics.DrawImage(bitmap, dst, 3);

            delete bitmap;

            if( ret !=  Gdiplus::Status::Ok )
                return false;

            return true;
        }

    private:
        CLSID myPngclsid;
        ULONG_PTR gdiplusToken;
    };
}
