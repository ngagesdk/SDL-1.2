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
    SDL_epocaudiosync.cpp
    Audio synchronizer for Epoc Media Server Streaming Audio

    Coded by Hannu Viitala (hannu.j.viitala@mbnet.fi)
    Thanks to Petteri Kallio for help!
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_epocaudiosync.h,v 1.1.2.2 2001/02/10 07:20:03 hercules Exp $";
#endif

#ifndef __EPOC_AUDIO_SYNC_H__
#define __EPOC_AUDIO_SYNC_H__
#include <mda/common/audio.h>
#include <mdaaudiooutputstream.h>

#include <flogger.h>
#define DEBUG_PRINT_COUNTER 60

//#define EPOC_AUDIO_SYNC_DEBUG
//#define EPOC_AUDIO_SYNC_DEBUG_A
#define TRACE_TO_FILE

#ifdef TRACE_TO_FILE
  #define _TRACE(x)         RFileLogger::Write(_L("SDLAudio"), _L("SDLAudio.txt"), EFileLoggingModeAppend, (x));
  #define _TRACE2(x,y)      RFileLogger::WriteFormat(_L("SDLAudio"), _L("SDLAudio.txt"), EFileLoggingModeAppend, (x), (y));
#else
  #define _TRACE(x)         RDebug::Print((x))
  #define _TRACE2(x,y)      RDebug::Print((x),(y))
#endif
#ifdef EPOC_AUDIO_SYNC_DEBUG
  #define TRACE(x)      _TRACE(x)
  #define TRACE2(x,y)   _TRACE2(x,y)
#else
  #define TRACE(x)    
  #define TRACE2(x,y)
#endif
#ifdef EPOC_AUDIO_SYNC_DEBUG_A
  #define TRACE_A(x)    _TRACE(x)
  #define TRACE2_A(x,y) _TRACE2(x,y)
#else
  #define TRACE_A(x)    
  #define TRACE2_A(x,y)
#endif


const TInt KMaxSoundBufsInOneWrite = 2;

#ifdef SYMBIAN_CRYSTAL
const TInt KSampleFreq = 8000;
#else
const TInt KSampleFreq = 16000;
#endif

const TInt KLatencyResetLimit = 40;

// Only for debugging
const TInt KDebugMaxLeadIndex = 200;
struct AudioDebugData {
    TInt iTickPeriod_ys2;
    TUint iCountSamplesCopied;
    TUint iPrevCountSamplesCopied;
    TUint iCountLeadTooLate;
    TUint iCountLeadTooFast;
    TUint iPrevCountLeadTooLate;
    TUint iPrevCountLeadTooFast;
    TUint iCountNormalBlocks;
    TUint iPrevCountNormalBlocks;
    TUint iCountStarvation;
    TInt iPlayCompleteCounter;
    TInt iBufferLengthTooBigCounter;
    TUint iPrevTickCount;
    TTimeIntervalMicroSeconds iBufferLength;
    TInt iLeadInBlocks;
    TInt iDebugPrintCounter;
	TInt iLeadValues[KDebugMaxLeadIndex];
	TInt iLeadIndex;
    } ;

class CAudioSynchronizer : public CActive, public MMdaAudioOutputStreamCallback 
{
public:

    CAudioSynchronizer(TInt aBlockSampleCount);
    ~CAudioSynchronizer();
    int StartThread();
    void StopThread();
    void BlockingWriteRequest();
    void WaitForSync();
    void ResetSync();

public:
    int MainL(); // Called only internally!

private:  
    void RunL();
    void DoCancel();
    TBool Synchronize();
    void SetVolumeL(TInt aVolume);

    // From MMdaAudioOutputStreamCallback
	void MaoscOpenComplete(TInt aError);
	void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);
	void MaoscPlayComplete(TInt aError);

public:
    TPtr8* iBuffer;

private:

    enum TAudioSyncState {EAudioSyncNotReady, EAudioSyncInitialised, EAudioSyncInitialisedAll, 
                 EAudioSyncReady, EAudioSyncCopying, EAudioSyncPlaying, EAudioSyncStopped};

    RThread iClientThread;
    RThread iAudioSyncThread;

    RSemaphore iSemReadyForNextWrite;
    RCriticalSection iLock;
    RCriticalSection iLockDuringStartup;
    RCriticalSection iAllInitialized;
  
    // Audio sync variables
    TInt iBlockSampleCount; 
    TUint iStartPlayTickCount;
    TUint iCountSampleBlocksCopied;
    TInt iTickPeriod_ys;
    TUint iBlockDuration_ys;
	TUint iBlockDurationInTics;
	TUint iMomentOfWriteInTics;
    TUint iWaitDelay_ys;
	TUint iLddBufferFullCounter;
	TBool iIsLddBufferFull;
    TBool iStarvation;

    AudioDebugData iDebugData;

    CActiveScheduler* iScheduler;
    CMdaAudioOutputStream* iAStream;

	TMdaAudioDataSettings iSettings;
    TBool iCloseEngine;
    TAudioSyncState iDspState;
    TInt iVolume;
};


#endif
