// GearGainer.cpp : implementation file
//

#include "stdafx.h"
#include "Psycle2.h"
#include "GearGainer.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGearGainer dialog


CGearGainer::CGearGainer(CChildView* pParent /*=NULL*/)
	: CDialog(CGearGainer::IDD, pParent)
{
	m_pParent = pParent;
	//{{AFX_DATA_INIT(CGearGainer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGearGainer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGearGainer)
	DDX_Control(pDX, IDC_VOLABEL, m_volabel);
	DDX_Control(pDX, IDC_SLIDER1, m_volsider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGearGainer, CDialog)
	//{{AFX_MSG_MAP(CGearGainer)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGearGainer message handlers

BOOL CGearGainer::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char buffer[64];
	sprintf(buffer,_pMachine->_editName);
	SetWindowText(buffer);

	m_volsider.SetRange(0, 1024);
	m_volsider.SetPos(1024-_pMachine->_outWet);
	m_volsider.SetTic(0);
	m_volsider.SetTicFreq(64);

	return TRUE;
}

void CGearGainer::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (doit)
	{
		_pMachine->_outWet = 1024-m_volsider.GetPos();
	}

	float wet = (float)_pMachine->_outWet*0.00390625f;
	char buffer[32];
	if (wet > 1.0f)
	{	
		sprintf(buffer,"+%.1f dB\n%.2f%%",20.0f * log10(wet),wet*100); 
	}
	else if (wet == 1.0f)
	{	
		sprintf(buffer,"0.0 dB\n100.00%%"); 
	}
	else if (wet > 0.0f)
	{	
		sprintf(buffer,"%.1f dB\n%.2f%%",20.0f * log10(wet),wet*100); 
	}
	else 
	{				
		sprintf(buffer,"-Inf. dB\n0.00%%"); 
	}

	m_volabel.SetWindowText(buffer);

	*pResult = 0;
}


BOOL CGearGainer::Create()
{
	return CDialog::Create(IDD, m_pParent);
}

void CGearGainer::OnCancel()
{
	m_pParent->GainerMachineDialog = NULL;
	DestroyWindow();
	delete this;
}
