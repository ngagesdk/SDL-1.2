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
 "@(#) $Id: SDL_epocaudio.c,v 0.0.0.0 2001/06/19 17:19:56 hercules Exp $";
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <e32hal.h>


extern "C" {
#include "SDL_audio.h"
#include "SDL_error.h"
#include "SDL_audiomem.h"
#include "SDL_audio_c.h"
#include "SDL_timer.h"
#include "SDL_audiodev_c.h"
};

#include "SDL_epocaudio.h"

#ifdef EPOC_AUDIOSYNC
#include "SDL_epocaudiosync.h"
#else
#include "eaudiolib.h"
#endif

#define EPOC_DRIVER_NAME         "epoc"

//#define DEBUG_AUDIO


/* Audio driver functions */

static int EPOC_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void EPOC_WaitAudio(_THIS);
static void EPOC_PlayAudio(_THIS);
static Uint8 *EPOC_GetAudioBuf(_THIS);
static void EPOC_CloseAudio(_THIS);
static void EPOC_ThreadInit(_THIS);

static int Audio_Available(void);
static SDL_AudioDevice *Audio_CreateDevice(int devindex);
static void Audio_DeleteDevice(SDL_AudioDevice *device);

#ifdef EPOC_AUDIOSYNC
#else
CEAudioLib* gAudioLib;
const TUid KSfxChannel = {4};
#endif


/* Audio driver bootstrap functions */

AudioBootStrap EPOCAudio_bootstrap = {
	EPOC_DRIVER_NAME, "EPOC streaming audio",
	Audio_Available, Audio_CreateDevice
};


static SDL_AudioDevice *Audio_CreateDevice(int /*devindex*/)
{
	SDL_AudioDevice *thisdevice;

	/* Initialize all variables that we clean on shutdown */
	thisdevice = (SDL_AudioDevice *)malloc(sizeof(SDL_AudioDevice));
	if ( thisdevice ) {
		memset(thisdevice, 0, (sizeof *thisdevice));
		thisdevice->hidden = (struct SDL_PrivateAudioData *)
				malloc((sizeof *thisdevice->hidden));
	}
	if ( (thisdevice == NULL) || (thisdevice->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( thisdevice ) {
			free(thisdevice);
		}
		return(0);
	}
	memset(thisdevice->hidden, 0, (sizeof *thisdevice->hidden));

	/* Set the function pointers */
	thisdevice->OpenAudio = EPOC_OpenAudio;
	thisdevice->WaitAudio = EPOC_WaitAudio;
	thisdevice->PlayAudio = EPOC_PlayAudio;
	thisdevice->GetAudioBuf = EPOC_GetAudioBuf;
	thisdevice->CloseAudio = EPOC_CloseAudio;
    thisdevice->ThreadInit = EPOC_ThreadInit;
	thisdevice->free = Audio_DeleteDevice;

	return thisdevice;
}


static void Audio_DeleteDevice(SDL_AudioDevice *device)
    {
	free(device->hidden);
	free(device);
    }

static int Audio_Available(void)
{
	return(1); // Audio stream modules should be always there!
}


LOCAL_C void CreateAudiolibL()
    {
   
//#ifdef SYMBIAN_CRYSTAL
//    const TBool stereo = ETrue; 
//    const TInt rate = 22050;    
//#else
    const TBool stereo = EFalse; 
    const TInt rate = 16000;
//#endif
    gAudioLib = CEAudioLib::NewL(TChannelType(stereo, rate));
    }

static int EPOC_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	SDL_TRACE("SDL:EPOC_OpenAudio");

    spec->format = AUDIO_S16LSB;
	spec->channels = 1;

//#ifdef SYMBIAN_CRYSTAL
    //spec->freq = 8000; //! This is just for 9210(i)
    spec->freq = 22050;	// 9300/9500
//#else
//    spec->freq = 16000;
//#endif
//    spec->freq = 11025; 

    SDL_CalculateAudioSpec(spec);

#ifdef EPOC_AUDIOSYNC
    audioSync=new(ELeave)CAudioSynchronizer(spec->samples);
	if ( audioSync == NULL ) {
		return(-1);
    
	}
#else
    delete gAudioLib;
    gAudioLib=NULL;
    
    TRAPD(err, CreateAudiolibL());
    if(err != KErrNone)
        return -1;
#endif
    

    

	/* Allocate mixing buffer */
	buflen = spec->size;
	audiobuf = NULL;
    
	audiobuf = (Uint8 *)SDL_AllocAudioMem(buflen * 2/* * KMaxSoundBufsInOneWrite*/);
	if ( audiobuf == NULL ) {
		return(-1);
	}
	memset(audiobuf, spec->silence, buflen);

#ifndef EPOC_AUDIOSYNC
    TChannelType type(EFalse, 22050);
    TRAP(err, gAudioLib->ChannelPlayer().OpenChannelL(KSfxChannel, type));
    if(err != KErrNone)
        return -1;
#endif

    isSDLAudioPaused = 1;

    thisdevice->enabled = 0; /* enable only after audio engine has been initialized!*/

	/* We're ready to rock and roll. :-) */
	return(0);
}


static void EPOC_CloseAudio(_THIS)
    {
#ifdef DEBUG_AUDIO
    SDL_TRACE("Close audio\n");
#endif
    gAudioLib->ChannelPlayer().WaitCompleted(KSfxChannel, MChannelPlayer::EBufferComplete);
    delete gAudioLib;
    gAudioLib = NULL;

	SDL_TRACE("SDL:EPOC_CloseAudio");
	if ( audiobuf != NULL ) {
		SDL_FreeAudioMem(audiobuf);
		audiobuf = NULL;
	}
/*
#ifdef EPOC_AUDIOSYNC
    if (audioSync)
        audioSync->StopThread();

    delete audioSync;
#else
    delete gAudioLib;
#endif*/
}


static void EPOC_ThreadInit(_THIS)
    {
	SDL_TRACE("SDL:EPOC_ThreadInit");
/*	CTrapCleanup* cleanup = CTrapCleanup::New(); // get clean-up stack
	if ( audioSync!=NULL )
        {
        audioSync->StartThread();
        thisdevice->enabled = 1;
    	}
    delete cleanup;*/
    RThread().SetPriority(EPriorityMore);
    thisdevice->enabled = 1;
    }

/* This function waits until it is possible to write a full sound buffer */
static void EPOC_WaitAudio(_THIS)
{
#ifdef DEBUG_AUDIO
    SDL_TRACE1("wait %d audio\n", gAudioLib->ChannelPlayer().SyncTime(KSfxChannel));
    TInt tics = User::TickCount();
#endif
#ifdef EPOC_AUDIOSYNC   
    audioSync->WaitForSync();
#else
//#ifdef AUDIOLIB_DOSYNC
    gAudioLib->ChannelPlayer().WaitCompleted(KSfxChannel, MChannelPlayer::ESynchronized);
//#endif
#endif
#ifdef DEBUG_AUDIO
    TInt ntics =  User::TickCount() - tics;
    SDL_TRACE1("audio waited %d\n", ntics);
    SDL_TRACE1("audio at %d\n", tics);
#endif
}


 
static void EPOC_PlayAudio(_THIS)
{
if (SDL_GetAudioStatus() == SDL_AUDIO_PAUSED)
        {
#ifndef EPOC_AUDIOSYNC
        if(!isSDLAudioPaused)
            {
            gAudioLib->SetPause(ETrue);
            }
        else
             SDL_Delay(500); //hold on the busy loop
#endif
        isSDLAudioPaused = 1; // Continue to play as "silence" is feeded to audio stream
#ifndef EPOC_AUDIOSYNC
        return; //audiolib handles silence feeding by nature
#endif
        }
    else
        {
        if (isSDLAudioPaused) // If we are quitting pause, reset audio stream
            {
#ifdef EPOC_AUDIOSYNC
            SDL_Delay(500); // Drain audio stream
            audioSync->ResetSync(); //Reset sync counters, timers, etc. after pause
#else
            gAudioLib->SetPause(EFalse);
#endif
            }
        isSDLAudioPaused = 0;
        }
	/* Write the audio data */

#ifdef DEBUG_AUDIO
    SDL_TRACE("buffer has audio data\n");
#endif
#ifdef EPOC_AUDIOSYNC 

    
    
   

    // Play the data. Blocks the execution until the buffer is copied to the audio stream server.
   audioSync->iBuffer->Set(audiobuf, buflen, buflen * KMaxSoundBufsInOneWrite);
   audioSync->BlockingWriteRequest();
#else
    const TPtrC8 buffer(audiobuf, buflen);
    gAudioLib->ChannelPlayer().WriteSound(buffer, KSfxChannel);
#endif
#ifdef DEBUG_AUDIO
	SDL_TRACE1("Wrote %d bytes of audio data\n", buflen);
#endif
}

static Uint8 *EPOC_GetAudioBuf(_THIS)
{
	return(audiobuf);
}
