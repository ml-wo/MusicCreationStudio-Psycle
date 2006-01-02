///\file
///\brief implementation file for psycle::host::CNewMachine.
#include <project.private.hpp>
#include "psycle.hpp"
#include "NewMachine.hpp"
#include "Plugin.hpp"
#include "VstHost.hpp"
#include "ProgressDialog.hpp"
#undef min //\todo : ???
#undef max //\todo : ???
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm> //std::transform
#include <cctype>	// std::tolower
#include ".\newmachine.hpp"

NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		int CNewMachine::pluginOrder = 2;
		bool CNewMachine::pluginName = true;
		int CNewMachine::_numPlugins = -1;
		int CNewMachine::LastType0=0;
		int CNewMachine::LastType1=0;


		PluginInfo* CNewMachine::_pPlugsInfo[MAX_BROWSER_PLUGINS];

		std::map<std::string,std::string> CNewMachine::dllNames;
		std::map<CString, int> CNewMachine::CustomFolders;
		int numCustCategories(1);
		CString CustomFolderName;
		CString tempCustomFolderName;
		const int IS_FOLDER=2000000000;
		const int IS_INTERNAL_MACHINE=1000000000;
		const int INTERNAL_MACHINE_COUNT=4;
		RECT UpPos;
		bool bScrollUp;
		bool bScrolling = false;
		int listitemheight;

		void CNewMachine::learnDllName(const std::string & fullname)
		{
			std::string str=fullname;
			// strip off path
			std::string::size_type pos=str.rfind('\\');
			if(pos != std::string::npos)
				str=str.substr(pos+1);

			// transform string to lower case
			std::transform(str.begin(),str.end(),str.begin(),std::tolower);

			dllNames[str]=fullname;
		}
		//\todo : Important: There can exists dlls with the same name (there is a phantom.dll which is a VST).
		//        what about adding a new parameter indicating if we want a VST or a Psycle plugin?
		//		  or maybe if found more than one entry, ask the user which one he wants to use?
		bool CNewMachine::lookupDllName(const std::string & name, std::string & result)
		{
			std::map<std::string,std::string>::iterator iterator
				= dllNames.find(name);
			if(iterator != dllNames.end())
			{
				result=iterator->second;
				return true;
			}
			return false;
		}

		CNewMachine::CNewMachine(CWnd* pParent)
			: CDialog(CNewMachine::IDD, pParent)
		{
			OutBus = false;
		}

		CNewMachine::~CNewMachine()
		{
		}

		void CNewMachine::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CNewMachine)
			DDX_Control(pDX, IDC_CHECK_ALLOW, m_Allow);
			DDX_Control(pDX, IDC_NAMELABEL, m_nameLabel);
			DDX_Control(pDX, IDC_BROWSER, m_browser);
			DDX_Control(pDX, IDC_VERSIONLABEL, m_versionLabel);
			DDX_Control(pDX, IDC_DESCLABEL, m_descLabel);
			DDX_Control(pDX, IDC_DLLNAMELABEL, m_dllnameLabel);
			DDX_Control(pDX, IDC_LISTSTYLE, comboListStyle);
			DDX_Control(pDX, IDC_NAMESTYLE, comboNameStyle);
			//}}AFX_DATA_MAP			
		}

		BEGIN_MESSAGE_MAP(CNewMachine, CDialog)
			//{{AFX_MSG_MAP(CNewMachine)
			ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER, OnSelchangedBrowser)
			ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
			ON_NOTIFY(NM_DBLCLK, IDC_BROWSER, OnDblclkBrowser)
			ON_WM_DESTROY()
			ON_BN_CLICKED(IDC_CHECK_ALLOW, OnCheckAllow)
			ON_CBN_SELENDOK(IDC_LISTSTYLE, OnCbnSelendokListstyle)
			ON_CBN_SELENDOK(IDC_NAMESTYLE, OnCbnSelendokNamestyle)
			ON_NOTIFY(TVN_BEGINDRAG, IDC_BROWSER, BeginDrag)
			ON_WM_MOUSEMOVE()
			ON_WM_CANCELMODE()
			ON_WM_LBUTTONUP()
			ON_WM_CONTEXTMENU()
			ON_COMMAND(ID__ADDSUBFOLDER, NMPOPUP_AddSubFolder)
			ON_COMMAND(ID__ADDFOLDERONSAMELEVEL, NMPOPUP_AddFolderSameLevel)
			ON_COMMAND(ID__RENAMEFOLDER, NMPOPUP_RenameFolder)
			ON_COMMAND(ID_DELETEFOLDER_MOVEPARNT, NMPOPUP_DeleteMoveToParent)
			ON_COMMAND(ID_DELETEFOLDER_MOVEUNCAT, NMPOPUP_DeleteMoveUncat)
			ON_COMMAND(ID__EXPANDALLFOLDERS, NMPOPUP_ExpandAll)
			ON_COMMAND(ID__COLLAPSEALLFOLDERS, NMPOPUP_CollapseAll)
			ON_COMMAND(ID__MOVETOTOPLEVEL, NMPOPUP_MoveToTopLevel)
			ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_BROWSER, BeginLabelEdit)
			ON_NOTIFY(TVN_ENDLABELEDIT, IDC_BROWSER, EndLabelEdit)
			ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)

			//}}AFX_MSG_MAP
			
			
		END_MESSAGE_MAP()

		BOOL CNewMachine::OnInitDialog() 
		{
			CDialog::OnInitDialog();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
			imgList.Create(IDB_MACHINETYPE,16,2,1);
			m_browser.SetImageList(&imgList,TVSIL_NORMAL);
			bAllowChanged = false;
			bCategoriesChanged = false;
			bEditing = false;
			LoadPluginInfo();
			UpdateList();
			
			//fill combo boxes
			comboListStyle.AddString ("Type of Plugin");
			comboListStyle.AddString ("Class of Machine");
			comboListStyle.AddString ("Custom Categories");
			comboListStyle.SetCurSel (pluginOrder);

			comboNameStyle.AddString ("Filename and Path");
			comboNameStyle.AddString ("Plugin Name");
            comboNameStyle.SetCurSel ((int)pluginName);

			numCustCategories = 1;
			
			listitemheight = m_browser.GetItemHeight ();
			//more properties
			return TRUE;

		}

		void CNewMachine::OnDestroy() 
		{
			CDialog::OnDestroy();
			if(imgList.GetSafeHandle()) imgList.DeleteImageList();
		}

		void CNewMachine::UpdateList(bool bInit)
		{
			int nodeindex;
			m_browser.DeleteAllItems();
			HTREEITEM intFxNode;
			switch(pluginOrder)
			{
			// sort by plugin type
			case 0:
				hNodes[0] = m_browser.InsertItem("Internal Machines",0,0 , TVI_ROOT, TVI_LAST);
				m_browser.SetItemData (hNodes[0], IS_FOLDER);
				m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
				hNodes[1] = m_browser.InsertItem("Native plug-ins",2,2,TVI_ROOT,TVI_LAST);
				m_browser.SetItemData (hNodes[1], IS_FOLDER);
				m_browser.SetItemState (hNodes[1], TVIS_BOLD, TVIS_BOLD);
				hNodes[2] = m_browser.InsertItem("VST2 plug-ins",4,4,TVI_ROOT,TVI_LAST);
				m_browser.SetItemData (hNodes[2], IS_FOLDER);
				m_browser.SetItemState (hNodes[2], TVIS_BOLD, TVIS_BOLD);
				intFxNode = hNodes[0];
				//nodeindex = 3;

				for(int i(_numPlugins - 1) ; i >= 0 ; --i)
				{
					if(_pPlugsInfo[i]->error.empty())
					{
						int imgindex;
						HTREEITEM hitem;
						if( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if( _pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 2; 
								hitem= hNodes[1]; 
							}
							else 
							{ 
								imgindex = 4; 
								hitem=hNodes[2]; 
							}
						}
						else
						{
							if( _pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 3; 
								hitem= hNodes[1];
							}
							else 
							{ 
								imgindex = 5; 
								hitem=hNodes[2];
							}
						}
						HTREEITEM newitem;
						if(pluginName)
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						else
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						//associate node with plugin number
						m_browser.SetItemData (newitem, i);
					}
				}
				HTREEITEM newitem;

				newitem = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE);

				newitem = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE + 1);

				newitem = m_browser.InsertItem("Sampulse",0, 0, hNodes[0], TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE + 2);

				newitem = m_browser.InsertItem("Note Duplicator",0, 0, hNodes[0], TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE + 3);

				m_browser.Select(hNodes[LastType0],TVGN_CARET);
			break;

			//sort by class of machine
			case 1:
				hNodes[0] = m_browser.InsertItem("Generators",2,2 , TVI_ROOT, TVI_LAST);
				m_browser.SetItemData (hNodes[0], IS_FOLDER);
				m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
				hNodes[1] = m_browser.InsertItem("Effects",3,3,TVI_ROOT,TVI_LAST);
				m_browser.SetItemData (hNodes[1], IS_FOLDER);
				m_browser.SetItemState (hNodes[1], TVIS_BOLD, TVIS_BOLD);
				intFxNode = hNodes[1];
				//nodeindex=2;
				for(int i(_numPlugins - 1) ; i >= 0 ; --i) // I Search from the end because when creating the array, the deepest dir comes first.
				{
					if(_pPlugsInfo[i]->error.empty())
					{
						int imgindex;
						HTREEITEM hitem;
						HTREEITEM newitem;
						if(_pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 2; 
								hitem= hNodes[0]; 
							}
							else 
							{ 
								imgindex = 4; 
								hitem=hNodes[0]; 
							}
						}
						else
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 3; 
								hitem= hNodes[1]; 
							}
							else 
							{ 
								imgindex = 5; 
								hitem=hNodes[1]; 
							}
						}
						if(pluginName)
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						else
							newitem = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hitem, TVI_SORT);
						//associate the plugin number with the node
						m_browser.SetItemData (newitem,i);
					}

				}
				newitem = m_browser.InsertItem("Sampler",0, 0, hNodes[0], TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE);

				newitem = m_browser.InsertItem("Dummy plug",1,1,intFxNode,TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE + 1);

				newitem = m_browser.InsertItem("Sampulse",0, 0, hNodes[0], TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE + 2);

				newitem = m_browser.InsertItem("Note Duplicator",0, 0, hNodes[0], TVI_SORT);
				m_browser.SetItemData (newitem, IS_INTERNAL_MACHINE + 3);

				m_browser.Select(hNodes[LastType1],TVGN_CARET);
			break;
			
			//sort into custom folders
			case 2:
				hNodes[0] = m_browser.InsertItem(" Uncategorised",8,9, TVI_ROOT, TVI_LAST);
				m_browser.SetItemData (hNodes[0], IS_FOLDER);
				m_browser.SetItemState (hNodes[0], TVIS_BOLD, TVIS_BOLD);
				numCustCategories = 1;
				for(int i(_numPlugins - 1) ; i >= 0 ; --i) // I Search from the end because when creating the array, the deepest dir comes first.
				{
					if(_pPlugsInfo[i]->error.empty())
					{
						//determine plugin icons
						int imgindex;
						HTREEITEM hitem;
						if(_pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 2; 
							}
							else 
							{ 
								imgindex = 4; 
							}
						}
						else
						{
							if(_pPlugsInfo[i]->type == MACH_PLUGIN) 
							{ 
								imgindex = 3; 
							}
							else 
							{ 
								imgindex = 5; 
							}
						}
						//determine plugin folder
						if (_pPlugsInfo[i]->category == "")
						{
							hCategory = hNodes[0];
						}
						else
						{
							// INCOMPLETE!!!!!
							CString restofpath = _pPlugsInfo[i]->category.c_str ();
							int barpos = restofpath.Find ("|",0);
							CString curcategory = restofpath.Left (barpos);
							curcategory = curcategory.TrimRight ("|");
							//restofpath = restofpath.Right (restofpath.GetLength () - barpos - 1);
							hCategory = TVI_ROOT;
							HTREEITEM hParent;
							while (restofpath != "")
							{
								hParent = hCategory;
								hCategory = CategoryExists (hParent, curcategory);

								if (hCategory == NULL)
								{
									//category doesn't exist
									hNodes[numCustCategories] = m_browser.InsertItem (" " + curcategory, 6,7, hParent, TVI_SORT);
									hCategory = hNodes[numCustCategories];
									m_browser.SetItemState (hCategory, TVIS_BOLD, TVIS_BOLD);
									m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
									numCustCategories++;
								}
								else
								{
									//category exists, just add to tree
									hitem = hCategory;
								}
								restofpath = restofpath.Right (restofpath.GetLength () - barpos - 1);
								barpos = restofpath.Find ("|",0);
								
								if (barpos > -1)
								{
									curcategory = restofpath.Left (barpos);
									curcategory = curcategory.TrimRight ("|");
								}
							}



							
						}
						// add plugin to appropriate node on tree
						if(pluginName)
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->name.c_str(), imgindex, imgindex, hCategory, TVI_SORT);
						else
							hPlug[i] = m_browser.InsertItem(_pPlugsInfo[i]->dllname.c_str(), imgindex, imgindex, hCategory, TVI_SORT);
						
						m_browser.SetItemData (hPlug[i],i);
					}

				}
				// remove leading space from all folders
				RemoveCatSpaces(NULL);
			break;
			}
			Outputmachine = -1;
		}

		HTREEITEM CNewMachine::CategoryExists (HTREEITEM hParent, CString category)
		{
			bool bCatFound = false;
			HTREEITEM hChild = m_browser.GetChildItem (hParent);
			HTREEITEM hReturn = NULL;
			while ((hChild != NULL) && (bCatFound == false))
			{
				if ((m_browser.GetItemText (hChild) == (" " + category)) && (m_browser.GetItemData (hChild) == IS_FOLDER))
				{
					hReturn = hChild;
					bCatFound = true;
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}

			// item not found, so return nothing.
			if (!bCatFound)
				return NULL;
			else
				return hReturn;
		}

		void CNewMachine::RemoveCatSpaces (HTREEITEM hParent)
		{
			HTREEITEM hChild;
			if (hParent == NULL)
                hChild = m_browser.GetChildItem (TVI_ROOT);
			else
				hChild = m_browser.GetChildItem (hParent);
			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					CString tempstring = m_browser.GetItemText (hChild);
					tempstring.Delete (0,1);
					m_browser.SetItemText (hChild, tempstring);
					RemoveCatSpaces (hChild);
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}

		}

		void CNewMachine::SortChildren (HTREEITEM hParent)
		{
			HTREEITEM hChild = m_browser.GetChildItem (hParent);
			//add space to front of folders, to sort
			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					CString tempstring = m_browser.GetItemText (hChild);
					m_browser.SetItemText (hChild, " " + tempstring);
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}
			//sort alphabetically - folders will be at top because of the space
			m_browser.SortChildren (hParent);
			//remove the space
			hChild = m_browser.GetChildItem (hParent);
			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					CString tempstring = m_browser.GetItemText (hChild);
					tempstring.Delete (0,1);  //delete first char, which is the space
					m_browser.SetItemText (hChild, tempstring);
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}


		}

		void CNewMachine::OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR; pNMTreeView; // not used
			tHand = m_browser.GetSelectedItem();
			Outputmachine = -1;
			OutBus = false;
			int i = m_browser.GetItemData (tHand);
			
			CString name, desc, dll, version;
			bool allow;

			if (i >= IS_FOLDER)
			{
				name = "";
				desc = "";
				dll = "";
				version = "";

				m_Allow.SetCheck(FALSE);
				m_Allow.EnableWindow(FALSE);
			}
			else if ((i >= IS_INTERNAL_MACHINE) && (i <= (IS_INTERNAL_MACHINE + INTERNAL_MACHINE_COUNT - 1)))
			{
				switch (i - IS_INTERNAL_MACHINE)
				{
				case 0:
					name = "Sampler";
					desc = "Stereo Sampler Unit. Inserts new sampler.";
					dll = "Internal Machine";
					version = "V0.5b";
					Outputmachine = MACH_SAMPLER;
					OutBus = true;
					LastType0 = 0;
					LastType1 = 0;
					m_Allow.SetCheck(FALSE);
					m_Allow.EnableWindow(FALSE);
					break;

				case 1: 
					name = "Dummy";
					desc = "Replaces inexistent plugins";
					dll = "Internal Machine";
					version = "V1.0";
					Outputmachine = MACH_DUMMY;
					LastType0 = 0;
					LastType1 = 1;
					m_Allow.SetCheck(FALSE);
					m_Allow.EnableWindow(FALSE);
					break;

				case 2:
					name = "Sampulse Sampler V2";
					desc = "Sampler with the essence of FastTracker II and Impulse Tracker 2";
					dll = "Internal Machine";
					version = "V0.5b";
					Outputmachine = MACH_XMSAMPLER;
					OutBus = true;
					LastType0 = 0;
					LastType1 = 0;
					m_Allow.SetCheck(FALSE);
					m_Allow.EnableWindow(FALSE);
					break;

				case 3:
					name = "Note Duplicator";
					desc = "Repeats the Events received to the selected machines";
					dll = "Internal Machine";
					version = "V1.0";
					Outputmachine = MACH_DUPLICATOR;
					OutBus = true;
					LastType0 = 0;
					LastType1 = 0;
					m_Allow.SetCheck(FALSE);
					m_Allow.EnableWindow(FALSE);
					break;
				}

			}
			else
			{
				//MessageBox (_pPlugsInfo[i]->category.c_str(),"Current Plugin Category");
				std::string str = _pPlugsInfo[i]->dllname;
				std::string::size_type pos = str.rfind('\\');
				if(pos != std::string::npos)
					str=str.substr(pos+1);
				dll = str.c_str();
				name = _pPlugsInfo[i]->name.c_str();
				desc = _pPlugsInfo[i]->desc.c_str();
				version = _pPlugsInfo[i]->version.c_str();
				if ( _pPlugsInfo[i]->type == MACH_PLUGIN )
				{
					Outputmachine = MACH_PLUGIN;
					LastType0 = 1;
					if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR)
					{
						OutBus = true;
						LastType1 = 0;
					}
					else
					{
						LastType1 = 1;
					}
				}
				else
				{
					LastType0 = 2;
					if ( _pPlugsInfo[i]->mode == MACHMODE_GENERATOR )
					{
						Outputmachine = MACH_VST;
						OutBus = true;
						LastType1 = 0;
					}
					else
					{
						Outputmachine = MACH_VSTFX;
						LastType1 = 1;
					}
				}
				psOutputDll = _pPlugsInfo[i]->dllname;
				m_Allow.SetCheck(!_pPlugsInfo[i]->allow);
				m_Allow.EnableWindow(TRUE);
			}
			//display plugin data
			m_descLabel.SetWindowText (desc);
			m_nameLabel.SetWindowText (name);
			m_dllnameLabel.SetWindowText (dll);
			m_versionLabel.SetWindowText (version);
			*pResult = 0;
		}

		void CNewMachine::OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult) 
		{
			OnOK();			
			*pResult = 0;
		}

		void CNewMachine::OnRefresh() 
		{
			if (MessageBox ("This operation will re-scan your plugins, and reset all plugin warnings.\n\nDo you wish to continue?","Warning!",MB_YESNO | MB_ICONWARNING) == IDYES)
			{
				DestroyPluginInfo();
				{
					char cache[1 << 10];
					GetModuleFileName(0, cache, sizeof cache);
					char * last(std::strrchr(cache,'\\'));
					std::strcpy(last, "\\psycle.plugin-scan.cache");
					DeleteFile(cache);
				}
				LoadPluginInfo();
				UpdateList();
				m_browser.Invalidate();
				SetFocus();
			}
		}

		void CNewMachine::OnCbnSelendokListstyle()
		{
			if ((bCategoriesChanged) && (pluginOrder == 2))
			{
				SetPluginCategories(NULL, NULL);
				bCategoriesChanged = false;
			}
			//set view style from entry in combobox
			pluginOrder=comboListStyle.GetCurSel();

			UpdateList();
			m_browser.Invalidate();				  
		}

		void CNewMachine::OnCbnSelendokNamestyle()
		{
			pluginName=comboNameStyle.GetCurSel()?true:false;

			if (bCategoriesChanged)
				{
					SetPluginCategories(NULL, NULL);
					bCategoriesChanged = false;
				}
			UpdateList();
			m_browser.Invalidate();	
		}

		void CNewMachine::LoadPluginInfo()
		{
			if(_numPlugins == -1)
			{
				host::loggers::info("Scanning plugins ...");

				::AfxGetApp()->DoWaitCursor(1); 
				int plugsCount(0);
				int badPlugsCount(0);
				_numPlugins = 0;
				bool progressOpen = !LoadCacheFile(plugsCount, badPlugsCount);

				class populate_plugin_list
				{
					public:
						populate_plugin_list(std::vector<std::string> & result, std::string directory)
						{
							::CFileFind finder;
							int loop = finder.FindFile(::CString((directory + "\\*").c_str()));
							while(loop)
							{
								loop = finder.FindNextFile();
								if(finder.IsDirectory()) {
									if(!finder.IsDots())
									{
										std::string sfilePath = finder.GetFilePath();
										populate_plugin_list(result,sfilePath);
									}
								}
								else
								{
									CString filePath=finder.GetFilePath();
									filePath.MakeLower();
									if(filePath.Right(4) == ".dll")
									{
										std::string sfilePath = filePath;
										result.push_back(sfilePath);
									}
								}
							}
							finder.Close();
						}
				};

				std::vector<std::string> nativePlugs;
				std::vector<std::string> vstPlugs;

				CProgressDialog Progress;
				{
					char c[1 << 10];
					::GetCurrentDirectory(sizeof c, c);
					std::string s(c);
					host::loggers::info("Scanning plugins ... Current Directory: " + s);
				}
				host::loggers::info("Scanning plugins ... Directory for Natives: " + Global::pConfig->GetPluginDir());
				host::loggers::info("Scanning plugins ... Directory for VSTs: " + Global::pConfig->GetVstDir());
				host::loggers::info("Scanning plugins ... Listing ...");
				if(progressOpen)
				{
					Progress.Create();
					Progress.SetWindowText("Scanning plugins ... Listing ...");
					Progress.ShowWindow(SW_SHOW);
				}

				populate_plugin_list(nativePlugs,Global::pConfig->GetPluginDir());
				populate_plugin_list(vstPlugs,Global::pConfig->GetVstDir());

				int plugin_count = nativePlugs.size() + vstPlugs.size();

				std::ostringstream s; s << "Scanning plugins ... Counted " << plugin_count << " plugins.";
				host::loggers::info(s.str());
				if(progressOpen) {
					Progress.m_Progress.SetStep(16384 / std::max(1,plugin_count));
					Progress.SetWindowText(s.str().c_str());
				}
				std::ofstream out;
				{
					std::string module_directory;
					{
						char module_file_name[MAX_PATH];
						::GetModuleFileName(0, module_file_name, sizeof module_file_name);
						module_directory = module_file_name;
						module_directory = module_directory.substr(0, module_directory.rfind('\\'));
					}
					out.open((module_directory + "/psycle.plugin-scan.log.txt").c_str());
				}
				out
					<< "==========================================" << std::endl
					<< "=== Psycle Plugin Scan Enumeration Log ===" << std::endl
					<< std::endl
					<< "If psycle is crashing on load, chances are it's a bad plugin, "
					<< "specifically the last item listed, if it has no comment after the library file name." << std::endl;
				
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing Natives ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::loggers::info("Scanning plugins ... Testing Natives ...");
				out
					<< std::endl
					<< "======================" << std::endl
					<< "=== Native Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPlugins(plugsCount, badPlugsCount, nativePlugs, MACH_PLUGIN, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanning " << plugin_count << " plugins ... Testing VSTs ...";
					Progress.SetWindowText(s.str().c_str());
				}
				host::loggers::info("Scanning plugins ... Testing VSTs ...");
				out
					<< std::endl
					<< "===================" << std::endl
					<< "=== VST Plugins ===" << std::endl
					<< std::endl;
				out.flush();
				FindPlugins(plugsCount, badPlugsCount, vstPlugs, MACH_VST, out, progressOpen ? &Progress : 0);
				out.flush();
				if(progressOpen)
				{
					std::ostringstream s; s << "Scanned " << plugin_count << " plugins.";
					host::loggers::info(s.str().c_str());
					Progress.SetWindowText(s.str().c_str());
				}
				out.close();
				_numPlugins = plugsCount;
				if(progressOpen)
				{
					Progress.m_Progress.SetPos(16384);
					Progress.SetWindowText("Saving scan cache file ...");
				}
				host::loggers::info("Saving scan cache file ...");
				SaveCacheFile();
				if(progressOpen)
					Progress.OnCancel();
				::AfxGetApp()->DoWaitCursor(-1); 
				host::loggers::info("Done.");
			}
		}

		void CNewMachine::FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress)
		{
			for(unsigned fileIdx=0;fileIdx<list.size();fileIdx++)
			{
				if(pProgress)
				{
					pProgress->m_Progress.StepIt();
					::Sleep(1);
				}
				std::string fileName = list[fileIdx];

				out << fileName << " ... ";
				out.flush();
				FILETIME time;
				ZeroMemory(&time,sizeof FILETIME);
				HANDLE hFile=CreateFile(fileName.c_str(),
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if(hFile!=INVALID_HANDLE_VALUE) {
					GetFileTime(hFile,0,0,&time);
					CloseHandle(hFile);
				}
				bool exists(false);
				for(int i(0) ; i < _numPlugins; ++i)
				{
					if(_pPlugsInfo[i])
					{
						if
							(
								_pPlugsInfo[i]->FileTime.dwHighDateTime == time.dwHighDateTime &&
								_pPlugsInfo[i]->FileTime.dwLowDateTime == time.dwLowDateTime
							)
						{
							if(_pPlugsInfo[i]->dllname == fileName)
							{
								exists = true;
								const std::string error(_pPlugsInfo[i]->error);
								std::stringstream s;
								if(error.empty())
									s << "found in cache.";
								else
									s << "cache says it has previously been disabled because:" << std::endl << error << std::endl;
								out << s.str();
								out.flush();
								host::loggers::info(fileName + '\n' + s.str());
								break;
							}
						}
					}
				}
				if(!exists)
				{
					try
					{
						out << "new plugin added to cache ; ";
						out.flush();
						host::loggers::info(fileName + "\nnew plugin added to cache.");
						_pPlugsInfo[currentPlugsCount]= new PluginInfo;
						_pPlugsInfo[currentPlugsCount]->dllname = fileName;
						_pPlugsInfo[currentPlugsCount]->FileTime = time;
						if(type == MACH_PLUGIN)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_PLUGIN;
							Plugin plug(0);
							try
							{
								 plug.Instance(fileName);
								 plug.Init(); // [bohan] hmm, we should get rid of two-stepped constructions.
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s << typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							catch(...)
							{
								std::ostringstream s; s
									<< "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << _pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								_pPlugsInfo[currentPlugsCount]->name = "???";
								_pPlugsInfo[currentPlugsCount]->desc = "???";
								_pPlugsInfo[currentPlugsCount]->version = "???";
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = plug.GetName();
								{
									std::ostringstream s; s << (plug.IsSynth() ? "Psycle instrument" : "Psycle effect") << " by " << plug.GetAuthor();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s; s << plug.GetInfo()->Version; // API VERSION
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								if(plug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
								out << plug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// [bohan] plug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling plug.Free(); explicitly.
							try
							{
								plug.Free();
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
							catch(...)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< "Type of exception is unknown, no further information available.";
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
						}
						else if(type == MACH_VST)
						{
							_pPlugsInfo[currentPlugsCount]->type = MACH_VST;
							vst::plugin vstPlug;
							try
							{
								 vstPlug.Instance(fileName);
							}
							catch(const std::exception & e)
							{
								std::ostringstream s; s << typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							catch(...)
							{
								std::ostringstream s; s << "Type of exception is unknown, cannot display any further information." << std::endl;
								_pPlugsInfo[currentPlugsCount]->error = s.str();
							}
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								out << "### ERRONEOUS ###" << std::endl;
								out.flush();
								out << _pPlugsInfo[currentPlugsCount]->error;
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + _pPlugsInfo[currentPlugsCount]->error);
								_pPlugsInfo[currentPlugsCount]->allow = false;
								_pPlugsInfo[currentPlugsCount]->name = "???";
								_pPlugsInfo[currentPlugsCount]->desc = "???";
								_pPlugsInfo[currentPlugsCount]->version = "???";
							}
							else
							{
								_pPlugsInfo[currentPlugsCount]->allow = true;
								_pPlugsInfo[currentPlugsCount]->name = vstPlug.GetName();
								{
									std::ostringstream s;
									s << (vstPlug.IsSynth() ? "VST2 instrument" : "VST2 effect") << " by " << vstPlug.GetVendorName();
									_pPlugsInfo[currentPlugsCount]->desc = s.str();
								}
								{
									std::ostringstream s;
									s << vstPlug.GetVersion();
									_pPlugsInfo[currentPlugsCount]->version = s.str();
								}
								
								if(vstPlug.IsSynth()) _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_GENERATOR;
								else _pPlugsInfo[currentPlugsCount]->mode = MACHMODE_FX;

								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
								out << vstPlug.GetName() << " - successfully instanciated";
								out.flush();
							}
							++currentPlugsCount;
							// [bohan] vstPlug is a stack object, so its destructor is called
							// [bohan] at the end of its scope (this cope actually).
							// [bohan] The problem with destructors of any object of any class is that
							// [bohan] they are never allowed to throw any exception.
							// [bohan] So, we catch exceptions here by calling vstPlug.Free(); explicitly.
							try
							{
								vstPlug.Free();
								// [bohan] phatmatik crashes here...
								// <magnus> so does PSP Easyverb, in FreeLibrary
							}
							catch(const std::exception & e)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< typeid(e).name() << std::endl;
								if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
							catch(...)
							{
								std::stringstream s; s
									<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
									<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
									<< "Type of exception is unknown, no further information available.";
								out
									<< std::endl
									<< "### ERRONEOUS ###" << std::endl
									<< s.str().c_str();
								out.flush();
								std::stringstream title; title
									<< "Machine crashed: " << fileName;
								host::loggers::exception(title.str() + '\n' + s.str());
							}
						}
					}
					catch(const std::exception & e)
					{
						std::stringstream s; s
							<< std::endl
							<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl
							<< typeid(e).name() << std::endl;
							if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
						out
							<< s.str().c_str();
						out.flush();
						host::loggers::crash(s.str());
					}
					catch(...)
					{
						std::stringstream s; s
							<< std::endl
							<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl
							<< "Type of exception is unknown, no further information available.";
						out
							<< s.str().c_str();
						out.flush();
						host::loggers::crash(s.str());
					}
				}
				out << std::endl;
				out.flush();
			}
			out.flush();
		}

		void CNewMachine::DestroyPluginInfo()
		{
			for (int i=0; i<_numPlugins; i++)
			{
				zapObject(_pPlugsInfo[i]);
			}
			dllNames.clear();
			_numPlugins = -1;
		}

		bool CNewMachine::LoadCacheFile(int& currentPlugsCount, int& currentBadPlugsCount)
		{
			char modulefilename[_MAX_PATH];
			GetModuleFileName(NULL,modulefilename,_MAX_PATH);
			std::string path=modulefilename;
			std::string::size_type pos=path.rfind('\\');
			if(pos != std::string::npos)
				path=path.substr(0,pos);
			std::string cache=path + "\\psycle.plugin-scan.cache";

			RiffFile file;
			CFileFind finder;

			if (!file.Open(cache.c_str())) 
			{
				return false;
			}

			char Temp[MAX_PATH];
			file.Read(Temp,8);
			Temp[8]=0;
			if (strcmp(Temp,"PSYCACHE")!=0)
			{
				file.Close();
				DeleteFile(cache.c_str());
				return false;
			}

			UINT version;
			file.Read(&version,sizeof(version));
			if (version != CURRENT_CACHE_MAP_VERSION)
			{
				file.Close();
				DeleteFile(cache.c_str());
				return false;
			}

			file.Read(&_numPlugins,sizeof(_numPlugins));
			for (int i = 0; i < _numPlugins; i++)
			{
				PluginInfo p;
				file.ReadString(Temp,sizeof(Temp));
				file.Read(&p.FileTime,sizeof(_pPlugsInfo[currentPlugsCount]->FileTime));
				{
					UINT size;
					file.Read(&size, sizeof size);
					if(size)
					{
						char *chars(new char[size + 1]);
						file.Read(chars, size);
						chars[size] = '\0';
						p.error = (const char*)chars;
						zapArray(chars);
					}
				}
				file.Read(&p.allow,sizeof(p.allow));
				file.Read(&p.mode,sizeof(p.mode));
				file.Read(&p.type,sizeof(p.type));
				file.ReadString(p.name);
				file.ReadString(p.desc);
				file.ReadString(p.version);
				file.ReadString(p.category);
				if(finder.FindFile(Temp))
				{
					FILETIME time;
					finder.FindNextFile();
					if (finder.GetLastWriteTime(&time))
					{
						if
							(
								p.FileTime.dwHighDateTime == time.dwHighDateTime &&
								p.FileTime.dwLowDateTime == time.dwLowDateTime
							)
						{
							_pPlugsInfo[currentPlugsCount]= new PluginInfo;

							_pPlugsInfo[currentPlugsCount]->dllname = Temp;
							_pPlugsInfo[currentPlugsCount]->FileTime = p.FileTime;

							///\todo this could be better handled
							if(!_pPlugsInfo[currentPlugsCount]->error.empty())
							{
								_pPlugsInfo[currentPlugsCount]->error = "";
							}
							if(!p.error.empty())
							{
								_pPlugsInfo[currentPlugsCount]->error = p.error;
							}

							_pPlugsInfo[currentPlugsCount]->allow = p.allow;

							_pPlugsInfo[currentPlugsCount]->mode = p.mode;
							_pPlugsInfo[currentPlugsCount]->type = p.type;
							_pPlugsInfo[currentPlugsCount]->name = p.name;
							_pPlugsInfo[currentPlugsCount]->desc = p.desc;
							_pPlugsInfo[currentPlugsCount]->version = p.version;
							_pPlugsInfo[currentPlugsCount]->category = p.category;

							if(p.error.empty())
							{
								learnDllName(_pPlugsInfo[currentPlugsCount]->dllname);
							}
							++currentPlugsCount;
						}
					}
				}
			}
			file.Close();

			return true;
		}

		bool CNewMachine::SaveCacheFile()
		{
			char cache[_MAX_PATH];
			GetModuleFileName(NULL,cache,_MAX_PATH);
			char * last = strrchr(cache,'\\');
			strcpy(last,"\\psycle.plugin-scan.cache");
			DeleteFile(cache);
			RiffFile file;
			if (!file.Create(cache,true)) 
			{
				return false;
			}
			file.Write("PSYCACHE",8);
			UINT version = CURRENT_CACHE_MAP_VERSION;
			file.Write(&version,sizeof(version));
			file.Write(&_numPlugins,sizeof(_numPlugins));
			for (int i=0; i<_numPlugins; i++ )
			{
				file.Write(_pPlugsInfo[i]->dllname.c_str(),_pPlugsInfo[i]->dllname.length()+1);
				file.Write(&_pPlugsInfo[i]->FileTime,sizeof(_pPlugsInfo[i]->FileTime));
				{
					const std::string error(_pPlugsInfo[i]->error);
					UINT size(error.size());
					file.Write(&size, sizeof size);
					if(size) file.Write(error.data(), size);
				}
				file.Write(&_pPlugsInfo[i]->allow,sizeof(_pPlugsInfo[i]->allow));
				file.Write(&_pPlugsInfo[i]->mode,sizeof(_pPlugsInfo[i]->mode));
				file.Write(&_pPlugsInfo[i]->type,sizeof(_pPlugsInfo[i]->type));
				file.Write(_pPlugsInfo[i]->name.c_str(),_pPlugsInfo[i]->name.length()+1);
				file.Write(_pPlugsInfo[i]->desc.c_str(),_pPlugsInfo[i]->desc.length()+1);
				file.Write(_pPlugsInfo[i]->version.c_str(),_pPlugsInfo[i]->version.length()+1);
				file.Write(_pPlugsInfo[i]->category.c_str(),_pPlugsInfo[i]->category.length()+1);
			}
			file.Close();
			return true;
		}


		void CNewMachine::SetPluginCategories (HTREEITEM hItem, CString Category)
		{
			//traverse plugin tree recursively, saving plugin categories to _pPlugsInfo[i]
			// call this function with hItem = NULL to start right from the highest level
			if (hItem == NULL)
			{
				//deal with "Uncategorised folder
				HTREEITEM hChild = m_browser.GetChildItem (hNodes[0]);

				while (hChild != NULL)
				{
					int i = m_browser.GetItemData (hChild);
					_pPlugsInfo[i]->category = "";
					hChild = m_browser.GetNextSiblingItem (hChild);
				}
				//begin recursive saving
				Category = "";
				SetPluginCategories (TVI_ROOT, Category);
			}
			else if (hItem != hNodes [0])
			{
				HTREEITEM hChild = m_browser.GetChildItem (hItem);
				CString NewCategory;
				while (hChild != NULL)
				{
                    if (m_browser.GetItemData (hChild) == IS_FOLDER)
					{
						//continue recursion since it's a folder
						NewCategory = Category + m_browser.GetItemText (hChild) + "|";
						//MessageBox (Category, "2");
						SetPluginCategories (hChild, NewCategory);
					}
					else
					{
						//save plugin's position in tree to _pPlugsInfo[i]
						int i = m_browser.GetItemData (hChild);
						if (Category == "")
							_pPlugsInfo[i]->category = "ROOT";
						else
							_pPlugsInfo[i]->category = Category;

					}
					hChild = m_browser.GetNextSiblingItem (hChild);
				}

				//everything else, recursive searching
			}
			
			
		}


		void CNewMachine::OnOK() 
		{	
			if (bEditing)
			{
				// ADD CODE TO ALLOW USER TO PRESS <ENTER> TO FINISH EDITING CATEGORY NAME.
			}
			else
			{
				if (Outputmachine > -1) // Necessary so that you cannot doubleclick a Node
				{
					
					if (bCategoriesChanged)
					{	
						SetPluginCategories(NULL, NULL);
					}
	
					if ((bAllowChanged) || (bCategoriesChanged))
						SaveCacheFile();
					if (Outputmachine == MACH_XMSAMPLER ) MessageBox("This version of the machine is for demonstration purposes. It is unusable except for Importing Modules","Sampulse Warning");
					CDialog::OnOK();
				}
			}
		}

		void CNewMachine::OnCheckAllow() 
		{
			for (int i=0; i<_numPlugins; i++)
			{
				if (tHand == hPlug[i])
				{
					_pPlugsInfo[i]->allow = !m_Allow.GetCheck();
					bAllowChanged = TRUE;
				}
			}
		}

		bool CNewMachine::TestFilename(const std::string & name)
		{
			for(int i(0) ; i < _numPlugins ; ++i)
			{
				if(name == _pPlugsInfo[i]->dllname)
				{
					// bad plugins always have allow = false
					if(_pPlugsInfo[i]->allow) return true;
					std::ostringstream s; s
						<< "Plugin " << name << " is disabled because:" << std::endl
						<< _pPlugsInfo[i]->error << std::endl
						<< "Try to load anyway?";
					return ::MessageBox(0, s.str().c_str(), "Plugin Warning!", MB_YESNO | MB_ICONWARNING) == IDYES;
				}
			}
			return false;
		}

		void CNewMachine::BeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
		{
			LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);

			//edit folder name

			HTREEITEM hSelectedItem = m_browser.GetSelectedItem ();
			
		
			//make sure item is a folder that is allowed to be edited
			if ((m_browser.GetItemData (hSelectedItem) >= IS_FOLDER) && (m_browser.GetSelectedItem () != hNodes[0]))
			{			
				bEditing = true;

				*pResult = 0;
			}
			else
				*pResult = true;

		}

		void CNewMachine::EndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
		{
			LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
			CEdit* pEdit = m_browser.GetEditControl();
			
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem ();
			int intNameCount = 0;

			if (pEdit != NULL)
			{	
				CString tempstring;
				CString currenttext = m_browser.GetItemText (hSelectedItem);
				pEdit->GetWindowText(tempstring);
				*pResult = false;
				m_browser.SetItemText (hSelectedItem, tempstring);

				//make sure user has entered a category name			
				if (tempstring != "")
				{
					//make sure category name doesn't exist

					HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
					HTREEITEM hChild = m_browser.GetChildItem (hParent);

					while ((hChild != NULL) && (intNameCount < 3))
					{
						if (tempstring == m_browser.GetItemText (hChild))
							intNameCount++;
						hChild = m_browser.GetNextSiblingItem (hChild);
					}
					
					if (intNameCount >= 2)
					{
						//user has entered an invalid name
						MessageBox ("This folder already exists!", "Error!");
						intNameCount = 0;
						//let user continue editing
						m_browser.EditLabel (hSelectedItem);
						CEdit* pEdit2 = m_browser.GetEditControl();
						pEdit2->SetWindowText(tempstring);
						pEdit2->SetSel (0,-1,0);
					}
					else
					{
                        if ((tempstring.Find("\\") == -1) && (tempstring.Find("/",0) == -1) && (tempstring.Find(":",0) == -1) && (tempstring.Find("*",0) == -1) && (tempstring.Find("\?",0) == -1) && (tempstring.Find("\"",0) == -1) && (tempstring.Find("\'",0) == -1) && (tempstring.Find("<",0) == -1)  && (tempstring.Find(">",0) == -1)  && (tempstring.Find("|",0) == -1))
						{
							//set folder name
							m_browser.SetItemText (hSelectedItem, tempstring);
							m_browser.SetItemState (hSelectedItem, TVIS_BOLD, TVIS_BOLD);
							bEditing = false;
							bCategoriesChanged = true;
							//sort items
							SortChildren (hParent);
						}
						else
						{
							MessageBox ("Incorrect Character Entered!  The following characters are not allowed: \n \\ / : * ? \" \' < > | ", "Error!");
							m_browser.EditLabel (hSelectedItem);
							CEdit* pEdit2 = m_browser.GetEditControl();
							pEdit2->SetWindowText(tempstring);
							pEdit2->SetSel (0,-1,0);
						}
					}
				}
				else
				{
						MessageBox ("You must enter a category name!", "Error!");
						m_browser.EditLabel (hSelectedItem);
						CEdit* pEdit2 = m_browser.GetEditControl();
						pEdit2->SetWindowText(tempstring);
						pEdit2->SetSel (0,-1,0);
				}
			}
			else 
				*pResult = false;

		}

		void CNewMachine::BeginDrag(NMHDR *pNMHDR, LRESULT *pResult)
		{
			if (pluginOrder == 2)  //make sure plugins are being displayed by custom category
			{
				LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)pNMHDR;
				*pResult = 0;	// allow drag
	
				CImageList* piml = NULL;    /* handle of image list */
				POINT ptOffset;
				RECT rcItem;
				  
				if ((piml = m_browser.CreateDragImage(lpnmtv->itemNew.hItem)) == NULL)
					return;
				
				/* get the bounding rectangle of the item being dragged (rel to top-left of control) */
				if (m_browser.GetItemRect(lpnmtv->itemNew.hItem, &rcItem, TRUE))
				{
					CPoint ptDragBegin;
					int nX, nY;
					/* get offset into image that the mouse is at */
					/* item rect doesn't include the image */
					ptDragBegin = lpnmtv->ptDrag;
					ImageList_GetIconSize(piml->GetSafeHandle(), &nX, &nY);
					ptOffset.x = (ptDragBegin.x - rcItem.left) + (nX - (rcItem.right - rcItem.left));
					ptOffset.y = (ptDragBegin.y - rcItem.top) + (nY - (rcItem.bottom - rcItem.top));
					/* convert the item rect to screen co-ords, for use later */
					MapWindowPoints(NULL, &rcItem);
				}
				else
				{
					GetWindowRect(&rcItem);
					ptOffset.x = ptOffset.y = 8;
				}
				
				BOOL bDragBegun = piml->BeginDrag(0, ptOffset);
				if (! bDragBegun)
				{
					delete piml;
					return;
				}
				
				CPoint ptDragEnter = lpnmtv->ptDrag;
				ClientToScreen(&ptDragEnter);
				if (!piml->DragEnter(NULL, ptDragEnter))
				{
					delete piml;
					return;
				}
				
				delete piml;
				
				/* set the focus here, so we get a WM_CANCELMODE if needed */
				SetFocus();
				
				/* redraw item being dragged, otherwise it remains (looking) selected */
				InvalidateRect(&rcItem, TRUE);
				UpdateWindow();
				
				/* Hide the mouse cursor, and direct mouse input to this window */
				SetCapture(); 
				m_hItemDrag = lpnmtv->itemNew.hItem;
			}

		}



		HTREEITEM CNewMachine::MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo, HTREEITEM hItemPos,  bool bAllowReplace)
		{		
  			//check if destination is a plugin, if so, make parent folder the destination
			if (hItemTo != TVI_ROOT)
			{
				if (m_browser.GetItemData (hItemTo) != IS_FOLDER)
					hItemTo = m_browser.GetParentItem (hItemTo);	
			}


			
			if (hItem == NULL || hItemTo == NULL)
    			return NULL;
			if (!bAllowReplace)
                if (hItem == hItemTo || hItemTo == m_browser.GetParentItem(hItem))
	    			return hItem;
  			// check we're not trying to move to a descendant
			HTREEITEM hItemParent = hItemTo;
  			
			if (hItemParent != TVI_ROOT)
			{
				while (hItemParent != TVI_ROOT && (hItemParent = m_browser.GetParentItem(hItemParent)) != NULL)
	    			if (hItemParent == hItem)
      					return NULL;

			}
			
			
			HTREEITEM hItemNew;

			
			//check if item already exists
			bool bExists = false;
			HTREEITEM hChild = m_browser.GetChildItem (hItemTo);
			CString ItemText =  m_browser.GetItemText (hItem);
			while ((hChild != NULL) && (!bExists))
			{
				if (m_browser.GetItemText(hChild) == ItemText)
				{
					bExists = true;
					hItemNew = hChild;
				}
				hChild = m_browser.GetNextSiblingItem (hChild);
			}
			if (!bExists)
			{
				// copy items to new location, recursively, then delete old heirarchy
  				// get text, and other info
  				CString sText = m_browser.GetItemText(hItem);
				int itemdata = m_browser.GetItemData (hItem);
  				TVINSERTSTRUCT tvis;
  				tvis.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | 
	     					TVIF_SELECTEDIMAGE | TVIF_STATE;
  				tvis.item.hItem = hItem;
  				// we don't want to copy selection/expanded state etc
  				tvis.item.stateMask = (UINT)-1 & ~(TVIS_DROPHILITED | TVIS_EXPANDED | 
							TVIS_EXPANDEDONCE | TVIS_EXPANDPARTIAL | TVIS_SELECTED);
  				m_browser.GetItem(&tvis.item);
  				tvis.hParent = hItemTo;
  				tvis.hInsertAfter = hItemPos;
  				// if we're only copying, then ask for new data
  				//if (bCopyOnly && pfnCopyData != NULL)
	    		//	tvis.item.lParam = pfnCopyData(tree, hItem, tvis.item.lParam);
  				hItemNew = m_browser.InsertItem(&tvis);
				m_browser.SetItemData (hItemNew, itemdata);
  				m_browser.SetItemText(hItemNew, sText);
			}
			
			// now move children to under new item
 			HTREEITEM hItemChild = m_browser.GetChildItem(hItem);
 			while (hItemChild != NULL)
  			{
	   			HTREEITEM hItemNextChild = m_browser.GetNextSiblingItem(hItemChild);
    			MoveTreeItem(hItemChild, hItemNew, TVI_SORT, false);
    			hItemChild = hItemNextChild;
  			}
			
			// clear old item data
    		m_browser.SetItemData(hItem, 0);
    		// no (more) children, so we can safely delete top item
    		m_browser.DeleteItem(hItem);
			
  			return hItemNew; 
		}



		void CNewMachine::OnMouseMove(UINT nFlags, CPoint point)
		{
			if (m_hItemDrag != NULL)
			{
				CPoint pt;
			
    			/* drag the item to the current position */
    			pt = point;
    			ClientToScreen(&pt);
			
    			CImageList::DragMove(pt);
    			CImageList::DragShowNolock(FALSE);
			
			    if (CWnd::WindowFromPoint(pt) != &m_browser)
				{
					//SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
				}
    			else
    			{
      			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
      			TVHITTESTINFO tvhti;
      			tvhti.pt = pt;
      			m_browser.ScreenToClient(&tvhti.pt);
      			HTREEITEM hItemSel = m_browser.HitTest(&tvhti);
      			m_browser.SelectDropTarget(tvhti.hItem);
    			}
			
    			CImageList::DragShowNolock(TRUE);
  			}
			
			
			//check if mouse is over the browser up/down labels

			CPoint curpoint = point;
			ClientToScreen (&curpoint);
			m_browser.GetWindowRect (&UpPos);
			
			
			//sometime, fix this code to do automatic scrolling when user is dragging.

/*			if ((UpPos.left < curpoint.x) && (UpPos.right > curpoint.x))
			{
				if ((UpPos.top < curpoint.y) && ((UpPos.top + 20) > curpoint.y))
				{
					bScrollUp = true;
					Sleep (600);
				}
				else if (((UpPos.bottom - 20) < curpoint.y) && (UpPos.bottom > curpoint.y))
				{
					bScrollUp = false;
					Sleep (600);
				}

				if (bScrolling)
				{
					int ScrollPosition = m_browser.GetScrollPos (SB_VERT);
                    if (bScrollUp)
                        m_browser.SetScrollPos (SB_VERT, ScrollPosition - 5,0);
				}
				bScrolling = true;

			}
			else
			{
				bScrolling = false;
			}*/
			
				
			CDialog::OnMouseMove(nFlags, point);
		}

		void CNewMachine::OnCancelMode()
		{
			CDialog::OnCancelMode();

			if (m_hItemDrag != NULL)
			OnEndDrag(nFlags, point);
			else
			CDialog::OnLButtonUp(nFlags, point);
		}

		void CNewMachine::OnLButtonUp(UINT nFlags, CPoint point)
		{
			if (m_hItemDrag != NULL)
			OnEndDrag(nFlags, point);
			else
			CDialog::OnLButtonUp(nFlags, point);

		}

		void CNewMachine::OnEndDrag(UINT nFlags, CPoint point)
		{
			if (m_hItemDrag == NULL)
			return;

			CPoint pt;

			pt = point;
			ClientToScreen(&pt);

			BOOL bCopy = (GetKeyState(VK_CONTROL) & 0x10000000);

  			// do drop

  			HTREEITEM hItemDrop = m_browser.GetDropHilightItem();

  			m_browser.SelectDropTarget(NULL);
  			
			if (hItemDrop != NULL)
			{
				if (!((m_browser.GetItemData(m_hItemDrag) == IS_FOLDER) && ((m_hItemDrag == hNodes[0]) || (hItemDrop == hNodes[0]) || (m_browser.GetParentItem (hItemDrop) == hNodes[0]))))
				{
					HTREEITEM hItemDropped = hItemDrop;
					MoveTreeItem (m_hItemDrag, hItemDrop == NULL ? TVI_ROOT : hItemDrop, TVI_SORT, false);

					bCategoriesChanged = true;

					if (m_browser.GetItemData (hItemDropped) == IS_FOLDER)
						SortChildren(hItemDropped);
					else
						SortChildren(m_browser.GetParentItem (hItemDropped));
				}
			}

  			FinishDragging(TRUE);
			
  			RedrawWindow();
		}

		void CNewMachine::FinishDragging(BOOL bDraggingImageList)
		{
  			if (m_hItemDrag != NULL)
  			{
    			if (bDraggingImageList)
    			{
      			CImageList::DragLeave(NULL);
      			CImageList::EndDrag();
    			}
    			ReleaseCapture();
    			ShowCursor(TRUE);
    			m_hItemDrag = NULL;
    			m_browser.SelectDropTarget(NULL);
  			}
		}

		void CNewMachine::OnContextMenu(CWnd* pWnd, CPoint point)
		{
			UINT nFlags;
			CPoint newpoint = point;
			ScreenToClient (&newpoint);
			newpoint.y = newpoint.y - listitemheight / 1.2;   //arbitrary number, but it seems to work.
			
			HTREEITEM hItem = m_browser.HitTest(newpoint, &nFlags);
			if (hItem != NULL)
				m_browser.SelectItem(hItem);

			if ((hItem != NULL) && (pluginOrder == 2)/*&& (TVHT_ONITEM & nFlags)*/)
			{				
				//check if selected item is a folder

				CMenu popupmenu;
				VERIFY(popupmenu.LoadMenu(IDR_NEWMACHINESPOPUP));
		
				// TO DO:  Delete/Add menu items depending on selection.
				if (m_browser.GetItemData(hItem) == IS_FOLDER)
				{
					//Check if the "Uncategorised" folder has been selected
					if (hItem == hNodes[0])
					{
						//grey out items that can't be used.
						popupmenu.EnableMenuItem (ID__RENAMEFOLDER, MF_GRAYED);
						popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEPARNT, MF_GRAYED);
						popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEUNCAT, MF_GRAYED);
						popupmenu.EnableMenuItem (ID__ADDSUBFOLDER, MF_GRAYED);
						popupmenu.EnableMenuItem (ID__MOVETOTOPLEVEL, MF_GRAYED);
					}

					//check if selected item is a root folder
					if (m_browser.GetParentItem (hItem) == NULL)
					{
						//prevent plugins from being moved to the root
						//THIS SHOULD BE FIXED SO IT CAN WORK
						popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEPARNT, MF_GRAYED);
						popupmenu.EnableMenuItem (ID__MOVETOTOPLEVEL, MF_GRAYED);
						
					}
				}
				else
				{
					popupmenu.EnableMenuItem (ID__ADDSUBFOLDER, MF_GRAYED);
					popupmenu.EnableMenuItem (ID__RENAMEFOLDER, MF_GRAYED);
					popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEPARNT, MF_GRAYED);
					popupmenu.EnableMenuItem (ID_DELETEFOLDER_MOVEUNCAT, MF_GRAYED);
					popupmenu.EnableMenuItem (ID__MOVETOTOPLEVEL, MF_GRAYED);
				}
       
				CMenu* pPopup = popupmenu.GetSubMenu(0);
				ASSERT(pPopup != NULL);
		
				pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, CNewMachine::GetActiveWindow());
		
				popupmenu.DestroyMenu();
			}

		}


		void CNewMachine::NMPOPUP_AddSubFolder()
		{
			HTREEITEM hSelectedItem;
			hSelectedItem = m_browser.GetSelectedItem ();
			//make sure folders aren't added to the "Uncategorised" folder
			hNodes[numCustCategories] = m_browser.InsertItem ("A New Category", 6,6,hSelectedItem, TVI_SORT);
			m_browser.SelectItem (hNodes[numCustCategories]);
			m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
			m_browser.SetItemState (hNodes[numCustCategories], TVIS_BOLD, TVIS_BOLD);
			//let user edit the name of the new category
			CEdit* EditNewFolder = m_browser.EditLabel (m_browser.GetSelectedItem ());
			numCustCategories++;
			bEditing = true;
		} 



		void CNewMachine::NMPOPUP_AddFolderSameLevel()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
			hNodes[numCustCategories] = m_browser.InsertItem ("A New Category", 6,6,hParent, TVI_SORT);

			m_browser.SelectItem (hNodes[numCustCategories]);
			m_browser.SetItemData (hNodes[numCustCategories], IS_FOLDER);
			m_browser.SetItemState (hNodes[numCustCategories], TVIS_BOLD, TVIS_BOLD);
			CEdit* EditNewFolder = m_browser.EditLabel (m_browser.GetSelectedItem ());
			numCustCategories++;
			bEditing = true;

		}

		void CNewMachine::NMPOPUP_RenameFolder()
		{	
			CEdit* EditNewFolder = m_browser.EditLabel (m_browser.GetSelectedItem());
		}
		void CNewMachine::NMPOPUP_DeleteMoveToParent()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			//add code to move items contained in the subfolder to the parent folder
			//ALSO must check if sub folders being moved up a level already exist;  if they do,
			// ask the user when to move plugins or not.
			HTREEITEM hParent = m_browser.GetParentItem (hSelectedItem);
			HTREEITEM hChild = m_browser.GetChildItem (hSelectedItem);
			while (hChild != NULL)
			{
				MoveTreeItem (hChild, hParent,TVI_SORT, false);
				hChild = m_browser.GetChildItem (hSelectedItem);

			}


			

			//delete category
			m_browser.DeleteItem (hSelectedItem);
			SortChildren(hParent);
			bCategoriesChanged = true;
		}

		void CNewMachine::NMPOPUP_MoveToTopLevel()
		{
			HTREEITEM hItem = m_browser.GetSelectedItem();
			MoveTreeItem (hItem, TVI_ROOT, TVI_SORT, 0);
			bCategoriesChanged = true;
		}

		void CNewMachine::NMPOPUP_DeleteMoveUncat()
		{
			HTREEITEM hSelectedItem = m_browser.GetSelectedItem();
			//add code to move items contained in the subfolder to the "Uncategorised" folder.
			DeleteMoveUncat (hSelectedItem);
			//delete category
			//m_browser.DeleteItem (hSelectedItem);
			SortChildren(hNodes[0]);
			bCategoriesChanged = true;
		}

		void CNewMachine::DeleteMoveUncat (HTREEITEM hParent)
		{
			HTREEITEM hChild = m_browser.GetChildItem (hParent);

			while (hChild != NULL)
			{
				if (m_browser.GetItemData (hChild) == IS_FOLDER)
				{
					//recurse into folder
					DeleteMoveUncat (hChild);
				}
				else
				{
					//hChild is a plugin, move it
					MoveTreeItem (hChild, hNodes[0], TVI_SORT, false);
				}
				hChild = m_browser.GetChildItem (hParent);
			}

			m_browser.DeleteItem (hParent);
		}
		
		void CNewMachine::NMPOPUP_ExpandAll()
		{
			// TODO: Add your command handler code here
		}

		void CNewMachine::NMPOPUP_CollapseAll()
		{
			// TODO: Add your command handler code here
		}

		void CNewMachine::OnBnClickedCancel()
		{
			if (bCategoriesChanged)
			{
				SetPluginCategories(NULL, NULL);
			}
			if ((bAllowChanged) || (bCategoriesChanged))
			{
				SaveCacheFile();
			}
			OnCancel();
		}

	NAMESPACE__END
NAMESPACE__END
