/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper, Johan Boule                  *
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
#ifndef ESOUNDOUT_H
#define ESOUNDOUT_H
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/esound_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_ESOUND
#include "audiodriver.h"
#include <pthread.h>
#include <exception>
namespace psycle
{
	namespace host
	{
		class ESoundOut : public AudioDriver
		{
			public:
				ESoundOut();
				~ESoundOut();
				
			public:
				virtual void Configure();

			public:
				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
				virtual bool Initialized();
			private:
				bool initialized_;
				
			public:
				virtual bool Enable(bool e);
			private:
				bool enabled_;

			private:
				unsigned int channels_;
				int channels_flag();
				
				unsigned int bits_;
				int bits_flag();
				
				unsigned int rate_;

				void setDefault();
				
				void open_output() throw(std::exception);
				std::string host_;
				int port_;
				std::string host_port();
				int output_;
				int fd_;

				pthread_t thread_id_;
				static void * thread_function_static(void *);
				void thread_function();
				bool stop_requested_;
				
				AUDIODRIVERWORKFN callback_;
				void * callback_context_;
				
				long device_buffer_;
				int read_buffer(char * buffer, long size);
				int write_buffer(char * buffer, long size);
		};
	}
}
#endif // !defined XPSYCLE__NO_ESOUND
#endif
