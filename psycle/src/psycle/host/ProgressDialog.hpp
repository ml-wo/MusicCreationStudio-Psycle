// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::CProgressDialog.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		/// progress meter window.
		class CProgressDialog : public CDialog
		{
		public:
			CProgressDialog(CWnd* pParent = 0);
			BOOL Create();
			afx_msg void OnCancel();
			CWnd* m_pParent;
		// Dialog Data
			//{{AFX_DATA(CProgressDialog)
			enum { IDD = IDD_PROGRESS_DIALOG };
			CProgressCtrl	m_Progress;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CProgressDialog)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CProgressDialog)
			virtual BOOL OnInitDialog();
				// NOTE: the ClassWizard will add member functions here
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
