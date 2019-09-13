// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "graphics.h"

void ui_graphics_init(ui_graphics* g, HDC hdc)
{
	g->hdc = hdc;	
	g->hFontPrev = 0;
}	

void ui_graphics_dispose(ui_graphics* g)
{
	if (g->hFontPrev != 0) {		
		SelectObject (g->hdc, g->hFontPrev);
	}
}


void ui_textout(ui_graphics* self, int x, int y, const char* str, int len)
{	
	TextOut(self->hdc, x, y, str, len);
}

void ui_textoutrectangle(ui_graphics* g, int x, int y, unsigned int options,
	ui_rectangle r, const char* text, int len)
{
	RECT rect;
		                
    SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     	
	ExtTextOut(g->hdc, x, y, options, &rect, text, len, NULL);
}

ui_size ui_textsize(ui_graphics* g, const char* text)
{
	ui_size	rv;
	SIZE size;
	
	GetTextExtentPoint(g->hdc, text, strlen(text), &size) ;	
	rv.width = size.cx; 
	rv.height = size.cy;
	return rv;
}

void ui_drawrectangle(ui_graphics* self, const ui_rectangle r)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;

	hBrush = GetStockObject (NULL_BRUSH);
	hOldBrush = SelectObject (self->hdc, hBrush);
	Rectangle(self->hdc, r.left, r.top, r.right, r.bottom);
	SelectObject (self->hdc, hOldBrush);
}

void ui_drawsolidrectangle(ui_graphics* g, const ui_rectangle r, unsigned int color)
{
     HBRUSH hBrush;     
     RECT   rect;	 
	                
     SetRect (&rect, r.left, r.top, r.right, r.bottom) ;     
     hBrush = CreateSolidBrush(color) ;     
     FillRect (g->hdc, &rect, hBrush);     
     DeleteObject (hBrush) ;
}     

void ui_drawfullbitmap(ui_graphics* g, ui_bitmap* bitmap, int x, int y)
{
	HDC hdcMem;
	ui_size size;

	hdcMem = CreateCompatibleDC (g->hdc) ;
	SelectObject (hdcMem, bitmap->hBitmap) ;
	size = ui_bitmap_size(bitmap);
	BitBlt(g->hdc, x, y, size.width, size.height, hdcMem, 0, 0, SRCCOPY);
	DeleteDC (hdcMem);  
}

void ui_drawbitmap(ui_graphics* g, ui_bitmap* bitmap, int x, int y, int width,
	int height, int xsrc, int ysrc)
{
	HDC hdcMem;

	hdcMem = CreateCompatibleDC (g->hdc) ;
	SelectObject (hdcMem, bitmap->hBitmap) ;	
	BitBlt(g->hdc, x, y, width, height, hdcMem, xsrc, ysrc, SRCCOPY);
	DeleteDC (hdcMem);  
}

void ui_setbackgroundcolor(ui_graphics* g, unsigned int color)
{
	SetBkColor(g->hdc, color);
}

void ui_settextcolor(ui_graphics* g, unsigned int color)
{
	SetTextColor(g->hdc, color);
}

void ui_setfont(ui_graphics* g, ui_font* font)
{	
	if (font && font->hfont) {
		g->hFontPrev = SelectObject(g->hdc, font->hfont);
	}
}

ui_font ui_createfont(const char* name, int size)
{
	ui_font font;
	font.hfont = CreateFont(size, 0, 0, 0, 0, 0, 0, 0, 
		1, 0, 0, 0, VARIABLE_PITCH | FF_DONTCARE, name);          
	font.stock = 0;
	return font;
}

void ui_deletefont(HFONT hfont)
{	
	DeleteObject(hfont);
}

void ui_drawline(ui_graphics* g, int x1, int y1, int x2, int y2)
{
	MoveToEx(g->hdc, x1, y1, NULL) ;
	LineTo (g->hdc, x2, y2);
}
