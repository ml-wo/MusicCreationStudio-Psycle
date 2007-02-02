/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "player.h"
#include <iostream>
#include <cmath>

namespace psy {
  namespace core {

    Player::Player() 
      : outputPlugin_(0)
    {
    }

    Player::~Player()
    {
    }

    Player* Player::clone() const {
      return new Player(*this);
    }

    void Player::loadAudioOutPlugin( const std::string& path ) {
      if ( pluginLoad.open( path ) ) {
        void* (*gpi) (void);                
        gpi = reinterpret_cast<void*(__cdecl*)(void)>(pluginLoad.loadProcAdress("getPsyAudioOutPlugin"));
        outputPlugin_ = reinterpret_cast<PsyAudioOut*>(gpi());
      }
      if ( outputPlugin_ ) {
        std::cout << "Outputplugin is : " << outputPlugin_->name << std::endl;
        PsyAudioSettings settings;
        outputPlugin_->setCallback( Player::process, this );
        std::cout << outputPlugin_->settings()->samplesPerSec << std::endl;
        std::cout << outputPlugin_->settings()->channelSize << std::endl;
        if ( outputPlugin_->open() ) {
          std::cout << "device started" << std::endl;
          std::cout << "device has " << outputPlugin_->channelSize() << " channels" << std::endl;
        }
      }
    }

    int Player::process( unsigned int nframes, void *arg ) {
      Player* player = reinterpret_cast<Player*>(arg);
      return player->process( nframes );
    }

    int Player::process( unsigned int nframes ) {
      // produce a sin test wave
      int channelSize = outputPlugin_->channelSize();
      if ( channelSize == 2 ) {
        float* left  = outputPlugin_->buffer( 0 );
        float* right = outputPlugin_->buffer( 1 );
        for ( unsigned int i = 0; i < nframes; i++ ) {
          *left++ = sin(2*3.14/180*i);
          *right++ = sin(2*3.14/180*i);
        }
      }
      return 0;
    }

    // starts to play.
    void Player::start( double pos ) {
    }

    // stops playing.
    void Player::stop() {
    }

    // is the player in playmode.
    bool Player::playing() const {
      return true;
    }

    // set the actualPlaypos
    void Player::setPlayPos( double pos ) {
    }

    // the current playPos
    double Player::playPos() const {
      return 0;
    }

  }
}
