///\file
///\brief implementation file for psycle::host::CChildView.

#include "ChildView.hpp"
#include "Version.hpp"
#include "Psycle.hpp"
#include "Configuration.hpp"
#include "Player.hpp"
//#include "Helpers.hpp"
#include "MainFrm.hpp"
//#include "Bitmap.hpp"
#include "InputHandler.hpp"
#include "MidiInput.hpp"
#include "ConfigDlg.hpp"
#include "GreetDialog.hpp"
#include "SaveWavDlg.hpp"
#include "SongpDlg.hpp"
#include "XMSongLoader.hpp"
#include "XMSongExport.hpp"
#include "ITModule2.h"
#include "NativeGui.hpp"
#include "XMSamplerUI.hpp"
//#include "VstEditorDlg.hpp"
#include "WireDlg.hpp"
#include "NewMachine.hpp"
#include "VstHost24.hpp" //included because of the usage of a call in the Timer function. It should be standarized to the Machine class.
#include <cmath>
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		CMainFrame		*pParentMain;

		CChildView::CChildView(CMainFrame* main_frame)
			:pParentFrame(0)			
			,SamplerMachineDialog(NULL)
			,XMSamplerMachineDialog(NULL)
			,WaveInMachineDialog(NULL)
			,blockSelectBarState(1)
			,bScrollDetatch(false)			
			,updateMode(0)
			,updatePar(0)
			,viewMode(view_modes::machine)
			,_outputActive(false)
			,CW(300)
			,CH(200)
			,textLeftEdge(2)
			,hbmMachineSkin(0)
			,hbmMachineDial(0)
			,bmpDC(NULL)
			,UndoMacCounter(0)
			,UndoMacSaved(0)			
			,machine_view_(this, main_frame, Global::_pSong)
			,pattern_view_(this, main_frame, Global::_pSong)
		{			
			for (int c=0; c<256; c++)	{ FLATSIZES[c]=8; }	

			Global::pInputHandler->SetChildView(this);

			// Creates a new song object. The application Song.
			Global::_pSong->New();

			// Referencing the childView song pointer to the
			// Main Global::_pSong object [The application Global::_pSong]
			_pSong = Global::_pSong;

			// machine_view_.Rebuild();
			// its done in psycle.cpp, todo check config load order
		}

		CChildView::~CChildView()
		{
			Global::pInputHandler->SetChildView(NULL);


			if ( bmpDC != NULL )
			{
				char buf[100];
				sprintf(buf,"CChildView::~CChildView(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
				TRACE(buf);
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			machineskinmask.DeleteObject();			
		}

		BEGIN_MESSAGE_MAP(CChildView,CWnd )
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_RBUTTONDOWN()
			ON_WM_RBUTTONUP()
			ON_WM_LBUTTONUP()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONDBLCLK()
			ON_COMMAND(ID_CPUPERFORMANCE, OnHelpPsycleenviromentinfo)
			ON_COMMAND(ID_MIDI_MONITOR, OnMidiMonitorDlg)
			ON_WM_DESTROY()
			ON_COMMAND(ID_APP_EXIT, OnAppExit)
			ON_COMMAND(ID_MACHINEVIEW, OnMachineview)
			ON_COMMAND(ID_PATTERNVIEW, OnPatternView)	
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_COMMAND(ID_BARPLAY, OnBarplay)
			ON_COMMAND(ID_BARPLAYFROMSTART, OnBarplayFromStart)
			ON_COMMAND(ID_BARREC, OnBarrec)
			ON_COMMAND(ID_BARSTOP, OnBarstop)
			ON_COMMAND(ID_RECORDB, OnRecordWav)
			ON_WM_TIMER()
			ON_UPDATE_COMMAND_UI(ID_RECORDB, OnUpdateRecordWav)
			ON_COMMAND(ID_FILE_NEW, OnFileNew)
			ON_COMMAND_EX(ID_EXPORT, OnExport)
			ON_COMMAND_EX(ID_FILE_SAVE, OnFileSave)
			ON_COMMAND_EX(ID_FILE_SAVE_AS, OnFileSaveAs)
			ON_COMMAND(ID_FILE_LOADSONG, OnFileLoadsong)
			ON_COMMAND(ID_FILE_REVERT, OnFileRevert)
			ON_COMMAND(ID_HELP_SALUDOS, OnHelpSaludos)
			ON_COMMAND(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
			ON_UPDATE_COMMAND_UI(ID_PATTERNVIEW, OnUpdatePatternView)
			ON_UPDATE_COMMAND_UI(ID_MACHINEVIEW, OnUpdateMachineview)
			ON_UPDATE_COMMAND_UI(ID_BARPLAY, OnUpdateBarplay)
			ON_UPDATE_COMMAND_UI(ID_BARPLAYFROMSTART, OnUpdateBarplayFromStart)
			ON_UPDATE_COMMAND_UI(ID_BARREC, OnUpdateBarrec)
			ON_COMMAND(ID_FILE_SONGPROPERTIES, OnFileSongproperties)
			ON_COMMAND(ID_VIEW_INSTRUMENTEDITOR, OnViewInstrumenteditor)
			ON_COMMAND(ID_NEWMACHINE, OnNewmachine)
			ON_COMMAND(ID_BUTTONPLAYSEQBLOCK, OnButtonplayseqblock)
			ON_UPDATE_COMMAND_UI(ID_BUTTONPLAYSEQBLOCK, OnUpdateButtonplayseqblock)
			ON_COMMAND(ID_POP_CUT, OnPopCut)
			ON_UPDATE_COMMAND_UI(ID_POP_CUT, OnUpdateCutCopy)
			ON_COMMAND(ID_POP_COPY, OnPopCopy)
			ON_COMMAND(ID_POP_PASTE, OnPopPaste)
			ON_UPDATE_COMMAND_UI(ID_POP_PASTE, OnUpdatePaste)
			ON_COMMAND(ID_POP_MIXPASTE, OnPopMixpaste)
			ON_COMMAND(ID_POP_DELETE, OnPopDelete)
			ON_COMMAND(ID_POP_INTERPOLATE, OnPopInterpolate)
			ON_COMMAND(ID_POP_INTERPOLATE_CURVE, OnPopInterpolateCurve)
			ON_COMMAND(ID_POP_CHANGEGENERATOR, OnPopChangegenerator)
			ON_COMMAND(ID_POP_CHANGEINSTRUMENT, OnPopChangeinstrument)
			ON_COMMAND(ID_POP_TRANSPOSE1, OnPopTranspose1)
			ON_COMMAND(ID_POP_TRANSPOSE12, OnPopTranspose12)
			ON_COMMAND(ID_POP_TRANSPOSE_1, OnPopTranspose_1)
			ON_COMMAND(ID_POP_TRANSPOSE_12, OnPopTranspose_12)
			ON_COMMAND(ID_AUTOSTOP, OnAutostop)
			ON_UPDATE_COMMAND_UI(ID_AUTOSTOP, OnUpdateAutostop)
			ON_COMMAND(ID_POP_TRANSFORMPATTERN, OnPopTransformpattern)
			ON_COMMAND(ID_POP_PATTENPROPERTIES, OnPopPattenproperties)
			ON_COMMAND(ID_POP_BLOCK_SWINGFILL, OnPopBlockSwingfill)
			ON_COMMAND(ID_POP_TRACK_SWINGFILL, OnPopTrackSwingfill)
			ON_WM_SIZE()
			ON_COMMAND(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
			ON_WM_CONTEXTMENU()
			ON_WM_HSCROLL()
			ON_WM_VSCROLL()
			ON_COMMAND(ID_FILE_IMPORT_XMFILE, OnFileImportModulefile)
			ON_COMMAND(ID_FILE_RECENT_01, OnFileRecent_01)
			ON_COMMAND(ID_FILE_RECENT_02, OnFileRecent_02)
			ON_COMMAND(ID_FILE_RECENT_03, OnFileRecent_03)
			ON_COMMAND(ID_FILE_RECENT_04, OnFileRecent_04)
			ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
			ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateUndo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateRedo)
			ON_WM_MOUSEWHEEL()
			ON_WM_MBUTTONDOWN()
			ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePatternPaste)
			ON_COMMAND(ID_FILE_SAVEAUDIO, OnFileSaveaudio)
			ON_COMMAND(ID_HELP_KEYBTXT, OnHelpKeybtxt)
			ON_COMMAND(ID_HELP_README, OnHelpReadme)
			ON_COMMAND(ID_HELP_TWEAKING, OnHelpTweaking)
			ON_COMMAND(ID_HELP_WHATSNEW, OnHelpWhatsnew)
			ON_COMMAND(ID_CONFIGURATION_LOOPPLAYBACK, OnConfigurationLoopplayback)
			ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_LOOPPLAYBACK, OnUpdateConfigurationLoopplayback)
			ON_UPDATE_COMMAND_UI(ID_POP_COPY, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_MIXPASTE, OnUpdatePaste)
			ON_UPDATE_COMMAND_UI(ID_POP_DELETE, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_INTERPOLATE, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_INTERPOLATE_CURVE, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_CHANGEGENERATOR, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_CHANGEINSTRUMENT, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE1, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE12, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_1, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_12, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCK_SWINGFILL, OnUpdateCutCopy)
			ON_COMMAND(ID_EDIT_CUT, patCut)
			ON_COMMAND(ID_EDIT_COPY, patCopy)
			ON_COMMAND(ID_EDIT_PASTE, patPaste)
			ON_COMMAND(ID_EDIT_MIXPASTE, patMixPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_MIXPASTE, OnUpdatePatternPaste)
			ON_COMMAND(ID_EDIT_DELETE, patDelete)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdatePatternCutCopy)
			ON_COMMAND(ID_SHOWPSEQ, OnShowPatternSeq)
			ON_UPDATE_COMMAND_UI(ID_SHOWPSEQ, OnUpdatePatternSeq)
			ON_COMMAND(ID_POP_BLOCKSWITCH, OnPopBlockswitch)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCKSWITCH, OnUpdatePopBlockswitch)
			END_MESSAGE_MAP()

		BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if (!CWnd::PreCreateWindow(cs))
				return FALSE;
			
			cs.dwExStyle |= WS_EX_CLIENTEDGE;
			cs.style &= ~WS_BORDER;
			cs.lpszClass = AfxRegisterWndClass
				(
					CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
					//::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);
					::LoadCursor(NULL, IDC_ARROW),
					0,
					0
				);
			return TRUE;
		}

		/// This function gives to the pParentMain the pointer to a CMainFrm
		/// object. Call this function from the CMainframe side object to
		/// allow CCHildView call functions of the CMainFrm parent object
		/// Call this function after creating both the CCHildView object and
		/// the cmainfrm object
		void CChildView::ValidateParent()
		{
			pParentMain=(CMainFrame *)pParentFrame;
			pParentMain->_pSong=Global::_pSong;
		}

		/// Timer initialization
		void CChildView::InitTimer()
		{
			KillTimer(31);
			KillTimer(159);
			if (!SetTimer(31,30,NULL)) // GUI update. 
			{
				AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
			}

			if ( Global::pConfig->autosaveSong )
			{
				if (!SetTimer(159,Global::pConfig->autosaveSongTime*60000,NULL)) // Autosave Song
				{
					AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
				}
			}
		}

		/// Timer handler
		void CChildView::OnTimer( UINT nIDEvent )
		{
			if (nIDEvent == 31)
			{
				///\todo : IMPORTANT! change this lock to a more flexible one
				// It is causing skips on sound when there is a pattern change because
				// it is not allowing the player to work. Do the same in the one inside Player::Work()
				CSingleLock lock(&_pSong->door,TRUE);
				if (Global::_pSong->_pMachine[MASTER_INDEX])
				{
					pParentMain->UpdateVumeters
						(
							//((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->_LMAX,
							//((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->_RMAX,
							((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->_lMax,
							((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->_rMax,
							Global::pConfig->vu1,
							Global::pConfig->vu2,
							Global::pConfig->vu3,
							((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->_clip
						);
					pParentMain->UpdateMasterValue(((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->_outDry);
					//if ( MasterMachineDialog ) MasterMachineDialog->UpdateUI(); maybe a todo
					//((Master*)Global::_pSong->_pMachine[MASTER_INDEX])->vuupdated = true;
				}
				if (viewMode == view_modes::machine)
				{
					//\todo : Move the commented code to a "Tweak", so we can reuse the code below of "Global::pPlayer->Tweaker"
/*					if (Global::pPlayer->_playing && Global::pPlayer->_lineChanged)
					{
						// This is meant to repaint the whole machine in case the panning/mute/solo/bypass has changed. (not really implemented right now)
						Repaint(draw_modes::all_machines);
					}
					else
					{*/
						CClientDC dc(this);
						DrawAllMachineVumeters(&dc);
//					}
				}

				for(int c=0; c<MAX_MACHINES; c++)
				{
					if (_pSong->_pMachine[c])
					{
						if ( _pSong->_pMachine[c]->_type == MACH_PLUGIN )
						{
							//if (pParentMain->isguiopen[c] && Global::pPlayer->Tweaker) maybe a todo
							//	pParentMain->m_pWndMac[c]->Invalidate(false);
						}
						else if ( _pSong->_pMachine[c]->_type == MACH_VST ||
								_pSong->_pMachine[c]->_type == MACH_VSTFX )
						{
							((vst::plugin*)_pSong->_pMachine[c])->Idle();
//							if (pParentMain->isguiopen[c] && Global::pPlayer->Tweaker)
//								((CVstEditorDlg*)pParentMain->m_pWndMac[c])->Refresh(-1,0);
						}
					}
				}
				Global::pPlayer->Tweaker = false;

				if (XMSamplerMachineDialog != NULL ) XMSamplerMachineDialog->UpdateUI();
				if (Global::pPlayer->_playing)
				{
					if (Global::pPlayer->_lineChanged)
					{
						Global::pPlayer->_lineChanged = false;
						pParentMain->SetAppSongBpm(0);
						pParentMain->SetAppSongTpb(0);

						if (Global::pConfig->_followSong)
						{
							CListBox* pSeqList = (CListBox*)pParentMain->m_wndSeq.GetDlgItem(IDC_SEQLIST);
							pattern_view()->editcur.line=Global::pPlayer->_lineCounter;
							if (pattern_view()->editPosition != Global::pPlayer->_playPosition)
							//if (pSeqList->GetCurSel() != Global::pPlayer->_playPosition)
							{
								pSeqList->SelItemRange(false,0,pSeqList->GetCount());
								pSeqList->SetSel(Global::pPlayer->_playPosition,true);
								int top = Global::pPlayer->_playPosition - 0xC;
								if (top < 0) top = 0;
								pSeqList->SetTopIndex(top);
								pattern_view()->editPosition=Global::pPlayer->_playPosition;
								if ( viewMode == view_modes::pattern ) 
								{ 
									Repaint(draw_modes::pattern);//draw_modes::playback_change);  // Until this mode is coded there is no point in calling it since it just makes patterns not refresh correctly currently
									Repaint(draw_modes::playback);
								}
							}
							else if( viewMode == view_modes::pattern ) Repaint(draw_modes::playback);
						}
						else if ( viewMode == view_modes::pattern ) Repaint(draw_modes::playback);
						if ( viewMode == view_modes::sequence ) Repaint(draw_modes::playback);
					}
				}
			}
			if (nIDEvent == 159 && !Global::pPlayer->_recording)
			{
				//MessageBox("Saving Disabled");
				//return;
				CString filepath = Global::pConfig->GetSongDir().c_str();
				filepath += "\\autosave.psy";
				OldPsyFile file;
				if(!file.Create(filepath.GetBuffer(1), true)) return;
				_pSong->Save(&file,true);
				/// \todo _pSong->Save() should not close a file which doesn't open. Add the following
				// line when fixed. There are other places which need this too.
				//file.Close();
			}
		}

		void CChildView::EnableSound()
		{
			if (_outputActive)
			{
				AudioDriver* pOut = Global::pConfig->_pOutputDriver;
				if (pOut->Enabled()) return;

				_outputActive = false;
				if (!pOut->Initialized())
				{
					pOut->Initialize(m_hWnd, Global::pPlayer->Work, Global::pPlayer);
				}
				if (!pOut->Configured())
				{
					pOut->Configure();
					Global::pPlayer->SampleRate(pOut->_samplesPerSec);
					_outputActive = true;
				}
				if (pOut->Enable(true))
				{
					_outputActive = true;
				}
				// MIDI IMPLEMENTATION
				Global::pConfig->_pMidiInput->Open();

				// set midi input mode to real-time or step
				if(Global::pConfig->_midiMachineViewSeqMode)
					CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
				else
					CMidiInput::Instance()->m_midiMode = MODE_STEP;

				Global::pPlayer->SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
				Global::pPlayer->SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
			}
		}


		/// Put exit destroying code here...
		void CChildView::OnDestroy()
		{
			if (Global::pConfig->_pOutputDriver->Initialized())
			{
				Global::pConfig->_pOutputDriver->Reset();
			}
			KillTimer(31);
			KillTimer(159);
		}

		void CChildView::OnAppExit() 
		{
			pParentMain->ClosePsycle();
		}

		void CChildView::OnPaint() 
		{		
			if (!GetUpdateRect(NULL) ) return; // If no area to update, exit.	
			CRgn pRgn;
			pRgn.CreateRectRgn(0, 0, 0, 0);
			GetUpdateRgn(&pRgn, FALSE);
			CPaintDC dc(this);		

			if ( bmpDC == NULL && Global::pConfig->useDoubleBuffer ) // buffer creation
			{
				CRect rc;
				GetClientRect(&rc);
				bmpDC = new CBitmap;
				bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
				char buf[100];
				sprintf(buf,"CChildView::OnPaint(). Initialized bmpDC to 0x%.8X\n",(int)bmpDC);
				TRACE(buf);
			}
			else if ( bmpDC != NULL && !Global::pConfig->useDoubleBuffer ) // buffer deletion
			{
				char buf[100];
				sprintf(buf,"CChildView::OnPaint(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
				TRACE(buf);
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			if ( Global::pConfig->useDoubleBuffer )
			{
				CDC bufDC;
				bufDC.CreateCompatibleDC(&dc);
				CBitmap* oldbmp;
				oldbmp = bufDC.SelectObject(bmpDC);
				if (viewMode==view_modes::machine)	// Machine view paint handler
				{
					machine_view_.Draw(&bufDC, pRgn); 
				}
				else if (viewMode == view_modes::pattern)	// Pattern view paint handler
				{
					pattern_view_.Draw(&bufDC, pRgn);
				}
				else if ( viewMode == view_modes::sequence)
				{
					DrawSeqEditor(&bufDC);
				}

				CRect rc;
				GetClientRect(&rc);
				dc.BitBlt(0,0,rc.right-rc.left,rc.bottom-rc.top,&bufDC,0,0,SRCCOPY);
				bufDC.SelectObject(oldbmp);
				bufDC.DeleteDC();
			}
			else
			{
				if (viewMode==view_modes::machine) // Machine view paint handler
				{
					machine_view_.Draw(&dc, pRgn);
				}
				else if (viewMode == view_modes::pattern)	// Pattern view paint handler
				{
					pattern_view_.Draw(&dc, pRgn);
				}
				else if ( viewMode == view_modes::sequence)
				{
					DrawSeqEditor(&dc);
				}
			}
		}

		void CChildView::Repaint(draw_modes::draw_mode drawMode)
		{
			if ( viewMode == view_modes::machine )
			{
				if ( drawMode <= draw_modes::machine )
				{
					updateMode = drawMode;
					Invalidate(false);
				}
			}
			else if ( viewMode == view_modes::pattern )
			{
				if (drawMode >= draw_modes::pattern || drawMode == draw_modes::all )	
				{
					pattern_view()->PreparePatternRefresh(drawMode);
				}
			}
			if ( viewMode == view_modes::sequence )
			{
				Invalidate(false);
			}
		}

		void CChildView::OnSize(UINT nType, int cx, int cy) 
		{
			CWnd ::OnSize(nType, cx, cy);

			machine_view_.OnSize(cx, cy);

			CW = cx;
			CH = cy;

			_pSong->viewSize.x=cx; // Hack to move machines boxes inside of the visible area.
			_pSong->viewSize.y=cy;
			
			if ( bmpDC != NULL && Global::pConfig->useDoubleBuffer ) // remove old buffer to force recreating it with new size
			{
				TRACE("CChildView::OnResize(). Deleted bmpDC");
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			if (viewMode == view_modes::pattern)
			{
				pattern_view()->OnSize(nType, cx, cy);				
			}
			Repaint();
		}

		/// "Save Song" Function
		BOOL CChildView::OnExport(UINT id) 
		{
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = Global::_pSong->fileName.substr(0,Global::_pSong->fileName.length()-4) + ".xm";
			std::string if2 = ifile.substr(0,ifile.find_first_of("\\/:*\"<>|"));
			
			char szFile[_MAX_PATH];

			szFile[_MAX_PATH-1]=0;
			strncpy(szFile,if2.c_str(),_MAX_PATH-1);
			
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "FastTracker 2 Song (*.xm)\0*.xm\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = TRUE;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CString str = ofn.lpstrFile;

				CString str2 = str.Right(3);
				if ( str2.CompareNoCase(".xm") != 0 ) str.Insert(str.GetLength(),".xm");
				int index = str.ReverseFind('\\');
				XMSongExport file;

				if (index != -1)
				{
					Global::pConfig->SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
				}
				
				if (!file.Create(str.GetBuffer(1), true))
				{
					MessageBox("Error creating file!", "Error!", MB_OK);
					return FALSE;
				}
				file.exportsong(*Global::_pSong);
				file.Close();
			}
			else
			{
				return FALSE;
			}
			return bResult;
		}

		/// "Save Song" Function
		BOOL CChildView::OnFileSave(UINT id) 
		{
			//MessageBox("Saving Disabled");
			//return false;
			BOOL bResult = TRUE;
			if ( Global::_pSong->_saved )
			{
				if (MessageBox("Proceed with Saving?","Song Save",MB_YESNO) == IDYES)
				{
					std::string filepath = Global::pConfig->GetCurrentSongDir();
					filepath += '\\';
					filepath += Global::_pSong->fileName;
					
					OldPsyFile file;
					if (!file.Create((char*)filepath.c_str(), true))
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}
					if (!_pSong->Save(&file))
					{
						MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}
					else 
					{
						_pSong->_saved=true;
						if (pattern_view()->pUndoList)
						{
							pattern_view()->UndoSaved = pattern_view()->pUndoList->counter;
						}
						else
						{
							pattern_view()->UndoSaved = 0;
						}
						UndoMacSaved = UndoMacCounter;
						SetTitleBarText();
					}				
					//file.Close();  <- save handles this 
				}
				else 
				{
					return FALSE;
				}
			}
			else 
			{
				return OnFileSaveAs(0);
			}
			return bResult;
		}

		//////////////////////////////////////////////////////////////////////
		// "Save Song As" Function

		BOOL CChildView::OnFileSaveAs(UINT id) 
		{
			//MessageBox("Saving Disabled");
			//return false;
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = Global::_pSong->fileName;
			std::string if2 = ifile.substr(0,ifile.find_first_of("\\/:*\"<>|"));
			
			char szFile[_MAX_PATH];

			szFile[_MAX_PATH-1]=0;
			strncpy(szFile,if2.c_str(),_MAX_PATH-1);
			
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Songs (*.psy)\0*.psy\0Psycle Pattern (*.psb)\0*.psb\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = TRUE;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CString str = ofn.lpstrFile;
				if ( ofn.nFilterIndex == 2 ) 
				{
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psb") != 0 ) str.Insert(str.GetLength(),".psb");
					sprintf(szFile,str);
					FILE* hFile=fopen(szFile,"wb");
					pattern_view()->SaveBlock(hFile);
					fflush(hFile);
					fclose(hFile);
				}
				else 
				{ 
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psy") != 0 ) str.Insert(str.GetLength(),".psy");
					int index = str.ReverseFind('\\');
					OldPsyFile file;

					if (index != -1)
					{
						Global::pConfig->SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
						Global::_pSong->fileName = str.Mid(index+1);
					}
					else
					{
						Global::_pSong->fileName = str;
					}
					
					if (!file.Create(str.GetBuffer(1), true))
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}
					if (!_pSong->Save(&file))
					{
						MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}
					else 
					{
						_pSong->_saved=true;
						AppendToRecent(str.GetBuffer(1));
						
						if (pattern_view()->pUndoList)
						{
							pattern_view()->UndoSaved = pattern_view()->pUndoList->counter;
						}
						else
						{
							pattern_view()->UndoSaved = 0;
						}
						UndoMacSaved = UndoMacCounter;
						SetTitleBarText();
					}
					//file.Close(); <- save handles this
				}
			}
			else
			{
				return FALSE;
			}
			return bResult;
		}

		#include <cderr.h>

		void CChildView::OnFileLoadsong()
		{
			OPENFILENAME ofn; // common dialog box structure
			char szFile[_MAX_PATH]; // buffer for file name
			
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Songs (*.psy)\0*.psy\0Psycle Pattern (*.psb)\0*.psb\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			
			// Display the Open dialog box. 
			if(::GetOpenFileName(&ofn)==TRUE)
			{
				OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
			}
			else
			{
				DWORD comDlgErr = CommDlgExtendedError();
				switch(comDlgErr)
				{
				case CDERR_DIALOGFAILURE:
					::MessageBox(0, "CDERR_DIALOGFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_FINDRESFAILURE:
					::MessageBox(0, "CDERR_FINDRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_INITIALIZATION:
					::MessageBox(0, "CDERR_INITIALIZATION", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOADRESFAILURE:
					::MessageBox(0, "CDERR_LOADRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOADSTRFAILURE:
					::MessageBox(0, "CDERR_LOADSTRFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOCKRESFAILURE:
					::MessageBox(0, "CDERR_LOCKRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_MEMALLOCFAILURE:
					::MessageBox(0, "CDERR_MEMALLOCFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_MEMLOCKFAILURE:
					::MessageBox(0, "CDERR_MEMLOCKFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOHINSTANCE:
					::MessageBox(0, "CDERR_NOHINSTANCE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOHOOK:
					::MessageBox(0, "CDERR_NOHOOK", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOTEMPLATE:
					::MessageBox(0, "CDERR_NOTEMPLATE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_REGISTERMSGFAIL:
					::MessageBox(0, "CDERR_REGISTERMSGFAIL", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_STRUCTSIZE:
					::MessageBox(0, "CDERR_STRUCTSIZE", "exception", MB_OK | MB_ICONERROR);
					break;
				}
			}
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnFileNew() 
		{
			if (CheckUnsavedSong("New Song"))
			{
				pattern_view()->KillUndo();
				pattern_view()->KillRedo();
				machine_view_.LockVu();
				Global::pPlayer->Stop();
				///\todo lock/unlock
				Sleep(256);
				_outputActive = false;
				Global::pConfig->_pOutputDriver->Enable(false);
				// midi implementation
				Global::pConfig->_pMidiInput->Close();
				///\todo lock/unlock
				Sleep(256);

				Global::_pSong->New();
				_outputActive = true;
				if (!Global::pConfig->_pOutputDriver->Enable(true))
				{
					_outputActive = false;
				}
				else
				{
					// midi implementation
					Global::pConfig->_pMidiInput->Open();
				}
				Global::pPlayer->SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
				SetTitleBarText();
				pattern_view()->editPosition=0;
				Global::_pSong->seqBus=0;
				pParentMain->PsybarsUpdate(); // Updates all values of the bars
				pParentMain->WaveEditorBackUpdate();
				pParentMain->m_wndInst.WaveUpdate();
				pParentMain->RedrawGearRackList();
				pParentMain->m_wndSeq.UpdateSequencer();
				pParentMain->m_wndSeq.UpdatePlayOrder(false); // should be done always after updatesequencer
				//pParentMain->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
				pattern_view()->RecalculateColourGrid();
				Repaint();
				machine_view_.Rebuild();
				machine_view_.UnlockVu();
			}
			pParentMain->StatusBarIdle();
		}


		void CChildView::OnFileSaveaudio() 
		{
			OnBarstop();
			KillTimer(31);
			KillTimer(159);
			OnTimer(159); // Autosave
			CSaveWavDlg dlg(this, &pattern_view()->blockSel);
			dlg.DoModal();
			InitTimer();
		}

		///\todo that method does not take machine changes into account  
		//  <JosepMa> is this still the case? or what does "machine changes" mean?
		BOOL CChildView::CheckUnsavedSong(std::string szTitle)
		{
			BOOL bChecked = TRUE;
			if (pattern_view()->pUndoList)
			{
				if (pattern_view()->UndoSaved != pattern_view()->pUndoList->counter)
				{
					bChecked = FALSE;
				}
			}
			else if (UndoMacSaved != UndoMacCounter)
			{
				bChecked = FALSE;
			}
			else
			{
				if (pattern_view()->UndoSaved != 0)
				{
					bChecked = FALSE;
				}
			}
			if (!bChecked)
			{
				if (Global::pConfig->bFileSaveReminders)
				{
					std::string filepath = Global::pConfig->GetCurrentSongDir();
					filepath += '\\';
					filepath += Global::_pSong->fileName;
					OldPsyFile file;
					std::ostringstream szText;
					szText << "Save changes to \"" << Global::_pSong->fileName
						<< "\"?";
					int result = MessageBox(szText.str().c_str(),szTitle.c_str(),MB_YESNOCANCEL | MB_ICONEXCLAMATION);
					switch (result)
					{
					case IDYES:
						if (!file.Create((char*)filepath.c_str(), true))
						{
							std::ostringstream szText;
							szText << "Error writing to \"" << filepath << "\"!!!";
							MessageBox(szText.str().c_str(),szTitle.c_str(),MB_ICONEXCLAMATION);
							return FALSE;
						}
						_pSong->Save(&file);
						//file.Close(); <- save handles this
						return TRUE;
						break;
					case IDNO:
						return TRUE;
						break;
					case IDCANCEL:
						return FALSE;
						break;
					}
				}
			}
			return TRUE;
		}

		void CChildView::OnFileRevert()
		{
			if (MessageBox("Warning! You will lose all changes since song was last saved! Proceed?","Revert to Saved",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				if (Global::_pSong->_saved)
				{
					std::ostringstream fullpath;
					fullpath << Global::pConfig->GetCurrentSongDir().c_str()
						<< '\\' << Global::_pSong->fileName.c_str();
					FileLoadsongNamed(fullpath.str());
				}
			}
			pParentMain->StatusBarIdle();
		}

		/// Tool bar buttons and View Commands
		void CChildView::OnMachineview() 
		{
			if (viewMode != view_modes::machine)
			{
				viewMode = view_modes::machine;
				ShowScrollBar(SB_BOTH,FALSE);

				// set midi input mode to real-time or Step
				if(Global::pConfig->_midiMachineViewSeqMode)
					CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
				else
					CMidiInput::Instance()->m_midiMode = MODE_STEP;

				Repaint();
				pParentMain->StatusBarIdle();
			}
			SetFocus();
		}

		void CChildView::OnUpdateMachineview(CCmdUI* pCmdUI) 
		{
			if (viewMode==view_modes::machine)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnPatternView() 
		{
			if (viewMode != view_modes::pattern)
			{
				pattern_view()->RecalcMetrics();

				viewMode = view_modes::pattern;
				//ShowScrollBar(SB_BOTH,FALSE);
				
				// set midi input mode to step insert
				CMidiInput::Instance()->m_midiMode = MODE_STEP;
				
				GetParent()->SetActiveWindow();

				if
					(
						Global::pConfig->_followSong &&
						pattern_view()->editPosition  != Global::pPlayer->_playPosition &&
						Global::pPlayer->_playing
					)
				{
					pattern_view()->editPosition=Global::pPlayer->_playPosition;
				}
				Repaint();
				pParentMain->StatusBarIdle();
			}
			SetFocus();
		}

		void CChildView::OnUpdatePatternView(CCmdUI* pCmdUI) 
		{
			if(viewMode == view_modes::pattern)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnShowPatternSeq() 
		{
			/*
			if (viewMode != view_modes::sequence)
			{
				viewMode = view_modes::sequence;
				ShowScrollBar(SB_BOTH,FALSE);
				
				// set midi input mode to step insert
				CMidiInput::Instance()->m_midiMode = MODE_STEP;
				
				GetParent()->SetActiveWindow();
				Repaint();
				pParentMain->StatusBarIdle();
			}	
			*/
			SetFocus();
		}

		void CChildView::OnUpdatePatternSeq(CCmdUI* pCmdUI) 
		{
			if (viewMode==view_modes::sequence)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);	
		}

		void CChildView::OnBarplay() 
		{
			if (Global::pConfig->_followSong)
			{
				bScrollDetatch=false;
			}
			pattern_view()->prevEditPosition=pattern_view()->editPosition;
			Global::pPlayer->Start(pattern_view()->editPosition,0);
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnBarplayFromStart() 
		{
			if (Global::pConfig->_followSong)
			{
				bScrollDetatch=false;
			}
			pattern_view()->prevEditPosition=pattern_view()->editPosition;
			Global::pPlayer->Start(0,0);
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnUpdateBarplay(CCmdUI* pCmdUI) 
		{
			if (Global::pPlayer->_playing)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnUpdateBarplayFromStart(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(0);
		}

		void CChildView::OnBarrec() 
		{
			if (Global::pConfig->_followSong && pattern_view()->bEditMode)
			{
				pattern_view()->bEditMode = FALSE;
			}
			else
			{
				Global::pConfig->_followSong = TRUE;
				pattern_view()->bEditMode = TRUE;
				CButton*cb=(CButton*)pParentMain->m_wndSeq.GetDlgItem(IDC_FOLLOW);
				cb->SetCheck(1);
			}
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnUpdateBarrec(CCmdUI* pCmdUI) 
		{
			if (Global::pConfig->_followSong && pattern_view()->bEditMode)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnButtonplayseqblock() 
		{
			if (Global::pConfig->_followSong)
			{
				bScrollDetatch=false;
			}

			pattern_view()->prevEditPosition=pattern_view()->editPosition;
			int i=0;
			while ( Global::_pSong->playOrderSel[i] == false ) i++;
			
			if(!Global::pPlayer->_playing)
				Global::pPlayer->Start(i,0);

			Global::pPlayer->_playBlock=!Global::pPlayer->_playBlock;

			pParentMain->StatusBarIdle();
			if ( viewMode == view_modes::pattern ) Repaint(draw_modes::pattern);
		}

		void CChildView::OnUpdateButtonplayseqblock(CCmdUI* pCmdUI) 
		{
			if ( Global::pPlayer->_playBlock == true ) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
		}

		void CChildView::OnBarstop()
		{
			bool pl = Global::pPlayer->_playing;
			bool blk = Global::pPlayer->_playBlock;
			Global::pPlayer->Stop();
			pParentMain->SetAppSongBpm(0);
			pParentMain->SetAppSongTpb(0);

			if (pl)
			{
				if ( Global::pConfig->_followSong && blk)
				{
					pattern_view()->editPosition=pattern_view()->prevEditPosition;
					pParentMain->UpdatePlayOrder(false); // <- This restores the selected block
					Repaint(draw_modes::pattern);
				}
				else
				{
					memset(Global::_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
					Global::_pSong->playOrderSel[pattern_view()->editPosition] = true;
					Repaint(draw_modes::cursor); 
				}
			}
		}

		void CChildView::OnRecordWav() 
		{
			if (!Global::pPlayer->_recording)
			{
				static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
				
				CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
				if ( dlg.DoModal() == IDOK ) 
				{
					Global::pPlayer->StartRecording(dlg.GetFileName().GetBuffer(4));
				}
				if ( Global::pConfig->autoStopMachines ) 
				{
					OnAutostop();
				}
			}
			else
			{
				Global::pPlayer->StopRecording();
			}
		}

		void CChildView::OnUpdateRecordWav(CCmdUI* pCmdUI) 
		{
			if (Global::pPlayer->_recording)
			{
				pCmdUI->SetCheck(1);
			}
			else
			{
				pCmdUI->SetCheck(0);
			}
		}

		void CChildView::OnAutostop() 
		{
			if ( Global::pConfig->autoStopMachines )
			{
				Global::pConfig->autoStopMachines = false;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (Global::_pSong->_pMachine[c])
					{
						Global::_pSong->_pMachine[c]->Standby(false);
					}
				}
			}
			else Global::pConfig->autoStopMachines = true;
		}

		void CChildView::OnUpdateAutostop(CCmdUI* pCmdUI) 
		{
			if (Global::pConfig->autoStopMachines == true ) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
		}

		void CChildView::OnFileSongproperties() 
		{	CSongpDlg dlg(Global::_pSong);
			dlg.DoModal();
			pParentMain->StatusBarIdle();
			//Repaint();
		}

		void CChildView::OnViewInstrumenteditor()
		{
			pParentMain->ShowInstrumentEditor();
		}


		/// Show the CPU Performance dialog
		void CChildView::OnHelpPsycleenviromentinfo() 
		{
			pParentMain->ShowPerformanceDlg();
		}

		/// Show the MIDI monitor dialog
		void CChildView::OnMidiMonitorDlg() 
		{
			pParentMain->ShowMidiMonitorDlg();
		}
		
		void CChildView::OnNewmachine() 
		{
			machine_view()->ShowNewMachineDlg(-1, -1, 0, false);
		}	

		void CChildView::OnConfigurationSettings() 
		{
			CConfigDlg dlg("Psycle Settings");
			_outputActive = false;
			dlg.Init(Global::pConfig);
			if (dlg.DoModal() == IDOK)
			{
				KillTimer(159);
				if ( Global::pConfig->autosaveSong )
				{
					SetTimer(159,Global::pConfig->autosaveSongTime*60000,NULL);
				}
				_outputActive = true;
				EnableSound();
			}
			//Repaint();
		}

		void CChildView::OnHelpSaludos() 
		{
			CGreetDialog dlg;
			dlg.DoModal();
			//Repaint();
		}

		///\todo extemely toxic pollution
		#define TWOPI_F (2.0f*3.141592665f)

		//// Right Click Popup Menu
		void CChildView::OnPopCut() { 
			pattern_view()->CopyBlock(true);
		}

		void CChildView::OnUpdateCutCopy(CCmdUI* pCmdUI) 
		{
			if (pattern_view()->blockSelected && (viewMode == view_modes::pattern)) pCmdUI->Enable(TRUE);
			else pCmdUI->Enable(FALSE);
		}


		void CChildView::OnPopCopy() {
			pattern_view()->CopyBlock(false);
		}

		void CChildView::OnPopPaste() {
			pattern_view()->OnPopPaste();
		}
		void CChildView::OnUpdatePaste(CCmdUI* pCmdUI) 
		{
			if (pattern_view()->isBlockCopied  && (viewMode == view_modes::pattern)) pCmdUI->Enable(TRUE);
			else  pCmdUI->Enable(FALSE);
		}

		void CChildView::OnPopMixpaste() { 
			pattern_view()->OnPopMixpaste();
		}

		void CChildView::OnPopBlockswitch()
		{
			pattern_view()->OnPopBlockswitch();
		}

		void CChildView::OnUpdatePopBlockswitch(CCmdUI *pCmdUI)
		{
			if (pattern_view()->isBlockCopied && (viewMode == view_modes::pattern)) pCmdUI->Enable(true);
			else  pCmdUI->Enable(false);
		}

		void CChildView::OnPopDelete() {
			pattern_view()->DeleteBlock();
		}

		void CChildView::OnPopInterpolate() {
			pattern_view()->BlockParamInterpolate();
		}

		void CChildView::OnPopInterpolateCurve()
		{
			pattern_view()->OnPopInterpolateCurve();
		}


		void CChildView::OnPopChangegenerator() {
			pattern_view()->BlockGenChange(_pSong->seqBus);
		}

		void CChildView::OnPopChangeinstrument() { 
			pattern_view()->BlockInsChange(_pSong->auxcolSelected);
		}

		void CChildView::OnPopTranspose1() {
			pattern_view()->BlockTranspose(1);
		}

		void CChildView::OnPopTranspose12() {
			pattern_view()->BlockTranspose(12);
		}

		void CChildView::OnPopTranspose_1() {
			pattern_view()->BlockTranspose(-1);
		}

		void CChildView::OnPopTranspose_12() {
			pattern_view()->BlockTranspose(-12);
		}

		void CChildView::OnPopTransformpattern() 
		{
			pattern_view()->ShowTransformPatternDlg();			
		}

		void CChildView::OnPopPattenproperties() 
		{
			pattern_view()->ShowPatternDlg();
		}

		/// fill block
		void CChildView::OnPopBlockSwingfill()
		{
			pattern_view()->ShowSwingFillDlg(FALSE);
		}

		/// fill track
		void CChildView::OnPopTrackSwingfill()
		{
			pattern_view()->ShowSwingFillDlg(TRUE);
		}

		void CChildView::OnUpdateUndo(CCmdUI* pCmdUI)
		{
			pattern_view()->OnUpdateUndo(pCmdUI);
		}

		void CChildView::OnUpdateRedo(CCmdUI* pCmdUI)
		{
			pattern_view()->OnUpdateRedo(pCmdUI);
		}

		void CChildView::OnUpdatePatternCutCopy(CCmdUI* pCmdUI) 
		{
			if(viewMode == view_modes::pattern) pCmdUI->Enable(TRUE);
			else pCmdUI->Enable(FALSE);
		}

		void CChildView::OnUpdatePatternPaste(CCmdUI* pCmdUI) 
		{
			if(pattern_view()->patBufferCopy&&(viewMode == view_modes::pattern)) pCmdUI->Enable(TRUE);
			else pCmdUI->Enable(FALSE);
		}

		void CChildView::OnFileImportModulefile() 
		{
			OPENFILENAME ofn; // common dialog box structure
			char szFile[_MAX_PATH]; // buffer for file name
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter =
				"All Module Songs (*.xm *.it *.s3m *.mod)" "\0" "*.xm;*.it;*.s3m;*.mod" "\0"
				"FastTracker II Songs (*.xm)"              "\0" "*.xm"                  "\0"
				"Impulse Tracker Songs (*.it)"             "\0" "*.it"                  "\0"
				"Scream Tracker Songs (*.s3m)"             "\0" "*.s3m"                 "\0"
				"Original Mod Format Songs (*.mod)"        "\0" "*.mod"                 "\0"
				"All (*)"                                  "\0" "*"                     "\0"
				;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn)==TRUE)
			{
				pattern_view()->KillUndo();
				pattern_view()->KillRedo();
				machine_view_.LockVu();
				Global::pPlayer->Stop();
				///\todo lock/unlock
				Sleep(256);
				_outputActive = false;
				Global::pConfig->_pOutputDriver->Enable(false);
				// MIDI IMPLEMENTATION
				Global::pConfig->_pMidiInput->Close();
				///\todo lock/unlock
				Sleep(256);

				CString str = ofn.lpstrFile;
				int index = str.ReverseFind('.');
				if (index != -1)
				{
					CString ext = str.Mid(index+1);
					if (ext.CompareNoCase("XM") == 0)
					{
						XMSongLoader xmfile;
						xmfile.Open(ofn.lpstrFile);
						Global::_pSong->New();
						pattern_view()->editPosition=0;
						xmfile.Load(*_pSong);
						xmfile.Close();
						machine_view_.Rebuild();
						CSongpDlg dlg(Global::_pSong);
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "XM file imported", MB_OK);
*/
					} else if (ext.CompareNoCase("IT") == 0)
					{
						ITModule2 it;
						it.Open(ofn.lpstrFile);
						Global::_pSong->New();
						pattern_view()->editPosition=0;
						if(!it.LoadITModule(_pSong))
						{			
							MessageBox("Load failed");
							Global::_pSong->New();
							it.Close();
							return;
						}
						it.Close();
						machine_view_.Rebuild();
						CSongpDlg dlg(Global::_pSong);
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "IT file imported", MB_OK);
*/
					} else if (ext.CompareNoCase("S3M") == 0)
					{
						ITModule2 s3m;
						s3m.Open(ofn.lpstrFile);
						Global::_pSong->New();
						pattern_view()->editPosition=0;
						if(!s3m.LoadS3MModuleX(_pSong))
						{			
							MessageBox("Load failed");
							Global::_pSong->New();
							s3m.Close();
							return;
						}
						s3m.Close();
						machine_view_.Rebuild();
						CSongpDlg dlg(Global::_pSong);
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "S3M file imported", MB_OK);
*/
					} else if (ext.CompareNoCase("MOD") == 0)
					{
						MODSongLoader modfile;
						modfile.Open(ofn.lpstrFile);
						Global::_pSong->New();
						pattern_view()->editPosition=0;
						modfile.Load(*_pSong);
						modfile.Close();
						machine_view_.Rebuild();
						CSongpDlg dlg(Global::_pSong);
						dlg.SetReadOnly();
						dlg.DoModal();
/*						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().name.c_str(),
							Global::song().author.c_str(),
							Global::song().comments.c_str()
							);
						MessageBox(buffer, "MOD file imported", MB_OK);
*/
					}
				}

				str = ofn.lpstrFile;
				index = str.ReverseFind('\\');
				if (index != -1)
				{
					Global::pConfig->SetCurrentSongDir((LPCSTR)str.Left(index));
					Global::_pSong->fileName = str.Mid(index+1)+".psy";
				}
				else
				{
					Global::_pSong->fileName = str+".psy";
				}
				_outputActive = true;
				if (!Global::pConfig->_pOutputDriver->Enable(true))
				{
					_outputActive = false;
				}
				else
				{
					// MIDI IMPLEMENTATION
					Global::pConfig->_pMidiInput->Open();
				}
				pParentMain->PsybarsUpdate();
				pParentMain->WaveEditorBackUpdate();
				pParentMain->m_wndInst.WaveUpdate();
				pParentMain->RedrawGearRackList();
				pParentMain->m_wndSeq.UpdateSequencer();
				pParentMain->m_wndSeq.UpdatePlayOrder(false);
				pattern_view()->RecalculateColourGrid();
				Repaint();
				machine_view_.UnlockVu();
			}
			SetTitleBarText();
		}

		void CChildView::AppendToRecent(std::string fName)
		{
			int iCount;
			char* nameBuff;
			UINT nameSize;
			HMENU hFileMenu, hRootMenuBar;
			UINT ids[] =
				{
					ID_FILE_RECENT_01,
					ID_FILE_RECENT_02,
					ID_FILE_RECENT_03,
					ID_FILE_RECENT_04
				};
			MENUITEMINFO hNewItemInfo, hTempItemInfo;
			hRootMenuBar = ::GetMenu(this->GetParent()->m_hWnd);
			//pRootMenuBar = this->GetParent()->GetMenu();
			//hRootMenuBar = HMENU (*pRootMenuBar);
			hFileMenu = GetSubMenu(hRootMenuBar, 0);
			hRecentMenu = GetSubMenu(hFileMenu, 11);
			// Remove initial empty element, if present.
			if(GetMenuItemID(hRecentMenu, 0) == ID_FILE_RECENT_NONE)
			{
				DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
			}
			// Check for duplicates and eventually remove.
			for(iCount = 0; iCount<GetMenuItemCount(hRecentMenu);iCount++)
			{
				nameSize = GetMenuString(hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
				nameBuff = new char[nameSize];
				GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
				if ( !strcmp(nameBuff, fName.c_str()) )
				{
					DeleteMenu(hRecentMenu, iCount, MF_BYPOSITION);
				}
				delete[] nameBuff;
			}
			// Ensure menu size doesn't exceed 4 positions.
			if (GetMenuItemCount(hRecentMenu) == 4)
			{
				DeleteMenu(hRecentMenu, 4-1, MF_BYPOSITION);
			}
			hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
			hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
			hNewItemInfo.fType		= MFT_STRING;
			hNewItemInfo.wID		= ids[0];
			hNewItemInfo.cch		= fName.length();
			hNewItemInfo.dwTypeData = (LPSTR)fName.c_str();
			InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);
			// Update identifiers.
			for(iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
			{
				hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
				hTempItemInfo.fMask		= MIIM_ID;
				hTempItemInfo.wID		= ids[iCount];
				SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
			}
		}

		void CChildView::OnFileRecent_01()
		{
			CallOpenRecent(0);
		}

		void CChildView::OnFileRecent_02()
		{
			CallOpenRecent(1);
		}

		void CChildView::OnFileRecent_03()
		{
			CallOpenRecent(2);
		}

		void CChildView::OnFileRecent_04()
		{
			CallOpenRecent(3);
		}

		void CChildView::OnFileLoadsongNamed(std::string fName, int fType)
		{
			if( fType == 2 )
			{
				FILE* hFile=fopen(fName.c_str(),"rb");
				pattern_view()->LoadBlock(hFile);
				fclose(hFile);
			}
			else
			{
				if (CheckUnsavedSong("Load Song"))
				{
					FileLoadsongNamed(fName);
				}
			}
		}

		void CChildView::FileLoadsongNamed(std::string fName)
		{
			machine_view_.LockVu();
			Global::pPlayer->Stop();			
			///\todo lock/unlock
			Sleep(256);
			_outputActive = false;
			Global::pConfig->_pOutputDriver->Enable(false);
			// MIDI IMPLEMENTATION
			Global::pConfig->_pMidiInput->Close();
			///\todo lock/unlock
			Sleep(256);
			
			OldPsyFile file;
			if (!file.Open(fName.c_str()))
			{
				MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
				return;
			}
			pattern_view()->editPosition = 0;
			_pSong->Load(&file);
			//file.Close(); <- load handles this
			_pSong->_saved=true;
			AppendToRecent(fName);
			std::string::size_type index = fName.rfind('\\');
			if (index != std::string::npos)
			{
				Global::pConfig->SetCurrentSongDir(fName.substr(0,index));
				Global::_pSong->fileName = fName.substr(index+1);
			}
			else
			{
				Global::_pSong->fileName = fName;
			}
			Global::pPlayer->SetBPM(Global::_pSong->BeatsPerMin(), Global::_pSong->LinesPerBeat());
			_outputActive = true;
			if (!Global::pConfig->_pOutputDriver->Enable(true))
			{
				_outputActive = false;
			}
			else
			{
				// MIDI IMPLEMENTATION
				
				Global::pConfig->_pMidiInput->Open();
			}

			pParentMain->PsybarsUpdate();
			pParentMain->WaveEditorBackUpdate();
			pParentMain->m_wndInst.WaveUpdate();
			pParentMain->RedrawGearRackList();
			pParentMain->m_wndSeq.UpdateSequencer();
			pParentMain->m_wndSeq.UpdatePlayOrder(false);
			//pParentMain->UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
			pattern_view()->RecalculateColourGrid();
			Repaint();
			pattern_view()->KillUndo();
			pattern_view()->KillRedo();
			SetTitleBarText();
			machine_view_.Rebuild();
			machine_view_.UnlockVu();
			if (Global::pConfig->bShowSongInfoOnLoad)
			{
				CSongpDlg dlg(Global::_pSong);
				dlg.SetReadOnly();
				dlg.DoModal();
/*				std::ostringstream songLoaded;
				songLoaded << '\'' << _pSong->name << '\'' << std::endl
					<< std::endl
					<< _pSong->author << std::endl
					<< std::endl
					<< _pSong->comments;
				MessageBox(songLoaded.str().c_str(), "Psycle song loaded", MB_OK);
*/
			}
		}

		void CChildView::CallOpenRecent(int pos)
		{
			UINT nameSize;
			nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
			char* nameBuff = new char[nameSize];
			GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
			OnFileLoadsongNamed(nameBuff, 1);
			delete [] nameBuff; nameBuff = 0;
		}

		void CChildView::SetTitleBarText()
		{
			std::string titlename = "[";
			titlename+=Global::_pSong->fileName;
			/*
			if(!(Global::_pSong->_saved))
			{
				titlename+=" *";
			}
			else
			*/ 
			if(pattern_view()->pUndoList)
			{
				if (pattern_view()->UndoSaved != pattern_view()->pUndoList->counter)
				{
					titlename+=" *";
				}
			}
			else if (UndoMacSaved != UndoMacCounter)
			{
				titlename+=" *";
			}
			else
			{
				if (pattern_view()->UndoSaved != 0)
				{
					titlename+=" *";
				}
			}
			// don't know how to access to the IDR_MAINFRAME String Title.
			titlename += "] Psycle Modular Music Creation Studio (" PSYCLE__VERSION ")";
			pParentMain->SetWindowText(titlename.c_str());
		}

		void CChildView::OnHelpKeybtxt() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\keys.txt",Global::pConfig->appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpReadme() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\readme.txt",Global::pConfig->appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpTweaking() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\tweaking.txt",Global::pConfig->appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpWhatsnew() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\whatsnew.txt",Global::pConfig->appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::LoadMachineSkin()
		{
			std::string szOld;
			LoadMachineDial();
			if (!Global::pConfig->machine_skin.empty())
			{
				szOld = Global::pConfig->machine_skin;
				if (szOld != PSYCLE__PATH__DEFAULT_MACHINE_SKIN)
				{
					BOOL result = FALSE;
					FindMachineSkin(Global::pConfig->GetSkinDir().c_str(),Global::pConfig->machine_skin.c_str(), &result);
					if(result)
					{
						return;
					}
				}
				// load defaults
				szOld = PSYCLE__PATH__DEFAULT_MACHINE_SKIN;
				// and coords
			#if defined PSYCLE__CONFIGURATION__SKIN__UGLY_DEFAULT
				MachineCoords.sMaster.x = 0;
				MachineCoords.sMaster.y = 0;
				MachineCoords.sMaster.width = 148;
				MachineCoords.sMaster.height = 48;

				MachineCoords.sGenerator.x = 0;
				MachineCoords.sGenerator.y = 48;
				MachineCoords.sGenerator.width = 148;
				MachineCoords.sGenerator.height = 48;
				MachineCoords.sGeneratorVu0.x = 0;
				MachineCoords.sGeneratorVu0.y = 144;
				MachineCoords.sGeneratorVu0.width = 6;
				MachineCoords.sGeneratorVu0.height = 5;
				MachineCoords.sGeneratorVuPeak.x = 96;
				MachineCoords.sGeneratorVuPeak.y = 144;
				MachineCoords.sGeneratorVuPeak.width = 6;
				MachineCoords.sGeneratorVuPeak.height = 5;
				MachineCoords.sGeneratorPan.x = 21;
				MachineCoords.sGeneratorPan.y = 149;
				MachineCoords.sGeneratorPan.width = 24;
				MachineCoords.sGeneratorPan.height = 9;
				MachineCoords.sGeneratorMute.x = 7;
				MachineCoords.sGeneratorMute.y = 149;
				MachineCoords.sGeneratorMute.width = 7;
				MachineCoords.sGeneratorMute.height = 7;
				MachineCoords.sGeneratorSolo.x = 14;
				MachineCoords.sGeneratorSolo.y = 149;
				MachineCoords.sGeneratorSolo.width = 7;
				MachineCoords.sGeneratorSolo.height = 7;

				MachineCoords.sEffect.x = 0;
				MachineCoords.sEffect.y = 96;
				MachineCoords.sEffect.width = 148;
				MachineCoords.sEffect.height = 48;
				MachineCoords.sEffectVu0.x = 0;
				MachineCoords.sEffectVu0.y = 144;
				MachineCoords.sEffectVu0.width = 6;
				MachineCoords.sEffectVu0.height = 5;
				MachineCoords.sEffectVuPeak.x = 96;
				MachineCoords.sEffectVuPeak.y = 144;
				MachineCoords.sEffectVuPeak.width = 6;
				MachineCoords.sEffectVuPeak.height = 5;
				MachineCoords.sEffectPan.x = 21;
				MachineCoords.sEffectPan.y = 149;
				MachineCoords.sEffectPan.width = 24;
				MachineCoords.sEffectPan.height = 9;
				MachineCoords.sEffectMute.x = 7;
				MachineCoords.sEffectMute.y = 149;
				MachineCoords.sEffectMute.width = 7;
				MachineCoords.sEffectMute.height = 7;
				MachineCoords.sEffectBypass.x = 0;
				MachineCoords.sEffectBypass.y = 149;
				MachineCoords.sEffectBypass.width = 7;
				MachineCoords.sEffectBypass.height = 12;

				MachineCoords.dGeneratorVu.x = 8;
				MachineCoords.dGeneratorVu.y = 3;
				MachineCoords.dGeneratorVu.width = 96;
				MachineCoords.dGeneratorVu.height = 0;
				MachineCoords.dGeneratorPan.x = 3;
				MachineCoords.dGeneratorPan.y = 35;
				MachineCoords.dGeneratorPan.width = 117;
				MachineCoords.dGeneratorPan.height = 0;
				MachineCoords.dGeneratorMute.x = 137;
				MachineCoords.dGeneratorMute.y = 4;
				MachineCoords.dGeneratorSolo.x = 137;
				MachineCoords.dGeneratorSolo.y = 17;
				MachineCoords.dGeneratorName.x = 10;
				MachineCoords.dGeneratorName.y = 12;

				MachineCoords.dEffectVu.x = 8;
				MachineCoords.dEffectVu.y = 3;
				MachineCoords.dEffectVu.width = 96;
				MachineCoords.dEffectVu.height = 0;
				MachineCoords.dEffectPan.x = 3;
				MachineCoords.dEffectPan.y = 35;
				MachineCoords.dEffectPan.width = 117;
				MachineCoords.dEffectPan.height = 0;
				MachineCoords.dEffectMute.x = 137;
				MachineCoords.dEffectMute.y = 4;
				MachineCoords.dEffectBypass.x = 137;
				MachineCoords.dEffectBypass.y = 15;
				MachineCoords.dEffectName.x = 10;
				MachineCoords.dEffectName.y = 12;
				MachineCoords.bHasTransparency = false;
			#else
				MachineCoords.sMaster.x = 0;
				MachineCoords.sMaster.y = 0;
				MachineCoords.sMaster.width = 148;
				MachineCoords.sMaster.height = 47;//48;

				MachineCoords.sGenerator.x = 0;
				MachineCoords.sGenerator.y = 47;//48;
				MachineCoords.sGenerator.width = 148;
				MachineCoords.sGenerator.height = 47;//48;
				MachineCoords.sGeneratorVu0.x = 0;
				MachineCoords.sGeneratorVu0.y = 141;//144;
				MachineCoords.sGeneratorVu0.width = 7;//6;
				MachineCoords.sGeneratorVu0.height = 4;//5;
				MachineCoords.sGeneratorVuPeak.x = 128;//96;
				MachineCoords.sGeneratorVuPeak.y = 141;//144;
				MachineCoords.sGeneratorVuPeak.width = 2;//6;
				MachineCoords.sGeneratorVuPeak.height = 4;//5;
				MachineCoords.sGeneratorPan.x = 45;//102;
				MachineCoords.sGeneratorPan.y = 145;//144;
				MachineCoords.sGeneratorPan.width = 16;//24;
				MachineCoords.sGeneratorPan.height = 5;//9;
				MachineCoords.sGeneratorMute.x = 0;//133;
				MachineCoords.sGeneratorMute.y = 145;//144;
				MachineCoords.sGeneratorMute.width = 15;//7;
				MachineCoords.sGeneratorMute.height = 14;//7;
				MachineCoords.sGeneratorSolo.x = 15;//140;
				MachineCoords.sGeneratorSolo.y = 145;//144;
				MachineCoords.sGeneratorSolo.width = 15;//7;
				MachineCoords.sGeneratorSolo.height = 14;//7;

				MachineCoords.sEffect.x = 0;
				MachineCoords.sEffect.y = 94;//96;
				MachineCoords.sEffect.width = 148;
				MachineCoords.sEffect.height = 47;//48;
				MachineCoords.sEffectVu0.x = 0;
				MachineCoords.sEffectVu0.y = 141;//144;
				MachineCoords.sEffectVu0.width = 7;//6;
				MachineCoords.sEffectVu0.height = 4;//5;
				MachineCoords.sEffectVuPeak.x = 128;//96;
				MachineCoords.sEffectVuPeak.y = 141;//144;
				MachineCoords.sEffectVuPeak.width = 2;//6;
				MachineCoords.sEffectVuPeak.height = 4;//5;
				MachineCoords.sEffectPan.x = 45;//102;
				MachineCoords.sEffectPan.y = 145;//144;
				MachineCoords.sEffectPan.width = 16;//24;
				MachineCoords.sEffectPan.height = 5;//9;
				MachineCoords.sEffectMute.x = 0;//133;
				MachineCoords.sEffectMute.y = 145;//144;
				MachineCoords.sEffectMute.width = 15;//7;
				MachineCoords.sEffectMute.height = 14;//7;
				MachineCoords.sEffectBypass.x = 30;//126;
				MachineCoords.sEffectBypass.y = 145;//144;
				MachineCoords.sEffectBypass.width = 15;//7;
				MachineCoords.sEffectBypass.height = 14;//13;

				MachineCoords.dGeneratorVu.x = 10;//8;
				MachineCoords.dGeneratorVu.y = 35;//3;
				MachineCoords.dGeneratorVu.width = 130;//96;
				MachineCoords.dGeneratorVu.height = 0;
				MachineCoords.dGeneratorPan.x = 39;//3;
				MachineCoords.dGeneratorPan.y = 26;//35;
				MachineCoords.dGeneratorPan.width = 91;//117;
				MachineCoords.dGeneratorPan.height = 0;
				MachineCoords.dGeneratorMute.x = 11;//137;
				MachineCoords.dGeneratorMute.y = 5;//4;
				MachineCoords.dGeneratorSolo.x = 26;//137;
				MachineCoords.dGeneratorSolo.y = 5;//17;
				MachineCoords.dGeneratorName.x = 49;//10;
				MachineCoords.dGeneratorName.y = 7;//12;

				MachineCoords.dEffectVu.x = 10;//8;
				MachineCoords.dEffectVu.y = 35;//3;
				MachineCoords.dEffectVu.width = 130;//96;
				MachineCoords.dEffectVu.height = 0;
				MachineCoords.dEffectPan.x = 39;//3;
				MachineCoords.dEffectPan.y = 26;//35;
				MachineCoords.dEffectPan.width = 91;//117;
				MachineCoords.dEffectPan.height = 0;
				MachineCoords.dEffectMute.x = 11;//137;
				MachineCoords.dEffectMute.y = 5;//4;
				MachineCoords.dEffectBypass.x = 26;//137;
				MachineCoords.dEffectBypass.y = 5;//15;
				MachineCoords.dEffectName.x = 49;//10;
				MachineCoords.dEffectName.y = 7;//12;
				MachineCoords.bHasTransparency = false;
			#endif
				machineskin.DeleteObject();
				DeleteObject(hbmMachineSkin);
				machineskinmask.DeleteObject();
				machineskin.LoadBitmap(IDB_MACHINE_SKIN);
			}
		}

		void CChildView::FindMachineSkin(CString findDir, CString findName, BOOL *result)
		{
			CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*"); // check for subfolders.
			while (loop) 
			{								
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					FindMachineSkin(finder.GetFilePath(),findName,result);
					if ( *result == TRUE) return;
				}
			}
			finder.Close();
			loop = finder.FindFile(findDir + "\\" + findName + ".psm"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psm, does it have a valid matching .bmp?
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					char szOpenName[MAX_PATH];
					sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);

					machineskin.DeleteObject();
					if( hbmMachineSkin) DeleteObject(hbmMachineSkin);
					machineskinmask.DeleteObject();
					hbmMachineSkin = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hbmMachineSkin)
					{
						if (machineskin.Attach(hbmMachineSkin))
						{	
							memset(&MachineCoords,0,sizeof(MachineCoords));
							// load settings
							FILE* hfile;
							sprintf(szOpenName,"%s\\%s.psm",findDir,sName);
							if(!(hfile=fopen(szOpenName,"rb")))
							{
								MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
								return;
							}
							char buf[512];
							while (fgets(buf, 512, hfile))
							{
								if (strstr(buf,"\"master_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sMaster.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sMaster.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sMaster.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sMaster.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGenerator.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGenerator.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGenerator.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGenerator.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorVu0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVu0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorVu0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorVu0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu_peak_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorVuPeak.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVuPeak.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorVuPeak.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorVuPeak.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_pan_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_mute_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorMute.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorMute.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorMute.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_solo_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorSolo.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorSolo.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorSolo.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorSolo.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffect.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffect.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffect.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffect.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_vu0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectVu0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVu0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectVu0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectVu0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_vu_peak_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectVuPeak.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVuPeak.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectVuPeak.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectVuPeak.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_pan_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_mute_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectMute.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectMute.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectMute.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_bypass_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectBypass.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectBypass.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectBypass.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectBypass.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorVu.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorVu.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dGeneratorVu.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dGeneratorVu.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_pan_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dGeneratorPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dGeneratorPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_mute_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorMute.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"generator_solo_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorSolo.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorSolo.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"generator_name_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorName.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorName.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_vu_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectVu.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectVu.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dEffectVu.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dEffectVu.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_pan_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dEffectPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dEffectPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_mute_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectMute.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_bypass_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectBypass.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectBypass.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_name_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectName.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectName.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"transparency\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										helpers::hexstring_to_integer(q+1, MachineCoords.cTransparency);
										MachineCoords.bHasTransparency = true;
									}
								}
							}
							if (MachineCoords.bHasTransparency)
							{
								PrepareMask(&machineskin,&machineskinmask,MachineCoords.cTransparency);
							}
							fclose(hfile);
							*result = TRUE;
							break;
						}
					}
				}
			}
			finder.Close();
		}

		void CChildView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
		{
			BITMAP bm;
			// Get the dimensions of the source bitmap
			pBmpSource->GetObject(sizeof(BITMAP), &bm);
			// Create the mask bitmap
			pBmpMask->DeleteObject();
			pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
			// We will need two DCs to work with. One to hold the Image
			// (the source), and one to hold the mask (destination).
			// When blitting onto a monochrome bitmap from a color, pixels
			// in the source color bitmap that are equal to the background
			// color are blitted as white. All the remaining pixels are
			// blitted as black.
			CDC hdcSrc, hdcDst;
			hdcSrc.CreateCompatibleDC(NULL);
			hdcDst.CreateCompatibleDC(NULL);
			// Load the bitmaps into memory DC
			CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
			CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
			// Change the background to trans color
			hdcSrc.SetBkColor(clrTrans);
			// This call sets up the mask bitmap.
			hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
			// Now, we need to paint onto the original image, making
			// sure that the "transparent" area is set to black. What
			// we do is AND the monochrome image onto the color Image
			// first. When blitting from mono to color, the monochrome
			// pixel is first transformed as follows:
			// if  1 (black) it is mapped to the color set by SetTextColor().
			// if  0 (white) is is mapped to the color set by SetBkColor().
			// Only then is the raster operation performed.
			hdcSrc.SetTextColor(RGB(255,255,255));
			hdcSrc.SetBkColor(RGB(0,0,0));
			hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
			// Clean up by deselecting any objects, and delete the
			// DC's.
			hdcSrc.SelectObject(hbmSrcT);
			hdcDst.SelectObject(hbmDstT);
			hdcSrc.DeleteDC();
			hdcDst.DeleteDC();
		}

		void CChildView::TransparentBlt
			(
				CDC* pDC,
				int xStart,  int yStart,
				int wWidth,  int wHeight,
				CDC* pTmpDC,
				CBitmap* bmpMask,
				int xSource, // = 0
				int ySource // = 0
			)
		{
			// We are going to paint the two DDB's in sequence to the destination.
			// 1st the monochrome bitmap will be blitted using an AND operation to
			// cut a hole in the destination. The color image will then be ORed
			// with the destination, filling it into the hole, but leaving the
			// surrounding area untouched.
			CDC hdcMem;
			hdcMem.CreateCompatibleDC(pDC);
			CBitmap* hbmT = hdcMem.SelectObject(bmpMask);
			pDC->SetTextColor(RGB(0,0,0));
			pDC->SetBkColor(RGB(255,255,255));
			if (!pDC->BitBlt( xStart, yStart, wWidth, wHeight, &hdcMem, xSource, ySource, 
				SRCAND))
			{
				TRACE("Transparent Blit failure SRCAND");
			}
			// Also note the use of SRCPAINT rather than SRCCOPY.
			if (!pDC->BitBlt(xStart, yStart, wWidth, wHeight, pTmpDC, xSource, ySource,
				SRCPAINT))
			{
				TRACE("Transparent Blit failure SRCPAINT");
			}
			// Now, clean up.
			hdcMem.SelectObject(hbmT);
			hdcMem.DeleteDC();
		}

		void CChildView::patTrackMute()
		{
			if (viewMode == view_modes::pattern)
			{
				_pSong->_trackMuted[pattern_view()->editcur.track] = !_pSong->_trackMuted[pattern_view()->editcur.track];
				Repaint(draw_modes::track_header);
			}
		}

		void CChildView::patTrackSolo()
		{
			if (viewMode == view_modes::pattern)
			{
				if (_pSong->_trackSoloed == pattern_view()->editcur.track)
				{
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						_pSong->_trackMuted[i] = FALSE;
					}
					_pSong->_trackSoloed = -1;
				}
				else
				{
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						_pSong->_trackMuted[i] = TRUE;
					}
					_pSong->_trackMuted[pattern_view()->editcur.track] = FALSE;
					_pSong->_trackSoloed = pattern_view()->editcur.track;
				}
				Repaint(draw_modes::track_header);
			}
		}

		void CChildView::patTrackRecord()
		{
			if (viewMode == view_modes::pattern)
			{
				_pSong->_trackArmed[pattern_view()->editcur.track] = !_pSong->_trackArmed[pattern_view()->editcur.track];
				_pSong->_trackArmedCount = 0;
				for ( int i=0;i<MAX_TRACKS;i++ )
				{
					if (_pSong->_trackArmed[i])
					{
						_pSong->_trackArmedCount++;
					}
				}
				Repaint(draw_modes::track_header);
			}
		}
		
		void CChildView::OnConfigurationLoopplayback() 
		{
			Global::pPlayer->_loopSong = !Global::pPlayer->_loopSong;
		}

		void CChildView::OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI) 
		{
			if (Global::pPlayer->_loopSong)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);	
		}

		void CChildView::DrawAllMachineVumeters(CDC *devc)
		{
			if (Global::pConfig->draw_vus)
				machine_view_.UpdateVUs(devc);
		}


		void CChildView::LoadMachineDial()
		{
			CNativeGui::uiSetting().LoadMachineDial();
		}		

		void CChildView::AddMacViewUndo()
		{
			// i have not written the undo code yet for machine and instruments
			// however, for now it at least tracks changes for save/new/open/close warnings
			UndoMacCounter++;
			SetTitleBarText();
		}


	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

// graphics operations, private headers included only by this translation unit
#include "SeqView.private.hpp"

// User/Mouse Responses, private headers included only by this translation unit
#include "KeybHandler.private.hpp"
#include "MouseHandler.private.hpp"
