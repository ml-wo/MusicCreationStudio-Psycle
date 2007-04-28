/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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

#include "psycore/song.h"
#include "psycore/player.h"
#include "psycore/sampler.h"
#include "psycore/constants.h"
#include "psycore/machine.h"
#include "psycore/pluginfinder.h"
#include "psycore/patternevent.h"


#include "machinegui.h"
#include "machineview.h"
#include "mastergui.h"
#include "effectgui.h"
#include "wiregui.h"
#include "newmachinedlg.h"

#include <QtGui/QGraphicsScene>
#include <QPainter>
#include <iostream>
#include <QGraphicsLineItem>

MachineView::MachineView(psy::core::Song *song)
{
    song_ = song;
     scene_ = new MachineScene(this);
     scene_->setBackgroundBrush(Qt::black);

     setDragMode(QGraphicsView::RubberBandDrag);
     setSceneRect(0,0,width(),height());
     setScene(scene_);
     setBackgroundBrush(Qt::black);


     // A temporary line to display when user is making a new connection.
     tempLine_ = new QGraphicsLineItem(0, 0, 0, 0);
     tempLine_->setPen(QPen(Qt::gray,2,Qt::DashLine));
     tempLine_->setVisible(false);// We don't want it to be visible yet.
     scene_->addItem(tempLine_);

    // Create MachineGuis for the Machines in the Song.
    for(int c=0;c<psy::core::MAX_MACHINES;c++)
    {
        psy::core::Machine* mac = song_->_pMachine[c];
        if ( mac ) {
            createMachineGui( mac );
        }
    }
    // Create WireGuis for connections in Song file.
    for(int c=0;c<psy::core::MAX_MACHINES;c++)
    {
        psy::core::Machine* tmac = song_->_pMachine[c];
        if (tmac) for ( int w=0; w < psy::core::MAX_CONNECTIONS; w++ )
        {
            if (tmac->_connection[w]) {
                MachineGui* srcMacGui = findByMachine(tmac);
                if ( srcMacGui!=0 ) {
                    psy::core::Machine *pout = song_->_pMachine[tmac->_outputMachines[w]];
                    MachineGui* dstMacGui = findByMachine(pout);
                    if ( dstMacGui != 0 ) {
                        WireGui *wireGui = createWireGui( srcMacGui, dstMacGui );
                        scene_->addItem( wireGui );
                    }
                }
            }
        }
    }

    outtrack = 0;
    notetrack[psy::core::MAX_TRACKS];
    for ( int i=0; i<psy::core::MAX_TRACKS; i++ ) notetrack[i]=120;

 }

WireGui *MachineView::createWireGui( MachineGui *srcMacGui, MachineGui *dstMacGui )
{
    WireGui *wireGui = new WireGui(srcMacGui, dstMacGui, this);
/*    connect( wireGui, SIGNAL( startRewiringDest( WireGui* ) ), 
             this, SLOT( startRewiringDest( WireGui* ) ) );*/
    return wireGui;
}

 void MachineView::keyPressEvent(QKeyEvent *event)
 {
     switch (event->key()) {
     case Qt::Key_Plus:
         scaleView(1.2);
         break;
     case Qt::Key_Minus:
         scaleView(1 / 1.2);
         break;
     default:
         QGraphicsView::keyPressEvent(event);
     }
 }


 void MachineView::scaleView(qreal scaleFactor) 
 {
      qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
     if (factor < 0.07 || factor > 100)
         return;

     scale(scaleFactor, scaleFactor);
 }

void MachineView::startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
{
    tempLine_->setLine( QLineF( srcMacGui->centrePointInSceneCoords(), event->scenePos() ) );
    tempLine_->setVisible(true);
}

void MachineView::closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
{
    // See if we hit another machine gui.
    if ( scene_->itemAt( tempLine_->mapToScene( tempLine_->line().p2() ) )  ) {
        QGraphicsItem *itm = scene_->itemAt( tempLine_->mapToScene( tempLine_->line().p2() ) );
        if (itm->type() == 65537) { // FIXME: un-hardcode this
           MachineGui *dstMacGui = qgraphicsitem_cast<MachineGui *>(itm);
           connectMachines(srcMacGui, dstMacGui); 
        }
    }
    tempLine_->setVisible(false);     // We want the tempLine to disappear, whatever happens.
}

/*void MachineView::startRewiringDest( WireGui *wireGui, QGraphicsSceneMouseEvent *event )
{
    qDebug( "rewire dest" );
}*/

void MachineView::connectMachines( MachineGui *srcMacGui, MachineGui *dstMacGui )
{
   if ( dstMacGui->mac()->acceptsConnections() ) {
       // Check there's not already a connection.
       // ...
       
       // Make a connection in the song file..
       song_->InsertConnection( srcMacGui->mac()->_macIndex , dstMacGui->mac()->_macIndex, 1.0f);
       
       // Make a new wiregui connection.
       WireGui *newWireGui = createWireGui( srcMacGui, dstMacGui );
       scene_->addItem( newWireGui );
   }
}

void MachineView::onDeleteMachineRequest( MachineGui *macGui )
{
    int index = macGui->mac()->_macIndex;

    // Remove machine and connections from the Song. 
    song()->DestroyMachine( index );
    scene()->removeItem( macGui );

    // Remove machine and connections from the gui. 
    foreach ( WireGui *wireGui, macGui->wireGuiList() ) {
        scene()->removeItem( wireGui );
    }

    emit machineDeleted( index ); 
}

void MachineView::onMachineRenamed()
{
    emit machineRenamed();
}

void MachineView::deleteConnection( WireGui *wireGui )
{
    // Delete the connection in the song file.
    psy::core::Player::Instance()->lock();
    psy::core::Machine *srcMac = wireGui->sourceMacGui()->mac();
    psy::core::Machine *dstMac = wireGui->destMacGui()->mac();
    srcMac->Disconnect( *dstMac );
    psy::core::Player::Instance()->unlock();

    // Delete the connection in the GUI.
    scene_->removeItem( wireGui ); // FIXME: do we need to do more here?
}

MachineGui *MachineView::findByMachine( psy::core::Machine *mac )
{
    for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
        MachineGui* machineGui = *it;
        if ( machineGui->mac() == mac ) return machineGui;
    }
    return 0;
}

MachineGui *MachineView::findMachineGuiByMachineIndex( int index )
{
    for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
        MachineGui* machineGui = *it;
        if ( machineGui->mac()->_macIndex == index ) return machineGui;
    }
    return 0;
}

psy::core::Song *MachineView::song()
{
    return song_;
}

void MachineView::PlayNote( int note,int velocity,bool bTranspose, psy::core::Machine *pMachine )
{

    // stop any notes with the same value
    StopNote(note,bTranspose,pMachine);

    if(note<0) return;

    // octave offset
    if(note<120) {
        if(bTranspose)
            note+= song()->currentOctave*12;
        if (note > 119)
            note = 119;
    }

    // build entry
    psy::core::PatternEvent entry;
    entry.setNote( note );
    entry.setInstrument( song()->auxcolSelected );
    entry.setMachine( song()->seqBus );	// Not really needed.

    entry.setCommand( 0 );
    entry.setParameter( 0 );

    // play it
    if(pMachine==NULL)
    {
        int mgn = song()->seqBus;

        if (mgn < psy::core::MAX_MACHINES) {
            pMachine = song()->_pMachine[mgn];
        }
    }

    if (pMachine) {
        // pick a track to play it on	
        //        if(bMultiKey)
        {
            int i;
            for (i = outtrack+1; i < song()->tracks(); i++)
            {
                if (notetrack[i] == 120) {
                    break;
                }
            }
            if (i >= song()->tracks()) {
                for (i = 0; i <= outtrack; i++) {
                    if (notetrack[i] == 120) {
                        break;
                    }
                }
            }
            outtrack = i;
        }// else  {
        //   outtrack=0;
        //}
        // this should check to see if a note is playing on that track
        if (notetrack[outtrack] < 120) {
            StopNote(notetrack[outtrack], bTranspose, pMachine);
        }

        // play
        notetrack[outtrack]=note;
        pMachine->Tick(outtrack, entry );
    }
}

void MachineView::StopNote( int note, bool bTranspose, psy::core::Machine * pMachine )
{

    int notetrack[psy::core::MAX_TRACKS];
    for ( int i=0; i<psy::core::MAX_TRACKS; i++ ) notetrack[i]=120;

    if (!(note >=0 && note < 128)) return;

    // octave offset
    if(note<120) {
        if(bTranspose) note+=song()->currentOctave*12;
        if (note > 119) note = 119;
    }

    if(pMachine==NULL) {
        int mgn = song()->seqBus;

        if (mgn < psy::core::MAX_MACHINES) {
            pMachine = song()->_pMachine[mgn];
        }

        for(int i=0; i<song()->tracks(); i++) {
            if(notetrack[i]==note) {
                notetrack[i]=120;
                // build entry
                psy::core::PatternEvent entry;
                entry.setNote( 120+0 );
                entry.setInstrument( song()->auxcolSelected );
                entry.setMachine( song()->seqBus );
                entry.setCommand( 0 );
                entry.setParameter( 0 );

                // play it
                if (pMachine) {
                    pMachine->Tick( i, entry );
                }
            }
        }

    }
}

void MachineView::createMachineGui( psy::core::Machine *mac )
{
    MachineGui *macGui;
    switch ( mac->mode() ) {							
        case psy::core::MACHMODE_GENERATOR:
            macGui = new GeneratorGui(mac->GetPosX(), mac->GetPosY(), mac, this );
        break;
        case psy::core::MACHMODE_FX:
            macGui = new EffectGui(mac->GetPosX(), mac->GetPosY(), mac, this );
        break;
        case psy::core::MACHMODE_MASTER: 
            macGui = new MasterGui(mac->GetPosX(), mac->GetPosY(), mac, this);
        break;
        default:
            macGui = 0;
    }
    connect( macGui, SIGNAL( chosen( MachineGui* ) ), 
             this, SLOT( onMachineGuiChosen( MachineGui* ) ) );
    connect( macGui, SIGNAL( deleteRequest( MachineGui* ) ),
             this, SLOT( onDeleteMachineRequest( MachineGui* ) ) );
    connect( macGui, SIGNAL( renamed() ),
             this, SLOT( onMachineRenamed() ) );
    scene_->addItem(macGui);
    machineGuis.push_back(macGui);
}

void MachineView::onMachineGuiChosen( MachineGui *macGui )
{
    setTheChosenOne( macGui );
    emit machineGuiChosen( macGui );
    update();
}

int MachineView::octave() const
{
    return octave_;
}

void MachineView::setOctave( int newOctave )
{
    octave_ = newOctave;
}

MachineScene::MachineScene( MachineView *macView )
    : QGraphicsScene( macView )
{
    macView_ = macView;
    newMachineDlg = new NewMachineDlg();
}

void MachineScene::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{ 
    QGraphicsScene::mouseDoubleClickEvent( event );
    if ( !event->isAccepted() ) // Check whether one of the items on the scene ate it.
    {
        int accepted = newMachineDlg->exec();
        if (accepted) { // Add a new machine to the song.
             psy::core::PluginFinderKey key = newMachineDlg->pluginKey(); 

            // Create machine, tell where to place the new machine--get from mouse.	  
            psy::core::Machine *mac = macView_->song()->createMachine( pluginFinder_, key, event->scenePos().x(), event->scenePos().y() );
            if ( mac ) {
                macView_->createMachineGui( mac );
                emit newMachineCreated( mac );
                update();
            }
        } 
    }
}

