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
    seqView_ = new SequencerView( song_ );
    patternBox_ = new PatternBox( song_ );

    setupGui();
    setupSignals();

    patternBox_->populatePatternTree(); // FIXME: here because of bad design?
    populateMachineCombo();
    initSampleCombo();
    patternBox_->patternTree()->setFocus();

    startTimer( 10 );
}

void MainWindow::setupSong()
{
    // Setup a blank song.
    psy::core::PatternCategory* category0 = song_->patternSequence()->patternData()->createNewCategory("Category0");
    psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");

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

    dock_ = new QDockWidget( "Pattern Box", this );
    dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock_->setWidget(patternBox_);
    addDockWidget(Qt::LeftDockWidgetArea, dock_);


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
    connect( patternBox_, SIGNAL( addPatternToSequencerRequest( psy::core::SinglePattern* ) ),
             this, SLOT( onAddPatternToSequencerRequest( psy::core::SinglePattern* ) ) );
    connect( patternBox_, SIGNAL( patternNameChanged() ),
             this, SLOT( onPatternNameChanged() ) );
    connect( patternBox_, SIGNAL( categoryColorChanged() ),
             this, SLOT( onCategoryColorChanged() ) );

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
    QString fileName = QFileDialog::getOpenFileName( 
                            this, "Open Song", "/home/neil", "Psy (*.psy)" );

    if ( fileName.toStdString() != "" ) {
        // stop player
        psy::core::Player::Instance()->stop();
        // disable audio driver
        //Global::configuration()._pOutputDriver->Enable(false);
        // add a new Song tab
        // load the song
        song_ = new psy::core::Song();
        song_->load( fileName.toStdString() );

        // update gui to new song FIXME: very crappy way of doing it for now.
        delete patternBox_;
        delete macView_;
        delete patView_;
        delete wavView_;
        delete seqView_;
        macView_ = new MachineView( song_ );
        patView_ = new PatternView( song_ );
        wavView_ = new WaveView( song_ );
        seqView_ = new SequencerView( song_ );
        views_->addTab( macView_, "Machine View" );
        views_->addTab( patView_, "Pattern View" );
        views_->addTab( wavView_, "Wave Editor" );
        views_->addTab( seqView_, "Sequencer View" );
        patternBox_ = new PatternBox( song_ );
        dock_->setWidget( patternBox_ );
        patternBox_->populatePatternTree();
        populateMachineCombo();
        initSampleCombo();
        patternBox_->patternTree()->setFocus();
        setupSignals();
        psy::core::Player::Instance()->song( song_ );
        // enable audio driver
        //Global::configuration()._pOutputDriver->Enable(true);
        // update file recent open sub menu
        //recentFileMenu_->add(new ngrs::MenuItem(fileName));
    }

}

 void MainWindow::save()
 {
     QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Choose a file name"), ".",
                         tr("Psycle Songs (*.psy)"));
     if (fileName.isEmpty())
         return;

    qDebug("saving");
    std::cout << "filename is " << fileName.toStdString() << std::endl;
    song_->save( fileName.toStdString() );
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
     octCombo_ = new QComboBox();

     QLabel *macLabel = new QLabel(" Machines: ");
     QLabel *sampLabel = new QLabel(" Samples: ");
     machToolBar->addWidget(macLabel);
     machToolBar->addWidget( macCombo_ );
     machToolBar->addWidget(sampLabel);
     machToolBar->addWidget( sampCombo_ );
     
     octToolBar_ = addToolBar( "Octave" );
     octToolBar_->addWidget( new QLabel( "Octave: " ) );
     octToolBar_->addWidget( octCombo_ );
     for ( int i = 0; i < 9; i++ ) {
         octCombo_->addItem( QString::number( i ) );
     }
     connect( octCombo_, SIGNAL( currentIndexChanged( int ) ),
              this, SLOT( onOctaveComboBoxIndexChanged( int ) ) );
     octCombo_->setCurrentIndex( 4 );
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
            patView_->patDraw()->setFocus();
           patView_->patDraw()->scene()->setFocusItem( patView_->patDraw()->patternGrid() );
        break;
        case psy::core::cdefShowWaveEditor:
            views_->setCurrentWidget( wavView_ );        
        break;
        case psy::core::cdefShowSequencerView:
            views_->setCurrentWidget( seqView_ );        
        break;
        // Play controls.
        case psy::core::cdefPlayStart:
            psy::core::Player::Instance()->start( 0.0 );
        break;
        case psy::core::cdefPlayStop:
            psy::core::Player::Instance()->stop();
        break;
        case psy::core::cdefInstrInc:
            sampCombo_->setCurrentIndex( sampCombo_->currentIndex() + 1 );
        break;
        case psy::core::cdefInstrDec:
            sampCombo_->setCurrentIndex( sampCombo_->currentIndex() - 1 );
        break;
        case psy::core::cdefOctaveUp:
            octCombo_->setCurrentIndex( std::max( 0, octCombo_->currentIndex() + 1 ) );
        break;
        case psy::core::cdefOctaveDn:
            octCombo_->setCurrentIndex( std::min( 8, octCombo_->currentIndex() - 1 ) );
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

void MainWindow::onAddPatternToSequencerRequest( psy::core::SinglePattern *pattern )
{
    seqView_->addPattern( pattern );
}

void MainWindow::onPatternNameChanged()
{
    // FIXME: not good code, plus not efficient, don't need to repaint the whole thing...
    seqView_->sequencerDraw()->scene()->update( seqView_->sequencerDraw()->scene()->itemsBoundingRect() );

}

void MainWindow::onCategoryColorChanged()
{
    // FIXME: not good code, plus not efficient, don't need to repaint the whole thing...
    seqView_->sequencerDraw()->scene()->update( seqView_->sequencerDraw()->scene()->itemsBoundingRect() );

}

void MainWindow::timerEvent( QTimerEvent *ev )
{
    if ( psy::core::Player::Instance()->playing() ) {
        seqView_->updatePlayPos();				

        psy::core::SinglePattern* visiblePattern = 0;
        visiblePattern = patView_->pattern();
        if ( visiblePattern ) {			
            double entryStart = 0;
            bool isPlayPattern = song_->patternSequence()->getPlayInfo( visiblePattern, psy::core::Player::Instance()->playPos() , 4 , entryStart );
            if ( isPlayPattern ) {
                patView_->onTick( entryStart );
            }			
        }
    }
}

void MainWindow::onOctaveComboBoxIndexChanged( int newIndex )
{
    patView_->setOctave( newIndex - 1 );
}
