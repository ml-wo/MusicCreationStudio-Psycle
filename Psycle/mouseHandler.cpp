void CChildView::OnRButtonDown( UINT nFlags, CPoint point )
{	
	if (viewMode == VMMachine)
	{
		// Check for right pressed connection
		int propMac = GetMachine(point);
		
		if (propMac > 0)
		{
			// Shows machine properties dialog
			CMacProp dlg;
			dlg.m_view=this;
			dlg.pMachine = Global::_pSong->_pMachines[propMac];
			dlg.pSong = Global::_pSong;
			dlg.thisMac = propMac;
			
			if (dlg.DoModal() == IDOK)
			{
				sprintf(dlg.pMachine->_editName, dlg.txt);
				pParentMain->StatusBarText(dlg.txt);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
			}
			if (dlg.deleted)
			{
				pParentMain->CloseMacGui(propMac);
				Global::_pSong->DestroyMachine(propMac);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
			}
//			Repaint();
		}
	}
	/*
	else if (viewMode == VMPattern)
	{
		editcur.track = tOff + (point.x-XOFFSET)/ROWWIDTH;
		if ( editcur.track >= _pSong->SONGTRACKS ) editcur.track = _pSong->SONGTRACKS-1;
		else if ( editcur.track < 0 ) editcur.track = 0;

		int plines = _pSong->patternLines[_ps()];
		editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
		if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
		else if ( editcur.line < 0 ) editcur.line = 0;

		editcur.col=_xtoCol((point.x-XOFFSET)%ROWWIDTH);
	}
*/
}

void CChildView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (viewMode == VMPattern)
	{
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_POPUPMENU));
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
		
		menu.DestroyMenu();
//		Repaint(DMCursorMove);
	}
}


void CChildView::OnLButtonDown( UINT nFlags, CPoint point )
{
	
	SetCapture();

	int c;
	
	switch(viewMode)
	{
		
	case VMMachine: // User is in machine view mode
		if (Global::_pSong->_machineLock)
		{
			return;
		}
		smac = -1;
		smacmode = 0;
		
		wiresource = -1;
		wiredest = -1;

		if ( nFlags & MK_CONTROL)
		{
			smac=GetMachine(point);
			_pSong->seqBus = FindBusFromIndex(smac);
			pParentMain->UpdateComboGen();
		}
		else if (nFlags & MK_SHIFT)
		{
			wiresource = GetMachine(point);
			if (wiresource > 0)
			{
				wireSX = Global::_pSong->_pMachines[wiresource]->_x+74;
				wireSY = Global::_pSong->_pMachines[wiresource]->_y+24;
				OnMouseMove(nFlags,point);
			}
			else wiresource = -1; // wiresource=0 -> Master.
		}// Shift
		
		else if (nFlags == MK_LBUTTON)
		{
			smac=GetMachine(point);

			if ( smac != -1 )
			{
				mcd_x = point.x - Global::_pSong->_pMachines[smac]->_x;
				mcd_y = point.y - Global::_pSong->_pMachines[smac]->_y;
				if ((mcd_y > 34) && (mcd_y < 46)) //changing panning
				{
					smacmode = 1;
					OnMouseMove(nFlags,point);
				}
				else if ((mcd_x > 136) && ( mcd_x < 145) && (Global::_pSong->_pMachines[smac]->_mode != MACHMODE_MASTER))
				{
					if ((mcd_y> 3) && (mcd_y < 12)) //Mute 
					{
						Global::_pSong->_pMachines[smac]->_mute = !Global::_pSong->_pMachines[smac]->_mute;
						Global::_pSong->_pMachines[smac]->_volumeCounter=0;
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_y> 14) && (mcd_y < 27) &&  //Bypass
						(Global::_pSong->_pMachines[smac]->_mode != MACHMODE_GENERATOR))
					{
						Global::_pSong->_pMachines[smac]->_bypass = !Global::_pSong->_pMachines[smac]->_bypass;
						Global::_pSong->_pMachines[smac]->_volumeCounter = 0;
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_y> 14) && (mcd_y < 23) &&  //Solo
						(Global::_pSong->_pMachines[smac]->_mode == MACHMODE_GENERATOR))
					{
						if (Global::_pSong->machineSoloed == smac )
						{
							Global::_pSong->machineSoloed = 0;
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if (( Global::_pSong->_machineActive[i] ) && 
								    ( Global::_pSong->_pMachines[i]->_mode == MACHMODE_GENERATOR ))
										Global::_pSong->_pMachines[i]->_mute = false;
							}
						}
						else 
						{
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if (( Global::_pSong->_machineActive[i] ) && 
								    ( Global::_pSong->_pMachines[i]->_mode == MACHMODE_GENERATOR ))
								{
									Global::_pSong->_pMachines[i]->_mute = true;
									Global::_pSong->_pMachines[i]->_volumeCounter=0;
								}
							}
							Global::_pSong->_pMachines[smac]->_mute = false;
							Global::_pSong->machineSoloed = smac;
						}
						updatePar = smac;
						Repaint(DMAllMacsRefresh);
					}
				}
			}
			else
			{
				// Check for pressed connection
				
				for (c=0; c<MAX_MACHINES; c++)
				{
					if (Global::_pSong->_machineActive[c])
					{
						Machine *tmac = Global::_pSong->_pMachines[c];
						
						for (int w = 0; w<MAX_CONNECTIONS; w++)
						{
							if (tmac->_connection[w])
							{
								int xt = tmac->_connectionPoint[w].x;
								int yt = tmac->_connectionPoint[w].y;
								
								if ((point.x > xt) && (point.x < xt+20) && (point.y > yt) && (point.y < yt+20))
								{
									CWireDlg dlg;
									dlg.wireIndex = w;
									dlg._pSong = Global::_pSong;
									dlg.isrcMac = c;
									dlg._pSrcMachine = tmac;
									sprintf(dlg.destName,"%s", Global::_pSong->_pMachines[tmac->_outputMachines[w]]->_editName);
									dlg.DoModal();
//									Repaint();
								}
							}
						}
					}
				}
			}
		}// No Shift
		
		

		break;
		
	case VMPattern:
		
		int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
		if ( ttm >= _pSong->SONGTRACKS ) ttm = _pSong->SONGTRACKS-1;
		else if ( ttm < 0 ) ttm = 0;
		
		if (point.y >= 19 && point.y <= 37 ) // Mouse is in Track Header.
		{	
			int pointpos= (point.x-XOFFSET)%ROWWIDTH;

			if ( pointpos > 55-17 && pointpos < 79-17 ) {
				_pSong->_trackArmed[ttm] = !_pSong->_trackArmed[ttm];
				_pSong->_trackArmedCount = 0;
				for ( int i=0;i<MAX_TRACKS;i++ )
				{
					if (_pSong->_trackArmed[i])
					{
						_pSong->_trackArmedCount++;
					}
				}
			}
			else if ( pointpos > 55+6 && pointpos < 79+6 ) {
				_pSong->_trackMuted[ttm] = !_pSong->_trackMuted[ttm];
			}
			else if ( pointpos > 85 && pointpos < 108 ) {
				if (Global::_pSong->_trackSoloed != ttm )
				{
					for ( int i=0;i<MAX_TRACKS;i++ )
					{
						_pSong->_trackMuted[i] = true;
					}
					_pSong->_trackMuted[ttm] = false;
					_pSong->_trackSoloed = ttm;
				}
				else
				{
					for ( int i=0;i<MAX_TRACKS;i++ )
					{
						_pSong->_trackMuted[i] = false;
					}
					_pSong->_trackSoloed = -1;
				}
			}
			oldm.track = -1;
			Repaint(DMTrackHeader);
			break;
		}
		else if ( point.y >= YOFFSET )
		{
			oldm.track=ttm;

			int plines = _pSong->patternLines[_ps()];
			oldm.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
			if ( oldm.line >= plines ) { oldm.line = plines - 1; }
			else if ( oldm.line < 0 ) oldm.line = 0;

			oldm.col=_xtoCol((point.x-XOFFSET)%ROWWIDTH);

			blockSelected=false;
			blockSel.end.line=0;
			blockSel.end.track=0;
			Repaint(DMSelection);

		}

	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

void CChildView::OnLButtonUp( UINT nFlags, CPoint point )
{
	ReleaseCapture();
	
	switch (viewMode)
	{
	case VMMachine:
		
		if (wiresource != -1)
		{
			wiredest = GetMachine(point);
			if ((wiredest !=-1) && (wiredest != wiresource))
			{
				if (!Global::_pSong->InsertConnection(wiresource, wiredest))
				{
					MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
				}
			}
			wiresource = -1;
			Repaint();
		}
		else if ( smacmode == 0 && smac != -1 )
		{
			if (point.x-mcd_x < 0 ) { _pSong->_pMachines[smac]->_x = 0; Repaint(); }
			else if (point.x-mcd_x+148 > CW) { _pSong->_pMachines[smac]->_x = CW-148; Repaint(); }
			if (point.y-mcd_y < 0 ) { _pSong->_pMachines[smac]->_y = 0; Repaint(); }
			else if (point.y-mcd_y+48 > CH) { _pSong->_pMachines[smac]->_y = CH-48; Repaint(); }
		}
		smac = -1;
		smacmode = 0;
		
		wiresource = -1;
		wiredest = -1;
		break;

	case VMPattern:
		if ( ( !blockSelected )
			&& ( point.y > YOFFSET && point.y < YOFFSET+(maxl*ROWHEIGHT))
			&& (point.x > XOFFSET && point.x < XOFFSET+(maxt*ROWWIDTH)))
		{
			editcur.track = tOff + (point.x-XOFFSET)/ROWWIDTH;
//			if ( editcur.track >= _pSong->SONGTRACKS ) editcur.track = _pSong->SONGTRACKS-1;
//			else if ( editcur.track < 0 ) editcur.track = 0;

//			int plines = _pSong->patternLines[_ps()];
			editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
//			if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
//			else if ( editcur.line < 0 ) editcur.line = 0;

			editcur.col = _xtoCol((point.x-XOFFSET)%ROWWIDTH);
			Repaint(DMCursorMove);
		}
		break;
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

void CChildView::OnMouseMove( UINT nFlags, CPoint point )
{
	switch (viewMode)
	{
	case VMMachine:
		if (smac > -1 && (nFlags == MK_LBUTTON))
		{
			if (smacmode == 0)
			{
				_pSong->_pMachines[smac]->_x = point.x-mcd_x;
				_pSong->_pMachines[smac]->_y = point.y-mcd_y;
	
				char buffer[64];
				sprintf(buffer, "%s (%d,%d)", Global::_pSong->_pMachines[smac]->_editName, Global::_pSong->_pMachines[smac]->_x, Global::_pSong->_pMachines[smac]->_y);
				pParentMain->StatusBarText(buffer);
				Repaint();
			}
			else if ((smacmode == 1) && (Global::_pSong->_pMachines[smac]->_mode != MACHMODE_MASTER))
			{
				char buffer[64];
				int newpan = point.x - Global::_pSong->_pMachines[smac]->_x - 9;
				Global::_pSong->_pMachines[smac]->SetPan(newpan);
				newpan= Global::_pSong->_pMachines[smac]->_panning;
				
				if (newpan != 64)
				{
					sprintf(buffer, "%s pan: %.0f%% Left / %.0f%% Right", Global::_pSong->_pMachines[smac]->_editName, 100.0f - ((float)newpan*0.78125f), (float)newpan*0.78125f);
				}
				else
				{
					sprintf(buffer, "%s pan: Center", Global::_pSong->_pMachines[smac]->_editName);
				}
				
				pParentMain->StatusBarText(buffer);
				updatePar = smac;
				Repaint(DMMacRefresh);
			}
		}
		
		if ((nFlags == (MK_SHIFT | MK_LBUTTON)) && (wiresource != -1))
		{
			wireDX = point.x;
			wireDY = point.y;
			Repaint();
		}
		break;

	case VMPattern:

		if ( nFlags == MK_LBUTTON && oldm.track != -1)
		{
			ntOff = tOff;
			nlOff = lOff;

			int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
			if ( point.x < XOFFSET ) ttm--; // 1/2 = 0 , -1/2 = 0 too!
			int ccm;
			if ( ttm < tOff ) // Exceeded from left
			{
				ccm=0;
				if ( ttm < 0 ) { ttm = 0; } // Out of Range
				// and Scroll
				ntOff = ttm;
				if (ntOff != tOff) Repaint(DMScroll);
			}
			else if ( ttm - tOff >= VISTRACKS ) // Exceeded from right
			{
				ccm=8;
				if ( ttm >= _pSong->SONGTRACKS ) // Out of Range
				{	ttm = _pSong->SONGTRACKS-1;
					if ( tOff != ttm-VISTRACKS ) { ntOff = ttm-VISTRACKS+1; Repaint(DMScroll); }
				}
				else	//scroll
				{	ntOff = ttm-VISTRACKS+1;
					if ( ntOff != tOff ) Repaint(DMScroll);
				}
			}
			else // Not exceeded
			{
				ccm=_xtoCol((point.x-XOFFSET)%ROWWIDTH);
			}

			int plines = _pSong->patternLines[_ps()];
			int llm = lOff + (point.y-YOFFSET)/ROWHEIGHT;
			if ( point.y < YOFFSET ) llm--; // 1/2 = 0 , -1/2 = 0 too!

			if ( llm < lOff ) // Exceeded from top
			{
				if ( llm < 0 ) // Out of range
				{	llm = 0;
					if ( lOff != 0 ) { nlOff = 0; Repaint(DMScroll); }
				}
				else	//scroll
				{	nlOff = llm;
					if ( nlOff != lOff ) Repaint(DMScroll);
				}
			}
			else if ( llm - lOff >= VISLINES ) // Exceeded from bottom
			{
				if ( llm >= plines ) //Out of Range
				{	llm = plines-1;
					if ( lOff != llm-VISLINES) { nlOff = llm-VISLINES+1; Repaint(DMScroll); }
				}
				else	//scroll
				{	nlOff = llm-VISLINES+1;
					if ( nlOff != lOff ) Repaint(DMScroll);
				}
			}
			
			else if ( llm >= plines ) { llm = plines-1; } //Out of Range

			if ((ttm != oldm.track ) || (llm != oldm.line) || (ccm != oldm.col))
			{
				if (!blockSelected) StartBlock(oldm.track,oldm.line,oldm.col);
				ChangeBlock(ttm,llm,ccm);
				oldm.track=ttm;
				oldm.line=llm;
				oldm.col=ccm;
			}
		}
		else if (nFlags == MK_MBUTTON)
		{
			// scrolling
			if (abs(point.y - MBStart.y) > 8)
			{
				int delta = (point.y - MBStart.y)/8;
				int nPos = lOff - delta;
				if (nPos < 0)
					nPos = 0;
				if (nPos > lOff )
				{
					nlOff=nPos;
					AdvanceLine(nPos-lOff,false,false); 
					Repaint(DMScroll);
				}
				else if (nPos < lOff )
				{
					nlOff=nPos;
					PrevLine(lOff-nPos,false,false);
					Repaint(DMScroll);
				}
				MBStart.y += delta*8;
			}
			// switching tracks
			if (abs(point.x - MBStart.x) > (8*11))
			{
				int delta = (point.x - MBStart.x)/(8*11);
				int nPos = tOff - delta;
				if (nPos < 0)
					nPos = 0;
				if (nPos > tOff)
				{
					ntOff=nPos;
					AdvanceTrack(nPos-tOff,false,false);
					Repaint(DMScroll);
				}
				else if (nPos < tOff)
				{
					ntOff=nPos;
					PrevTrack(tOff-nPos,false,false);
					Repaint(DMScroll);
				}
				MBStart.x += delta*8*11;
			}

			/*
			dx = point.x - MBStart.x;
			dy = point.y - MBStart.y;

			if ((point.y - MBStart.y) > 8)
			{
				int nPos = lOff - (zDelta/30);
				if (nPos < 0)
					nPos = 0;
				if (nPos > lOff )
				{
					nlOff=nPos;
					AdvanceLine(nPos-lOff,false,false); 
					Repaint(DMScroll);
				}
				else if (nPos < lOff )
				{
					nlOff=nPos;
					PrevLine(lOff-nPos,false,false);
					Repaint(DMScroll);
				}
			}
			*/

		}
		break;
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

//////////////////////////////////////////////////////////////////////
// Double Click Handler

void CChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	int tmac=-1;
	
	switch (viewMode)
	{
		case VMMachine: // User is in machine view mode
		
			tmac = GetMachine(point);

			if(tmac!=-1)
			{
				if ((mcd_x > 136) && ( mcd_x < 145) && (Global::_pSong->_pMachines[tmac]->_mode != MACHMODE_MASTER))
				{
					if ((mcd_y> 3) && (mcd_y < 12)) //Mute 
					{
						Global::_pSong->_pMachines[tmac]->_mute = !Global::_pSong->_pMachines[tmac]->_mute;
						Global::_pSong->_pMachines[tmac]->_volumeCounter=0;
						updatePar = tmac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_y> 14) && (mcd_y < 27) &&  //Bypass
						(Global::_pSong->_pMachines[tmac]->_mode != MACHMODE_GENERATOR))
					{
						Global::_pSong->_pMachines[tmac]->_bypass = !Global::_pSong->_pMachines[tmac]->_bypass;
						Global::_pSong->_pMachines[tmac]->_volumeCounter = 0;
						updatePar = tmac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_y> 14) && (mcd_y < 23) &&  //Solo
						(Global::_pSong->_pMachines[tmac]->_mode == MACHMODE_GENERATOR))
					{
						if (Global::_pSong->machineSoloed == tmac )
						{
							Global::_pSong->machineSoloed = 0;
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if (( Global::_pSong->_machineActive[i] ) && 
								    ( Global::_pSong->_pMachines[i]->_mode == MACHMODE_GENERATOR ))
										Global::_pSong->_pMachines[i]->_mute = false;
							}
						}
						else 
						{
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if (( Global::_pSong->_machineActive[i] ) && 
								    ( Global::_pSong->_pMachines[i]->_mode == MACHMODE_GENERATOR ))
								{
									Global::_pSong->_pMachines[i]->_mute = true;
									Global::_pSong->_pMachines[i]->_volumeCounter=0;
								}
							}
							Global::_pSong->_pMachines[tmac]->_mute = false;
							Global::_pSong->machineSoloed = tmac;
						}
						updatePar = tmac;
						Repaint(DMAllMacsRefresh);
					}
				}
				else
				{
					pParentMain->ShowMachineGui(tmac);
//					Repaint();
				}
			}

			else
			{
				// Show new machine dialog
				CNewMachine dlg;
				
				if (dlg.DoModal() == IDOK)
				{
					// Stop driver to handle possible conflicts
					// between threads.
					_outputActive = false;
					Global::pConfig->_pOutputDriver->Enable(false);
					// MIDI IMPLEMENTATION
					Global::pConfig->_pMidiInput->Close();
					
					int fb;
					if (dlg.OutBus) fb = Global::_pSong->GetFreeBus();
					else fb = Global::_pSong->GetFreeFxBus();

					if ( fb == -1 || !Global::_pSong->CreateMachine((MachineType)dlg.Outputmachine, point.x-74,point.y-24, dlg.psOutputDll))
					{
						MessageBox("Machine Creation Failed","Error!",MB_OK);
					}
					else
					{
						if ( dlg.OutBus)
						{
							Global::_pSong->seqBus = fb;
							Global::_pSong->busMachine[fb] = Global::_lbc;
						}
						else
						{
							Global::_pSong->busEffect[fb] = Global::_lbc;
						}
						pParentMain->UpdateComboGen();
					}

					// Restarting the driver...
					pParentMain->UpdateEnvInfo();
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
				}
//				Repaint();
			}
		
			break;

	} // <-- End switch(viewMode)
}

int CChildView::FindBusFromIndex(int smac)
{
	int i; 	// Code to Update the Combobox when clicking on a machine.
	for (i=0;i<MAX_BUSES;i++)
	{
		if (_pSong->busMachine[i] == smac)
		{
			return i;
		}
	}
	for (i=0;i<MAX_BUSES;i++)
	{
		if (_pSong->busEffect[i] == smac)
		{
			return i+MAX_BUSES;
		}
	}
	return 255;
}



BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if ( viewMode == VMPattern )
	{
		int nPos = lOff - (zDelta/30);
		if (nPos < 0)
			nPos = 0;
		if (nPos > lOff )
		{
			nlOff=nPos;
			AdvanceLine(nPos-lOff,false,false); 
			Repaint(DMScroll);
		}
		else if (nPos < lOff )
		{
			nlOff=nPos;
			PrevLine(lOff-nPos,false,false);
			Repaint(DMScroll);
		}
	}
	return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnMButtonDown( UINT nFlags, CPoint point )
{
	MBStart.x = point.x;
	MBStart.y = point.y;
	CWnd ::OnMButtonDown(nFlags, point);
}
