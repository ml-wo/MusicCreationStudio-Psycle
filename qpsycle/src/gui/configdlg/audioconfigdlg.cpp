// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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
#include <qpsyclePch.hpp>

#include <psycle/core/player.h>
#include <psycle/audiodrivers/audiodriver.h>

#include "audioconfigdlg.hpp"
#include "../global.hpp"
#include "../configuration.hpp"

#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QKeyEvent> // QKeyEvent
#include <QLabel>
#include <QMessageBox>

namespace qpsycle {

AudioConfigDlg::AudioConfigDlg( QWidget *parent )
	: QDialog( parent )
{
	setWindowTitle(tr("Select Audio Driver"));
	config_ = Global::pConfig();
	selectedDriver_ = 0;

	QGridLayout *mainLay = new QGridLayout();
		
	//QWidget *settingsPanel = new QWidget( this );
	//QWidget *notes = new QWidget( this );

	driverCbx_ = new QComboBox( this );
	driverLbl_ = new QLabel( tr("Driver"), this );
	deviceBox_ = new QLineEdit( "", this );
	deviceLbl_ = new QLabel( tr("Device"), this );

	restartBtn_ = new QPushButton( tr("Restart Driver"), this );
	QPushButton *closeBtn = new QPushButton( tr("Close"), this );


	connect( driverCbx_, SIGNAL( currentIndexChanged( const QString & ) ),
			this, SLOT( onDriverSelected( const QString & ) ) );
	connect( restartBtn_, SIGNAL( clicked() ),
			this, SLOT( onRestartDriver() ) );
	connect( closeBtn, SIGNAL( clicked() ),
			this, SLOT( reject() ) );

	mainLay->addWidget( driverLbl_, 0, 0 );
	mainLay->addWidget( deviceLbl_, 1, 0 );
	mainLay->addItem( new QSpacerItem( 5, 0 ), 0, 1);
	mainLay->addItem( new QSpacerItem( 5, 0 ), 1, 1);
	mainLay->addWidget( driverCbx_, 0, 2 );
	mainLay->addWidget( deviceBox_, 1, 2 );
	mainLay->addItem( new QSpacerItem( 15,0 ), 0, 3);
	mainLay->addItem( new QSpacerItem( 15,0 ), 1, 3);
	mainLay->addWidget( restartBtn_, 0, 4 );
	mainLay->addWidget( closeBtn, 1, 4 );

	mainLay->setColumnStretch(2, 1);

	setLayout( mainLay );

	initDriverList();
}

void AudioConfigDlg::initDriverList( )
{
	std::map<std::string, psy::core::AudioDriver*> & driverMap =  config_->driverMap();
	std::map<std::string, psy::core::AudioDriver*>::iterator it = driverMap.begin();
	for ( ; it != driverMap.end(); it++ ) {
		if ( !it->second->info().show() )
			continue;

		QString driverName = QString::fromStdString( it->first );
		driverCbx_->addItem( driverName );
		if ( it->second == Global::pConfig()->_pOutputDriver ) {
			driverCbx_->setCurrentIndex(driverCbx_->count()-1);
			if ( driverName == "alsa" )
				deviceBox_->setText( it->second->settings().deviceName().c_str() );
		}
	}
}

void AudioConfigDlg::keyPressEvent( QKeyEvent *event)
{
	if ( event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier ) {
		reject(); // closes the dialog
	}
	else if ( event->key() == Qt::Key_Escape) {
		reject(); // close the dialog
	}
}

void AudioConfigDlg::onDriverSelected( const QString & text )
{
	std::map<std::string, psy::core::AudioDriver*> & driverMap =  config_->driverMap();
	std::map<std::string, psy::core::AudioDriver*>::iterator it = driverMap.find( text.toStdString() );
	if ( it != driverMap.end() ) {
		psy::core::AudioDriver* driver = it->second;
		selectedDriver_ = driver;
		if ( text == "alsa" ) {
			deviceBox_->setText( it->second->settings().deviceName().c_str() );
			deviceBox_->setVisible(true);
			deviceLbl_->setVisible(true);
		} else {
			deviceBox_->setText( "" );
			deviceBox_->setVisible(false);
			deviceLbl_->setVisible(false);
		}
	}
}

void AudioConfigDlg::onRestartDriver()
{
	if ( selectedDriver_ ) {
		// disable old driver
		psy::core::Player::Instance()->driver().Enable( false );
		// set the device
		if ( ! deviceBox_->text().isEmpty() ) {
			psy::core::AudioDriverSettings settings = selectedDriver_->settings();
			settings.setDeviceName( deviceBox_->text().toStdString() );
			selectedDriver_->setSettings(settings);
		}
		// set new Driver to Player
		psy::core::Player::Instance()->setDriver( *selectedDriver_ );

		//check that it worked
		///\todo find a better/more descriptive way to do this
		if (
			psy::core::Player::Instance()->driver().info().name() == "silent" &&
			selectedDriver_->info().name() != "silent"
		) {
			std::string drivername = selectedDriver_->info().name();
			QString msg = QString(tr("The %1 driver failed to load."))
				.arg( drivername.c_str() );

			if ( drivername == "alsa" )
				msg.append( tr(" Did you specify a valid alsa device name?") );
			else if ( drivername == "jack" )
				msg.append( tr(" Are you sure jackd is running?") );

			QMessageBox::warning(
				this, tr("Driver Restart Failed"), msg,
				QMessageBox::Ok, QMessageBox::Ok
			);

		}
	}
}

} // namespace qpsycle
