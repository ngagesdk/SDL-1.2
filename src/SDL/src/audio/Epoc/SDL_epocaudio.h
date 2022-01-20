/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@devolution.com
*/

/*
    SDL_epocaudio.cpp
    Epoc based SDL audio driver implementation

    Epoc version by Petteri Kallio and Hannu Viitala (hannu.j.viitala@mbnet.fi)
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_epocaudio.h,v 1.1.2.2 2001/02/10 07:20:03 hercules Exp $";
#endif

#ifndef _SDL_EPOCAUDIO_H
#define _SDL_EPOCAUDIO_H

extern "C" {
#include "SDL_sysaudio.h"
};

//#include "SDL_epocaudiosync.h"
//static CAudioSynchronizer *audioSync;

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_AudioDevice *thisdevice

struct SDL_PrivateAudioData {
	/* Raw mixing buffer */
	Uint8 *audiobuf;
	int    buflen;
    int    isSDLAudioPaused;
};

/* Variable names */
#define audiobuf		 (thisdevice->hidden->audiobuf)
#define buflen			 (thisdevice->hidden->buflen)
#define isSDLAudioPaused (thisdevice->hidden->isSDLAudioPaused)

#endif /* _SDL_EPOCAUDIO_H */
