///\file
///\brief implementation file for psycle::host::CParamList.
#pragma once
#include <psycle/host/detail/project.private.hpp>
#include "ParamMap.hpp"
#include "PsycleGlobal.hpp"
#include "PsycleConfig.hpp"
#include "Registry.hpp"
#include "Machine.hpp"
#include "PresetIO.hpp"
#include "MfcUi.hpp"

namespace psycle {
namespace host {
  
ParamMapSkin::ParamMapSkin() {
  PsycleConfig::MachineParam& machine_params = PsycleGlobal::conf().macParam();
  using namespace ui;
  background_color = ToARGB(machine_params.topColor);  
  list_view_background_color = ToARGB(machine_params.bottomColor);
  title_background_color = ToARGB(machine_params.titleColor);  
  font_color = ToARGB(machine_params.fontTopColor);
  title_font_color = ToARGB(machine_params.fonttitleColor);
  range_end_color = ToARGB(machine_params.fontBottomColor);  
  background.reset(OrnamentFactory::Instance().CreateFill(background_color));  
  title_background.reset(OrnamentFactory::Instance().CreateFill(title_background_color));  
  font = CastCFont(&machine_params.font);
  title_font = CastCFont(&machine_params.font_bold);
}

ui::Font ParamMapSkin::CastCFont(CFont* font) {
  ui::Font result;
  LOGFONT lfLogFont;
  memset(&lfLogFont, 0, sizeof(lfLogFont));  
  font->GetLogFont(&lfLogFont);  
  ui::FontInfo font_info;
  font_info.name = lfLogFont.lfFaceName;
  font_info.height = lfLogFont.lfHeight;
  font_info.bold = (lfLogFont.lfWeight == FW_BOLD);
  return ui::Font(font_info);
}

ParamMap::ParamMap(Machine* machine,ParamMap** windowVar) 
    : machine_(machine),
	 windowVar_(windowVar),
      top_client_group_(new ui::Group()),
      list_view_(new ui::ListView()),
      cbx_box_(new ui::ComboBox()),
      allow_auto_learn_chk_box_(new ui::CheckBox()),
      machine_param_end_txt_(new ui::Text()),
      root_node_(new ui::Node()) {      
  set_title("Parameter map");
  PreventResize();
  ui::Canvas::Ptr view_(new ui::Canvas());
  view_->SetSave(false);  
  ui::LineBorder* border = 
    new ui::LineBorder(skin_.font_color);
  border->set_border_radius(ui::BorderRadius(5, 5, 5, 5));
  border_.reset(border);
  view_->add_ornament(skin_.background);
  set_viewport(view_);   
  view_->set_aligner(ui::Aligner::Ptr(new ui::DefaultAligner()));      
  FillListView();
  AddListView();  
  InitLayout();    
  AddMachineParamSelect(top_client_group_);      
  FillComboBox();
  UpdateMachineParamEndText();
}

void ParamMap::UpdateNew(int par,int value) {
  if (allow_auto_learn_chk_box_->checked()) {
    cbx_box_->set_item_index(par);
    UpdateMachineParamEndText();
    ReplaceSelection();
  }
}

void ParamMap::RefreshParamMap() { 
  list_view_->PreventDraw();
  ParamTranslator param(*machine_);   
  std::vector<ui::Node::Ptr>::iterator it = root_node_->begin();
  for (int i = 0; it != root_node_->end(); ++i, ++it) {
    std::stringstream str;
    str << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << i + cbx_box_->item_index();
    str << " [" << param_name(param.translate(i)) << "]";
    ui::Node::Ptr col2_node = *(*it)->begin();
    col2_node->set_text(str.str());              
  }
  list_view_->EnableDraw();
  list_view_->FLS();  
  UpdateMachineParamEndText();
}

void ParamMap::FillListView() {  
  for (int i = 0; i < 256; ++i) {    
    std::stringstream str;
	  str << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << i << " ->";
    ui::Node::Ptr col1_node(new ui::Node(str.str()));                
    std::stringstream str1;    
	  str1 << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << machine_->translate_param(i) 
         << " ["
         << param_name(machine_->translate_param(i)) 
         << "]";
    ui::Node::Ptr col2_node(new ui::Node(str1.str()));
	col1_node->AddNode(col2_node);
    root_node_->AddNode(col1_node);
  }
  if (list_view_) {
    //list_view_->UpdateList();
  }
}


void ParamMap::RecalculateListView() {  
//	ui::Node::iterator it = root_node_->begin();
//	for (int i=0; it != root_node_->end(); ++i, ++it) {
/*		std::stringstream str1;    
		str1 << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << machine_->translate_param(i) 
			 << " ["
			 << param_name(machine_->translate_param(i)) 
			 << "]";
     }
*/
  list_view_->UpdateList();
}

void ParamMap::FillComboBox() {
  for (int i = 0; i != machine_->GetNumParams(); ++i) {    
    std::stringstream str;
    str << i << " [" << param_name(i) << "]";
    cbx_box_->add_item(str.str());
  }
  cbx_box_->set_item_index(0);
}

void ParamMap::OnReplaceButtonClick(ui::Button&) {
  ReplaceSelection();
}

void ParamMap::OnSaveDefaultButtonClick(ui::Button&)
{
	ParamTranslator param(*machine_);
	machine_->get_virtual_param_map(param);
	char name[128];
	strcpy(name,machine_->GetDllName());
	if (strcmp(name,"") == 0) { strcpy(name,machine_->GetName()); }
	PresetIO::SaveDefaultMap(PsycleGlobal::conf().GetAbsolutePresetsDir() + "/" + name + ".map",param, machine_->GetNumParams());
}

void ParamMap::OnLoadDefaultButtonClick(ui::Button&)
{
	ParamTranslator param(*machine_);
	char name[128];
	strcpy(name,machine_->GetDllName());
	if (strcmp(name,"") == 0) { strcpy(name,machine_->GetName()); }
	PresetIO::LoadDefaultMap(PsycleGlobal::conf().GetAbsolutePresetsDir() + "/" + name + ".map",param);
	machine_->set_virtual_param_map(param);
	RefreshParamMap();
}

void ParamMap::OnResetMapButtonClick(ui::Button&) {
  for (int i = 0; i < 256; ++i) {
	  machine_->set_virtual_param_index(i, i);
  }
  RefreshParamMap();
}

void ParamMap::ReplaceSelection() {  
  list_view_->PreventDraw();
  std::vector<ui::Node::Ptr> nodes = list_view_->selected_nodes();
  std::vector<ui::Node::Ptr>::iterator it = nodes.begin();
  for (int i = 0; it != nodes.end(); ++i, ++it) {
    if (i + cbx_box_->item_index() < machine_->GetNumParams()) {
      int virtual_index = (*it)->imp(*list_view_->imp())->position();
      std::stringstream str;
      str << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << i + cbx_box_->item_index();
      str << " [" << param_name(i + cbx_box_->item_index()) << "]";
      ui::Node::Ptr col2_node = *it;
      col2_node->set_text(str.str());
      machine_->set_virtual_param_index(
        virtual_index, i + cbx_box_->item_index());
    } else {
      break;
    }    
  }  
  list_view_->EnableDraw();
}

void ParamMap::OnListViewChange(ui::ListView&, const ui::Node::Ptr&) {
  UpdateMachineParamEndText();
}

void ParamMap::OnComboBoxSelect(ui::ComboBox&) {
  UpdateMachineParamEndText();
}

void ParamMap::UpdateMachineParamEndText() {
  int end = cbx_box_->item_index() + list_view_->selected_nodes().size() - 1;
  if (end >= 0 && end < machine_->GetNumParams()) {         
    machine_param_end_txt_->set_text(param_name(end));
  }
}

std::string ParamMap::param_name(int index) const {
  if (index < machine_->GetNumParams()) {
    char s[1024];
    machine_->GetParamName(index, s);
    return s;
  } 
  return "";  
}

void ParamMap::InitLayout() {
  Window::Ptr client_group(new ui::Group());  
  viewport()->Add(client_group);
  client_group->set_align(ui::ALCLIENT);    
  client_group->set_aligner(
    ui::Aligner::Ptr(new ui::DefaultAligner()));
  client_group->add_ornament(skin_.background);
      
  client_group->Add(top_client_group_);
  top_client_group_->set_auto_size(false, true);
  top_client_group_->set_margin(ui::BoxSpace(5));
  top_client_group_->set_align(ui::ALTOP);
  
  top_client_group_->set_aligner(ui::Aligner::Ptr(
    new ui::DefaultAligner()));
  top_client_group_->add_ornament(border_);

  AddHelpGroup(client_group);
}

void ParamMap::AddListView() {  
  viewport()->Add(list_view_);
  list_view_->set_background_color(skin_.list_view_background_color);
  list_view_->set_text_color(skin_.font_color);
  list_view_->set_align(ui::ALLEFT);
  list_view_->ViewReport();
  list_view_->EnableRowSelect();
  list_view_->AddColumn("Virtual", 50);
  list_view_->AddColumn("Machine Parameter", 200);  
  list_view_->set_root_node(root_node_);
  list_view_->UpdateList();
  list_view_->change.connect(
    boost::bind(&ParamMap::OnListViewChange, this, _1, _2));
}

ui::Group::Ptr ParamMap::CreateRow(const ui::Window::Ptr& parent) {
  ui::Group::Ptr header_group(new ui::Group());
  header_group->set_aligner(
    ui::Aligner::Ptr(new ui::DefaultAligner()));
  header_group->set_auto_size(false, true);
  header_group->set_align(ui::ALTOP);
  header_group->set_margin(ui::BoxSpace(5));
  parent->Add(header_group);  
  return header_group;
}

void ParamMap::AddMachineParamSelect(const ui::Group::Ptr& parent) {
  CreateTitleRow(parent, "Offset To Machine Parameter");
    
  ui::Group::Ptr cbx_group = CreateRow(parent);  
  cbx_group->set_margin(ui::BoxSpace(0, 0, 5, 5));  
  ui::Text::Ptr txt1(new ui::Text("FROM"));
  txt1->set_auto_size(false, false);
  cbx_group->Add(txt1);
  txt1->set_align(ui::ALLEFT);
  txt1->set_vertical_alignment(ui::ALCENTER);
  txt1->set_color(skin_.font_color);      
  txt1->set_margin(ui::BoxSpace(0, 10, 0, 0));
  txt1->set_justify(ui::RIGHTJUSTIFY);  
  txt1->set_position(ui::Rect(ui::Point(), ui::Dimension(50, 0)));
  txt1->set_font(skin_.font);
  cbx_group->Add(cbx_box_);
  cbx_box_->set_auto_size(false, false);
  cbx_box_->set_align(ui::ALLEFT);
  cbx_box_->set_position(ui::Rect(ui::Point(0, 0), ui::Dimension(150, 20)));  
  cbx_box_->select.connect(
    boost::bind(&ParamMap::OnComboBoxSelect, this, _1));

  ui::Group::Ptr end_group = CreateRow(parent);
  end_group->set_margin(ui::BoxSpace(5, 0, 10, 5));  
  ui::Text::Ptr txt2(new ui::Text("TO"));  
  end_group->Add(txt2);
  txt2->set_margin(ui::BoxSpace(0, 10, 0, 0));
  txt2->set_justify(ui::RIGHTJUSTIFY);
  txt2->set_auto_size(false, true);
  txt2->set_color(skin_.font_color);  
  txt2->set_align(ui::ALLEFT);
  txt2->set_position(ui::Rect(ui::Point(), ui::Dimension(50, 0)));
  txt2->set_font(skin_.font);
  end_group->Add(machine_param_end_txt_);    
  machine_param_end_txt_->set_align(ui::ALLEFT); 
  machine_param_end_txt_->set_color(skin_.range_end_color);
  machine_param_end_txt_->set_font(skin_.font);
    
  ui::Group::Ptr btn_group = CreateRow(parent);
  btn_group->set_margin(ui::BoxSpace(0, 0, 5, 5));  
    
  ui::Button::Ptr replace_btn(new ui::Button());
  btn_group->Add(replace_btn);
  replace_btn->set_align(ui::ALLEFT);
  replace_btn->set_position(ui::Rect(ui::Point(0, 0), ui::Dimension(100, 20)));
  replace_btn->set_text("replace");    
  replace_btn->click.connect(
    boost::bind(&ParamMap::OnReplaceButtonClick, this, _1));

  btn_group->Add(allow_auto_learn_chk_box_);
  allow_auto_learn_chk_box_->set_align(ui::ALLEFT);
  allow_auto_learn_chk_box_->Check();
  allow_auto_learn_chk_box_->set_margin(
    ui::BoxSpace(0, 0, 0, 5));
  allow_auto_learn_chk_box_->set_position(
    ui::Rect(ui::Point(), ui::Dimension(100, 20)));
  allow_auto_learn_chk_box_->set_text("Auto Learn");

  ui::Group::Ptr preset_group = CreateRow(parent);  
  preset_group->set_margin(ui::BoxSpace(0, 0, 5, 5));  
  ui::Button::Ptr save_btn(new ui::Button());
  preset_group->Add(save_btn);
  save_btn->set_align(ui::ALLEFT);
  save_btn->set_position(ui::Rect(ui::Point(0, 0), ui::Dimension(100, 20)));
  save_btn->set_text("Save as Default");
  save_btn->click.connect(
    boost::bind(&ParamMap::OnSaveDefaultButtonClick, this, _1));

  ui::Group::Ptr preset2_group = CreateRow(parent);  
  preset2_group->set_margin(ui::BoxSpace(0, 0, 5, 5));  
  ui::Button::Ptr load_btn(new ui::Button());
  preset2_group->Add(load_btn);
  load_btn->set_align(ui::ALLEFT);
  load_btn->set_margin(ui::BoxSpace(0, 5, 0, 0));  
  load_btn->set_position(ui::Rect(ui::Point(0, 0), ui::Dimension(100, 20)));
  load_btn->set_text("Load Default");
  load_btn->click.connect(
  boost::bind(&ParamMap::OnLoadDefaultButtonClick, this, _1));

  ui::Button::Ptr reset_btn(new ui::Button());
  preset2_group->Add(reset_btn);  
  reset_btn->set_align(ui::ALLEFT);
  reset_btn->set_position(ui::Rect(ui::Point(0, 0), ui::Dimension(100, 20)));
  reset_btn->set_text("Reset mapping");
  reset_btn->click.connect(
  boost::bind(&ParamMap::OnResetMapButtonClick, this, _1));

}

void ParamMap::AddHelpGroup(const ui::Window::Ptr& parent) {
  ui::Group::Ptr help_group = CreateRow(parent);
  help_group->set_auto_size(false, false);
  help_group->set_position(ui::Rect(ui::Point(), ui::Dimension(0, 200)));
  help_group->add_ornament(border_);

  CreateTitleRow(help_group, "Help");
  
  ui::Text::Ptr txt1(new ui::Text("Shift + Up/Down: Select a range."));
  txt1->set_align(ui::ALTOP);
  txt1->set_font(skin_.font);
  txt1->set_color(skin_.font_color);
  txt1->set_margin(ui::BoxSpace(5, 5, 5, 0));
  help_group->Add(txt1);

  ui::Text::Ptr txt2(new ui::Text("Auto Learn Checked: Select a range and tweak a knob"));
  txt2->set_align(ui::ALTOP);
  txt2->set_font(skin_.font);
  txt2->set_color(skin_.font_color);
  txt2->set_margin(ui::BoxSpace(5, 5, 5, 0));
  help_group->Add(txt2);
}

ui::Group::Ptr ParamMap::CreateTitleRow(const ui::Window::Ptr& parent, const std::string& header_text) {
  ui::Group::Ptr header_group = CreateRow(parent);
  header_group->add_ornament(skin_.title_background);  
  ui::Text::Ptr txt(new ui::Text(header_text));
  txt->set_font(skin_.title_font);
  header_group->Add(txt);
  txt->set_color(skin_.title_font_color);  
  txt->set_auto_size(true, true);
  txt->set_margin(ui::BoxSpace(5));
  txt->set_align(ui::ALLEFT);
  return header_group;
}

}   // namespace
}   // namespace
