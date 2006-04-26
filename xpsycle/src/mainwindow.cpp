/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
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
#include "mainwindow.h"
#include "configuration.h"
#include "song.h"
#include "player.h"
#include "defaultbitmaps.h"
#include "greetdlg.h"
#include "aboutdlg.h"
#include "vumeter.h"
#include "instrumenteditor.h"
#include <napp.h>
#include <nitem.h>
#include <ncheckmenuitem.h>
#include <nmessagebox.h>
#include <nbevelborder.h>


MainWindow::MainWindow()
 : NWindow()
{
  setPosition(0,0,1024,768);
  pane()->setName("pane");

  initMenu();
  initDialogs();
  childView_ = new ChildView();
  initBars();
  initViews();
  initSignals();

  childView_->timer.timerEvent.connect(this,&MainWindow::onTimer);

  greetDlg =  new GreetDlg();
    add(greetDlg);
  aboutDlg =  new AboutDlg();
    add(aboutDlg);
  wavRecFileDlg = new NFileDialog();
    wavRecFileDlg->setMode(nSave);
  add(wavRecFileDlg);

}


MainWindow::~MainWindow()
{
}

void MainWindow::initMenu( )
{
   menuBar_ = new NMenuBar();
   pane()->add(menuBar_);


   fileMenu_ = new NMenu("File",'i'
      ,"New,Open,Import Module,Save,Save as,Render as Wav,|,Song properties,|,revert to Saved,recent Files,Exit");
   menuBar_->add(fileMenu_);

   fileMenu_->itemByName("New")->click.connect(this,&MainWindow::onFileNew);
   fileMenu_->itemByName("Open")->click.connect(this,&MainWindow::onFileOpen);
   fileMenu_->itemByName("Save as")->click.connect(this,&MainWindow::onFileSaveAs);

   recentFileMenu_ = new NMenu("recent Files");
   fileMenu_->itemByName("recent Files")->add(recentFileMenu_);

   editMenu_ = new NMenu("Edit",'e',
       "Undo,Redo,Pattern Cut,Pattern Copy,Pattern Paste,Pattern Mix Paster,Pattern Delete,|,Block Cut,Block Copy,Block Paste,Block Mix Paste,Block Delete,|,Sequence Cut,Sequence Copy,Sequence Delete");
       editMenu_->itemClicked.connect(this, &MainWindow::onEditMenuItemClicked);
   menuBar_->add(editMenu_);

   viewMenu_ = new NMenu("View",'v',
       "&&Toolbar,&&MachineBar,&&SequencerBar,&&StatusBar,|,MachineView,PatternEditor,PatternSequencer,|,Add machine,Instrument Editor");
   menuBar_->add(viewMenu_);

   configurationMenu_ = new NMenu("Configuration",'c',
       "Free Audio,AutoStop,|,Loop Playback,|,Settings");
   menuBar_->add(configurationMenu_);

   performanceMenu_ = new NMenu("Performance",'p',"CPU Monitor ...,Midi Monitor ...");
   menuBar_->add(performanceMenu_);

   helpMenu_ = new NMenu("Help",'h',
       "Help,|,./doc/readme.txt,./doc/tweaking.txt,./doc/keys.txt,./doc/tweaking.txt,./doc/whatsnew.txt,|,About,Greetings");
   menuBar_->add(helpMenu_);
}

void MainWindow::initDialogs( )
{
  add( songpDlg_ = new SongpDlg() );
  add( instrumentEditor = new InstrumentEditor() );
}

// events from menuItems

void MainWindow::showSongpDlg( NObject * sender )
{
  songpDlg_->setVisible(true);
}

void MainWindow::initViews( )
{
  pane()->add(childView_);
  childView_->setTitleBarText();
  sequencerBar_->setPatternView(childView_->patternView());
}

void MainWindow::initBars( )
{
  toolBarPanel_ = new NPanel();
    toolBarPanel_->setLayout(new NFlowLayout(nAlLeft,0,2), true);
    toolBarPanel_->setWidth(500);
  pane()->add(toolBarPanel_, nAlTop);

  initToolBar();

  statusBar_ = new NPanel();
    statusBar_->setLayout(new NFlowLayout(nAlLeft),true);
      progressBar_ = new NProgressBar();
        progressBar_->setValue(0);
        progressBar_->setWidth(200);
        progressBar_->setHeight(25);
        progressBar_->setVisible(false);
        Global::pSong()->loadProgress.connect(this,&MainWindow::onSongLoadProgress);
    statusBar_->add(progressBar_);
  pane()->add(statusBar_,nAlBottom);

  pane()->add(sequencerBar_ = new SequencerBar(), nAlLeft);

  updateComboIns(true);
  insCombo_->setIndex(0);
  octaveCombo_->setIndex(4);
  childView_->patternView()->setEditOctave(4);
  trackCombo_->setIndex(12);  // starts at 4 .. so 16 - 4 = 12 ^= 16*/
}

void MainWindow::initToolBar( )
{
  toolBar1_ = new NToolBar();

  toolBarPanel_->add(toolBar1_);
    NImage* img;
    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->newfile()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "new.xpm");
    img->setPreferredSize(25,25);
    NButton* newBtn = new NButton(img);
     newBtn->setHint("New song");
    toolBar1_->add(newBtn)->clicked.connect(this,&MainWindow::onFileNew);;

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->open()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "open.xpm");
    img->setPreferredSize(25,25);
    NButton* fileOpenBtn = new NButton(img);
      fileOpenBtn->setHint("Song load");
    toolBar1_->add(fileOpenBtn)->clicked.connect(this,&MainWindow::onFileOpen);;

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->save()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "save.xpm");
    img->setPreferredSize(25,25);
    NButton* saveBtn = new NButton(img);
      saveBtn->setHint("Save");
    toolBar1_->add(saveBtn)->clicked.connect(this,&MainWindow::onFileSaveAs);;

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->save_audio()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "saveaudio.xpm");
    img->setPreferredSize(25,25);
    NButton* saveAsAudioFileBtn = new NButton(img);
       saveAsAudioFileBtn->setHint("Save as audio file");
    toolBar1_->add(saveAsAudioFileBtn);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->recordwav()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "recordwav.xpm");
    img->setPreferredSize(25,25);
    NButton* recWav = new NButton(img);
       recWav->setToggle(true);
       recWav->setFlat(false);
       recWav->setHint("Record to .wav");
       recWav->clicked.connect(this, &MainWindow::onRecordWav);
    toolBar1_->add(recWav);


    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->undo()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "undo.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->redo()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "redo.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->recordnotes()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "recordnotes.xpm");

    img->setPreferredSize(25,25);
    NButton* recNotes = new NButton(img);
       recNotes->setHint("Record Notes Mode");
    toolBar1_->add(recNotes);


    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->playstart()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "playstart.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(barPlayFromStartBtn_ = new NButton(img));
    barPlayFromStartBtn_->setHint("Play fro start");

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->play()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "play.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->playselpattern()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "playselpattern.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->stop()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "stop.xpm");
    img->setPreferredSize(25,25);
    NButton* stopBtn_ = new NButton(img);
       stopBtn_->click.connect(this,&MainWindow::onBarStop);
       stopBtn_->setHint("Stop");
    toolBar1_->add(stopBtn_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->autoStop()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "autostop.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

    toolBar1_->add(new NToolBarSeparator());


    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->machines()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "machines.xpm");
    img->setPreferredSize(25,25);
    NButton* macBtn_ = new NButton(img);
      macBtn_->setFlat(false);
      macBtn_->setToggle(true);
      macBtn_->setHint("Machines");
      macBtn_->clicked.connect(this,&MainWindow::onMachineView);
    toolBar1_->add(macBtn_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->patterns()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "patterns.xpm");
    img->setPreferredSize(25,25);
    NButton* patBtn_ = new NButton(img);
       patBtn_->clicked.connect(this,&MainWindow::onPatternView);
       patBtn_->setFlat(false);
       patBtn_->setToggle(true);
       patBtn_->setHint("Patterns");
    toolBar1_->add(patBtn_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->sequencer()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "sequencer.xpm");
    img->setPreferredSize(25,25);
    NButton* seqBtn = new NButton(img);
       seqBtn->setHint("Sequencer");
    toolBar1_->add(seqBtn)->clicked.connect(this,&MainWindow::onSequencerView);

    toolBar1_->add(new NToolBarSeparator());


    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->newmachine()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "newmachine.xpm");
    img->setPreferredSize(25,25);
    NButton* newMacBtn = new NButton(img);
      newMacBtn->setHint("New Machine");
    toolBar1_->add(newMacBtn)->clicked.connect(childView_,&ChildView::onMachineViewDblClick);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->openeditor()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "openeditor.xpm");
    img->setPreferredSize(25,25);
    NButton* editInsBtn = new NButton(img);
      editInsBtn->setHint("Edit Instrument");
    toolBar1_->add(editInsBtn)->clicked.connect(this,&MainWindow::onEditInstrument);

    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->p()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "p.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->help()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "help.xpm");
    img->setPreferredSize(25,25);
    toolBar1_->add(new NButton(img));

  toolBar1_->resize();

  psycleControlBar_ = new NToolBar();
    psycleControlBar_->add(new NLabel("Tracks"));
    trackCombo_ = new NComboBox();
      trackCombo_->setWidth(40);
      trackCombo_->setHeight(20);
      trackCombo_->itemSelected.connect(this,&MainWindow::onTrackChange);
    psycleControlBar_->add(trackCombo_);
      for(int i=4;i<=MAX_TRACKS;i++) {
       trackCombo_->add(new NItem(stringify(i)));
      }
    psycleControlBar_->add(new NLabel("Tempo"));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") 
      img->setSharedBitmap(&Global::pBitmaps()->lessless());
    else
      img->loadFromFile(Global::pConfig()->iconPath+ "lessless.xpm");
    img->setPreferredSize(25,25);
    NButton* bpmDecBtnTen = new NButton(img);
    bpmDecBtnTen->setFlat(false);
    psycleControlBar_->add(bpmDecBtnTen)->clicked.connect(this,&MainWindow::onBpmDecTen);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") 
       img->setSharedBitmap(&Global::pBitmaps()->less()); 
    else
       img->loadFromFile(Global::pConfig()->iconPath+ "less.xpm");
    img->setPreferredSize(25,25);
    NButton* bpmDecBtnOne = new NButton(img);
    bpmDecBtnOne->setFlat(false);
    psycleControlBar_->add(bpmDecBtnOne)->clicked.connect(this,&MainWindow::onBpmDecOne);

    bpmDisplay_ = new N7SegDisplay(3);
      bpmDisplay_->setColors(NColor(250,250,250),NColor(100,100,100),NColor(230,230,230));
      bpmDisplay_->setNumber(125);
    psycleControlBar_->add(bpmDisplay_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") 
       img->setSharedBitmap(&Global::pBitmaps()->more()); 
    else
       img->loadFromFile(Global::pConfig()->iconPath+ "more.xpm");
    img->setPreferredSize(25,25);

    NButton* bpmIncBtnOne = new NButton(img);
      bpmIncBtnOne->setFlat(false);
    psycleControlBar_->add(bpmIncBtnOne)->clicked.connect(this,&MainWindow::onBpmIncOne);


     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->moremore()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "moremore.xpm");
     img->setPreferredSize(25,25);
     NButton* moremoreBmp = new NButton(img);
       moremoreBmp->setFlat(false);
       moremoreBmp->clicked.connect(this,&MainWindow::onBpmAddTen);
     psycleControlBar_->add(moremoreBmp);


    psycleControlBar_->add(new NLabel("Lines per beat"));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->less()); 
       else
    img->loadFromFile(Global::pConfig()->iconPath+ "less.xpm");
    img->setPreferredSize(25,25);
    NButton* lessTpbButton = new NButton(img);
       lessTpbButton->setFlat(false);
       lessTpbButton->clicked.connect(this,&MainWindow::onTpbDecOne);
    psycleControlBar_->add(lessTpbButton);

    tpbDisplay_ = new N7SegDisplay(2);
      tpbDisplay_->setColors(NColor(250,250,250),NColor(100,100,100),NColor(230,230,230));
      tpbDisplay_->setNumber(4);
    psycleControlBar_->add(tpbDisplay_);

   img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->more()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "more.xpm");
    img->setPreferredSize(25,25);
    NButton* moreTpbButton = new NButton(img);
       moreTpbButton->setFlat(false);
       moreTpbButton->clicked.connect(this,&MainWindow::onTpbIncOne);
    psycleControlBar_->add(moreTpbButton);

    psycleControlBar_->add(new NLabel("Octave"));
    octaveCombo_ = new NComboBox();
      for (int i=0; i<9; i++) octaveCombo_->add(new NItem(stringify(i)));
      octaveCombo_->itemSelected.connect(this,&MainWindow::onOctaveChange);
      octaveCombo_->setWidth(40);
      octaveCombo_->setHeight(20);
    psycleControlBar_->add(octaveCombo_);

    psycleControlBar_->add(new NLabel("VU"));
    NPanel* vuPanel = new NPanel();
    vuPanel->setPosition(0,0,225,10);
       vuMeter_ = new VuMeter();
       vuPanel->add(vuMeter_);
       vuMeter_->setPosition(0,0,225,10);

       masterSlider_ = new NSlider();
       masterSlider_->setOrientation(nHorizontal);
       masterSlider_->setPosition(0,10,225,10);
       vuPanel->add(masterSlider_);
    psycleControlBar_->add(vuPanel);

  toolBarPanel_->add(psycleControlBar_);

  psycleToolBar_ = new NToolBar();
     psycleToolBar_->add(new NLabel("Pattern Step"));
     patternCombo_ = new NComboBox();
     for (int i = 1; i <=16; i++) 
       patternCombo_->add(new NItem(stringify(i)));
     patternCombo_->setIndex(0);
     patternCombo_->itemSelected.connect(this,&MainWindow::onPatternStepChange);
     patternCombo_->setWidth(40);
     patternCombo_->setHeight(20);
     psycleToolBar_->add(patternCombo_);
     psycleToolBar_->add(new NToolBarSeparator());
     genCombo_ = new NComboBox();
       genCombo_->setWidth(158);
       genCombo_->setHeight(20);
       updateComboGen();
       genCombo_->setIndex(0);
     psycleToolBar_->add(genCombo_);

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleleft()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleleft.xpm");
     img->setPreferredSize(25,25);
     psycleToolBar_->add(new NButton(img));

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleright()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleright.xpm");
     img->setPreferredSize(25,25);
     psycleToolBar_->add(new NButton(img));

     psycleToolBar_->add(new NButton("Gear Rack"));
     psycleToolBar_->add(new NToolBarSeparator());
     auxSelectCombo_ = new NComboBox();
     auxSelectCombo_->setWidth(70);
     auxSelectCombo_->setHeight(20);
     auxSelectCombo_->add(new NItem("Wave"));
     auxSelectCombo_->setIndex(0);
     psycleToolBar_->add(auxSelectCombo_);
     insCombo_ = new NComboBox();
     insCombo_->setWidth(158);
     insCombo_->setHeight(20);
     psycleToolBar_->add(insCombo_);

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleleft()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleleft.xpm");
     img->setPreferredSize(25,25);
     psycleToolBar_->add(new NButton(img))->clicked.connect(this,&MainWindow::onDecInsBtn);

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleright()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleright.xpm");
     img->setPreferredSize(25,25);
     psycleToolBar_->add(new NButton(img))->clicked.connect(this,&MainWindow::onIncInsBtn);

     psycleToolBar_->add(new NButton("Load"))->clicked.connect(this,&MainWindow::onLoadWave);
     psycleToolBar_->add(new NButton("Save"));
     psycleToolBar_->add(new NButton("Edit"))->clicked.connect(this,&MainWindow::onEditInstrument);
     psycleToolBar_->add(new NButton("Wave Ed"));
     psycleToolBar_->resize();

  toolBarPanel_->add(psycleToolBar_);

  toolBarPanel_->resize();
}

void MainWindow::initSignals( )
{
  fileMenu_->itemClicked.connect(this, &MainWindow::onFileMenuItemClicked);
  helpMenu_->itemClicked.connect(this, &MainWindow::onHelpMenuItemClicked);
  viewMenu_->itemClicked.connect(this, &MainWindow::onViewMenuItemClicked);
  childView_->newSongLoaded.connect(sequencerBar_,&SequencerBar::updateSequencer);
  barPlayFromStartBtn_->click.connect(this,&MainWindow::onBarPlayFromStart);
}

void MainWindow::onBarPlayFromStart( NButtonEvent * ev )
{
  childView_->playFromStart();
}

void MainWindow::onFileNew( NButtonEvent * ev )
{
  appNew();
}

void MainWindow::onFileOpen( NButtonEvent * ev )
{
  usleep(200); // ugly hack but works
  progressBar_->setVisible(true);
  childView_->onFileLoadSong(0);
  progressBar_->setVisible(false);
  updateComboGen();
  pane()->repaint();
}

void MainWindow::onFileSave( NButtonEvent * ev )
{
}

void MainWindow::onFileSaveAs( NButtonEvent * ev )
{
  usleep(200); // ugly hack but works
  progressBar_->setVisible(true);
  childView_->onFileSaveSong(0);
  progressBar_->setVisible(false);
  pane()->repaint();
}

void MainWindow::onFileMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv)
{
  if (itemEv->text()=="Song properties") {
     songpDlg_->setVisible(true);
  } else
  if (itemEv->text()=="Exit") {
     closePsycle();
  }
}

void MainWindow::onViewMenuItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text()=="Toolbar") {
     toolBar1_->setVisible(!toolBar1_->visible());
     pane()->resize();
     pane()->repaint();
  } else
  if (itemEv->text()=="MachineBar") {
     psycleToolBar_->setVisible(!psycleToolBar_->visible());
     pane()->resize();
     pane()->repaint();
  } else
  if (itemEv->text()=="SequencerBar") {
     sequencerBar_->setVisible(!sequencerBar_->visible());
     pane()->resize();
     pane()->repaint();
  } else
  if (itemEv->text()=="StatusBar") {
     statusBar_->setVisible(!statusBar_->visible());
     pane()->resize();
     pane()->repaint();
  } else
  if (itemEv->text()=="Add machine") {
     if (childView_->newMachineDlg()->execute()) {
         if (childView_->newMachineDlg()->outBus()) {
           // Generator selected
           int x = 10; int y = 10;
           int fb = Global::pSong()->GetFreeBus();
           Global::pSong()->CreateMachine(MACH_PLUGIN, x, y, childView_->newMachineDlg()->getDllName().c_str(),fb);
           childView_->machineView()->addMachine(Global::pSong()->_pMachine[fb]);
           childView_->machineView()->repaint();
         }
     }
  }
}

void MainWindow::onSongLoadProgress( int chunkCount, int max, const std::string & header)
{
  progressBar_->setMax(max);
  progressBar_->setValue(chunkCount);
  progressBar_->repaint();
  NApp::flushEventQueue();
}

void MainWindow::onOctaveChange( NItemEvent * ev )
{
  std::stringstream str; 
  str << ev->item()->text();
  int octave = 0;
  str >> octave;
  childView_->patternView()->setEditOctave(octave);
}


void MainWindow::onTrackChange( NItemEvent * ev )
{
  std::stringstream str; 
  str << ev->item()->text();
  int track = 0;
  str >> track;
  Global::pSong()->SONGTRACKS=track;
  if (childView_->patternView()->cursor().x() >= Global::pSong()->SONGTRACKS)
  {
    childView_->patternView()->setCursor(NPoint3D(Global::pSong()->SONGTRACKS,childView_->patternView()->cursor().y(),0));
  }
  childView_->patternView()->repaint();
}

void MainWindow::onBarStop(NButtonEvent* ev)
{
  bool pl = Global::pPlayer()->_playing;
  bool blk = Global::pPlayer()->_playBlock;
  Global::pPlayer()->Stop();
  // pParentMain->SetAppSongBpm(0);
  // pParentMain->SetAppSongTpb(0);

  if (pl) {
    if ( Global::pConfig()->_followSong && blk) {
        //editPosition=prevEditPosition;
        //pParentMain->UpdatePlayOrder(false); // <- This restores the selected block
        //Repaint(DMPattern);
    } else {
       memset(Global::pSong()->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
       Global::pSong()->playOrderSel[childView_->patternView()->editPosition()] = true;
       //Repaint(DMCursor); 
    }
  }
}

void MainWindow::closePsycle()
{
  exit(0);
}


void MainWindow::updateComboGen() {

  bool filled=false;
  bool found=false;
  int selected = -1;
  int line = -1;
  char buffer[64];

  genCombo_->removeChilds();

  for (int b=0; b<MAX_BUSES; b++) // Check Generators
  {
   if( Global::pSong()->_pMachine[b]) {
     sprintf(buffer,"%.2X: %s",b,Global::pSong()->_pMachine[b]->_editName);
     genCombo_->add(new NItem(buffer));
       //cb->SetItemData(cb->GetCount()-1,b);

     if (!found) selected++;
     if (Global::pSong()->seqBus == b) found = true;
     filled = true;
   }
  }

  genCombo_->add(new NItem("----------------------------------------------------"));
  //cb->SetItemData(cb->GetCount()-1,65535);
  if (!found)  {
    selected++;
    line = selected;
  }

  for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
  {
    if(Global::pSong()->_pMachine[b]) {
       sprintf(buffer,"%.2X: %s",b,Global::pSong()->_pMachine[b]->_editName);
       genCombo_->add(new NItem(buffer));
        //cb->SetItemData(cb->GetCount()-1,b);
     if (!found) selected++;
     if (Global::pSong()->seqBus == b) found = true;
     filled = true;
    }
  }

  if (!filled) {
     genCombo_->removeChilds();
     genCombo_->add(new NItem("No Machines Loaded"));
     selected = 0;
  } else if (!found)  {
    selected=line;
  }
}

void MainWindow::appNew( )
{
  if (checkUnsavedSong())
  //if (CheckUnsavedSong("New Song"))
  {
    //KillUndo();
    //KillRedo();
    Global::pPlayer()->Stop();
    childView_->machineView()->removeMachines();

    //Sleep(LOCK_LATENCY);
    //_outputActive = false;
    //Global::pConfig->_pOutputDriver->Enable(false);
    // midi implementation
    //Global::pConfig->_pMidiInput->Close();
    //Sleep(LOCK_LATENCY);
    Global::pSong()->New();
    //_outputActive = true;
    //if (!Global::pConfig->_pOutputDriver->Enable(true))
    //{
      //_outputActive = false;
    //}
    //else
    //{
    // midi implementation
    //Global::pConfig->_pMidiInput->Open();
  }
  childView_->setTitleBarText();
  childView_->patternView()->setEditPosition(0);
  Global::pSong()->seqBus=0;
  sequencerBar_->updateSequencer();
  childView_->machineView()->createGUIMachines();
  //pParentMain->PsybarsUpdate(); // Updates all values of the bars
//  pParentMain->WaveEditorBackUpdate();
//  pParentMain->m_wndInst.WaveUpdate();
//  pParentMain->RedrawGearRackList();
//  pParentMain->UpdateSequencer();
//  pParentMain->UpdatePlayOrder(false); // should be done always after updatesequencer
				//pParentMain->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
  childView_->patternView()->repaint();
  childView_->machineView()->repaint();
  sequencerBar_->repaint();


}


void MainWindow::onHelpMenuItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text()=="Greetings") {
    greetDlg->setVisible(true);
  } else 
  if (itemEv->text()=="About") {
    aboutDlg->setVisible(true);
  }
}

void MainWindow::onEditMenuItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text()=="Undo") {

  } else
  if (itemEv->text()=="Redo") {

  } else
  if (itemEv->text()=="Pattern Cut") {

  } else 
  if (itemEv->text()=="Pattern Copy") {

  } else
  if (itemEv->text()=="Pattern Paste") {

  } else
  if (itemEv->text()=="Pattern Mix Paster") {

  } else
  if (itemEv->text()=="Pattern Delete") {

  } else
  if (itemEv->text()=="Block Cut") {
    childView_->patternView()->copyBlock(true);
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Block Copy") {
    childView_->patternView()->copyBlock(false);
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Block Paste") {
    PatternView* pView = childView_->patternView();
    pView->pasteBlock(pView->cursor().x(),pView->cursor().y(),false);
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Block Mix Paste") {

  } else
  if (itemEv->text()=="Block Delete") {
    childView_->patternView()->deleteBlock();
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Sequence Cut") {

  } else
  if (itemEv->text()=="Sequence Copy") {

  } else
  if (itemEv->text()=="Sequence Delete") {

  }
}

void MainWindow::onBpmIncOne(NButtonEvent* ev)  // OnBpmAddOne
{
  setAppSongBpm(1);
}

void MainWindow::onBpmAddTen(NButtonEvent* ev)
{
  setAppSongBpm(10);
}

void MainWindow::onBpmDecOne(NButtonEvent* ev)
{
  setAppSongBpm(-1);
}

void MainWindow::onBpmDecTen(NButtonEvent* ev)
{
  setAppSongBpm(-10);
}

void MainWindow::setAppSongBpm(int x)
{
   int bpm = 0;
   if ( x != 0 ) {
     if (Global::pPlayer()->_playing )  {
        Global::pSong()->BeatsPerMin(Global::pPlayer()->bpm+x);
     } else Global::pSong()->BeatsPerMin(Global::pSong()->BeatsPerMin()+x);
     Global::pPlayer()->SetBPM(Global::pSong()->BeatsPerMin(),Global::pSong()->LinesPerBeat());
     bpm = Global::pSong()->BeatsPerMin();
   }
   else bpm = Global::pPlayer()->bpm;

   bpmDisplay_->setNumber(Global::pPlayer()->bpm);

   bpmDisplay_->repaint();
}

void MainWindow::setAppSongTpb(int x)
{
  int tpb = 0;

  if ( x != 0)
  {
     if (Global::pPlayer()->_playing )
       Global::pSong()->LinesPerBeat(Global::pPlayer()->tpb+x);
     else 
       Global::pSong()->LinesPerBeat(Global::pSong()->LinesPerBeat()+x);
       Global::pPlayer()->SetBPM(Global::pSong()->BeatsPerMin(), Global::pSong()->LinesPerBeat());
       tpb = Global::pSong()->LinesPerBeat();
  } else tpb = Global::pPlayer()->tpb;

  tpbDisplay_->setNumber(tpb);

  psycleControlBar_->repaint();
}


void MainWindow::onTpbDecOne(NButtonEvent* ev)
{
  setAppSongTpb(-1);
  childView_->patternView()->repaint();
}

void MainWindow::onTpbIncOne(NButtonEvent* ev)
{
  setAppSongTpb(1);
  childView_->patternView()->repaint();
}

void MainWindow::onRecordWav( NButtonEvent * ev )
{
  if (!Global::pPlayer()->_recording)
  {
     if (wavRecFileDlg->execute()) {
        Global::pPlayer()->StartRecording(wavRecFileDlg->fileName());
      }
      if ( Global::pConfig()->autoStopMachines )
      {
        //OnAutostop();
      }
  }
  else
  {
     Global::pPlayer()->StopRecording();
  }
}

void MainWindow::onTimer( )
{
  if (Global::pPlayer()->_playing) {
    int oldPos = childView_->patternView()->editPosition();
    childView_->patternView()->updatePlayBar(sequencerBar_->followSong());

    if (sequencerBar_->followSong() && oldPos != Global::pPlayer()->_playPosition) {
       sequencerBar_->updatePlayOrder(true);
       sequencerBar_->updateSequencer();
    }
  }

  vuMeter_->setPegel(Global::pSong()->_pMachine[MASTER_INDEX]->_lMax,
  Global::pSong()->_pMachine[MASTER_INDEX]->_rMax );
  vuMeter_->repaint();
  ((Master*)Global::pSong()->_pMachine[MASTER_INDEX])->vuupdated = true;

  childView_->machineView()->updateVUs();
}

void MainWindow::updateBars( )
{
 int p[] = {1, 2, 3, 4, 5};
 std::vector<int> a(p, p+5);
}

int MainWindow::close( )
{
  closePsycle();
}

void MainWindow::onMachineView(NButtonEvent* ev) {
  childView_->setActivePage(0);
  childView_->repaint();
}

void MainWindow::onPatternView(NButtonEvent* ev) {
  childView_->setActivePage(1);
  childView_->repaint();
}

void MainWindow::onLoadWave( NButtonEvent * ev )
{
  NFileDialog* dialog = new NFileDialog();
  add(dialog);

  dialog->addFilter("Wav Files(*.wav)","!S*.wav");

  if (dialog->execute()) {
    int si = Global::pSong()->instSelected;
    //added by sampler
    if ( Global::pSong()->_pInstrument[si]->waveLength != 0)
    {
       //if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;
    }

    if (Global::pSong()->WavAlloc(si,dialog->fileName().c_str()))
    {
      updateComboIns(true);
     //m_wndStatusBar.SetWindowText("New wave loaded");
     //WaveEditorBackUpdate();
     //m_wndInst.WaveUpdate();
    }
  }

  NApp::addRemovePipe(dialog);
}

void MainWindow::updateComboIns( bool updatelist )
{
  if (updatelist)  {
    insCombo_->removeChilds();
    char buffer[64];
    int listlen = 0;
    for (int i=0;i<PREV_WAV_INS;i++)
    {
      sprintf(buffer, "%.2X: %s", i, Global::pSong()->_pInstrument[i]->_sName);
      insCombo_->add(new NItem(buffer));
      listlen++;
    }
    if (Global::pSong()->auxcolSelected >= listlen) {
      Global::pSong()->auxcolSelected = 0;
    }
  }
}

void MainWindow::onEditInstrument( NButtonEvent * ev )
{
  instrumentEditor->setInstrument(Global::pSong()->instSelected);
  instrumentEditor->setVisible(true);
}

void MainWindow::onDecInsBtn( NButtonEvent * ev )
{
  int index = Global::pSong()->instSelected -1;
  if (index >=0 ) {
    Global::pSong()->instSelected=   index;
    Global::pSong()->auxcolSelected= index;

    insCombo_->setIndex(index);
    insCombo_->repaint();
  }
}

void MainWindow::onIncInsBtn( NButtonEvent * ev )
{
  int index = Global::pSong()->instSelected +1;
  if (index <= 255) {
    Global::pSong()->instSelected=   index;
    Global::pSong()->auxcolSelected= index;

    insCombo_->setIndex(index);
    insCombo_->repaint();
  }
}

bool MainWindow::checkUnsavedSong( )
{
  NMessageBox* box = new NMessageBox("Save changes of : "+Global::pSong()->fileName+" ?");
  box->setTitle("New Song");
  box->setButtonText("Yes","No","Abort");
  add(box);
  bool result = box->execute();
  if (result == true) {
     childView_->onFileSaveSong(0);
  }
  NApp::addRemovePipe(box);
  return result;
}

void MainWindow::onPatternStepChange( NItemEvent * ev )
{
  if (patternCombo_->selIndex()!=-1) {
     childView_->patternView()->setPatternStep(patternCombo_->selIndex()+1);
  }
}

void MainWindow::onSequencerView( NButtonEvent * ev )
{
  NMessageBox* box = new NMessageBox("This feature is unimplemented in this release. Use the left side sequence now");
  box->setTitle("Psycle Notice");
  box->setButtons(nMsgOkBtn);
  box->execute();
  NApp::addRemovePipe(box);
}









