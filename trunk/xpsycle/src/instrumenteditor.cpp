/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "instrumenteditor.h"
#include "global.h"
#include "song.h"
#include <nalignlayout.h>
#include <nflowlayout.h>
#include <nlabel.h>
#include <nbevelborder.h>
#include <nframeborder.h>
#include <nbutton.h>
#include <nedit.h>
#include <ngroupbox.h>
#include <ncombobox.h>
#include <nitem.h>
#include <nslider.h>
#include <ncheckbox.h>

InstrumentEditor::InstrumentEditor()
 : NWindow()
{
  init();
}


InstrumentEditor::~InstrumentEditor()
{
}

void InstrumentEditor::init( )
{
  setTitle("Instrument Editor");

  NPanel* header = new NPanel();
    header->setHeight(20);
    header->setLayout(new NFlowLayout(nAlLeft,5,5),true);
    header->add(new NLabel("Instrument"), nAlLeft);
    instNumberLbl = new NLabel("   ");
       instNumberLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
       instNumberLbl->border()->setSpacing(NSize(2,2,2,2));
    header->add(instNumberLbl,nAlLeft);
    decInstBtn = new NButton("<");
      decInstBtn->setFlat(false);
    header->add(decInstBtn,nAlLeft);
    incInstBtn = new NButton(">");
      incInstBtn->setFlat(false);
    header->add(incInstBtn,nAlLeft);
    instNameEd = new NEdit();
      instNameEd->setWidth(100);
      instNameEd->setHeight(instNameEd->preferredHeight());
    header->add(instNameEd,nAlLeft);
    NButton* killBtn = new NButton("Kill...");
      killBtn->setFlat(false);
    header->add(killBtn, nAlLeft);
  pane()->add(header, nAlTop);

  NGroupBox* properties = new NGroupBox();
     properties->setLayout(new NAlignLayout(),true);
     properties->setHeaderText("Instrument Properties");
     NPanel* noteActionPnl = new NPanel();
        noteActionPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        noteActionPnl->add(new NLabel("New Note Action"));
          newNoteActionCb = new NComboBox();
          newNoteActionCb->setWidth(100);
          newNoteActionCb->setHeight(newNoteActionCb->preferredHeight());
          newNoteActionCb->add(new NItem("Note Cut"));
          newNoteActionCb->add(new NItem("Note Release"));
          newNoteActionCb->add(new NItem("None"));
        noteActionPnl->add(newNoteActionCb,nAlTop);
     properties->add(noteActionPnl,nAlTop);
     NPanel* panningPnl = new NPanel();
        panningPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        panningSlider = new NSlider();
          panningSlider->setOrientation(nHorizontal);
          panningSlider->setWidth(150);
          panningSlider->setHeight(20);
          panningSlider->setRange(0,256);
        panningPnl->add(panningSlider,nAlLeft);
        panningLbl = new NLabel("   ");
           panningLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           panningLbl->border()->setSpacing(NSize(2,2,2,2));
        panningPnl->add(panningLbl,nAlLeft);
     properties->add(panningPnl,nAlTop);
     rndPanningCbx = new NCheckBox("Random panning");
     properties->add(rndPanningCbx,nAlTop);
     rndVCFCutCbx  = new NCheckBox("Random VCF Cutoff");
     properties->add(rndVCFCutCbx,nAlTop);
     rndVCFResoCbx = new NCheckBox("Random VCF Reso/Bw.");
     properties->add(rndVCFResoCbx,nAlTop);

     NGroupBox* tempoGrpBox = new NGroupBox();
        tempoGrpBox->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        tempoGrpBox->setHeaderText("Tempo Looping Tool");
        playSampleFitCbx = new NCheckBox("Play sample to fit");
        tempoGrpBox->add(playSampleFitCbx);
        patRowLbl = new NLabel("   ");
           patRowLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           patRowLbl->border()->setSpacing(NSize(2,2,2,2));
        tempoGrpBox->add(patRowLbl,nAlLeft);
        tempoGrpBox->add(new NLabel("Pattern rows"),nAlLeft);
     properties->add(tempoGrpBox,nAlTop);

     NGroupBox* waveLayerGrpBox = new NGroupBox();
        waveLayerGrpBox->setLayout(new NAlignLayout(5,5),true);
        waveLayerGrpBox->setHeaderText("Instrument Wave Layer");
        NPanel* volumePnl = new NPanel();
        volumePnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        volumePnl->add(new NLabel("Volume"),nAlLeft);
        volumeSlider = new NSlider();
          volumeSlider->setOrientation(nHorizontal);
          volumeSlider->setWidth(150);
          volumeSlider->setHeight(20);
          volumeSlider->setRange(0,512);
        volumePnl->add(volumeSlider,nAlLeft);
        volumeLbl = new NLabel("   ");
           volumeLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           volumeLbl->border()->setSpacing(NSize(2,2,2,2));
        volumePnl->add(volumeLbl,nAlLeft);
        waveLayerGrpBox->add(volumePnl,nAlTop);
        NPanel* fineTunePnl = new NPanel();
        fineTunePnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
        fineTunePnl->add(new NLabel("Finetune"),nAlLeft);
        fineTuneSlider = new NSlider();
          fineTuneSlider->setOrientation(nHorizontal);
          fineTuneSlider->setWidth(150);
          fineTuneSlider->setHeight(20);
          fineTuneSlider->setRange(0,256);
        fineTunePnl->add(fineTuneSlider,nAlLeft);
        fineTuneLbl = new NLabel("   ");
           fineTuneLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
           fineTuneLbl->border()->setSpacing(NSize(2,2,2,2));
        fineTunePnl->add(fineTuneLbl,nAlLeft);
        waveLayerGrpBox->add(fineTunePnl,nAlTop);
        NPanel* tunePnl = new NPanel();
           tunePnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
           tunePnl->add(new NLabel("Tune"));
           octDecBtn = new NButton("Oct-");
             octDecBtn->setFlat(false);
           tunePnl->add(octDecBtn);
           noteDecBtn = new NButton("Note-");
             noteDecBtn->setFlat(false);
           tunePnl->add(noteDecBtn);
           noteIncBtn = new NButton("Note+");
             noteIncBtn->setFlat(false);
           tunePnl->add(noteIncBtn);
           octIncBtn  = new NButton("Oct+");
             octIncBtn->setFlat(false);
           tunePnl->add(octIncBtn);
           octLbl   = new NLabel("  ");
             octLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             octLbl->border()->setSpacing(NSize(2,2,2,2));
           tunePnl->add(octLbl);
        waveLayerGrpBox->add(tunePnl,nAlTop);
        NPanel* loopPnl = new NPanel();
           loopPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
           loopPnl->add(new NLabel("Loop"));
           NButton* offBtn = new NButton("Off");
             offBtn->setFlat(false);
           loopPnl->add(offBtn);
           NButton* forwardBtn = new NButton("Forward");
             forwardBtn->setFlat(false);
           loopPnl->add(forwardBtn);
           loopLbl   = new NLabel("  ");
             loopLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             loopLbl->border()->setSpacing(NSize(2,2,2,2));
           loopPnl->add(loopLbl);
        waveLayerGrpBox->add(loopPnl,nAlTop);
        NPanel* loopAtPnl = new NPanel();
           loopAtPnl->setLayout(new NFlowLayout(nAlLeft,5,5),true);
           loopAtPnl->add(new NLabel("Loop At"),nAlLeft);
           loopAtFromLbl = new NLabel("  ");
             loopAtFromLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             loopAtFromLbl->border()->setSpacing(NSize(2,2,2,2));
           loopAtPnl->add(loopAtFromLbl,nAlLeft);
           loopAtPnl->add(new NLabel("to"),nAlLeft);
           loopAtToLbl   = new NLabel("  ");
             loopAtToLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             loopAtToLbl->border()->setSpacing(NSize(2,2,2,2));
           loopAtPnl->add(loopAtToLbl,nAlLeft);
           loopAtPnl->add(new NLabel("Length"),nAlLeft);
           lenLbl   = new NLabel("  ");
             lenLbl->setBorder(new NBevelBorder(nNone,nLowered),true);
             lenLbl->border()->setSpacing(NSize(2,2,2,2));
           loopAtPnl->add(lenLbl,nAlLeft);
        waveLayerGrpBox->add(loopAtPnl,nAlTop);
        NButton* amplitudeBtn = new NButton("Amplitudes/Filter Envelopes");
          amplitudeBtn->setFlat(false);
        waveLayerGrpBox->add(amplitudeBtn,nAlTop);
     properties->add(waveLayerGrpBox,nAlClient);

  pane()->add(properties,nAlClient);

  setPosition(0,0,400,600);
}

int InstrumentEditor::onClose( )
{
  setVisible(false);
  return nHideWindow;
}

void InstrumentEditor::setInstrument( int index )
{
  Global::pSong()->instSelected=   index;
  Global::pSong()->auxcolSelected= index;

  char buf[64]; sprintf(buf,"%.2X",index);
  instNumberLbl->setText(buf);

  instNameEd->setText(Global::pSong()->_pInstrument[index]->_sName);

  newNoteActionCb->setIndex(Global::pSong()->_pInstrument[index]->_NNA);

  // Volume bar
  volumeSlider->setPos(Global::pSong()->_pInstrument[index]->waveVolume);
  fineTuneSlider->setPos(Global::pSong()->_pInstrument[index]->waveFinetune+256);

  panningSlider->setPos(Global::pSong()->_pInstrument[index]->_pan);
  rndPanningCbx->setCheck(Global::pSong()->_pInstrument[index]->_RPAN);
  rndVCFCutCbx->setCheck(Global::pSong()->_pInstrument[index]->_RCUT);
  rndVCFResoCbx->setCheck(Global::pSong()->_pInstrument[index]->_RRES);

  panningLbl->setText(stringify(Global::pSong()->_pInstrument[index]->_pan));

  octLbl->setText(noteToString((Global::pSong()->_pInstrument[index]->waveTune+48)));

  bool const ils = Global::pSong()->_pInstrument[index]->_loop;

  playSampleFitCbx->setCheck(ils);
  patRowLbl->setText(stringify(Global::pSong()->_pInstrument[index]->_lines));

  // Set looptype
  if(Global::pSong()->_pInstrument[index]->waveLoopType)
     loopLbl->setText("Forward");
  else
     loopLbl->setText("Off");

  // Display Loop Points & Wave Length	

  loopAtFromLbl->setText(stringify((int)Global::pSong()->_pInstrument[index]->waveLoopStart));
  loopAtToLbl->setText(stringify((int)Global::pSong()->_pInstrument[index]->waveLoopEnd));
  lenLbl->setText(stringify((int)Global::pSong()->_pInstrument[index]->waveLength));

  fineTuneLbl->setText(stringify(Global::pSong()->_pInstrument[index]->waveFinetune));
  volumeLbl->setText(stringify(Global::pSong()->_pInstrument[index]->waveVolume)+"%");

  pane()->resize();

}

std::string InstrumentEditor::noteToString( int value )
{
  int octave = value / 12;

  switch (value % 12) {
     case 0:   return "C-" + stringify(octave); break;
     case 1:   return "C#" + stringify(octave); break;
     case 2:   return "D-" + stringify(octave); break;
     case 3:   return "D#" + stringify(octave); break;
     case 4:   return "E-" + stringify(octave); break;
     case 5:   return "F-" + stringify(octave); break;
     case 6:   return "F#" + stringify(octave); break;
     case 7:   return "G-" + stringify(octave); break;
     case 8:   return "G#" + stringify(octave); break;
     case 9:   return "A-" + stringify(octave); break;
     case 10:  return "A#" + stringify(octave); break;
     case 11:  return "B-" + stringify(octave); break;
  }
  return "err";
}
