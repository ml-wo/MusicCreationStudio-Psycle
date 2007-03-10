/***************************************************************************
*   Copyright (C) 2006 by Neil Mather   *
*   nmather@sourceforge   *
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

#include <QtGui>

#include "mainwindow.h"
#include "patternbox.h"

#include "psycore/player.h"
#include "psycore/alsaout.h"
#include "psycore/song.h"
#include "psycore/singlepattern.h"
#include "psycore/patterndata.h"
#include "psycore/patternsequence.h"

#include <QTreeWidgetItem>

#include <iostream>
#include <iomanip>

MainWindow::MainWindow()
{
    song_ = new psy::core::Song();
    setupSong();
    setupSound();

    macView_ = new MachineView( song_ );
    patView_ = new PatternView( song_ );
    wavView_ = new WaveView( song_ );
    seqView_ = new SequencerView();
    patternBox_ = new PatternBox( song_ );

    setupGui();
    setupSignals();

    patternBox_->populatePatternTree(); // FIXME: here because of bad design?
    populateMachineCombo();
    initSampleCombo();
}

void MainWindow::setupSong()
{
    psy::core::Machine *master = song_->_pMachine[psy::core::MASTER_INDEX] ; 

    int fb = song_->GetFreeBus();
    song_->CreateMachine(psy::core::MACH_SAMPLER, 100, 20, "SAMPLER", fb);  
    psy::core::Machine *sampler0 = song_->_pMachine[fb];
    song_->InsertConnection( sampler0->_macIndex , master->_macIndex, 1.0f);

    psy::core::PatternCategory* category0 = song_->patternSequence()->patternData()->createNewCategory("Category0");
    psy::core::PatternCategory* category1 = song_->patternSequence()->patternData()->createNewCategory("Category1");
    psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");
    psy::core::SinglePattern* pattern1 = category1->createNewPattern("Pattern1");

    psy::core::PatternEvent event0 = pattern0->event( 0, 0 );
    psy::core::Machine* tmac = sampler0;// song_->_pMachine[ song_->seqBus ];
    event0.setNote( 4 * 12 + 0);
    event0.setSharp( false );
    if (tmac) event0.setMachine( tmac->_macIndex );
    if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
        event0.setInstrument( 0 );
    }
    pattern0->setEvent( 0, 0, event0 );

    fb = song_->GetFreeBus();
    song_->CreateMachine(psy::core::MACH_SAMPLER, 300, 20, "SAMPLER", fb);  
    psy::core::Machine *sampler1 = song_->_pMachine[fb];
    song_->InsertConnection( sampler1->_macIndex , master->_macIndex, 1.0f);

    tmac = sampler1;
    psy::core::PatternEvent event1 = pattern0->event( 2, 1 );
    event1.setNote( 4 * 12 + 0);
    event1.setSharp( false );
    if (tmac) event1.setMachine( tmac->_macIndex );
    if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
        event1.setInstrument( 1 );
    }
    pattern0->setEvent( 2, 2, event1 );

    tmac = sampler0;
    psy::core::PatternEvent event2 = pattern0->event( 4, 0 );
    event2.setNote( 4 * 12 + 0);
    event2.setSharp( false );
    if (tmac) event2.setMachine( tmac->_macIndex );
    if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
        event2.setInstrument( 2 );
    }
    pattern0->setEvent( 4, 0, event2 );

    psy::core::SequenceLine *seqLine = song_->patternSequence()->createNewLine();
	psy::core::SequenceEntry *seqEntry = seqLine->createEntry( pattern0, 0 );
}

void MainWindow::setupSound() 
{
    psy::core::AudioDriver *driver = new psy::core::AlsaOut;
    psy::core::AudioDriverSettings settings = driver->settings();
    settings.setDeviceName( "plughw:0" );
    driver->setSettings( settings );

    psy::core::Player::Instance()->song( song_ );
    psy::core::Player::Instance()->setDriver( *driver );  
}

void MainWindow::setupGui()
{
    QWidget *workArea = new QWidget();

    QDockWidget *dock = new QDockWidget( "Pattern Box", this );
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(patternBox_);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    views_ = new QTabWidget();
    views_->addTab( macView_, "Machine View" );
    views_->addTab( patView_, "Pattern View" );
    views_->addTab( wavView_, "Wave Editor" );
    views_->addTab( seqView_, "Sequencer View" );



    QGridLayout *layout = new QGridLayout;
    layout->addWidget( views_ );
    //layout->addWidget( views_, 0, 1, 0, 2 );
    //layout->setColumnStretch(1, 15);
    workArea->setLayout(layout);
    setCentralWidget(workArea);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowTitle(tr("] Psycle Modular Music Creation Studio [ ( Q v0.00001087 alpha ) "));
}

void MainWindow::setupSignals()
{
    connect( patternBox_, SIGNAL( patternSelectedInPatternBox( psy::core::SinglePattern* ) ),
             this, SLOT( onPatternSelectedInPatternBox( psy::core::SinglePattern* ) ) );
    connect( patternBox_, SIGNAL( patternDeleted() ),
             this, SLOT( onPatternDeleted() ) );

    connect( macView_, SIGNAL( newMachineCreated( int ) ), 
             this, SLOT( onNewMachineCreated( int ) ) );
    connect( macView_, SIGNAL( machineGuiChosen( MachineGui* ) ), 
             this, SLOT( onMachineGuiChosen( MachineGui* ) ) );

    connect( wavView_, SIGNAL( sampleAdded() ), 
             this, SLOT( refreshSampleComboBox() ) );

    connect( macCombo_, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( onMachineComboBoxIndexChanged( int ) ) );

    connect( sampCombo_, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( onSampleComboBoxIndexChanged( int ) ) );
}

 void MainWindow::newSong()
 {

 }

 void MainWindow::open()
 {

 }

 void MainWindow::save()
 {
     QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Choose a file name"), ".",
                         tr("Psycle Songs (*.psy)"));
     if (fileName.isEmpty())
         return;

     statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
 }

 void MainWindow::undo()
 {
 }

 void MainWindow::redo()
 {
 }

 void MainWindow::about()
 {
    QMessageBox::about(this, tr("About qpsycle"),
             tr("It makes music and stuff."));
 }

 void MainWindow::createActions()
 {
     newAct = new QAction(QIcon(":/images/new.png"), tr("&New Song"), this);
     newAct->setShortcut(tr("Ctrl+N"));
     newAct->setStatusTip(tr("Create a new song"));
     connect(newAct, SIGNAL(triggered()), this, SLOT(newSong()));

     openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
     openAct->setShortcut(tr("Ctrl+O"));
     openAct->setStatusTip(tr("Open an existing song"));
     connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

     saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
     saveAct->setShortcut(tr("Ctrl+S"));
     saveAct->setStatusTip(tr("Save the current song"));
     connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

     undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
     undoAct->setShortcut(tr("Ctrl+Z"));
     undoAct->setStatusTip(tr("Undo the last action"));
     connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

     redoAct = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
     redoAct->setShortcut(tr("Ctrl+Y"));
     redoAct->setStatusTip(tr("Redo the last undone action"));
     connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

     quitAct = new QAction(tr("&Quit"), this);
     quitAct->setShortcut(tr("Ctrl+Q"));
     quitAct->setStatusTip(tr("Quit the application"));
     connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

     aboutAct = new QAction(tr("&About qpsycle"), this);
     aboutAct->setStatusTip(tr("About qpsycle"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

     playStartAct = new QAction(QIcon(":/images/playstart.png"), tr("&Play from start"), this);
     playAct = new QAction(QIcon(":/images/play.png"), tr("Play from &edit position"), this);
     playPatAct = new QAction(QIcon(":/images/playselpattern.png"), tr("Play selected p&attern"), this);
     stopAct = new QAction(QIcon(":images/stop.png"), tr("&Stop playback"), this);

 }

 void MainWindow::createMenus()
 {
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(newAct);
     fileMenu->addAction(openAct);
     fileMenu->addAction(saveAct);
     fileMenu->addSeparator();
     fileMenu->addAction(quitAct);

     editMenu = menuBar()->addMenu(tr("&Edit"));
     editMenu->addAction(undoAct);
     editMenu->addAction(redoAct);

     viewMenu = menuBar()->addMenu(tr("&View"));
     configMenu = menuBar()->addMenu(tr("&Configuration"));
     performMenu = menuBar()->addMenu(tr("&Performance"));
     communityMenu = menuBar()->addMenu(tr("&Community"));

     menuBar()->addSeparator();

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
 }

 void MainWindow::createToolBars()
 {
     fileToolBar = addToolBar(tr("File"));
     fileToolBar->addAction(newAct);
     fileToolBar->addAction(openAct);
     fileToolBar->addAction(saveAct);

     editToolBar = addToolBar(tr("Edit"));
     editToolBar->addAction(undoAct);
     editToolBar->addAction(redoAct);

     playToolBar = addToolBar(tr("Play"));
     playToolBar->addAction(playStartAct);
     playToolBar->addAction(playAct);
     playToolBar->addAction(playPatAct);
     playToolBar->addAction(stopAct);

     machToolBar = addToolBar(tr("Machines"));
     macCombo_ = new QComboBox();
     sampCombo_ = new QComboBox();

     QLabel *macLabel = new QLabel(" Machines: ");
     QLabel *sampLabel = new QLabel(" Samples: ");
     machToolBar->addWidget(macLabel);
     machToolBar->addWidget( macCombo_ );
     machToolBar->addWidget(sampLabel);
     machToolBar->addWidget( sampCombo_ );
 }

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::keyPressEvent( QKeyEvent * event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );

    switch ( command ) {
        case psy::core::cdefShowPatternBox:
            patternBox_->patternTree()->setFocus();        
        break;
        case psy::core::cdefShowMachineView:
            views_->setCurrentWidget( macView_ );        
        break;
        case psy::core::cdefShowPatternView:
            views_->setCurrentWidget( patView_ );        
            patView_->setFocus();
        break;
        case psy::core::cdefShowWaveEditor:
            views_->setCurrentWidget( wavView_ );        
        break;
        case psy::core::cdefShowSequencerView:
            views_->setCurrentWidget( seqView_ );        
        break;
        case psy::core::cdefPlayStart:
            psy::core::Player::Instance()->start( 0.0 );
        break;
        default:;
    }
}

void MainWindow::populateMachineCombo() 
{
    if (!song_) return;

    macCombo_->clear();

    bool filled=false;
    bool found=false;
    int selected = -1;
    int line = -1;
    std::ostringstream buffer;
    buffer.setf(std::ios::uppercase);

    for (int b=0; b<psy::core::MAX_BUSES; b++) // Check Generators
    {
        if( song_->_pMachine[b]) {
            buffer.str("");
            buffer << std::setfill('0') << std::hex << std::setw(2);
            buffer << b << ": " << song_->_pMachine[b]->_editName;
            macCombo_->addItem( QString::fromStdString( buffer.str() ) );

            //cb->SetItemData(cb->GetCount()-1,b);
            if (!found) selected++;
            if (song_->seqBus == b) found = true;
            filled = true;
        }
    }

    macCombo_->addItem( "--------------------------");
    //cb->SetItemData(cb->GetCount()-1,65535);
    if (!found)  {
        selected++;
        line = selected;
    }

    for (int b=psy::core::MAX_BUSES; b<psy::core::MAX_BUSES*2; b++) // Write Effects Names.
    {
        if(song_->_pMachine[b]) {
            buffer.str("");
            buffer << std::setfill('0') << std::hex << std::setw(2);
            buffer << b << ": " << song_->_pMachine[b]->_editName;
            macCombo_->addItem( QString::fromStdString( buffer.str() ) );
            //cb->SetItemData(cb->GetCount()-1,b);
            if (!found) selected++;
            if (song_->seqBus == b) found = true;
            filled = true;
        }
    }

    if (!filled) {
//        macCombo_->removeChilds();
        macCombo_->addItem( "No Machines Loaded" );
        selected = 0;
    } else if (!found)  {
        selected=line;
    }
    macCombo_->setCurrentIndex(selected);
    macCombo_->update();
}

void MainWindow::initSampleCombo()
{
    for ( int i=0; i < psy::core::MAX_INSTRUMENTS; i++ ) // PREV_WAV_INS = 255
    {
        sampCombo_->addItem( "empty" );
    }
}

void MainWindow::refreshSampleComboBox()
{
    std::ostringstream buffer;
    buffer.setf(std::ios::uppercase);

    int listlen = 0;
    for ( int i=0; i < psy::core::MAX_INSTRUMENTS; i++ ) // PREV_WAV_INS = 255
    {
        buffer.str("");
        buffer << std::setfill('0') << std::hex << std::setw(2);
        buffer << i << ": " << song_->_pInstrument[i]->_sName;
        QString name = QString::fromStdString( buffer.str() );
        sampCombo_->setItemText( i, name );
        listlen++;
    }
    if (song_->auxcolSelected >= listlen) {
        song_->auxcolSelected = 0;
    }    

}

void MainWindow::onMachineComboBoxIndexChanged( int newIndex )
{
    song_->seqBus = newIndex;

    // Choose the necessary MachineGui in the MachineView.
    MachineGui *macGui = macView_->findMachineGuiByMachineIndex( newIndex );
    macView_->setTheChosenOne( macGui );
    macView_->update();
}

void MainWindow::onSampleComboBoxIndexChanged( int newIndex )
{
	song_->instSelected   = newIndex;
    song_->auxcolSelected = newIndex;
    // FIXME: when wave editor is more advanced, we need to notify it of this change.
}

void MainWindow::onPatternSelectedInPatternBox( psy::core::SinglePattern* selectedPattern )
{
    patView_->setPattern( selectedPattern );
}

void MainWindow::onNewMachineCreated( int bus )
{
    populateMachineCombo();
    macCombo_->setCurrentIndex( bus );
}

void MainWindow::onMachineGuiChosen( MachineGui *macGui )
{
    // FIXME: shouldn't rely on macCombo to set seqBus as we do here.
    macCombo_->setCurrentIndex( macGui->mac()->_macIndex );
}

void MainWindow::onPatternDeleted()
{
    patView_->setPattern( 0 );
}
