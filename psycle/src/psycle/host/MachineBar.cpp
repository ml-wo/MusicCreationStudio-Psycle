/** @file 
 *  @brief MachineBar dialog
 *  $Date: 2010-08-15 18:18:35 +0200 (dg., 15 ag 2010) $
 *  $Revision: 9831 $
 */
#include <psycle/host/detail/project.private.hpp>
#include "MachineBar.hpp"
#include "ChildView.hpp"
#include "InputHandler.hpp"
#include "MainFrm.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Plugin.hpp"
#include "WavFileDlg.hpp"
#include "GearRackDlg.hpp"
#include "WaveEdFrame.hpp"

namespace psycle{ namespace host{
	extern CPsycleApp theApp;
IMPLEMENT_DYNAMIC(MachineBar, CDialogBar)

	MachineBar::MachineBar()
	{
	}

	MachineBar::~MachineBar()
	{
	}

	void MachineBar::DoDataExchange(CDataExchange* pDX)
	{
		CDialogBar::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_COMBOSTEP, m_stepcombo);
		DDX_Control(pDX, IDC_BAR_COMBOGEN, m_gencombo);
		DDX_Control(pDX, IDC_AUXSELECT, m_auxcombo);
		DDX_Control(pDX, IDC_BAR_COMBOINS, m_inscombo);
	}

	//Message Maps are defined in CMainFrame, since this isn't a window, but a DialogBar.
	BEGIN_MESSAGE_MAP(MachineBar, CDialogBar)
		ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
	END_MESSAGE_MAP()

	void MachineBar::InitializeValues(CMainFrame* frame, CChildView* view, Song& song)
	{
		m_pParentMain = frame;
		m_pWndView = view;
		m_pSong = &song;
	}


	// MachineBar message handlers
	LRESULT MachineBar::OnInitDialog ( WPARAM wParam, LPARAM lParam)
	{
		BOOL bRet = HandleInitDialog(wParam, lParam);

		if (!UpdateData(FALSE))
		{
		   TRACE0("Warning: UpdateData failed during dialog init.\n");
		}

		macComboInitialized = false;

		((CButton*)GetDlgItem(IDC_B_DECGEN))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_LESS),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_B_INCGEN))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_MORE),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_B_DECWAV))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_LESS),IMAGE_ICON,16,16,0));
		((CButton*)GetDlgItem(IDC_B_INCWAV))->SetIcon((HICON)
				::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_MORE),IMAGE_ICON,16,16,0));

	//	for(int i=0;i<=16;i++)
	//	{
	//		char s[4];
	//		_snprintf(s,4,"%i",i);
	//		m_stepcombo.AddString(s);
	//	}
	//	m_stepcombo.SetCurSel(1);
		
	//  m_gencombo.SetCurSel(0);
		
	//	m_auxcombo.AddString("MIDI");
	//	m_auxcombo.AddString("Params");
	//	m_auxcombo.AddString("Waves");
		
	//	UpdateComboGen(); // Initializes Gen and Ins combobox.

		m_stepcombo.SetCurSel(m_pWndView->patStep);

		m_auxcombo.SetCurSel(AUX_PARAMS);
			

		return bRet;
	}


	void MachineBar::OnSelchangeCombostep()
	{
		int sel=m_stepcombo.GetCurSel();
		m_pWndView->patStep=sel;
		m_pWndView->SetFocus();
	}

	void MachineBar::OnCloseupCombostep()
	{
		m_pWndView->SetFocus();
	}

	void MachineBar::EditQuantizeChange(int diff) // User Called (Hotkey)
	{
		const int total = m_stepcombo.GetCount();
		const int nextsel = (total + m_stepcombo.GetCurSel() + diff) % total;
		m_stepcombo.SetCurSel(nextsel);
		m_pWndView->patStep=nextsel;
	}

	void MachineBar::OnBDecgen() // called by Button and Hotkey.
	{
		const int val = m_gencombo.GetCurSel();
		if ( val > 0 ) m_gencombo.SetCurSel(val-1);
		else m_gencombo.SetCurSel(m_gencombo.GetCount()-1);
		if ( m_gencombo.GetItemData(m_gencombo.GetCurSel()) == 65535 )
		{
			if ( val >1) m_gencombo.SetCurSel(val-2);
			else m_gencombo.SetCurSel(val);
		}
		OnSelchangeBarCombogen();
		((CButton*)GetDlgItem(IDC_B_DECGEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnBIncgen() // called by Button and Hotkey.
	{
		const int val = m_gencombo.GetCurSel();
		if ( val < m_gencombo.GetCount()-1 ) m_gencombo.SetCurSel(val+1);
		else m_gencombo.SetCurSel(0);
		if ( m_gencombo.GetItemData(m_gencombo.GetCurSel()) == 65535 )
		{
			if ( val < m_gencombo.GetCount()-2) m_gencombo.SetCurSel(val+2);
			else m_gencombo.SetCurSel(val);
		}
		OnSelchangeBarCombogen();
		((CButton*)GetDlgItem(IDC_B_INCGEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::UpdateComboGen(bool updatelist)
	{
		bool filled=false;
		bool found=false;
		int selected = -1;
		int line = -1;
		char buffer[64];
		
		if (m_pSong == NULL) 
		{
			return; // why should this happen?
		}
		
		macComboInitialized = false;
		if (updatelist) 
		{
			m_gencombo.ResetContent();
		}
		
		for (int b=0; b<MAX_BUSES; b++) // Check Generators
		{
			if( m_pSong->_pMachine[b])
			{
				if (updatelist)
				{	
					sprintf(buffer,"%.2X: %s",b,m_pSong->_pMachine[b]->_editName);
					m_gencombo.AddString(buffer);
					m_gencombo.SetItemData(m_gencombo.GetCount()-1,b);
				}
				if (!found) 
				{
					selected++;
				}
				if (m_pSong->seqBus == b) 
				{
					found = true;
				}
				filled = true;
			}
		}
		if ( updatelist) 
		{
			m_gencombo.AddString("----------------------------------------------------");
			m_gencombo.SetItemData(m_gencombo.GetCount()-1,65535);
		}
		if (!found) 
		{
			selected++;
			line = selected;
		}
		
		for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
		{
			if(m_pSong->_pMachine[b])
			{
				if (updatelist)
				{	
					sprintf(buffer,"%.2X: %s",b,m_pSong->_pMachine[b]->_editName);
					m_gencombo.AddString(buffer);
					m_gencombo.SetItemData(m_gencombo.GetCount()-1,b);
				}
				if (!found) 
				{
					selected++;
				}
				if (m_pSong->seqBus == b) 
				{
					found = true;
				}
				filled = true;
			}
		}
		if (!filled)
		{
			m_gencombo.ResetContent();
			m_gencombo.AddString("No Machines Loaded");
			selected = 0;
		}
		else if (!found) 
		{
			selected=line;
		}
		
		m_gencombo.SetCurSel(selected);

		// Select the appropiate Option in Aux Combobox.
		if (found) // If found (which also means, if it exists)
		{
			if (m_pSong->_pMachine[m_pSong->seqBus])
			{
				if ( m_pSong->seqBus < MAX_BUSES ) // it's a Generator
				{
					if (m_pSong->_pMachine[m_pSong->seqBus]->NeedsAuxColumn())
					{
						m_auxcombo.SetCurSel(AUX_INSTRUMENT);
						m_pSong->auxcolSelected = m_pSong->instSelected;
					}
					else
					{
						m_auxcombo.SetCurSel(AUX_PARAMS);
						m_pSong->auxcolSelected = 0;
					}
				}
				else
				{
					m_auxcombo.SetCurSel(AUX_PARAMS);
					m_pSong->auxcolSelected = 0;
				}
			}
		}
		else
		{
			m_auxcombo.SetCurSel(AUX_INSTRUMENT); // WAVES
			m_pSong->auxcolSelected = m_pSong->instSelected;
		}
		UpdateComboIns();
		macComboInitialized = true;
	}

	void MachineBar::OnSelchangeBarCombogen() 
	{
		if(macComboInitialized)
		{
			int nsb = GetNumFromCombo(&m_gencombo);

			if(m_pSong->seqBus!=nsb)
			{
				m_pSong->seqBus=nsb;
				UpdateComboGen(false);
			}
			
			m_pParentMain->RedrawGearRackList();
			//Added by J.Redfern, repaints main view after changing selection in combo
			m_pWndView->Repaint();

		}
	}

	void MachineBar::OnCloseupBarCombogen()
	{
		m_pWndView->SetFocus();
	}


	void MachineBar::ChangeGen(int i)	// Used to set an specific seqBus (used in "CChildView::SelectMachineUnderCursor")
	{
		if(i>=0 && i <(MAX_BUSES*2))
		{
			if ( (m_pSong->seqBus & MAX_BUSES) == (i & MAX_BUSES))
			{
				m_pSong->seqBus=i;
				UpdateComboGen(false);
			}
			else
			{
				m_pSong->seqBus=i;
				UpdateComboGen(true);
			}
		}
	}

	void MachineBar::OnGearRack() 
	{
		if (m_pParentMain->pGearRackDialog == NULL)
		{
			m_pParentMain->pGearRackDialog = new CGearRackDlg(m_pParentMain,m_pWndView, &m_pParentMain->pGearRackDialog);
			m_pParentMain->pGearRackDialog->ShowWindow(SW_SHOW);
		}
		else {

			m_pParentMain->pGearRackDialog->SetActiveWindow();
		}
		((CButton*)GetDlgItem(IDC_GEAR_RACK))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	void MachineBar::OnCloseupAuxselect() 
	{
		m_pWndView->SetFocus();
	}

	void MachineBar::OnSelchangeAuxselect() 
	{
		if ( m_auxcombo.GetCurSel() == AUX_INSTRUMENT )	// WAVES
		{
			m_pSong->auxcolSelected=m_pSong->instSelected;
		}
		UpdateComboIns();
	}
	void MachineBar::OnBDecwav() 
	{
		ChangeIns(m_pSong->auxcolSelected-1);
		((CButton*)GetDlgItem(IDC_B_DECWAV))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnBIncwav() 
	{
		ChangeIns(m_pSong->auxcolSelected+1);
		((CButton*)GetDlgItem(IDC_B_INCWAV))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::UpdateComboIns(bool updatelist)
	{
		int listlen = 0;
		
		if (updatelist) 
		{
			m_inscombo.ResetContent();
		}

		if ( m_auxcombo.GetCurSel() == AUX_PARAMS)	// Params
		{
			int nmac = m_pSong->seqBus;
			Machine *tmac = m_pSong->_pMachine[nmac];
			if (tmac) 
			{
				int i=0;
				if (updatelist) 
				{
					for (i=0;i<tmac->GetNumParams();i++)
					{
						char buffer[64],buffer2[64];
						std::memset(buffer2,0,64);
						tmac->GetParamName(i,buffer2);
						bool label(false);
						if(tmac->_type == MACH_PLUGIN)
						{
							if(!(static_cast<Plugin*>(tmac)->GetInfo()->Parameters[i]->Flags & psycle::plugin_interface::MPF_STATE))
								label = true;
						}
						if(label)
							// just a label
							sprintf(buffer, "------ %s ------", buffer2);
						else
							sprintf(buffer, "%.2X:  %s", i, buffer2);
						m_inscombo.AddString(buffer);
						listlen++;
					}
				}
				else
				{
					listlen = m_inscombo.GetCount();
				}
			}
			else
			{
				if (updatelist) 
				{
					m_inscombo.AddString("No Machine");
				}
				listlen = 1;
			}
		}
		else
		{
			char buffer[64];
			if (updatelist) 
			{
				Machine *tmac = m_pSong->_pMachine[m_pSong->seqBus];
				if (tmac && tmac->NeedsAuxColumn()) 
				{
					listlen= tmac->NumAuxColumnIndexes();
					for (int i(0); i<listlen; i++)
					{
						sprintf(buffer, "%.2X: %s", i, tmac->AuxColumnName(i));
						m_inscombo.AddString(buffer);
					}
				}
				else
				{
					m_inscombo.AddString("No Machine");
				}
			}
			else
			{
				listlen = m_inscombo.GetCount();
			}
	//		m_pSong->instSelected=m_pSong->auxcolSelected;
	//		WaveEditorBackUpdate();
	//		m_pParentMain->UpdateInstrumentEditor();
	//		RedrawGearRackList();
		}
		if (m_pSong->auxcolSelected >= listlen)
		{
			m_pSong->auxcolSelected = 0;
		}
		m_inscombo.SetCurSel(m_pSong->auxcolSelected);
	}

	void MachineBar::OnSelchangeBarComboins() 
	{
		if ( m_auxcombo.GetCurSel() == AUX_PARAMS ) {
			m_pSong->paramSelected=m_inscombo.GetCurSel();
		} else {
			m_pSong->instSelected=m_inscombo.GetCurSel();
			m_pParentMain->WaveEditorBackUpdate();
			m_pParentMain->UpdateInstrumentEditor();
			m_pParentMain->RedrawGearRackList();
		}

		m_pSong->auxcolSelected=m_inscombo.GetCurSel();
	}

	void MachineBar::OnCloseupBarComboins()
	{
		m_pWndView->SetFocus();
	}
	void MachineBar::ChangeWave(int i)
	{
		if ( m_pSong->waveSelected == i) return;
		if (i<0 || i >= MAX_INSTRUMENTS) return;
		m_pSong->waveSelected=i;
		m_pParentMain->UpdateInstrumentEditor();
		m_pParentMain->WaveEditorBackUpdate();
	}
	void MachineBar::ChangeIns(int i)	// User Called (Hotkey, button or list change)
	{
		if ( m_inscombo.GetCurSel() == i) return;
		if (i<0 || i >= m_inscombo.GetCount()) return;

		if ( m_auxcombo.GetCurSel() == AUX_PARAMS ) {
			m_pSong->paramSelected=i;
		} else {
			m_pSong->instSelected=i;
			m_pParentMain->UpdateInstrumentEditor();
			m_pParentMain->RedrawGearRackList();
		}
		m_pSong->auxcolSelected=i;
		m_inscombo.SetCurSel(m_pSong->auxcolSelected);
	}

	void MachineBar::OnLoadwave() 
	{
		int nmac = m_pSong->seqBus;
		Machine *tmac = m_pSong->_pMachine[nmac];
		bool found=false;
		if (!tmac || (tmac->_type != MACH_SAMPLER
						&& tmac->_type != MACH_XMSAMPLER)) {
			for(int i=0;i<MAX_MACHINES;i++) {
				if (m_pSong->_pMachine[i] && (m_pSong->_pMachine[i]->_type == MACH_SAMPLER ||
						m_pSong->_pMachine[i]->_type == MACH_XMSAMPLER)	) {
					m_pSong->seqBus = i;
					m_pParentMain->UpdateComboGen();
					m_pWndView->Repaint();
					found=true;
					break;
				}
			}
		}
		else {
			found = true;
		}
		if(!found) {
			int i = m_pSong->GetFreeMachine();
			m_pSong->CreateMachine(MACH_SAMPLER,16,16,NULL, i);
			m_pSong->seqBus = i;
			m_pParentMain->UpdateComboGen();
			m_pWndView->Repaint();
		}

		static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|IFF Samples (*.iff)|*.iff|All Files (*.*)|*.*||";
		
		CWavFileDlg dlg(true,"wav", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
		dlg.m_pSong = m_pSong;
		std::string tmpstr = PsycleGlobal::conf().GetCurrentInstrumentDir();
		dlg.m_ofn.lpstrInitialDir = tmpstr.c_str();
		bool update=false;
		if (dlg.DoModal() == IDOK)
		{
			PsycleGlobal::inputHandler().AddMacViewUndo();

			int si = m_pSong->instSelected;
			
			//added by sampler
			if ( m_pSong->samples.IsEnabled(si))
			{
				if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;					
			}
			//end of added by sampler

			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			CString CurrExt=dlg.GetFileExt();
			CurrExt.MakeLower();
			if ( CurrExt == "wav" )
			{
				if (m_pSong->WavAlloc(si,dlg.GetPathName()))
				{
					update=true;
				}
			}
			else if ( CurrExt == "iff" )
			{
				if (m_pSong->IffAlloc(si,dlg.GetPathName()))
				{
					update=true;
				}
			}
			CString str = dlg.m_ofn.lpstrFile;
			int index = str.ReverseFind('\\');
			if (index != -1)
			{
				PsycleGlobal::conf().SetCurrentInstrumentDir(static_cast<char const *>(str.Left(index)));
			}
		}
		{
			CExclusiveLock lock(&m_pSong->semaphore, 2, true);
			// Stopping wavepreview if not stopped.
			m_pSong->wavprev.Stop();
		}
		if (update){
			UpdateComboIns();
			m_pParentMain->m_wndStatusBar.SetWindowText("New wave loaded");
			m_pParentMain->WaveEditorBackUpdate();
			m_pParentMain->UpdateInstrumentEditor();
			m_pParentMain->RedrawGearRackList();
		}
		((CButton*)GetDlgItem(IDC_LOADWAVE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnSavewave()
	{
		WaveFile output;
		static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";

		if (m_pSong->samples.IsEnabled(m_pSong->instSelected))
		{
			const XMInstrument::WaveData<> & wave = m_pSong->samples[m_pSong->instSelected];
			CFileDialog dlg(FALSE, "wav", wave.WaveName().c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
			if (dlg.DoModal() == IDOK)
			{
				output.OpenForWrite(dlg.GetPathName(), 44100, 16, (wave.IsWaveStereo()) ? 2 : 1 );
				if (wave.IsWaveStereo())
				{
					for ( unsigned int c=0; c < wave.WaveLength(); c++)
					{
						output.WriteStereoSample( *(wave.pWaveDataL() + c), *(wave.pWaveDataR() + c) );
					}
				}
				else
				{
					output.WriteData(wave.pWaveDataL(), wave.WaveLength());
				}

				output.Close();
			}
		}
		else MessageBox("Nothing to save...\nSelect nonempty wave first.", "Error", MB_ICONERROR);
		((CButton*)GetDlgItem(IDC_SAVEWAVE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void MachineBar::OnEditwave() 
	{
		int nmac = m_pSong->seqBus;
		Machine *tmac = m_pSong->_pMachine[nmac];
		bool found=false;
		if (!tmac || (tmac->_type != MACH_SAMPLER
						&& tmac->_type != MACH_XMSAMPLER)) {
			for(int i=0;i<MAX_MACHINES;i++) {
				if (m_pSong->_pMachine[i] && (m_pSong->_pMachine[i]->_type == MACH_SAMPLER ||
						m_pSong->_pMachine[i]->_type == MACH_XMSAMPLER)	) {
					m_pSong->seqBus = i;
					m_pParentMain->UpdateComboGen();
					m_pWndView->Repaint();
					found=true;
					break;
				}
			}
		}
		else {
			found = true;
		}
		if(!found) {
			int i = m_pSong->GetFreeMachine();
			m_pSong->CreateMachine(MACH_SAMPLER,16,16,NULL, i);
			m_pSong->seqBus = i;
			m_pParentMain->UpdateComboGen();
			m_pWndView->Repaint();
		}
		m_pParentMain->ShowInstrumentEditor();
		((CButton*)GetDlgItem(IDC_EDITWAVE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	void MachineBar::OnWavebut() 
	{
		int nmac = m_pSong->seqBus;
		Machine *tmac = m_pSong->_pMachine[nmac];
		bool found=false;
		if (!tmac || (tmac->_type != MACH_SAMPLER
						&& tmac->_type != MACH_XMSAMPLER)) {
			for(int i=0;i<MAX_MACHINES;i++) {
				if (m_pSong->_pMachine[i] && (m_pSong->_pMachine[i]->_type == MACH_SAMPLER ||
						m_pSong->_pMachine[i]->_type == MACH_XMSAMPLER)	) {
					m_pSong->seqBus = i;
					m_pParentMain->UpdateComboGen();
					m_pWndView->Repaint();
					found=true;
					break;
				}
			}
		}
		else {
			found = true;
		}
		if(!found) {
			int i = m_pSong->GetFreeMachine();
			m_pSong->CreateMachine(MACH_SAMPLER,16,16,NULL, i);
			m_pSong->seqBus = i;
			m_pParentMain->UpdateComboGen();
			m_pWndView->Repaint();
		}
		m_pParentMain->m_pWndWed->ShowWindow(SW_SHOWNORMAL);
		m_pParentMain->m_pWndWed->SetActiveWindow();
		((CButton*)GetDlgItem(IDC_WAVEBUT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}


	

	int MachineBar::GetNumFromCombo(CComboBox *cb)
	{
		CString str;
		cb->GetWindowText(str);
		int result;
		helpers::hexstring_to_integer(static_cast<LPCTSTR>(str.Left(2)), result);
		return result;
	}
}}