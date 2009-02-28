///\file
///\brief interface file for psycle::host::CChildView.
#pragma once

#include "Song.hpp"
#include "Configuration.hpp"
#include "MachineView.hpp"
#include "PatternView.hpp"
#include "MachineGui.hpp"
#include "mfc_namespace.hpp"


PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class CGearTracker;
		class XMSamplerUI;
		class CWaveInMacDlg;
	
		#define MAX_DRAW_MESSAGES 32

		struct draw_modes
		{
			enum draw_mode
			{
				all, ///< Repaints everything (means, slow). Used when switching views, or when a
				///< whole update is needed (For example, when changing pattern Properties, or TPB)

				all_machines, ///< Used to refresh all the machines, without refreshing the background/wires

				machine, ///< Used to refresh the image of one machine (mac num in "updatePar")

				pattern, ///< Use this when switching Patterns (changing from one to another)

				data, ///< Data has Changed. Which data to update is indicated with DrawLineStart/End
				///< and DrawTrackStart/End
				///< Use it when editing and copy/pasting

				horizontal_scroll, ///< Refresh called by the scrollbars or by mouse scrolling (when selecting).
				///< New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);

				vertical_scroll, ///< Refresh called by the scrollbars or by mouse scrolling (when selecting).
				///< New values in ntOff and nlOff variables ( new_track_offset and new_line_offset);

				//resize, ///< Indicates the Refresh is called from the "OnSize()" event.

				playback, ///< Indicates it needs a refresh caused by Playback (update playback cursor)

				playback_change, ///< Indicates that while playing, a pattern switch is needed.

				cursor, ///< Indicates a movement of the cursor. update the values to "editcur" directly
				///< and call this function.
				///< this is arbitrary message as cursor is checked

				selection, ///< The selection has changed. use "blockSel" to indicate the values.

				track_header, ///< Track header refresh (mute/solo, Record updating)

				//pattern_header, ///< Octave, Pattern name, Edit Mode on/off

				none ///< Do not use this one directly. It is used to detect refresh calls from the OS.

				// If you add any new method, please, add the proper code to "PreparePatternRefresh()" and to
				// "DrawPatternEditor()".
				// Note: Modes are sorted by priority. (although it is not really used)

				// !!!BIG ADVISE!!! : The execution of Repaint(mode) does not imply an instant refresh of
				//						the Screen, and what's worse, you might end calling Repaint(anothermode)
				//						previous of the first refresh. In PreparePatternRefresh() there's code
				//						to avoid problems when two modes do completely different things. On
				//						other cases, it still ends to wrong content being shown.
			};
		};

		struct view_modes
		{
			enum view_mode
			{
				machine,
				pattern,
				sequence
			};
		};
		
		/// child view window
		class CChildView : public CWnd
		{
		public:
			CChildView(CMainFrame* main_frame);
			virtual ~CChildView();

			void InitTimer();
			void ValidateParent();
			void EnableSound();
			void Repaint(draw_modes::draw_mode drawMode = draw_modes::all);					

			void MidiPatternNote(int outnote , int velocity);	// called by the MIDI input to insert pattern notes
			void MidiPatternCommand(int command, int value); // called by midi to insert pattern commands
			void MidiPatternTweak(int command, int value); // called by midi to insert pattern commands
			void MidiPatternTweakSlide(int command, int value); // called by midi to insert pattern commands
			void MidiPatternMidiCommand(int command, int value); // called by midi to insert midi pattern commands
			void MidiPatternInstrument(int value); // called by midi to insert pattern commands
			void MousePatternTweak(int machine, int command, int value);
			void MousePatternTweakSlide(int machine, int command, int value);													

			void PlayCurrentRow(void);
			void PlayCurrentNote(void);

			void patCopy();
			void patPaste();
			void patMixPaste();
			void patCut();
			void patDelete();
			void patTranspose(int trp);
			
			void AddMacViewUndo(); // place holder
			
			void SetTitleBarText();
			void LoadMachineSkin();			
			void LoadMachineDial();
			void patTrackMute();
			void patTrackSolo();
			void patTrackRecord();
			void KeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
			void KeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );			
			void FileLoadsongNamed(std::string fName);
			void OnFileLoadsongNamed(std::string fName, int fType);

			MachineView* machine_view() { return &machine_view_; }
			PatternView* pattern_view() { return &pattern_view_; }
			
		public:
			//RECENT!!!//
			HMENU hRecentMenu;

//			CBitmap machinedial; //the machine dial bitmap

			CFrameWnd* pParentFrame;
			Song* _pSong;
		//	bool multiPattern;			
			CGearTracker * SamplerMachineDialog;
			XMSamplerUI* XMSamplerMachineDialog;
			CWaveInMacDlg* WaveInMachineDialog;			

			int blockSelectBarState; //This is used to remember the state of the select bar function
			bool bScrollDetatch;

			int updateMode;
			int updatePar;			// view_modes::pattern: Display update mode. view_modes::machine: Machine number to update.
			int viewMode;
			int CH;
			int CW;
			bool _outputActive;	// This variable indicates if the output (audio or midi) is active or not.
								// Its function is to prevent audio (and midi) operations while it is not
								// initialized, or while song is being modified (New(),Load()..).
								// 

			SPatternHeaderCoords PatHeaderCoords;
			SMachineCoords	MachineCoords;

			int textLeftEdge;

			inline bool InRect(int _x,int _y,SSkinDest _src,SSkinSource _src2,int _offs=0);

		// Overrides
			protected:
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

		//////////////////////////////////////////////////////////////////////
		// Private operations

		private:

			//Recent Files!!!!//
			void AppendToRecent(std::string fName);
			void CallOpenRecent(int pos);
			//Recent Files!!!!//

			inline void TXT(CDC *devc,char const *txt, int x,int y,int w,int h);
			inline void TXTFLAT(CDC *devc,char const *txt, int x,int y,int w,int h);
			
			void DrawAllMachineVumeters(CDC *devc);													
			void FindMachineSkin(CString findDir, CString findName, BOOL *result);
			void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
			void TransparentBlt(CDC* pDC, int xStart,  int yStart, int wWidth,  int wHeight, CDC* pTmpDC, CBitmap* bmpMask, int xSource = 0, int ySource = 0);
			void DrawSeqEditor(CDC *devc);
				
		private:
			MachineView machine_view_;
			PatternView pattern_view_;

			// GDI Stuff		
			CBitmap machineskin;
			CBitmap machineskinmask;			
			HBITMAP hbmMachineSkin;			
			HBITMAP hbmMachineDial;

			CBitmap* bmpDC;
			int FLATSIZES[256];

			int bkgx;
			int bkgy;				
			
			int UndoMacCounter;
			int UndoMacSaved;

		public:
			
			void SelectMachineUnderCursor(void);
			BOOL CheckUnsavedSong(std::string szTitle);
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
			afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
			afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
			afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
			afx_msg void OnMouseMove( UINT nFlags, CPoint point );
			afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
			afx_msg void OnHelpPsycleenviromentinfo();
			afx_msg void OnMidiMonitorDlg();
			afx_msg void OnDestroy();
			afx_msg void OnAppExit();
			afx_msg void OnMachineview();
			afx_msg void OnPatternView();
			afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
			afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
			afx_msg void OnBarplay();
			afx_msg void OnBarplayFromStart();
			afx_msg void OnBarrec();
			afx_msg void OnBarstop();
			afx_msg void OnRecordWav();
			afx_msg void OnTimer( UINT nIDEvent );
			afx_msg void OnUpdateRecordWav(CCmdUI* pCmdUI);
			afx_msg void OnFileNew();
			afx_msg BOOL OnExport(UINT id);
			afx_msg BOOL OnFileSave(UINT id);
			afx_msg BOOL OnFileSaveAs(UINT id);
			afx_msg void OnFileLoadsong();
			afx_msg void OnFileRevert();
			afx_msg void OnHelpSaludos();
			afx_msg void OnUpdatePatternView(CCmdUI* pCmdUI);
			afx_msg void OnUpdateMachineview(CCmdUI* pCmdUI);
			afx_msg void OnUpdateBarplay(CCmdUI* pCmdUI);
			afx_msg void OnUpdateBarplayFromStart(CCmdUI* pCmdUI);
			afx_msg void OnUpdateBarrec(CCmdUI* pCmdUI);
			afx_msg void OnFileSongproperties();
			afx_msg void OnViewInstrumenteditor();
			afx_msg void OnNewmachine();
			afx_msg void OnButtonplayseqblock();
			afx_msg void OnUpdateButtonplayseqblock(CCmdUI* pCmdUI);
			afx_msg void OnPopCut();
			afx_msg void OnUpdateCutCopy(CCmdUI* pCmdUI);
			afx_msg void OnPopCopy();
			afx_msg void OnPopPaste();
			afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
			afx_msg void OnPopMixpaste();
			afx_msg void OnPopDelete();
			afx_msg void OnPopInterpolate();
			afx_msg void OnPopChangegenerator();
			afx_msg void OnPopChangeinstrument();
			afx_msg void OnPopTranspose1();
			afx_msg void OnPopTranspose12();
			afx_msg void OnPopTranspose_1();
			afx_msg void OnPopTranspose_12();
			afx_msg void OnAutostop();
			afx_msg void OnUpdateAutostop(CCmdUI* pCmdUI);
			afx_msg void OnPopTransformpattern();
			afx_msg void OnPopPattenproperties();
			afx_msg void OnPopBlockSwingfill();
			afx_msg void OnPopTrackSwingfill();
			afx_msg void OnSize(UINT nType, int cx, int cy);
			afx_msg void OnConfigurationSettings();
			afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
			afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
			afx_msg void OnFileImportModulefile();
			afx_msg void OnFileRecent_01();
			afx_msg void OnFileRecent_02();
			afx_msg void OnFileRecent_03();
			afx_msg void OnFileRecent_04();
			afx_msg void OnEditUndo();
			afx_msg void OnEditRedo();
			afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
			afx_msg void OnUpdateRedo(CCmdUI* pCmdUI);
			afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
			afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
			afx_msg void OnUpdatePatternCutCopy(CCmdUI* pCmdUI);
			afx_msg void OnUpdatePatternPaste(CCmdUI* pCmdUI);
			afx_msg void OnFileSaveaudio();
			afx_msg void OnHelpKeybtxt();
			afx_msg void OnHelpReadme();
			afx_msg void OnHelpTweaking();
			afx_msg void OnHelpWhatsnew();
			afx_msg void OnConfigurationLoopplayback();
			afx_msg void OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI);
			afx_msg void OnShowPatternSeq();
			afx_msg void OnUpdatePatternSeq(CCmdUI* pCmdUI);
			afx_msg void OnPopBlockswitch();
			afx_msg void OnUpdatePopBlockswitch(CCmdUI *pCmdUI);
			afx_msg void OnPopInterpolateCurve();
			DECLARE_MESSAGE_MAP()
};


		/////////////////////////////////////////////////////////////////////////////



		inline void CChildView::TXTFLAT(CDC *devc,char const *txt, int x,int y,int w,int h)
		{
			CRect Rect;
			Rect.left=x;
			Rect.top=y;
			Rect.right=x+w;
			Rect.bottom=y+h;
			devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,FLATSIZES);
		}

		inline void CChildView::TXT(CDC *devc,char const *txt, int x,int y,int w,int h)
		{
			CRect Rect;
			Rect.left=x;
			Rect.top=y;
			Rect.right=x+w;
			Rect.bottom=y+h;
			devc->ExtTextOut(x+textLeftEdge,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,NULL);
		}

			
		inline bool CChildView::InRect(int _x,int _y,SSkinDest _src,SSkinSource _src2,int _offs)
		{
			return (_x >= _offs+_src.x) && (_x < _offs+_src.x+_src2.width) && 
				(_y >= _src.y) && (_y < _src.y+_src2.height);
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
