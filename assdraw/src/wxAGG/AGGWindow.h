#ifndef WX_AGG_WINDOW_H
#define WX_AGG_WINDOW_H

#include "PixelFormatConvertor.h"

#include <wx/bitmap.h>
#include <wx/rawbmp.h>
#include <wx/window.h>
#include <wx/dcmemory.h>

#include "agg_rendering_buffer.h"

namespace GUI {
    
    /// A simple widget that displays a bitmap that AGG can draw on.
    /// It reallocates the bitmap so that it always is the current size of the 
    /// entire panel and calls the virtual method draw() to draw to the bitmap.
    /// This should be useable anywhere a wxWindow can be, e.g. in actual windows, 
    /// buttons, etc.
    class AGGWindow: public wxWindow {
        public:
        /// Create an AGGWindow.  Defaults are taken from wxWindow::wxWindow(), see 
        /// that documentation for more information.
        AGGWindow(wxWindow* parent, wxWindowID id = wxID_ANY, 
                  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
                  long style = wxTAB_TRAVERSAL);
        
        /// Clean up resources held
        virtual ~AGGWindow();
        
        protected:
           
        /// The conversion between wxWidgets' pixel format and AGG's pixel format
        typedef PixelFormatConvertor<wxNativePixelFormat> PixelFormat;

        /// The wxWidgets "pixel data" type, an accessor to the raw pixels
        typedef wxPixelData<wxBitmap, PixelFormat::wxWidgetsType> PixelData;

        
        /// Create the bitmap given the current size.
        void init(const int width, const int height);

        /// Attach the AGG rendering buffer to the bitmap and call the user draw() code.
        void attachAndDraw();
    
        /// Paint the bitmap onto the panel.
        void onPaint(wxPaintEvent& event);
        
        /// Resize the bitmap to match the window.
        void onSize(wxSizeEvent& event);

        /// Handle the erase-background event.
        void onEraseBackground(wxEraseEvent& event);
        
        /// Draw into the bitmap using AGG.
        virtual void draw() = 0;

        
        wxBitmap* bitmap;               ///< wxWidgets bitmap for AGG to draw into
        wxMemoryDC memDC;               ///< Memory "device context" for drawing the bitmap
        
        agg::rendering_buffer rBuf;     ///< AGG's rendering buffer, pointing into the bitmap
    
        DECLARE_EVENT_TABLE()           /// Allocate wxWidgets storage for event handlers
    };
}
    
#endif
