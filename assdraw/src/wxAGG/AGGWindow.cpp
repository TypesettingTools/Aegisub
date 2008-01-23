#include "AGGWindow.h"

#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgb.h"

namespace GUI {
    
    BEGIN_EVENT_TABLE(AGGWindow, wxWindow)
        EVT_PAINT(AGGWindow::onPaint)
        EVT_SIZE(AGGWindow::onSize)
        EVT_ERASE_BACKGROUND(AGGWindow::onEraseBackground)
    END_EVENT_TABLE()
    
    AGGWindow::AGGWindow(wxWindow* parent, wxWindowID id, 
                       const wxPoint& pos, const wxSize& size, long style):
        wxWindow(parent, id, pos, size, style, wxT("AGGWindow")),
        bitmap(NULL) {
    }
    
    void AGGWindow::init(const int width, const int height) {
        memDC.SelectObject(wxNullBitmap);
        delete bitmap;  
        
        int ncheight = height, ncwidth = width;
        if (ncwidth < 1) ncwidth = 1;
        if (ncheight < 1) ncheight = 1;
  
        bitmap = new wxBitmap(ncwidth, ncheight, PixelFormat::wxWidgetsType::BitsPerPixel);
        memDC.SelectObject(*bitmap);

        // Draw the bitmap
        attachAndDraw();
        
        // Request a full redraw of the window
        Refresh(false);
    }
    
    AGGWindow::~AGGWindow() {
        memDC.SelectObject(wxNullBitmap);
        delete bitmap;
    }

    void AGGWindow::attachAndDraw() {
        // Get raw access to the wxWidgets bitmap -- this locks the pixels and 
        // unlocks on destruction.
        PixelData data(*bitmap);
        assert(data);

#if 1
        // This cast looks like it is ignoring byte-ordering, but the 
        // pixel format already explicitly handles that.
        assert(data.GetPixels().IsOk());
        wxAlphaPixelFormat::ChannelType* pd = (wxAlphaPixelFormat::ChannelType*) &data.GetPixels().Data();

        // wxWidgets always returns a pointer to the first row of pixels, whether
        // that is stored at the beginning of the buffer (stride > 0) or at the 
        // end of the buffer (stride < 0).  AGG always wants a pointer to the 
        // beginning of the buffer, no matter what the stride.  (AGG does handle
        // negative strides correctly.)
        // Upshot: if the stride is negative, rewind the pointer from the end of 
        // the buffer to the beginning. 
        const int stride = data.GetRowStride();
        if (stride < 0) 
            pd += (data.GetHeight() - 1) * stride;

        rBuf.attach(pd, data.GetWidth(), data.GetHeight(), stride);

        // Call the user code to actually draw.
        draw();
#else
        PixelData::Iterator p(data);

        // we draw a (10, 10)-(20, 20) rect manually using the given r, g, b
        p.Offset(data, 10, 10);

        for ( int y = 0; y < 10; ++y )
        {
            PixelData::Iterator rowStart = p;

            for ( int x = 0; x < 10; ++x, ++p )
            {
                p.Red() = 255;
                p.Green() = 0;
                p.Blue() = 255;
            }

            p = rowStart;
            p.OffsetY(data, 1);
        }
#endif
    }
    
    void AGGWindow::onSize(wxSizeEvent& event) {
        const wxSize size = GetClientSize();
        if (bitmap && size.GetWidth() == bitmap->GetWidth() && size.GetHeight() == bitmap->GetHeight())
            return;
        
        init(size.GetWidth(), size.GetHeight());
    }
    
    void AGGWindow::onPaint(wxPaintEvent& event) {
        wxPaintDC dc(this);

        wxCoord width, height;
        dc.GetSize(&width, &height);
        if (!bitmap || bitmap->GetWidth() != width || bitmap->GetHeight() != height) 
            init(width, height);
        
        // Iterate over regions needing repainting
        wxRegionIterator regions(GetUpdateRegion());
        wxRect rect;
        while (regions) {
            rect = regions.GetRect();
            dc.Blit(rect.x, rect.y, rect.width, rect.height, &memDC, rect.x, rect.y);
            ++regions;
        }
    }

    void AGGWindow::onEraseBackground(wxEraseEvent& WXUNUSED(event)) {
	    // Do nothing to "avoid flashing in MSW"  Grr.
    }

}

