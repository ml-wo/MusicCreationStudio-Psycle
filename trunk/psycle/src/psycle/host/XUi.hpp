#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Ui.hpp"
#include <assert.h> 	

#define NIL (0)  

namespace psycle {
namespace host {
namespace ui {
namespace x {

extern Display* display();

struct DummyWindow {
  static ::Window dummy();
	
 private:
  static ::Window dummy_wnd_;
};	
		
class AppImp : public ui::AppImp {
 public:
   AppImp();
   virtual ~AppImp();
  
   Display* dpy() { return dpy_; }
   Colormap default_colormap() const { return default_colormap_; }   
   virtual void DevRun();   
   void RegisterWindow(::Window w, ui::WindowImp* imp) { windows_[w] = imp; }
   void UnregisterWindow(::Window w) { windows_[w] = 0; }
   ui::WindowImp* find(::Window w);
   
 private:
   Display *dpy_;
   Colormap default_colormap_;
   std::map< ::Window, ui::WindowImp*> windows_;
};

extern AppImp* app_imp();


class FontInfoImp : public ui::FontInfoImp {
 public:
  FontInfoImp() : stock_id_(-1), name_("6x10"), height_(10), is_italic_(false) {    
  }
  ~FontInfoImp() {}

  virtual FontInfoImp* DevClone() const { 
    FontInfoImp* clone(new FontInfoImp(*this));
    return clone;
  }

  virtual void dev_set_style(FontStyle::Type style) {  
    if (style == FontStyle::ITALIC) {
      is_italic_ = true;
    }
  }
  virtual FontStyle::Type dev_style() const {
    FontStyle::Type result = FontStyle::NORMAL;
    if (is_italic_) {
      result = FontStyle::ITALIC;
    }
    return result;
  }
  virtual void dev_set_size(int size) { height_ = size; }
  virtual int dev_size() const { return height_; }
  virtual void dev_set_weight(int weight) { weight_ = weight; }  
  virtual int dev_weight() const { return weight_; }  
  virtual void dev_set_family(const std::string& family) {
     name_ = family;
  }
  virtual std::string dev_family() const {
    return name_;    
  }
  virtual void dev_set_stock_id(int id) { stock_id_ = id; }
  virtual int dev_stock_id() const { return stock_id_; }

 private:  
  int height_;  
  int  weight_;
  bool  is_italic_;
  std::string name_;  
  int stock_id_;
};

class SystemMetrics : public ui::SystemMetrics {
 public:    
  SystemMetrics() {}
  virtual ~SystemMetrics() {}

  virtual ui::Dimension screen_dimension() const {
    return ui::Dimension(800, 600); 
	  // GetSystemMetrics(SM_CXFULLSCREEN),
          // GetSystemMetrics(SM_CYFULLSCREEN));
  }
  virtual ui::Dimension scrollbar_size() const {
    return ui::Dimension(17, 17);
	  // GetSystemMetrics(SM_CXHSCROLL),
          // GetSystemMetrics(SM_CXVSCROLL));
  }
};


class FontImp : public ui::FontImp {
 public:
  FontImp() : stock_id_(-1) {
    xfont_ = XLoadQueryFont(display(), "6x10");
  }

  FontImp(int stock_id) : stock_id_(stock_id) {
    // cfont_ = (HFONT) GetStockObject(stock_id);	  
    xfont_ = NIL;
  }
  
  virtual ~FontImp() {
    if (stock_id_ == -1 && xfont_ != NIL) {      
      XFreeFont(display(), xfont_);
    }
  }

  virtual FontImp* DevClone() const { 
    FontImp* clone(NIL);
    if (stock_id_  == -1) {
      clone = new FontImp();
      clone->dev_set_font_info(dev_font_info());
    } else {
      clone = new FontImp(stock_id_);      
    }
    return clone;
  }

  virtual void dev_set_font_info(const FontInfo& info) {
    if (info.stock_id() == -1) {      
      XFreeFont(display(), xfont_);
      stock_id_ = info.stock_id();
      xfont_ = XLoadQueryFont(display(), info.family().c_str());
    } else {
      XFreeFont(display(), xfont_);
      //cfont_ = (HFONT) ::GetStockObject(info.stock_id());
      stock_id_ = info.stock_id();
    }
  }

  virtual FontInfo dev_font_info() const {
    /*LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));		
	::GetObject(cfont_, sizeof(LOGFONT), &lfLogFont);    
    FontInfo info = TypeConverter::font_info(lfLogFont);    
    info.set_stock_id(stock_id_);*/
    FontInfo info;	  
    return info;
  }

  XFontStruct* xfont() const { return xfont_; }

 private:
   mutable XFontStruct* xfont_;   
   mutable int stock_id_;
};

class GraphicsImp : public ui::GraphicsImp {
 public:	
   GraphicsImp() {gc_ = 0; }   
   GraphicsImp(::Window w);         
   virtual ~GraphicsImp() { DevDispose(); }
	   
   virtual void DevDispose();
   void DevFillRect(const Rect& rect);
   void DevSetColor(ARGB color);
   ARGB dev_color() const { return argb_color_; }
   void DevDrawString(const std::string& str, const Point& position) {
      // check_pen_update();		
      XDrawString(display(), w_, gc_, position.x(), position.y() + 10, str.c_str(), str.length());	   
      //cr_->TextOut(static_cast<int>(position.x()), static_cast<int>(position.y()),
      //Charset::utf8_to_win(str).c_str());
  }   
   
  private:
   void Init();
   GC gc_;
   ARGB argb_color_;   
   ::Window w_;  
   ui::Font font_;
};

class RegionImp : public ui::RegionImp {
 public:
  RegionImp();   
  RegionImp(const ::Region& rgn);
  virtual ~RegionImp();
  
  virtual RegionImp* DevClone() const;
  void DevOffset(double dx, double dy);
  virtual void DevUnion(const ui::Region& other);
  ui::Rect DevBounds() const;
  bool DevIntersect(double x, double y) const;
  bool DevIntersectRect(const ui::Rect& rect) const;
  virtual void DevSetRect(const ui::Rect& rect);
  void DevClear();
  ::Region& xrgn() { return rgn_; }

 private:  
  ::Region rgn_;
};

template <typename I>
class WindowTemplateImp : public I {
  public:    
   ::Window x_window() { return x_window_; }
    ::Window x_window() const { return x_window_; }
  
  virtual void DevShow() {
    Display* dpy(display());
    XMapWindow(dpy, x_window_);    
    XFlush(dpy);
  }
  virtual void dev_set_parent(Window*  parent);  
  virtual void dev_set_position(const ui::Rect& pos);
  virtual ui::Rect dev_position() const;
  
  protected:       
   ::Window x_window_;  
   bool is_double_buffered_;
   BoxSpace margin_;
   BoxSpace padding_;
   BoxSpace border_space_;
   bool capslock_on_;
   bool map_capslock_to_ctrl_;  
};

class WindowImp : public WindowTemplateImp<ui::WindowImp> {
 public:  
  WindowImp(::Window parent);
  static WindowImp* Make(ui::Window* w, ::Window parent) {
    WindowImp* imp = new WindowImp(parent);    
    imp->set_window(w);    
    return imp;
  }
};

class ButtonImp : public WindowTemplateImp<ui::ButtonImp> {
 public:    
  ButtonImp(::Window parent);	 
  static ButtonImp* Make(ui::Window* w, ::Window parent) {
    ButtonImp* imp = new ButtonImp(parent);	
    imp->set_window(w);  
    return imp;
  }
  virtual bool dev_checked() const { return checked_; }
  virtual void DevCheck() { checked_ = true; }
  virtual void DevUnCheck() { checked_ = false; }  
  void OnDevDraw(Graphics* g, Region& draw_region);
  virtual void dev_set_text(const std::string& text) { text_ = text; }
  virtual std::string dev_text() const { return text_; }
  virtual void OnDevMouseDown(MouseEvent& ev);
  virtual void OnDevMouseUp(MouseEvent& ev);
  virtual void dev_set_font(const Font& font) { font_ = font; }  
  virtual const Font& dev_font() const { return font_; }
 private:
  ui::Font font_;
  std::string text_;
  bool checked_;
};

class FrameImp : public WindowTemplateImp<ui::FrameImp> {
 public:  
  FrameImp();
   
  static FrameImp* Make(ui::Window* w) {
    FrameImp* imp = new FrameImp();    
    imp->set_window(w);        
    return imp;
  }
  
  virtual void DevShow();  
  virtual void dev_set_viewport(const ui::Window::Ptr& viewport);
};

class ScintillaImp : public WindowTemplateImp<ui::ScintillaImp> {
 public:       
  virtual int dev_f(int sci_cmd, void* lparam, void* wparam) { return 0; }
  virtual void DevAddText(const std::string& text) {}
  virtual void DevClearAll() {} // not implemented yet
  virtual void DevFindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const {} // not implemented yet
  virtual void DevGotoLine(int line_pos) {} // not implemented yet
  virtual void DevGotoPos(int char_pos) {} // not implemented yet
  virtual void DevLineUp() {} // not implemented yet
  virtual void DevLineDown() {} // not implemented yet
  virtual void DevCharLeft() {} // not implemented yet
  virtual void DevCharRight() {} // not implemented yet
  virtual void DevWordLeft() {} // not implemented yet
  virtual void DevWordRight() {} // not implemented yet
  virtual int dev_length() const { return 0; } // not implemented yet
  virtual int dev_selectionstart() { return 0; } // not implemented yet
  virtual int dev_selectionend() const { return 0; } // not implemented yet
  virtual void DevSetSel(int cpmin, int cpmax) {} // not implemented yet
  virtual bool dev_has_selection() const { return false; } // not implemented yet
  virtual void DevReplaceSel(const std::string& text) {} // not implemented yet
  virtual void dev_set_find_match_case(bool on) {} // not implemented yet
  virtual void dev_set_find_whole_word(bool on) {} // not implemented yet
  virtual void dev_set_find_regexp(bool on) {} // not implemented yet
  virtual void DevLoadFile(const std::string& filename) {} // not implemented yet
  virtual void DevReload() {} // not implemented yet
  virtual void DevSaveFile(const std::string& filename) {} // not implemented yet
  virtual bool dev_has_file() const { return false; } // not implemented yet
  virtual void dev_set_lexer(const Lexer& lexer) {} // not implemented yet
  virtual void dev_set_foreground_color(ARGB color) {} // not implemented yet
  virtual ARGB dev_foreground_color() const { return 0; } // not implemented yet
  virtual void dev_set_background_color(ARGB color) {} // not implemented yet
  virtual ARGB dev_background_color() const { return 0; } // not implemented yet
  virtual void dev_set_linenumber_foreground_color(ARGB color) {} // not implemented yet
  virtual ARGB dev_linenumber_foreground_color() const { return 0; } // not implemented yet
  virtual void dev_set_linenumber_background_color(ARGB color) {} // not implemented yet
  virtual ARGB dev_linenumber_background_color() const { return 0; }  // not implemented yet
  virtual void dev_set_folding_background_color(ARGB color) {} // not implemented yet
  virtual void dev_set_sel_foreground_color(ARGB color) {} // not implemented yet
  //ARGB sel_foreground_color() const { return ToARGB(ctrl().sel_foreground_color()); }  
  virtual void dev_set_sel_background_color(ARGB color) {} // not implemented yet
  //ARGB sel_background_color() const { return ToARGB(ctrl().sel_background_color()); }
  virtual void dev_set_sel_alpha(int alpha) {} // not implemented yet
  virtual void dev_set_ident_color(ARGB color) {} // not implemented yet
  virtual void dev_set_caret_color(ARGB color) {} // not implemented yet
  virtual ARGB dev_caret_color() const { return 0; } // not implemented yet
  virtual void DevStyleClearAll() {} // not implemented yet
  virtual const std::string& dev_filename() const {} // not implemented yet
  virtual void dev_set_font_info(const FontInfo& font_info) {} // not implemented yet
  virtual const FontInfo& dev_font_info() const { }; // not implemented yet
  virtual int dev_column() const { return 0; } // not implemented yet
  virtual int dev_line() const { return 0; } // not implemented yet
  virtual bool dev_over_type() const { return false; } // not implemented yet
  virtual bool dev_modified() const { return false; } // not implemented yet
  virtual int dev_add_marker(int line, int id) {} // not implemented yet
  virtual int dev_delete_marker(int line, int id)  {} // not implemented yet
  virtual void dev_define_marker(int id, int symbol, ARGB foreground_color, ARGB background_color) {} // not implemented yet
  virtual void DevShowCaretLine()  {} // not implemented yet
  virtual void DevHideCaretLine() {} // not implemented yet
  virtual void DevHideLineNumbers() {} // not implemented yet
  virtual void DevHideBreakpoints() {} // not implemented yet
  virtual void DevHideHorScrollbar() {} // not implemented yet
  virtual void dev_set_caret_line_background_color(ARGB color)  {} // not implemented yet
  virtual void dev_set_tab_width(int width_in_chars) {} // not implemented yet
  virtual int dev_tab_width() const { return 0; } // not implemented yet
  virtual void dev_undo() {} // not implemented yet
  virtual void dev_redo() {} // not implemented yet
  virtual void dev_enable_input() {} // not implemented yet
  virtual void dev_prevent_input() {} // not implemented yet
};

class MenuContainerImp : public ui::MenuContainerImp {
 public:  
  virtual void DevTrack(const Point& pos) {} // not implemented yet
  virtual void DevInvalidate() {} // not implemented yet      
};

class MenuImp : ui::MenuImp {
  public:   
   virtual void dev_set_text(const std::string& text) {}
   virtual void dev_set_position(int pos)  {}
   virtual int dev_position() const { return 0; }
};

class TreeViewImp : public ui::TreeViewImp {
 public:  
  virtual void dev_set_background_color(ARGB color)  {} // not implemented yet
  virtual ARGB dev_background_color() const  {} // not implemented yet
  virtual void dev_set_color(ARGB color)  {} // not implemented yet
  virtual ARGB dev_color() const  {} // not implemented yet
  virtual void DevClear()  {} // not implemented yet
  virtual void dev_select_node(const Node::Ptr& node)  {} // not implemented yet
  virtual void dev_deselect_node(const Node::Ptr& node)  {} // not implemented yet
  virtual Node::WeakPtr dev_selected()  {} // not implemented yet
  virtual void DevEditNode(const Node::Ptr& node)  {} // not implemented yet
  virtual bool dev_is_editing() const  {} // not implemented yet
  virtual void DevShowLines()  {} // not implemented yet
  virtual void DevHideLines()  {} // not implemented yet
  virtual void DevShowButtons()  {} // not implemented yet
  virtual void DevHideButtons()  {} // not implemented yet
  virtual void DevExpandAll()  {} // not implemented yet
  virtual void dev_set_images(const Images::Ptr& images)  {} // not implemented yet
};

class ListViewImp : public ui::ListViewImp {
 public:  
  virtual void dev_set_background_color(ARGB color)  {}// not implemented yet
  virtual ARGB dev_background_color() const  {} // not implemented yet
  virtual void dev_set_color(ARGB color)  {} // not implemented yet
  virtual ARGB dev_color() const  {} // not implemented yet
  virtual void DevClear()  {} // not implemented yet
  virtual void dev_select_node(const Node::Ptr& node)  {} // not implemented yet
  virtual void dev_deselect_node(const Node::Ptr& node)  {} // not implemented yet
  virtual std::vector<Node::Ptr> dev_selected_nodes()  {} // not implemented yet
  virtual Node::WeakPtr dev_selected()  {} // not implemented yet
  virtual void DevEditNode(const Node::Ptr& node)  {} // not implemented yet
  virtual bool dev_is_editing() const  {} // not implemented yet
  virtual void DevAddColumn(const std::string& text, int width) {}  // not implemented yet
  virtual void DevViewList()  {} // not implemented yet
  virtual void DevViewReport()  {} // not implemented yet
  virtual void DevViewIcon()  {} // not implemented yet
  virtual void DevViewSmallIcon()  {} // not implemented yet
  virtual void DevEnableRowSelect()  {} // not implemented yet
  virtual void DevDisableRowSelect()  {} // not implemented yet
  virtual void dev_set_images(const Images::Ptr& images)  {} // not implemented yet
  virtual int dev_top_index() const  {} // not implemented yet
  virtual void DevEnsureVisible(int index)  {} // not implemented yet
  virtual void DevEnableDraw()  {} // not implemented yet
  virtual void DevPreventDraw()  {} // not implemented yet
};

class ScrollBarImp : public ui::ScrollBarImp {
 public:  
  virtual void OnScroll(int pos) {}  
  virtual void dev_set_scroll_range(int minpos, int maxpos) {} // not implemented yet
  virtual void dev_scroll_range(int& minpos, int& maxpos) {} // not implemented yet
  virtual void dev_set_scroll_position(int pos) {} // not implemented yet
  virtual int dev_scroll_position() const {} // not implemented yet
  virtual Dimension dev_system_size() const {} // not implemented yet
  virtual void OnDevScroll(int pos) {} // not implemented yet
};

class ComboBoxImp : public ui::ComboBoxImp {
 public:  
  virtual void dev_add_item(const std::string& item)  {} // not implemented yet
  virtual std::vector<std::string> dev_items() const  {} // not implemented yet
  virtual void dev_set_items(const std::vector<std::string>& itemlist) {} // not implemented yet
  virtual void dev_set_item_index(int index)  {} // not implemented yet
  virtual int dev_item_index() const {} // not implemented yet
  virtual void dev_set_text(const std::string& text) {} // not implemented yet
  virtual std::string dev_text() const {} // not implemented yet
  virtual void dev_set_font(const Font& font) {} // not implemented yet
  virtual const Font& dev_font() const {} // not implemented yet
  virtual void dev_clear() {} // not implemented yet
};

class CheckBoxImp : public ui::CheckBoxImp {
 public:
  virtual void dev_set_background_color(ARGB color)  {}  // not implemented yet
  virtual void OnDevClick() {}  // not implemented yet
};

class RadioButtonImp : public ui::RadioButtonImp {
 public:  
  virtual void dev_set_background_color(ARGB color) {}  // not implemented yet
  virtual bool dev_checked() const {}  // not implemented yet
  virtual void DevCheck()  {}  // not implemented yet
  virtual void DevUnCheck() {}  // not implemented yet
  virtual void dev_set_font(const Font& font) {}  // not implemented yet
  virtual const Font& dev_font() const {}  // not implemented yet
};

class GroupBoxImp : public ui::GroupBoxImp {
  public:
   virtual void dev_set_background_color(ARGB color) {}  // not implemented yet
   virtual bool dev_checked() const {}  // not implemented yet
   virtual void DevCheck() {}  // not implemented yet
   virtual void DevUnCheck() {}  // not implemented yet
};

class EditImp : public ui::EditImp {
 public:
  virtual void dev_set_text(const std::string& text) {}  // not implemented yet
  virtual std::string dev_text() const {}  // not implemented yet
  virtual void dev_set_background_color(ARGB color) {}  // not implemented yet
  virtual ARGB dev_background_color() const {}  // not implemented yet
  virtual void dev_set_color(ARGB color) {}  // not implemented yet
  virtual ARGB dev_color() const {}  // not implemented yet
  virtual void dev_set_font(const Font& font) {}  // not implemented yet
  virtual const Font& dev_font() const {}  // not implemented yet
  virtual void dev_set_sel(int cpmin, int cpmax) {}  // not implemented yet  
  virtual void OnDevChange()  {} // not implemented yet
};

class GameControllersImp : public ui::GameControllersImp {
 public:
   virtual void DevScanPluggedControllers(std::vector<int>& plugged_controller_ids) { assert(0); }
   virtual void DevUpdateController(ui::GameController& controller) { assert(0);  }
};

class FileObserverImp : public ui::FileObserverImp {
 public:
  FileObserverImp() : file_observer_(0) {}
  FileObserverImp(FileObserver* file_observer) : file_observer_(file_observer) {}
  virtual ~FileObserverImp() {};

  virtual void DevStartWatching() { assert(0); } // not implemented yet 
  virtual void DevStopWatching() { assert(0); } // not implemented yet
  virtual void DevSetDirectory(const std::string& path) { assert(0); } // not implemented yet
  virtual std::string dev_directory() const { assert(0); return "todo"; } // not implemented yet
  
 private:
   FileObserver* file_observer_;
};

	
class ImpFactory : public ui::ImpFactory {
 public:
   virtual bool DestroyWindowImp(ui::WindowImp* imp) { return false; }
   virtual ui::AppImp* CreateAppImp() { return new AppImp(); }
   virtual ui::WindowImp* CreateWindowImp();
   virtual ui::FrameImp* CreateFrameImp() {     
     return FrameImp::Make(0);
  }  
  virtual ui::FontInfoImp* CreateFontInfoImp() { return new x::FontInfoImp(); }
  virtual ui::FontImp* CreateFontImp() { return new x::FontImp(); }
  virtual ui::FontImp* CreateFontImp(int stockid) {
     return new x::FontImp(stockid);
  }
  virtual ui::ButtonImp* CreateButtonImp() {     
    return ButtonImp::Make(0, DummyWindow::dummy());  
  }
  virtual ui::RegionImp* CreateRegionImp() {     
    return new x::RegionImp();
  }  
  virtual ui::GameControllersImp* CreateGameControllersImp() {
    return new GameControllersImp();
  }
  virtual ui::FileObserverImp* CreateFileObserverImp(FileObserver* file_observer) {
    return new FileObserverImp(file_observer);
  }
};

} // namespace x
} // namespace ui
} // namespace host
} // namespace psycle

#endif