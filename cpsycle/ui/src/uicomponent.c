// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "uicomponent.h"
#include "uimenu.h"
#include "hashtbl.h"
#include <memory.h>
#include <commctrl.h>   // includes the common control header


static TCHAR szAppClass[] = TEXT ("PsycleApp") ;
static TCHAR szComponentClass[] = TEXT ("PsycleComponent") ;
IntHashTable selfmap;
IntHashTable winidmap;
int winid = 20000;
extern IntHashTable menumap;

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                           WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);

HINSTANCE appInstance = 0;

void ui_init(HINSTANCE hInstance)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	int succ;
	
	InitIntHashTable(&selfmap, 100);
	InitIntHashTable(&winidmap, 100);

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = ui_winproc ;
    wndclass.cbClsExtra    = 0 ;
    wndclass.cbWndExtra    = 0 ;
    wndclass.hInstance     = hInstance ;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = szAppClass ;	
	if (!RegisterClass (&wndclass))
    {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      szAppClass, MB_ICONERROR) ;		
    }
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = ui_winproc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (long); 
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szComponentClass;
     

	RegisterClass (&wndclass) ;

	ui_menu_setup();


	// Ensure that the common control DLL is loaded.     		
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);        

	appInstance = hInstance;
}

void ui_dispose()
{
	DisposeIntHashTable(&selfmap);
	DisposeIntHashTable(&winidmap);
}


LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{	
    PAINTSTRUCT  ps ;     
	ui_component*   component;
	ui_graphics	 g;
	HMENU		 hMenu;
	ui_menu*	 menu;
	int			 menu_id;		

	switch (message)
    {		
		case WM_SIZE:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.size) {				                    
				component->events.size(component->events.target, LOWORD (lParam), HIWORD (lParam));
				return 0 ;
			}			
		break;
		case WM_TIMER:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.timer) {
				component->events.timer(component->events.target, (int) wParam);
				return 0 ;
			}
		break;
		case WM_COMMAND:
          hMenu = GetMenu (hwnd) ;
		  menu_id = LOWORD (wParam);
          menu = SearchIntHashTable(&menumap, menu_id);		  
          if (menu && menu->execute) {	
			menu->execute(menu);
		  }
		  component = SearchIntHashTable(&winidmap, LOWORD(wParam));
			if (component && component->events.command) {				                    
				component->events.command(component->events.cmdtarget, wParam, lParam);
				return 0;
			}
		  return 0 ;  
		break;          
		case WM_CREATE:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.create) {	
				component->events.create(component->events.target);
			}
			return 0 ;
		break;
		case WM_PAINT :			
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.draw) {		
				HDC bufferDC;
				HBITMAP bufferBmp;
				HBITMAP oldBmp;
				HDC hdc;				
				RECT rect;

				ui_graphics_init(&g, BeginPaint (hwnd, &ps));				
				if (component->doublebuffered) {
					hdc = g.hdc;
					bufferDC = CreateCompatibleDC(hdc);
					GetClientRect(hwnd, &rect);
					bufferBmp = CreateCompatibleBitmap(hdc, rect.right,
						rect.bottom);
					oldBmp = SelectObject(bufferDC, bufferBmp);
					g.hdc = bufferDC;					
				}
				component->events.draw(component->events.target, &g);				
				if (component->doublebuffered) {
					g.hdc = hdc;
					BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom,
						bufferDC,0,0,SRCCOPY);				
					SelectObject(bufferDC, oldBmp);
					DeleteObject(bufferBmp);
					DeleteDC(bufferDC);
				}
				ui_graphics_dispose(&g);
				EndPaint (hwnd, &ps) ;
				return 0 ;
			}
		break;
		case WM_DESTROY:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.destroy) {	
				component->events.destroy(component->events.target, component);				
			}
			return 0;
		break;
		case WM_KEYDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.keydown) {	
				int propagate = component->events.keydown(component->events.target, (int)wParam, lParam);
				if (!propagate) {
					return 0;
				} else {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;               
				}
			}			
		break;
		case WM_KEYUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.keyup) {	
				component->events.keyup(component->events.target, (int)wParam, lParam);				
			}
			return 0;
		break;
		case WM_LBUTTONUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mouseup) {				
				component->events.mouseup(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);
				return 0 ;
			}
		break;
		case WM_RBUTTONUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mouseup) {				
				component->events.mouseup(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);
				return 0 ;
			}			
		break;
		case WM_MBUTTONUP:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mouseup) {				
				component->events.mouseup(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);
				return 0 ;
			}
		break;
		case WM_LBUTTONDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mousedown) {				
				component->events.mousedown(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);
				return 0 ;
			}			
		break;
		case WM_RBUTTONDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mousedown) {				
				component->events.mousedown(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 2);
				return 0 ;
			}
		break;
		case WM_MBUTTONDOWN:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mousedown) {				
				component->events.mousedown(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 3);
				return 0 ;
			}
		break;
		case WM_LBUTTONDBLCLK:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mousedown) {				
				component->events.mousedoubleclick(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);
				return 0 ;
			}
		break;
		case WM_MBUTTONDBLCLK:
			
		break;
		case WM_RBUTTONDBLCLK:
			
		break;
		case WM_MOUSEMOVE:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mousemove) {
				component->events.mousemove(component->events.target,
					(SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam), 1);
				return 0 ;
			}
		break;
		case WM_VSCROLL:
			handle_vscroll(hwnd, wParam, lParam);
			return 0;
		break;
		case WM_HSCROLL:
			component = SearchIntHashTable(&selfmap, (int) hwnd);
			if (component && component->events.mousemove) {
				component->events.scroll(component->events.target,
					LOWORD (lParam), HIWORD (lParam), 1);
				return 0 ;
			}
		break;
		default:
		break;
	}	
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}


void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iVertPos; //, iHorzPos;
	int				cyChar;
	ui_component*   component;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_VERT, &si) ;
	cyChar = 20;

	// Save the position for comparison later on

	iVertPos = si.nPos ;

	switch (LOWORD (wParam))
	{
		case SB_TOP:
		   si.nPos = si.nMin ;
		   break ;

		case SB_BOTTOM:
		   si.nPos = si.nMax ;
		   break ;

		case SB_LINEUP:
		   si.nPos -= 1 ;
		   break ;

		case SB_LINEDOWN:
		   si.nPos += 1 ;
		   break ;

		case SB_PAGEUP:
		   si.nPos -= si.nPage ;
		   break ;

		case SB_PAGEDOWN:
		   si.nPos += si.nPage ;
		   break ;

		case SB_THUMBTRACK:
		   si.nPos = si.nTrackPos ;
		   break ;

		default:
		   break ;         
	}
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.

	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_VERT, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_VERT, &si) ;

	// If the position has changed, scroll the window and update it

	if (si.nPos != iVertPos)
	{                    
		component = SearchIntHashTable(&selfmap, (int) hwnd);
		if (component && component->events.scroll) {
			component->events.scroll(component->events.target,
				0, cyChar * (iVertPos - si.nPos));			
		}
		ScrollWindow (hwnd, 0, cyChar * (iVertPos - si.nPos), 
                                   NULL, NULL) ;
		UpdateWindow (hwnd) ;
	}
//	return 0 ;          
}

void ui_frame_init(void* context, ui_component* frame, ui_component* parent)
{		
	HWND hWndParent = 0;
	int style = 0;

	if (parent) {
	  hWndParent = parent->hwnd;
	  // style  |= WS_CHILD;
	}
	memset(&frame->events, 0, sizeof(ui_events));	
	frame->doublebuffered = 0;
	frame->hwnd = CreateWindow (szAppClass, TEXT ("Psycle"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT | style,
                          hWndParent, NULL, appInstance, NULL);     	
	InsertIntHashTable(&selfmap, (int)frame->hwnd, frame);		
	frame->events.target = context;
	frame->events.cmdtarget = frame;	
}

void ui_component_init(void* self, ui_component* component, ui_component* parent)
{		
	memset(&component->events, 0, sizeof(ui_events));
	component->doublebuffered = 0;
	component->hwnd = CreateWindow (szComponentClass, NULL,
		WS_CHILDWINDOW | WS_VISIBLE,
		0, 0, 90, 90,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)component->hwnd, component);
	component->events.target = self;	
}

void ui_classcomponent_init(void* self, ui_component* component, ui_component* parent, const char* classname)
{
	memset(&component->events, 0, sizeof(ui_events));
	component->doublebuffered = 0;
	component->hwnd = CreateWindow (classname, NULL,
		WS_CHILDWINDOW | WS_VISIBLE,
		0, 0, 100, 100,
		parent->hwnd, NULL,
		(HINSTANCE) GetWindowLong (parent->hwnd, GWL_HINSTANCE),
		NULL);		
	InsertIntHashTable(&selfmap, (int)component->hwnd, component);	
	component->events.target = self;
	component->align = 0;
}

ui_size ui_component_size(ui_component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetClientRect (self->hwnd, &rect) ;
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

void ui_component_show_state(ui_component* self, int cmd)
{
	ShowWindow(self->hwnd, cmd);
	UpdateWindow(self->hwnd) ;
}

void ui_component_show(ui_component* self)
{
	ShowWindow(self->hwnd, SW_SHOW);
	UpdateWindow(self->hwnd) ;
}

void ui_component_hide(ui_component* self)
{
	ShowWindow(self->hwnd, SW_HIDE);
	UpdateWindow(self->hwnd) ;
}

void ui_component_showhorizontallscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_HSCROLL);
}


void ui_component_hidehorizontallscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
}

void ui_component_sethorizontalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;
	SetScrollInfo(self->hwnd, SB_HORZ, &si, TRUE);
}

void ui_component_showverticalscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) | WS_VSCROLL);
}

void ui_component_hideverticalscrollbar(ui_component* self)
{
	SetWindowLong(self->hwnd, GWL_STYLE, 
		GetWindowLong(self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
}

void ui_component_setverticalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo(self->hwnd, SB_VERT, &si, TRUE);
}

void ui_component_move(ui_component* self, int left, int top)
{
	SetWindowPos (self->hwnd, NULL, 
	   left,
	   top,
	   0, 0, SWP_NOZORDER | SWP_NOSIZE) ;	
}

void ui_component_resize(ui_component* self, int width, int height)
{

	SetWindowPos (self->hwnd, NULL, 
	   0,
	   0,
	   width, height, SWP_NOZORDER | SWP_NOMOVE) ;
}

void ui_component_setmenu(ui_component* self, ui_menu* menu)
{
	SetMenu(self->hwnd, menu->hmenu);
}

void ui_component_settitle(ui_component* self, const char* title)
{
	SetWindowText(self->hwnd, title);
}

void ui_component_enum_children(ui_component* self)
{
	EnumChildWindows (self->hwnd, ChildEnumProc, (LPARAM) self) ;
}

BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam)
{
	ui_component* self = (ui_component*) lParam;
	ui_component* child = SearchIntHashTable(&selfmap, (int)hwnd);
	if (child && self->events.childenum) {
	  return self->events.childenum(self->events.target, child);		  
	}     
    return FALSE ;
}

void ui_component_capture(ui_component* self)
{
	SetCapture(self->hwnd);
}

void ui_component_releasecapture()
{
	ReleaseCapture();
}

void ui_invalidate(ui_component* self)
{
	InvalidateRect(self->hwnd, NULL, TRUE);
}

void ui_component_setfocus(ui_component* self)
{
	SetFocus(self->hwnd);
}

void ui_component_setfont(ui_component* self, HFONT hFont)
{
     SendMessage (self->hwnd, WM_SETFONT, (WPARAM) hFont, 0) ;
}