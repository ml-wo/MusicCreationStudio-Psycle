/***************************************************************************
 *   Copyright (C) 2005, 2006 by Stefan Nattkemper                         *
 *   Made in Germany                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ngraphics.h"
#include "napp.h"
#include "nvisualcomponent.h"
#include <cmath>
#include <iostream>

#ifdef __unix__
#else

#define PI 3.14159265358979
#define XAngleToRadians(a) ((double)(a) / 64 * PI / 180);

#endif

NGraphics::NGraphics(WinHandle winID)
{
  dx_=dy_=0;
  dblWidth_ = 0; dblHeight_ = 0;
  dblBuffer_=false;
  #ifdef __unix__
  fFtColor.color.red   = 0x0000;
  fFtColor.color.green = 0x0000;
  fFtColor.color.blue  = 0x0000;
  fFtColor.color.alpha = 0xFFFF; // Alpha blending
  #else
  brush = CreateSolidBrush(RGB(0,0,0));
  // Set brush to hollow
  LOGBRUSH logbrush;
  logbrush.lbStyle = BS_HOLLOW;
  hollow = CreateBrushIndirect(&logbrush);
  hPen  = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  #endif
  win = winID;

  #ifdef __unix__
  gc_     = XCreateGC(NApp::system().dpy(),winID,0,0);
  drawWin = XftDrawCreate(NApp::system().dpy(), win, NApp::system().visual(),NApp::system().colormap());
  
  doubleBufferPixmap_=0;
  gcp = 0;
  drawDbl = 0;
  #else
  gc_ = GetDC( win );
  doubleBufferBitmap_=0;
  gcp = 0;
  #endif


  visible_ = false;

  setFont(fnt);
  setForeground(oldColor);

}


NGraphics::~NGraphics()
{
  #ifdef __unix__
  XftDrawDestroy(drawWin);
  #else  
  ReleaseDC( win , gc_ );
  DeleteObject( brush );
  DeleteObject( hollow );
  DeleteObject( hPen );
  #endif
  if (dblBuffer_) {
     destroyDblBufferHandles();
  }
}

void NGraphics::fillRect( int x, int y, int width, int height )
{
  // Note : XFillRectangle has the same
  // width as Rectangle, but XDrawRectangle is one pixel wider
  // for the same co-ordinates.   
     
  if (dblBuffer_)
     #ifdef __unix__
     XFillRectangle( NApp::system().dpy(), doubleBufferPixmap_, gcp, x+dx_, y+dy_, width, height );
     #else
     Rectangle( gcp, x + dx_,y + dy_, x + dx_ + width, y + dy_ +height);
     #endif
  else
     #ifdef __unix__
     XFillRectangle( NApp::system().dpy(), win, gc_, x+dx_, y+dy_, width, height);
     #else
     Rectangle( gcp, x + dx_,y + dy_, x + dx_ + width, y + dy_ +height);
     #endif
}

void NGraphics::createDblBufferHandles( )
{
  #ifdef __unix__
  XWindowAttributes attr;
  XGetWindowAttributes( NApp::system().dpy(), win, &attr );

  if (dblBuffer_) {
      doubleBufferPixmap_ = XCreatePixmap(NApp::system().dpy(), win, attr.width, attr.height, NApp::system().depth());
      gcp = XCreateGC(NApp::system().dpy(),doubleBufferPixmap_,0,0);
      dblWidth_  = attr.width;
      dblHeight_ = attr.height;
      XSetForeground( NApp::system().dpy(), gcp, oldColor.colorValue() );
      drawDbl = XftDrawCreate(NApp::system().dpy(), doubleBufferPixmap_, NApp::system().visual(), NApp::system().colormap());
  } else
  {
    doubleBufferPixmap_=0;
    gcp = 0;
    drawDbl = 0;
    XSetForeground( NApp::system().dpy(), gc_, oldColor.colorValue() );
  }
  #else
  if (dblBuffer_) {
    RECT r;
    GetWindowRect( win, &r );

    gcp = CreateCompatibleDC( gc_ );
    doubleBufferBitmap_ = CreateCompatibleBitmap( gc_, r.right - r.left, r.bottom - r.top);
    SelectObject( gcp, doubleBufferBitmap_ );
    
  } else {
    doubleBufferBitmap_ = 0;
    gcp = 0;
  }
  #endif
}

void NGraphics::destroyDblBufferHandles( )
{     
  if (dblBuffer_) {
    #ifdef __unix__
    XFreeGC(NApp::system().dpy(), gcp);
    XFreePixmap(NApp::system().dpy(), doubleBufferPixmap_);
    XftDrawDestroy(drawDbl);
    #else
    // Clean up - only need to do this one time as well
    DeleteDC( gcp);
    DeleteObject( doubleBufferBitmap_ );
    #endif
  }
}

void NGraphics::setTranslation( long dx, long dy )
{
  dx_ = dx;
  dy_ = dy;
}

void NGraphics::resize(int width, int height )
{
  if (dblBuffer_) {
    destroyDblBufferHandles();
    createDblBufferHandles();
  }
}

void NGraphics::copyDblBuffer( const NRect & repaintArea )
{
  #ifdef __unix__
   XCopyArea(NApp::system().dpy(), doubleBufferPixmap_, win, gc_,repaintArea.left(), repaintArea.top(),repaintArea.width(), repaintArea.height(),repaintArea.left(), repaintArea.top());
  #else
  // blit the gcp to gc_
  BitBlt( gc_, repaintArea.left(), repaintArea.top(), repaintArea.width(), repaintArea.height(), gcp, repaintArea.left(), repaintArea.top(), SRCCOPY);
  #endif     
}

void NGraphics::swap( const NRect & repaintArea )
{
  if ( dblBuffer_ ) copyDblBuffer( repaintArea );
}

long NGraphics::xTranslation( )
{
  return dx_;
}

long NGraphics::yTranslation( )
{
  return dy_;
}

void NGraphics::setForeground( const NColor & color )
{
  if (!(oldColor == color)) {
    #ifdef __unix__                 
    if (dblBuffer_) XSetForeground( NApp::system().dpy(), gcp, color.colorValue() );
               else XSetForeground( NApp::system().dpy(), gc_, color.colorValue() );
    #else
    DeleteObject( brush );
    brush =  CreateSolidBrush(RGB(color.red(), color.green(), color.blue() ));
    if ( dblBuffer_ )
       SelectObject( gcp, brush );
    else
       SelectObject( gc_, brush );
    
    LOGPEN logPen;
    GetObject( hPen, sizeof(logPen), &logPen );
    DeleteObject( hPen );
    hPen = CreatePen( logPen.lopnStyle, logPen.lopnWidth.x, RGB( color.red(), color.green(), color.blue()));
    
    if ( dblBuffer_ )
      SelectObject( gcp, hPen );
    else
      SelectObject( gc_, hPen );
    #endif               
    oldColor.setRGB(color.red(),color.green(),color.blue());
  }
}

void NGraphics::setFont( const NFont & font )
{
  fntStruct = font.systemFont();   
  #ifdef __unix__
  if ( !fntStruct.antialias ) {
    if ( dblBuffer_ )
       XSetFont( NApp::system().dpy(), gcp, fntStruct.xFnt->fid );
    else
       XSetFont( NApp::system().dpy(), gc_, fntStruct.xFnt->fid );
  }
  #else
  if ( dblBuffer_ )
    SelectObject( gcp, fntStruct.hFnt ); 
  else
    SelectObject( gc_, fntStruct.hFnt ); 
  #endif
}

void NGraphics::drawXftString( int x, int y, const char * s )
{
  #ifdef __unix__
   if (dblBuffer_)
      XftDrawString8(drawDbl, &fFtColor, fntStruct.xftFnt , x, y,reinterpret_cast<const FcChar8 *>(s), strlen(s));
   else
     XftDrawString8(drawWin, &fFtColor, fntStruct.xftFnt , x, y,reinterpret_cast<const FcChar8 *>(s), strlen(s));
  #endif
}

void NGraphics::drawText( int x, int y, const std::string & text )
{
  #ifdef __unix__
 // if (!fntStruct.font.transparent()) {
 //  NColor old = lastUsed;
   //setForeground(fntStruct.font.bgColor());
 //  fillRect(x,y-textAscent(),textWidth(text),textAscent()+textDescent());
  // setForeground(old);
 //}
 /*x +=dx_; y+=dy_;*/
 if (!fntStruct.antialias)  {

    if (!(fntStruct.textColor==oldColor)) {
       if (dblBuffer_) XSetForeground( NApp::system().dpy(), gcp, fntStruct.textColor.colorValue() );
                  else XSetForeground( NApp::system().dpy(), gc_, fntStruct.textColor.colorValue() );
    }
    if (dblBuffer_)
       XDrawString(NApp::system().dpy(),doubleBufferPixmap_,gcp,x+dx_,y+dy_,text.c_str(),strlen(text.c_str()));
    else
       XDrawString(NApp::system().dpy(),win,gc_,x+dx_,y+dy_,text.c_str(),strlen(text.c_str()));
    setForeground(old);
 } else
 {
    NColor color;
    color = fntStruct.textColor;
    fFtColor.color.red   = color.red() * ( 0xFFFF/255);
    fFtColor.color.green = color.green() * ( 0xFFFF/255);
    fFtColor.color.blue  = color.blue() * ( 0xFFFF/255);
    drawXftString(x+dx_,y+dy_,text.c_str());
  }
  #else    
  if ( dblBuffer_ ) {
    SetBkMode( gcp, TRANSPARENT );
    SetTextAlign( gcp, TA_BASELINE );
    SetTextColor( gcp, fntStruct.textColor.hColorRef() );
    
    TextOut( gcp, x + dx_, y+ dy_, text.c_str(), text.length() );    
  } else {
     SetBkMode( gc_, TRANSPARENT );
     SetTextAlign( gc_, TA_BASELINE );
     SetTextColor( gc_, fntStruct.textColor.hColorRef() );
     TextOut( gc_, x + dx_, y+ dy_, text.c_str(), text.length() );      
  }
  #endif
}

void NGraphics::drawText(int x, int y, const std::string & text, const NColor & color ) {
   #ifdef __unix__
	if (!fntStruct.antialias)  {
     if (dblBuffer_) XSetForeground( NApp::system().dpy(), gcp, color.colorValue() );
                  else XSetForeground( NApp::system().dpy(), gc_, color.colorValue() );
    
    if (dblBuffer_)
       XDrawString(NApp::system().dpy(),doubleBufferPixmap_,gcp,x+dx_,y+dy_,text.c_str(),strlen(text.c_str()));
    else
       XDrawString(NApp::system().dpy(),win,gc_,x+dx_,y+dy_,text.c_str(),strlen(text.c_str()));
    //setForeground(old);
 } else
 {
    fFtColor.color.red   = color.red() * ( 0xFFFF/255);
    fFtColor.color.green = color.green() * ( 0xFFFF/255);
    fFtColor.color.blue  = color.blue() * ( 0xFFFF/255);
    drawXftString( x+dx_, y+dy_, text.c_str() );
  }
  #else
  if (dblBuffer_) {
    SetBkMode( gcp, TRANSPARENT );
    SetTextAlign( gcp, TA_BASELINE );
    SetTextColor( gcp, color.hColorRef() );
    TextOut( gcp, x + dx_, y+ dy_, text.c_str(), text.length() );
  } else {
    SetBkMode( gc_, TRANSPARENT );
    SetTextAlign( gc_, TA_BASELINE );
    SetTextColor( gc_, color.hColorRef() );
    TextOut( gc_, x + dx_, y+ dy_, text.c_str(), text.length() );    
  }
  #endif
}

void NGraphics::drawRect( int x, int y, int width, int height )
{
  // Note : XFillRectangle has the same
  // width as Rectangle, but XDrawRectangle is one pixel wider
  // for the same co-ordinates.   
  
  if (dblBuffer_) {
    #ifdef __unix__
    XDrawRectangle(NApp::system().dpy(),doubleBufferPixmap_,gcp,x+dx_,y+dy_,width,height);
    #else
    HBRUSH holdbrush = (HBRUSH) SelectObject( gcp, hollow );
    Rectangle( gcp, x + dx_,y + dy_, x + dx_ + width + 1, y + dy_ +height + 1);
    SelectObject( gcp, holdbrush );
    #endif
  }  
  else
  {
    #ifdef __unix__
    XDrawRectangle(NApp::system().dpy(),win,gc_,x+dx_,y+dy_,width,height);
    #else
    HBRUSH holdbrush = (HBRUSH) SelectObject( gc_, hollow );
    Rectangle( gc_, x + dx_,y + dy_, x + dx_ + width + 1, y + dy_ + height + 1);
    SelectObject( gc_, holdbrush );
    #endif
  }  
}

void NGraphics::drawRect( const NRect & rect )
{
  drawRect(rect.left(),rect.top(),rect.width(),rect.height());
}

void NGraphics::drawLine( long x, long y, long x1, long y1 )
{
  if ( dblBuffer_ ) 
  {
    #ifdef __unix__
    XDrawLine(NApp::system().dpy(),doubleBufferPixmap_,gcp,x+dx_,y+dy_,x1+dx_,y1+dy_);
    #else
    MoveToEx( gcp, x + dx_, y + dy_, NULL);
    LineTo( gcp, x1 + dx_, y1 + dy_);
    #endif
  }
  else
  {
    #ifdef __unix__
    XDrawLine(NApp::system().dpy(),win,gc_,x+dx_,y+dy_,x1+dx_,y1+dy_);
    #else 
    MoveToEx( gc_, x + dx_, y + dy_, NULL);
    LineTo( gc_, x1 + dx_, y1 + dy_);
    #endif
  }   
}

void NGraphics::drawPolygon( NPoint* pts , int n )
{
  int p2x = 0;
  int p2y = 0;
  for (int i = 0; i < n; i++) {
      int p1x = pts[i].x();
      int p1y = pts[i].y();
      if (i<n-1)  {
        p2x = pts[i+1].x(); 
        p2y = pts[i+1].y();
      } else { p2x = pts[0].x(); p2y = pts[0].y(); }
      drawLine(p1x,p1y,p2x,p2y);
    }
}

void NGraphics::fillPolygon( NPoint * pts, int n )
{
  #ifdef __unix__
  XPoint* pt = new XPoint[n];
  #else
  POINT* pt = new POINT[n];
  #endif

  for (int i = 0; i< n; i++) {
    pt[i].x = pts[i].x() +  dx_;
    pt[i].y = pts[i].y() +  dy_;
  }
  
  #ifdef __unix__
  if (dblBuffer_)
     XFillPolygon(NApp::system().dpy(),doubleBufferPixmap_,gcp,pt,n,Complex,CoordModeOrigin);
  else
     XFillPolygon(NApp::system().dpy(),win,gc_,pt,n,Complex,CoordModeOrigin);
  #else   
  if (dblBuffer_)
    Polygon( gcp, pt, n );
  else
    Polygon( gc_, pt, n );
  #endif

  delete[] pt;
}

NRegion NGraphics::region( )
{
  return region_;
}

void NGraphics::setRegion( const NRegion & region )
{
  region_ = region;
}

void NGraphics::setClipping( const NRegion & region )
{
  #ifdef __unix__
  if (dblBuffer_) {
    XSetRegion(NApp::system().dpy(), gcp,region.sRegion());
    XftDrawSetClip(drawDbl,region.sRegion());
  }
  else {
    XSetRegion(NApp::system().dpy(), gc_,region.sRegion());
    XftDrawSetClip(drawWin,region.sRegion());
  }
  #else
  if ( dblBuffer_ )
    SelectClipRgn( gcp, region );
  else  
    SelectClipRgn( gc_, region );    
  #endif
}

void NGraphics::fillTranslucent( int x, int y, int width, int height, NColor color, int percent )
{
  #ifdef __unix__
  XImage* xi = XGetImage(NApp::system().dpy(),doubleBufferPixmap_,x+dx_,y+dy_,width,height,AllPlanes,ZPixmap);
  unsigned char* data = (unsigned char*) xi->data;
  int pixelsize = NApp::system().pixelSize( NApp::system().depth() );
  double anteil = percent / 100.0f;
  if (pixelsize == 4)
  for (int i = 0; i < xi->width*xi->height*pixelsize;i=i+pixelsize) {
       unsigned char r = data[i];
       unsigned char g = data[i+1];
       unsigned char b = data[i+2];

       data[i]   =  (int) ((anteil*r + (1 - anteil) * color.red()));
       data[i+1] =  (int) ((anteil*g + (1 - anteil) * color.green()));
       data[i+2] =  (int) ((anteil*b + (1 - anteil) * color.blue()));
    }
  XPutImage(NApp::system().dpy(),doubleBufferPixmap_,gcp,xi,0,0,x+dx_,y+dy_,xi->width,xi->height);
  XDestroyImage(xi);
  #endif
}

int NGraphics::textWidth( const std::string & text ) const
{
   #ifdef __unix__
   const char* s = text.c_str();
   if (!fntStruct.antialias) {
     return XTextWidth(fntStruct.xFnt,s,strlen(s));
   } else
   {
    XGlyphInfo info;
    XftTextExtents8(NApp::system().dpy(),fntStruct.xftFnt
     ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
    return info.xOff;
   }
   #else 

   SIZE size;
   if ( dblBuffer_ )
     GetTextExtentPoint32( gcp, text.c_str(), text.length(), &size);
   else  
     GetTextExtentPoint32( gc_, text.c_str(), text.length(), &size);
     
   return size.cx;
   #endif
}

int NGraphics::textHeight()
{
 #ifdef __unix__
 if (!fntStruct.antialias)
 {
   return (fntStruct.xFnt->max_bounds.ascent+ fntStruct.xFnt->max_bounds.descent);
 } else {
  int a = fntStruct.xftFnt->ascent;
  int d = fntStruct.xftFnt->descent;
  return a + d + 1;
 }
 #else
 TEXTMETRIC metrics;

 if (dblBuffer_)
   GetTextMetrics( gcp, &metrics );
 else    
   GetTextMetrics( gc_, &metrics );

 return metrics.tmHeight; 
 #endif
}

int NGraphics::textAscent( )
{ 
  #ifdef __unix__
  if (!fntStruct.antialias)
  {
    return (fntStruct.xFnt->max_bounds.ascent);
  } else {
   int a = fntStruct.xftFnt->ascent;
   return a;
  }
  #else
  TEXTMETRIC metrics;
 
  if (dblBuffer_)
    GetTextMetrics( gcp, &metrics );
  else    
    GetTextMetrics( gc_, &metrics );
    
  return metrics.tmAscent; 
  #endif
}

int NGraphics::textDescent( )
{
 #ifdef __unix__
 if (!fntStruct.antialias)
   return (fntStruct.xFnt->max_bounds.descent);
 else 
   return fntStruct.xftFnt->descent;
 #else
 TEXTMETRIC metrics;
 
 if (dblBuffer_)
   GetTextMetrics( gcp, &metrics );
 else    
   GetTextMetrics( gc_, &metrics );

 return metrics.tmDescent; 
 #endif
}


void NGraphics::putStretchBitmap(int x, int y, const NBitmap & bitmap, int width, int height ) {
   #ifdef __unix__
   if (bitmap.sysData() != 0) {

      if (dblBuffer_) {
         if (bitmap.clpData()==0) {
            // first create a new data for the stretched image
            int pixelsize = NApp::system().pixelSize( NApp::system().depth() );
            unsigned char* dst_data = reinterpret_cast<unsigned char*> ( malloc( width * height * pixelsize) );
            for ( int i = 0; i < width* height * pixelsize; i++ ) {
              dst_data[i];
						}

            // get the original data
            unsigned char* originalData = (unsigned char*) bitmap.sysData();
            // calculate the xtretch / ystretch
            double xstretch = 1; // (double)bitmap.width() / width;
            double ystretch = 1; //(double)bitmap.height() / height;

            // we have 24 bpp resolution
            if (pixelsize == 4)
                // loop over the original bitmap data
                for (int i = 0; i < bitmap.width(); i++) {
                  for (int j = 0; j < bitmap.height(); j++) {
                    unsigned char r = originalData[(i*bitmap.width()+j)*pixelsize];
                    unsigned char g = originalData[(i*bitmap.width()+j)*pixelsize+1];
                    unsigned char b = originalData[(i*bitmap.width()+j)*pixelsize+2];
  
                    int newDataPos = std::min( (int) (((i*ystretch)*width + j*xstretch)*pixelsize), width*height*pixelsize - pixelsize);

                    dst_data[newDataPos] =  r;
                    dst_data[newDataPos+1] =  g;
                    dst_data[newDataPos+2] =  b;
                  }
                }

                // create now a new XImage from the stretched data

                int depth  = bitmap.sysData()->depth;
                int pad    = bitmap.sysData()->bitmap_pad;
                int bytes_per_line = bitmap.sysData()->bytes_per_line;

                XImage* xi = XCreateImage(NApp::system().dpy(), NApp::system().visual(), depth, ZPixmap,0,(char*) dst_data , width, height, pad , bytes_per_line );

                // write the image to the screen

                XPutImage(NApp::system().dpy(),doubleBufferPixmap_,gcp,xi,0,0,x+dx_,y+dy_,xi->width,xi->height);

   	        // destroys the image
                XDestroyImage(xi);
              }
						} 
          }              
  #endif
}

void NGraphics::putBitmap( int x, int y, const NBitmap & bitmap )
{
  #ifdef __unix__
  if (bitmap.sysData() != 0) {

      if (dblBuffer_) {
            if (bitmap.clpData()==0) {
               XPutImage(NApp::system().dpy(), doubleBufferPixmap_, gcp,bitmap.sysData(),0, 0, x+dx_,y+dy_, bitmap.width(),bitmap.height());
            } else
           {
             // transparent bitmap;
             XImage* clp = bitmap.clpData();
             Pixmap pix;
             pix = XCreatePixmap(NApp::system().dpy(), doubleBufferPixmap_, clp->width, clp->height, clp->depth);

             GC gc1 = XCreateGC (NApp::system().dpy(), pix, 0, NULL);
             XPutImage(NApp::system().dpy(), pix, gc1, clp, 0, 0, 0, 0, clp->width, clp->height);

             XSetClipMask(NApp::system().dpy(), gcp, pix);
             XSetClipOrigin(NApp::system().dpy(), gcp, x+dx_, y+dy_);
						 // todo valgrind check error on some images
             XPutImage(NApp::system().dpy(), doubleBufferPixmap_, gcp, bitmap.sysData(), 0, 0, x+dx_, y+dy_, bitmap.width(), bitmap.height() );
						 // valgrind check error end
             XSetClipMask(NApp::system().dpy(), gcp, None);
             XFreeGC(NApp::system().dpy(), gc1);
             XFreePixmap(NApp::system().dpy(), pix);
           }}
      else
            XPutImage(NApp::system().dpy(), win, gc_,bitmap.sysData(),
                0, 0, x+dx_,y+dy_, bitmap.width(),bitmap.height());
  }
  #else

  if ( bitmap.clpData() ) {  
    // bitmap is transparent      
    HDC hdcMem = CreateCompatibleDC( gcp );
    HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, bitmap.clpData() );

    SelectObject( hdcMem, bitmap.clpData() );
    BitBlt( gcp, x+dx_, y+dy_, bitmap.width(), bitmap.height(), hdcMem , 0,0, SRCAND);    
    SelectObject( hdcMem, bitmap.sysData() );
    BitBlt( gcp, x+dx_, y+dy_, bitmap.width(), bitmap.height(), hdcMem , 0,0, SRCPAINT);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);

  } else
  if ( bitmap.sysData() ) {    

    HDC hdcMem = CreateCompatibleDC( gcp );
    HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, bitmap.sysData() );

    BitBlt( gcp, x+dx_, y+dy_, bitmap.width(), bitmap.height(), hdcMem , 0,0, SRCCOPY);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);

  }  
  #endif
}





void NGraphics::putBitmap( int destX, int destY, int width, int height, const NBitmap & bitmap, int srcX, int srcY )
{
  #ifdef __unix__
   if ( bitmap.sysData() ) {

      if (dblBuffer_)
            XPutImage(NApp::system().dpy(), doubleBufferPixmap_, gcp,bitmap.sysData(),
                srcX, srcY, destX+dx_,destY+dy_, width,height);
      else
            XPutImage(NApp::system().dpy(), win, gc_,bitmap.sysData(),
                srcX, srcY, destX+dx_,destY+dy_, width,height);
                
  }
  #else
  
  if ( bitmap.sysData() ) {    

    HDC hdcMem = CreateCompatibleDC( gcp );
    HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, bitmap.sysData() );

    BitBlt( gcp, destX+dx_, destY+dy_, width, height, hdcMem , srcX, srcY, SRCCOPY);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);

  } 
  
  #endif
}

void NGraphics::copyArea(int src_x,int src_y,unsigned width,unsigned height,int dest_x,int dest_y, bool dblBuffer_)
{
    #ifdef __unix__
    if (width != 0 && height != 0) {
      if (dblBuffer_)
           XCopyArea(NApp::system().dpy(),/*win,win,gc_*/doubleBufferPixmap_,doubleBufferPixmap_,gcp,
                  src_x, src_y, width, height, dest_x, dest_y); else
           XCopyArea(NApp::system().dpy(),win,win,gc_,
                  src_x, src_y, width, height, dest_x, dest_y);
    }
    #else
    
    if (width != 0 && height != 0) {
       if (dblBuffer_)
         BitBlt( gcp, dest_x+dx_, dest_y+dy_, width, height, gcp , src_x, src_y, SRCCOPY);      
       else
         BitBlt( gc_, dest_x+dx_, dest_y+dy_, width, height, gc_ , src_x, src_y, SRCCOPY);
    }          

    #endif
}

void NGraphics::setDoubleBuffer( bool on )
{
  if (dblBuffer_ == on) return;
  if (dblBuffer_ && !on) {
       dblBuffer_ = on;
  } else
  if (!dblBuffer_ && on) {
      dblBuffer_ = on;
  }
}

#ifdef __unix__
Pixmap NGraphics::dbPixmap( )
{
  return doubleBufferPixmap_;
}
#endif

GC NGraphics::dbGC( )
{
  return gcp;
}

GC NGraphics::gc( )
{
  return gc_;
}


void NGraphics::fillGradient(int x, int y, int width, int height, const NColor & start, const  NColor & end , int direction) {
  int middle = (direction == nHorizontal) ? width : height;

  int r1 = start.red();
  int g1 = start.green();
  int b1 = start.blue();

  int r2 = end.red();
  int g2 = end.green();
  int b2 = end.blue();

  double dr = (r2-r1) / (double) middle;
  double dg = (g2-g1) / (double) middle;
  double db = (b2-b1) / (double) middle;


  if (direction == nHorizontal)
    for (int i = 0; i < middle; i++) {
      setForeground(NColor((int) (r1 + i*dr),(int) (g1 + i*dg),(int) (b1 + i*db)));
      fillRect(x+i,y,1,height);
    }
  else
    for (int i = 0; i < middle; i++) {
      setForeground(NColor((int) (r1 + i*dr),(int) (g1 + i*dg), (int) (b1 + i*db)));
      fillRect(x,y+i,width,1);
    }
}


const NRegion & NGraphics::repaintArea( )
{
  return repaintArea_;
}

void NGraphics::setRepaintArea( const NRegion & region )
{
  repaintArea_ = region;
}

int NGraphics::dblWidth( ) const
{
  return dblWidth_;
}

int NGraphics::dblHeight( ) const
{
  return dblHeight_;
}


void NGraphics::drawRoundRect( int x, int y, int width, int height, int arcWidth, int arcHeight ) {

 #ifdef __unix__

 int nx = x;
 int ny = y;
 int nw = width;
 int nh = height;
 int naw = arcWidth;
 int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
                if (nh > nah) {
                        drawArc(nx, ny, naw, nah, 5760, 5760);
                        drawLine(nx + naw2, ny, nx + nw - naw2, ny);
                        drawArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                        drawLine(nx + nw, ny + nah2, nx + nw, ny + nh - nah2);
                        drawArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280, 5760);
                        drawLine(nx + naw2, ny + nh, nx + nw - naw2, ny + nh);
                        drawArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                        drawLine(nx, ny + nah2, nx, ny + nh - nah2);
                } else {
                        drawArc(nx, ny, naw, nh, 5760, 11520);
                        drawLine(nx + naw2, ny, nx + nw - naw2, ny);
                        drawArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
                        drawLine(nx + naw2, ny + nh, nx + nw - naw2, ny + nh);
                }
        } else {
                if (nh > nah) {
                        drawArc(nx, ny, nw, nah, 0, 11520);
                        drawLine(nx + nw, ny + nah2, nx + nw, ny + nh - nah2);
                        drawArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
                        drawLine(nx, ny + nah2, nx, ny + nh - nah2);
                } else {
                        drawArc(nx, ny, nw, nh, 0, 23040);
                }
        }
  #else
  
  if ( dblBuffer_ ) {
    HBRUSH holdbrush = (HBRUSH) SelectObject( gcp, hollow );
    RoundRect( gcp, x + dx_, y + dy_ , x + dx_ + width, y + dy_ + height, arcWidth, arcHeight );
    SelectObject( gcp, holdbrush );
  } else {
    HBRUSH holdbrush = (HBRUSH) SelectObject( gcp, hollow );   
    RoundRect( gc_, x + dx_, y + dy_ , x + dx_ + width, y + dy_ + height, arcWidth, arcHeight );
    SelectObject( gc_, holdbrush );    
  }
  #endif        
}

static double fTwoPi = 2.0 * 3.14; 


#ifdef __unix__
#else
void NGraphics::drawArcX( int x, int y, int width, int height, int start, int extent, bool fill ) {
  
  int clockwise = (extent < 0); /* non-zero if clockwise */
  int xstart, ystart, xend, yend;
  double radian_start, radian_end, xr, yr;

  //
  // Compute the absolute starting and ending angles in normalized radians.
  // Swap the start and end if drawing clockwise.
  //

  start = start % (64*360);
  if (start < 0) {
    start += (64*360);
  }
  extent = (start+extent) % (64*360);
  if (extent < 0) {
    extent += (64*360);
  }
  if (clockwise) {
    int tmp = start;
    start = extent;
    extent = tmp;
  }
  radian_start = XAngleToRadians(start);
  radian_end = XAngleToRadians(extent);

  //
  // Now compute points on the radial lines that define the starting and
  // ending angles.  Be sure to take into account that the y-coordinate
  // system is inverted.
  //

   xr = x + width / 2.0;
   yr = y + height / 2.0;
   xstart = (int)((xr + cos(radian_start)*width/2.0) + 0.5);
   ystart = (int)((yr + sin(-radian_start)*height/2.0) + 0.5);
     
   xend = (int)((xr + cos(radian_end)*width/2.0) + 0.5);
   yend = (int)((yr + sin(-radian_end)*height/2.0) + 0.5);

   if ( !fill ) {
     if ( dblBuffer_ )
       Arc( gcp, x , y  , x + width +1, y + height+1, xstart, ystart, xend, yend );
     else
       Arc( gc_, x , y  , x + width +1, y + height+1, xstart, ystart, xend, yend );     
   } else {
     if ( dblBuffer_ )
       Chord( gcp, x, y, x+width+1, y+height+1, xstart, ystart, xend, yend);
     else
       Chord( gc_, x, y, x+width+1, y+height+1, xstart, ystart, xend, yend);
   }    
}
#endif

void NGraphics::drawArc( int x, int y, int width, int height, int start, int extent )
{
  #ifdef __unix__
  if (dblBuffer_)
    XDrawArc( NApp::system().dpy(), doubleBufferPixmap_, gcp, x+dx_, y+dy_, width, height, start, extent );
  else
    XDrawArc( NApp::system().dpy(), win, gc_, x+dx_, y+dy_, width, height, start, extent );
  #else
  drawArcX( x +dx_, y+ dy_, width, height, start, extent , 0 );
  #endif     
}

void NGraphics::fillArc( int x, int y, int width, int height, int angle1, int angle2 )
{
  #ifdef __unix__   
  if (dblBuffer_)
     XFillArc(NApp::system().dpy(),doubleBufferPixmap_,gcp,x+dx_,y+dy_,width,height,angle1,angle2);
  else
     XFillArc(NApp::system().dpy(),win,gc_,x+dx_,y+dy_,width,height,angle1,angle2);
  #else
  drawArcX( x + dx_, y + dy_, width, height, angle1, angle2, 1 );
  #endif
}

void NGraphics::fillRect( const NRect & rect )
{
  fillRect(rect.left(),rect.top(),rect.width(),rect.height());
}

void NGraphics::fillRoundRect( int x, int y, int width, int height, int arcWidth, int arcHeight )
{
 int nx = x;
 int ny = y;
 int nw = width;
 int nh = height;
 int naw = arcWidth;
 int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
                if (nh > nah) {
                        fillArc(nx, ny, naw, nah, 5760, 5760);
                        fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                        fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
                        fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                        fillRect(nx,ny+nah2,nw,nh- 2*nah2);
                        fillRect(nx+naw2,ny,nw-2*naw2,nah2);
                        fillRect(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2);
                } else {
                        fillArc(nx, ny, naw, nh, 5760, 11520);
                        fillArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
                        fillRect(nx+naw2,ny,nw-2*naw2,nah2);
                }
        } else {
                if (nh > nah) {
                        fillArc(nx, ny, nw, nah, 0, 11520);
                        fillArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
                        fillRect(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2);
                } else {
                        fillArc(nx, ny, nw, nh, 0, 23040);
                }
        }
}

void NGraphics::fillRoundGradient( int x, int y, int width, int height, const NColor & start, const NColor & end, int direction, int arcWidth, int arcHeight )
{
  int nx = x;
  int ny = y;
  int nw = width;
  int nh = height;
  int naw = arcWidth;
  int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
                if (nh > nah) {
                        if (direction == nHorizontal) {
                          setForeground(start);
                          fillArc(nx, ny, naw, nah, 5760, 5760);
                          fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                          setForeground(end);
                          fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                          fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
                        } else {
                          fillArc(nx, ny, naw, nah, 5760, 5760);
                          fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                          setForeground(end);
                          fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                          fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
                        }
                        fillGradient(nx,ny+nah2,nw,nh- 2*nah2,start,end,direction);
                        fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,end,direction);
                        fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,end,direction);
                } else {
                        fillArc(nx, ny, naw, nh, 5760, 11520);
                        fillArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
                        fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,end,direction);
                }
        } else {
                if (nh > nah) {
                        fillArc(nx, ny, nw, nah, 0, 11520);
                        fillArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
                        fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,end,direction);
                } else {
                        fillArc(nx, ny, nw, nh, 0, 23040);
                }
        }
}


void NGraphics::fillGradient( int x, int y, int width, int height, const NColor & start, const NColor & mid, const NColor & end, int direction, int percent )
{
        int middle  = 0;
        int length  = 0;
        if (direction == nHorizontal) {
            middle = (int) (width  * percent/100.0f);
            length = width  - middle;
        } else {
            middle = (int) (height * (percent/100.0f));
            length = height - middle;
        }
        //frist part to middle
        if (direction == nHorizontal)
             fillGradient(x,y,middle,height, start,mid,nHorizontal);
        else 
            fillGradient(x,y,width,middle,start,mid,nVertical);
        // second part from middle to end
        if (direction == nHorizontal)
            fillGradient(x+middle,y,width-middle,height,mid,end,nHorizontal);
        else
            fillGradient(x,y+middle,width,height-middle,mid,end,nVertical);

}

void NGraphics::fillRoundGradient( int x, int y, int width, int height, const NColor & start, const NColor & mid, const NColor & end, int direction, int percent , int arcWidth, int arcHeight)
{
  int nx = x;
  int ny = y;
  int nw = width;
  int nh = height;
  int naw = arcWidth;
  int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
                if (nh > nah) {
                        if (direction == nHorizontal) {
                          setForeground(start);
                          fillArc(nx, ny, naw, nah, 5760, 5760);
                          fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                          setForeground(end);
                          fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                          fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
                          fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,mid,end,direction,percent);
                          fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,mid,end,direction,percent);
                        } else {
                          setForeground(start);
                          fillArc(nx, ny, naw, nah, 5760, 5760);
                          fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                          fillRect(nx+naw2,ny,nw-2*naw2,nah2);
                          setForeground(end);
                          fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                          fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
                          fillRect(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2);
                        }
                        fillGradient(nx,ny+nah2,nw,nh- 2*nah2,start,mid,end,direction,percent);
                } else {
                        fillArc(nx, ny, naw, nh, 5760, 11520);
                        fillArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
                        fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,mid,end,direction,percent);
                }
        } else {
                if (nh > nah) {
                        fillArc(nx, ny, nw, nah, 0, 11520);
                        fillArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
                        fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,mid,end,direction,percent);
                } else {
                        fillArc(nx, ny, nw, nh, 0, 23040);
                }
        }
}


int NGraphics::textWidth( const NFntString & text ) const
{
   #ifdef __unix__
   NFontStructure newFntStruct = fntStruct;
   int pos = 0; int w = 0;
   std::vector<NFont>::const_iterator fntIt = text.fonts().begin();
   for (std::vector<int>::const_iterator it = text.positions().begin(); it < text.positions().end(); it++) {
     int old = pos;
     pos = *it;

     const char* s = text.textsubstr(old,pos-old).c_str();
     if (!newFntStruct.antialias)
       w+=XTextWidth(newFntStruct.xFnt,s,strlen(s));
     else {
       XGlyphInfo info;
       XftTextExtents8(NApp::system().dpy(),newFntStruct.xftFnt
       ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
       w+= info.xOff;
     }

     newFntStruct = (*fntIt).systemFont();
     fntIt++;
   }
   const char* s = text.textsubstr(pos).c_str();

   if (!newFntStruct.antialias)
       w+=XTextWidth(newFntStruct.xFnt,s,strlen(s));
     else {
       XGlyphInfo info;
       XftTextExtents8(NApp::system().dpy(),newFntStruct.xftFnt
       ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
       w+= info.xOff;
     }

   return w;
   #else
   ///\todo port
   return 0;
   #endif
}

void NGraphics::drawText( int x, int y, const NFntString & text )
{
  int pos = 0; 
  int w = 0;
   std::vector<NFont>::const_iterator fntIt = text.fonts().begin();
   for (std::vector<int>::const_iterator it = text.positions().begin(); it < text.positions().end(); it++) {
     int old = pos;
     pos = *it;
     drawText(x+w,y,text.textsubstr(old,pos-old));
     w += textWidth(text.textsubstr(old,pos-old));
     NFont fnt = *fntIt;
     setFont(fnt);
     fntIt++;
   }
   drawText(x+w,y,text.textsubstr(pos));
}


int NGraphics::findWidthMax( long width, const std::string & data, bool wbreak ) const
{
  int Low = 0; int High = data.length();  int Mid=High;
  while( Low <= High ) {
    Mid = ( Low + High ) / 2;
    std::string s     = data.substr(0,Mid);
    std::string snext;
    if (Mid>0) snext  = data.substr(0,Mid+1); else snext = s;
    int w     = textWidth(s);
    if(  w < width  ) {
                        int wnext = textWidth(snext);
                        if (wnext  >= width ) break;
                        Low = Mid + 1;
                      } else
                      {
                        High = Mid - 1;
                      }
  }
  if (!wbreak || data.substr(0,Mid).find(" ")==std::string::npos || Mid == 0 || Mid>=data.length()) return Mid; else
  {
    int p = data.rfind(" ",Mid);
    if (p!=std::string::npos ) return p+1;
  }
  return Mid;
}

int NGraphics::findWidthMax(long width, const NFntString & data, bool wbreak) const
{
  int Low = 0; int High = data.length();  int Mid=High;
  while( Low <= High ) {
    Mid = ( Low + High ) / 2;
    NFntString s     = data.substr(0,Mid);
    NFntString snext;
    if (Mid>0) snext  = data.substr(0,Mid+1); else snext = s;
    int w     = textWidth(s);
    if(  w < width  ) {
                        int wnext = textWidth(snext);
                        if (wnext  >= width ) break;
                        Low = Mid + 1; 
                      } else
                      {
                        High = Mid - 1;
                      }
  }
  if (!wbreak || data.textsubstr(0,Mid).find(" ")==std::string::npos || Mid == 0 || Mid>=data.length()) return Mid; else
  {
    int p = data.rfind(" ",Mid);
    if (p!=std::string::npos ) return p+1;
  }
  return Mid;

}

void NGraphics::setVisible( bool on )
{
  visible_ = on;
  if (!on) {
    destroyDblBufferHandles();
    dblBuffer_ = false;
  } else {
    dblBuffer_ = true;
    createDblBufferHandles();
  }
}

void NGraphics::putPixmap( int destX, int destY, int width, int height, NPixmap & pixmap, int srcX, int srcY )
{
  #ifdef __unix__
  if (pixmap.X11Pixmap() != 0) {

      if (dblBuffer_)
            XCopyArea(NApp::system().dpy(),  pixmap.X11Pixmap(), doubleBufferPixmap_,gcp,
                srcX, srcY, width, height, destX+dx_,destY+dy_);
      else
            XCopyArea(NApp::system().dpy(),  pixmap.X11Pixmap(), win ,gc_,
                srcX, srcY, width, height, destX+dx_,destY+dy_);
  }
  #else
  putBitmap( destX, destY, width, height, ( NBitmap & ) pixmap, srcX, srcY )  ;
  #endif
}

// sets and gets the pen (line style etc ..)

void NGraphics::setPen( const NPen & pen )
{
  pen_ = pen;
  #ifdef __unix__
  if (dblBuffer_) {
    XSetLineAttributes(NApp::system().dpy(), gcp , pen.lineWidth(), (int) pen. lineStyle(), pen.capStyle(), pen.joinStyle() );
    XSetFillStyle(NApp::system().dpy(), gcp, pen.fillStyle() );
    XSetFunction(NApp::system().dpy(), gcp, pen.function() );
  } else {
    XSetLineAttributes(NApp::system().dpy(), gc() , pen.lineWidth(), (int) pen. lineStyle(), pen.capStyle(), pen.joinStyle() );
    XSetFillStyle(NApp::system().dpy(), gc() , pen.fillStyle() );
    XSetFunction(NApp::system().dpy(), gc() , pen.function() );
  }
  #else

  LOGPEN logPen;
  
  if ( dblBuffer_ ) {
    GetObject( hPen, sizeof(logPen), &logPen );
    logPen.lopnWidth.x = pen.lineWidth();
	logPen.lopnStyle = pen.lineStyle();
    DeleteObject( hPen ) ;
    hPen = CreatePenIndirect(&logPen);
	SelectObject( gcp, hPen );
  } else {
    GetObject( hPen, sizeof(logPen), &logPen );
    logPen.lopnWidth.x = pen.lineWidth();
	logPen.lopnStyle = pen.lineStyle();
    DeleteObject( hPen ) ;
    hPen = CreatePenIndirect(&logPen);
	SelectObject( gc_, hPen );
  }
  
  #endif
}

const NPen & NGraphics::pen( ) const
{
  return pen_;
}

void NGraphics::resetPen( )
{
  setPen(NPen());
}

