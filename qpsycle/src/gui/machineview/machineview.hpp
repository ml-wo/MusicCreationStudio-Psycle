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
#ifndef MACHINEVIEW_H
#define MACHINEVIEW_H

namespace psy { namespace core {
	class Song;
	class Machine;
	class PatternEvent;
}}

#include <psycle/core/constants.h>
#include <psycle/core/pluginfinder.h>
#include <psycle/core/machine.h>

#include <vector>

#include <QDialog>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QPushButton>

namespace qpsycle {

class NewMachineDlg;
class MachineGui;
class WireGui;

class MachineView : public QGraphicsView {
	Q_OBJECT

	public:
		MachineView( psy::core::Song *song_ );

		void addNewMachineGui( psy::core::Machine *mac );

		MachineGui* findMachineGuiByCoreMachineIndex( int index ) const;
		MachineGui* findMachineGuiByCoreMachine( psy::core::Machine *mac ) const;
		MachineGui *machineGuiAtPoint( QPointF point ) const;

		void playNote( int note, int velocity, bool bTranspose, psy::core::Machine*pMachine);
		void stopNote( int note, bool bTranspose=true, psy::core::Machine* pMachine=NULL);

	public slots:
		void onNotePress( int note, psy::core::Machine* mac );
		void onNoteRelease( int note, psy::core::Machine* mac );

	public:
		psy::core::Song *song() const { return song_; }
		void setSong( psy::core::Song *song ) { song_ = song; }
		
		void setChosenMachine( MachineGui* macGui ) { chosenMachine_ = macGui; }
		MachineGui* chosenMachine() const { return chosenMachine_; }

		int octave() const { return octave_; }
		void setOctave( int newOctave ) { octave_ = newOctave; }

		bool isCreatingWire() const { return creatingWire_; }

	protected:
		void keyPressEvent(QKeyEvent *event);
		void scaleView(qreal scaleFactor);

	public slots:
		void startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
		void closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
		void deleteConnection( WireGui *wireGui );
		void onMachineChosen( MachineGui *macGui );
		void onDeleteMachineRequest( MachineGui *macGui );
		void onMachineRenamed();
		void onCloneMachine( MachineGui *macGui );

	signals:
		void machineChosen( MachineGui *macGui );
		void machineDeleted( int macIndex );
		void machineRenamed();
		void newMachineCreated( psy::core::Machine* mac );

	private:
		void createMachineGuis();
		void createWireGuis();
		void createTempLine();
		void initKeyjazzSettings();

		void connectMachines(MachineGui *srcMacGui, MachineGui *dstMacGui );
		WireGui* createWireGui( MachineGui *srcMacGui, MachineGui *dstMacGui );
		MachineGui * createMachineGui( psy::core::Machine *mac );

		MachineGui *chosenMachine_;

		psy::core::Song *song_;
		std::vector<MachineGui*> machineGuis;

		int octave_;
	
		QGraphicsScene *scene_;
		QGraphicsLineItem *tempLine_;

		bool creatingWire_;
	
		/// For multi-key playback state.
		int notetrack[psy::core::MAX_TRACKS];
		int outtrack;
};

class QPortButton : public QPushButton {
	Q_OBJECT
	public:
		QPortButton( const QString & text, QWidget * parent = 0 ):QPushButton(text,parent) {}

		void setIndex(int index) { myindex = index; }
		int getIndex() { return myindex; }

	public slots:
		void iamclicked() { emit myclicked(myindex); }
	signals:
		void myclicked(int index);
	protected:
		int myindex;
};

class QPortsDialog : public QDialog {
	Q_OBJECT
	public:
		QPortsDialog(QWidget *parent = 0);

		psy::core::InPort::id_type GetInPort(psy::core::Machine* mac);
		psy::core::OutPort::id_type GetOutPort(psy::core::Machine* mac);

	public slots:
		void buttonClicked(int portidx);
	private:
		void addNewButton(std::string message,int portidx);

		int numButtons;
};

class MachineScene : public QGraphicsScene {
	Q_OBJECT
	public:
		MachineScene( MachineView *macView );
	protected:
		void keyPressEvent( QKeyEvent *event );
		void keyReleaseEvent( QKeyEvent *event );
		void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );

	private:
		MachineView *macView_;
		NewMachineDlg *newMachineDlg;
};

} // namespace qpsycle

#endif
