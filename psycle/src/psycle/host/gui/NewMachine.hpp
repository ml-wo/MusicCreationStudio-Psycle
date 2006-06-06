///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include <psycle/host/gui/resources/resources.hpp>
#include <psycle/host/engine/machine.hpp>
#include <afxcoll.h>
#include <iostream>
#include <typeinfo>
#include <map>
#include "afxwin.h"
#include <psycle/host/engine/cacheddllfinder.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		const int MAX_BROWSER_NODES = 64;
		const int MAX_BROWSER_PLUGINS = 2048;
		const int NUM_INTERNAL_MACHINES = 7;

		class CProgressDialog;

		class InternalMachineInfo
		{
		public:
			InternalMachineInfo()
			{
			}
			~InternalMachineInfo() throw()
			{
			}
			std::string name;
			std::string desc;
			std::string version;
			std::string category;
			///\todo [bohan] please someone check this type and document "Equivalent of MACH type". to be removed.
			Machine::type_type Outputmachine;
			///\todo [bohan] please someone check this type and document "Equipvalent of MACHMODE" (generator or effect). to be removed.
			Machine::id_type OutBus;
			///\todo [bohan] please someone check this type and document  "0 -> internal, 1 -> native, 2 -> vst"
			Machine::type_type LastType0;
			///\todo [bohan] please someone check this type and document  "0 -> generator , 1 -> effect" <- to be removed (use MACHMODE)
			Machine::type_type LastType1;
			///\todo [bohan] please someone check this type and document "Equipvalent of MACHMODE" (generator or effect). to be removed.
			bool machtype; //false = generator, true = effect ... erm.
		};

		/// new machine dialog window.
		class CNewMachine : public CDialog
		{
		public:
			CNewMachine(CWnd* pParent = 0);
			~CNewMachine();

			///< Adds the dll name -> full path mapping to the map.
			static void learnDllName(const std::string & fullpath);
			///< searches in the map the full path for a specified dll name
			static bool lookupDllName(const std::string &, std::string & result);
			///< Checks if the plugin that is going to be loaded is allowed to be loaded.
			static bool TestFilename(const std::string & name);
			static void DestroyPluginInfo();
			static void LoadPluginInfo();
			static void FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress = 0);
			static bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount);
			static bool SaveCacheFile();

			// Dialog Data
			enum { IDD = IDD_NEWMACHINE };
			CButton	m_Allow;
			CStatic	m_nameLabel;
			CTreeCtrl	m_browser;
			CStatic	m_versionLabel;
			CStatic	m_descLabel;
			CStatic	m_dllnameLabel;
			CComboBox comboListStyle;
			CComboBox comboNameStyle;

			CImageList imgList;
			HTREEITEM tHand;

			Machine::type_type Outputmachine;
			std::string psOutputDll;
			int OutBus;
			static int pluginOrder;
			static bool pluginName;
			static int LastType0;
			static int LastType1;
			
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			afx_msg void OnSelchangedBrowser(NMHDR* pNMHDR, LRESULT* pResult);
			virtual BOOL OnInitDialog();
			afx_msg void OnRefresh();
			virtual void OnOK();
			afx_msg void OnDblclkBrowser(NMHDR* pNMHDR, LRESULT* pResult);
			afx_msg void OnDestroy();
			afx_msg void OnCheckAllow();
			afx_msg void OnCbnSelendokListstyle();
			afx_msg void OnCbnSelendokNamestyle();
			afx_msg void BeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
			afx_msg void NMPOPUP_AddSubFolder();
			afx_msg void NMPOPUP_AddFolderSameLevel();
			afx_msg void NMPOPUP_RenameFolder();
			afx_msg void NMPOPUP_DeleteMoveToParent();
			afx_msg void NMPOPUP_DeleteMoveUncat();
			afx_msg void BeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
			afx_msg void EndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);

			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnCancelMode();
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

			HTREEITEM hNodes[MAX_BROWSER_NODES];
			HTREEITEM hInt[NUM_INTERNAL_MACHINES];
			HTREEITEM hPlug[MAX_BROWSER_PLUGINS];
			HTREEITEM m_hItemDrag;
			static std::map<std::string,std::string> dllNames;
			static std::map<CString, int> CustomFolders;
			bool bAllowChanged;
			bool bCategoriesChanged;

		DECLARE_MESSAGE_MAP()

		private:
			static int _numPlugins;
			static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
			static InternalMachineInfo * _pInternalMachines[NUM_INTERNAL_MACHINES];
			static int _numDirs;

			static bool LoadCategoriesFile();
			static bool SaveCategoriesFile();
			void UpdateList(bool bInit = false);
			HTREEITEM CategoryExists (HTREEITEM hParent, CString category);
			void DeleteMoveUncat (HTREEITEM hParent);
			void SortChildren (HTREEITEM hParent);
			void RemoveCatSpaces (HTREEITEM hParent);

		public:
			void FinishDragging(BOOL bDraggingImageList);
			void OnEndDrag(UINT nFlags, CPoint point);
			
			UINT nFlags;
			CPoint point;
			bool bEditing;
			

			HTREEITEM MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo, HTREEITEM hItemPos = TVI_SORT, bool bAllowReplace = false);
			afx_msg void NMPOPUP_MoveToTopLevel();
			void SetPluginCategories(HTREEITEM hItem, CString Category);

			HTREEITEM hCategory;
			static int NumPlugsInCategories;
			

			afx_msg void OnBnClickedCancel();
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnTvnKeydownBrowser(NMHDR *pNMHDR, LRESULT *pResult);
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
