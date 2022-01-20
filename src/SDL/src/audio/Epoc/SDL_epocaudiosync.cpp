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
 "@(#) $Id: SDL_epocaudiosync.cpp,v 1.1.2.2 2001/02/10 07:20:03 hercules Exp $";
#endif

#include "SDL_epocaudiosync.h"

#include <e32svr.h>

extern "C" {

//!! DEBUG
int GDEBUG_audio_stream_max_latency = -999; 
int GDEBUG_print_next_debug = 0; 
int GDEBUG_new_volume = 0;
int GDEBUG_reset_sync = 0;
};

TInt KLatencyMax = 3;


LOCAL_C TInt ThreadFuncL(TAny* aAudioSync)
{
    return(((CAudioSynchronizer*)aAudioSync)->MainL());   
}

CAudioSynchronizer::CAudioSynchronizer(TInt aBlockSampleCount)
    : CActive(EPriorityLow),iBlockSampleCount(aBlockSampleCount), iDspState(EAudioSyncNotReady)
{
    //!!DEBUG
    if (GDEBUG_audio_stream_max_latency != -999)
        KLatencyMax = GDEBUG_audio_stream_max_latency;

    TRACE_A(_L("CAudioSynchronizer::CAudioSynchronizer"));
    iSemReadyForNextWrite.CreateLocal(0); // "Not yet ready"
    iLock.CreateLocal();
	iLockDuringStartup.CreateLocal();
	iLockDuringStartup.Wait(); // Enable lock 
}

int CAudioSynchronizer::StartThread()
{
    TRACE_A(_L("CAudioSynchronizer::StartThread"));
    // Where is iAudioSyncThread closed?
	TInt status = iAudioSyncThread.Create(_L("SDL_Audio_thread"),ThreadFuncL,
			KDefaultStackSize, NULL, 
			this);

    if (status != KErrNone) 
    {
		//SDL_SetError("Not enough resources to create thread");
		return(-1);
	}
	iAudioSyncThread.Resume();
	iLockDuringStartup.Wait(); // Wait until audio sync thread startup is finished
	return(0);
}

void CAudioSynchronizer::StopThread()
    {
    TRACE_A(_L("CAudioSynchronizer::StopThread"));
    // Check that only client thread is calling this function, not Audio Sync thread!
    RThread currentThread;
    if (currentThread.Id() != iClientThread.Id())
        User::Panic(_L("SDLAUDIO"), 1);

    iLock.Wait();

    RUndertaker taker;
    taker.Create();
    TRequestStatus playerThreadStatus;
    TInt handle = iAudioSyncThread.Handle();
    taker.Logon(playerThreadStatus, handle);

    // Send close request
    iCloseEngine = ETrue;
    TRequestStatus* status = &iStatus;
    iAudioSyncThread.RequestComplete(status,KErrNone);
    iLock.Signal();

    // Wait for audio player thread to exit
    TRACE_A(_L("CAudioSynchronizer::StopPlayerThread: WaitForRequest"));
    User::WaitForRequest(playerThreadStatus);
    TRACE_A(_L("CAudioSynchronizer::StopPlayerThread: Close()"));
	taker.Close();
    TRACE_A(_L("CAudioSynchronizer::StopPlayerThread: end"));
    }

int CAudioSynchronizer::MainL()
{
    TRACE_A(_L("CAudioSynchronizer::MainL"));

    // Initialize

	CTrapCleanup* cleanup = CTrapCleanup::New();
    cleanup; // disable compiler warning

	iScheduler = new (ELeave) CActiveScheduler;
	CActiveScheduler::Install( iScheduler );

    // Add this AO to Scheduler
    CActiveScheduler::Add(this);
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);

    iBuffer = new TPtr8(NULL,0);

    ResetSync();

    iLock.Wait();
    TTimeIntervalMicroSeconds32 period;
    UserHal::TickPeriod(period);
    iTickPeriod_ys = period.Int();
    iBlockDuration_ys = (1000000 * iBlockSampleCount) / KSampleFreq;
    iBlockDurationInTics = iBlockDuration_ys / iTickPeriod_ys;
	iDspState = EAudioSyncInitialised;
    iLock.Signal();

    // Start scheduler
    TRACE_A(_L("CAudioSynchronizer::MainL: CActiveScheduler::Start()"));
    CActiveScheduler::Start();
    TRACE_A(_L("CAudioSynchronizer::MainL: CActiveScheduler::Start() ended"));

    // Cleanup
	if (iAStream)
        {
		delete iAStream;
        iAStream = NULL;
        }
    delete iBuffer;
    iBuffer = NULL;
    delete iScheduler;
    iScheduler = NULL;
   	delete cleanup;									
    TRACE_A(_L("CAudioSynchronizer::MainL: ended"));
    return 0;
}


CAudioSynchronizer::~CAudioSynchronizer()
{
    TRACE_A(_L("CAudioSynchronizer::~CAudioSynchronizer"));
    iLock.Close();
    iSemReadyForNextWrite.Close();
}

// This function should handle data exclusively! It is called from another (client) thread.
void CAudioSynchronizer::BlockingWriteRequest()
    {
    iLock.Wait();

    TRACE(_L("CAudioSynchronizer::BlockingWriteRequest"));

    // Check that only client thread is calling this function, not the audio sync thread!
    RThread currentThread;
    if (currentThread.Id() != iClientThread.Id())
        User::Panic(_L("SDLAUDIO"), 1);

    if (iDspState==EAudioSyncCopying) // Still copying previous sample to audio server?
        {
        // Ask player to signal when copying is finished
        TRACE(_L("CAudioSynchronizer::BlockingWriteRequest: iSemReadyForNextWrite.Wait()"));
        iLock.Signal();
        iSemReadyForNextWrite.Wait();
        iLock.Wait();
        }

    if (iDspState==EAudioSyncPlaying || iDspState==EAudioSyncReady) // Copying is done or initialization is done ?
        {
		
		// Issue a request of playing a new sample

        TRACE(_L("CAudioSynchronizer::BlockingWriteRequest: Issue request"));
		
		if ( iIsLddBufferFull ) 
			{
			iIsLddBufferFull = EFalse;
			User::After(TTimeIntervalMicroSeconds32(2000 *1000)); // Drain audio driver
			iLock.Signal();
			ResetSync();
			iLock.Wait();
			}
		
        iDspState=EAudioSyncCopying;
        //if (!IsActive())
        //    SetActive();
        TRequestStatus* status = &iStatus;
        iAudioSyncThread.RequestComplete(status,KErrNone);
        }
    else
        {
        // Not ready for writing samples!
        TRACE_A(_L("CAudioSynchronizer::BlockingWriteRequest: Audio not yet ready, sleep"));
        iLock.Signal(); 
        User::After(TTimeIntervalMicroSeconds32(500000));
        iLock.Wait();
        }
    iLock.Signal();
    }


void CAudioSynchronizer::RunL()
    {
    iLock.Wait();

    if (iCloseEngine)
        {
        TRACE2_A(_L("CAudioSynchronizer::RunL: iAStream->Stop(), iAStream=%u"), iAStream);
		iLock.Signal(); // Locking not necessarily needed?
        if (iAStream)
            iAStream->Stop();
		iLock.Wait();
        TRACE_A(_L("CAudioSynchronizer::RunL: CActiveScheduler::Stop"));
        CActiveScheduler::Stop();
        iLock.Signal();
        return;
        }

    // Write sample block to audio stream

    if (iDspState < EAudioSyncReady)
        {
        if (iDspState == EAudioSyncInitialised)
            {
            // Initialise CMdaAudioOutputStream here. AO would not start if initialized earlier(?).
            TRACE_A(_L("CAudioSynchronizer::RunL(): Initialisation done received"));
            TRAPD(leaveCode,iAStream=CMdaAudioOutputStream::NewL(*this));
	        iSettings.iCaps = 0;
            iSettings.iMaxVolume = 0;		
            iAStream->Open(&iSettings);
            }
        else
            {
            TRACE_A(_L("CAudioSynchronizer::RunL: Should never happen!"));
            }
        iDspState = EAudioSyncInitialisedAll; // Make sure that this is not called twice!!
        iStatus = KRequestPending; 
        SetActive();
        iLock.Signal();
        return;
        }

	//!!DEBUG
	if (GDEBUG_new_volume)
		{
		SetVolumeL(GDEBUG_new_volume);
		GDEBUG_new_volume = 0;
		}

    TInt len = iBuffer->Length();
    iLock.Signal();
    TBool ok = Synchronize();
    iLock.Wait();
    if (ok) 
        {
        // Play current block.
        iAStream->WriteL(*iBuffer);  
		iMomentOfWriteInTics = User::TickCount();
        }
    else 
        {
        iDspState=EAudioSyncPlaying;
        if (iSemReadyForNextWrite.Count()<0)
            iSemReadyForNextWrite.Signal();
        else
            ;
        }
    iBuffer->SetLength(len);

//#ifdef EPOC_AUDIO_SYNC_DEBUG_A
#if 1
    //!!Debug
    //if (++iDebugData.iDebugPrintCounter > DEBUG_PRINT_COUNTER)
    if (GDEBUG_print_next_debug)
        {
		GDEBUG_print_next_debug = 0;
        TBuf<256> text;
        
		TInt userTickCount = User::TickCount();
		
        TRACE_A(_L("======================================================="));
        TRACE_A(_L("--- TARGET --------------------------------------------"));
		
		TUint elapsedTics = userTickCount - iStartPlayTickCount + 1; // Add one to get a fast start
		TUint elapsedBlocks_ys = elapsedTics * iTickPeriod_ys;
		TUint countElapsedBlocks  = (elapsedBlocks_ys * (KSampleFreq/1000)) / (1000 * iBlockSampleCount);
        TInt targetRateHz = ((1000 * countElapsedBlocks * iBlockSampleCount) / (elapsedBlocks_ys/1000));
        text.Format(_L("%u blks, %u ms, %u tics, %d Hz"), 
			countElapsedBlocks, elapsedBlocks_ys/1000, elapsedTics, targetRateHz);
        TRACE_A(text);

        TRACE_A(_L("--- TOTAL ---------------------------------------------"));

		TUint totalElapsedTime_ms = ((userTickCount - iStartPlayTickCount + 1) 
            * iDebugData.iTickPeriod_ys2) / 1000; 
        TInt totalRateHz = 0;
        if (totalElapsedTime_ms)
            totalRateHz = (1000 * iDebugData.iCountSamplesCopied) / totalElapsedTime_ms;
        text.Format(_L("%u blks, %u ms, %d Hz"), 
            iCountSampleBlocksCopied, totalElapsedTime_ms, totalRateHz);
        TRACE_A(text);
        TRACE2_A(_L("Lead: %d"), iDebugData.iLeadInBlocks);
        text.Format(_L("blks:%d, too slow: %d, too fast: %d"), 
            iDebugData.iCountNormalBlocks, 
            iDebugData.iCountLeadTooLate, 
            iDebugData.iCountLeadTooFast);
        TRACE_A(text);
        text.Format(_L("LDD count: full=%d, ok=%d. 1Blk tics:%d"), 
			iLddBufferFullCounter, iCountSampleBlocksCopied-iLddBufferFullCounter,
			iBlockDurationInTics);
        TRACE_A(text);
        text.Format(_L("Strv: %d"), iDebugData.iCountStarvation);
        TRACE_A(text);

        TRACE_A(_L("--- CURRENT -------------------------------------------"));
		TUint elapsedTime_ms = ((userTickCount - iDebugData.iPrevTickCount) 
            * iDebugData.iTickPeriod_ys2) / 1000; 
        TInt rateHz = 0;
        if (elapsedTime_ms)
            rateHz = (1000 * (iDebugData.iCountSamplesCopied - iDebugData.iPrevCountSamplesCopied)) 
            / elapsedTime_ms;
        text.Format(_L("%d smpls / %u ms = %d Hz"), 
            iDebugData.iCountSamplesCopied - iDebugData.iPrevCountSamplesCopied, elapsedTime_ms, rateHz);
        TRACE_A(text);
        text.Format(_L("blks:%d, too slow: %d, too fast: %d"), 
            iDebugData.iCountNormalBlocks - iDebugData.iPrevCountNormalBlocks, 
            iDebugData.iCountLeadTooLate - iDebugData.iPrevCountLeadTooLate, 
            iDebugData.iCountLeadTooFast - iDebugData.iPrevCountLeadTooFast);
        TRACE_A(text);

        TRACE_A(_L("Lead values:"));
		for (TInt i=0; i < iDebugData.iLeadIndex; i++)
			{
			text.Format(_L("%d:%d"), i, iDebugData.iLeadValues[i]);
			TRACE_A(text);
			}
		iDebugData.iLeadIndex = 0;
       
        iDebugData.iPrevTickCount = userTickCount;
        iDebugData.iPrevCountSamplesCopied = iDebugData.iCountSamplesCopied;
        iDebugData.iPrevCountLeadTooLate = iDebugData.iCountLeadTooLate;
        iDebugData.iPrevCountLeadTooFast = iDebugData.iCountLeadTooFast;
        iDebugData.iPrevCountNormalBlocks = iDebugData.iCountNormalBlocks;
        iDebugData.iDebugPrintCounter = 0;
        }
#endif

    iStatus = KRequestPending; 
    SetActive();
    iLock.Signal();
    }


TBool CAudioSynchronizer::Synchronize()
    {
    iLock.Wait();

    /**
     * Calc lead value in blocks. Target audio frequency is KSampleFreq Hz. 
     */
    TUint elapsedTics = User::TickCount() - iStartPlayTickCount + 1; // Add one to get a fast start
    TUint elapsedBlocks_ys = elapsedTics * iTickPeriod_ys;
    // We could precalc some values to save time...
    // Add 0.5 (i.e. 500) for rounding. 
    TUint countElapsedBlocks  = (((elapsedBlocks_ys * (KSampleFreq/1000)) / iBlockSampleCount) + 500) / 1000;
    TInt leadInBlocks = iCountSampleBlocksCopied - countElapsedBlocks;
    iDebugData.iLeadInBlocks = leadInBlocks;   
    /**
     * Synchronize audio: If we are not in sync, either duplicate or skip current sample block.
     * If we are lagging too much (real speed is < 50%), even duplicating every block is not enough 
     * and there will be breaks in audio.
     */

//	if (iDebugData.iLeadIndex < KDebugMaxLeadIndex)
//		iDebugData.iLeadValues[iDebugData.iLeadIndex++] = leadInBlocks;

     if (/*leadInBlocks < -KLatencyResetLimit ||*/ GDEBUG_reset_sync)
        {
        // Totally out of sync. Reset sync. 
        User::After(TTimeIntervalMicroSeconds32(/*500*/ 2000 *1000)); // Drain audio driver
        iLock.Signal();
        ResetSync();

		GDEBUG_reset_sync = 0;

        return(EFalse);
        }

    if (leadInBlocks < -KLatencyMax)
        {
        // Audio is generated too slowly. Duplicate current block.
        iBuffer->Append(*iBuffer);
        iCountSampleBlocksCopied++;
        iDebugData.iCountSamplesCopied += iBlockSampleCount; // Only for debug !! TODO: Handle overflow!
        iDebugData.iCountLeadTooLate++; // Only for debug
        }
    else if (iStarvation)
        {
        // Adds extra duplicated block to fix starvation. Do not count these blocks in any other counters.
        // iBuffer->Append(*iBuffer);
        iDebugData.iCountStarvation++;
        iStarvation = EFalse;
        }


    if (leadInBlocks > KLatencyMax) 
        {
        // Audio is generated too fast.
        iWaitDelay_ys = 100*1000 /*iSyncDelay_ys * (leadInBlocks - KLatencyMax)*/;
        }

    // No need to skip. Play current block.
    iCountSampleBlocksCopied++;
    iDebugData.iCountNormalBlocks++;
    iDebugData.iCountSamplesCopied += iBlockSampleCount; // Only for debug !! TODO: Handle overflow!
    
    iLock.Signal();
    
    return(ETrue);
    }


void CAudioSynchronizer::WaitForSync()
    {
    iLock.Wait();
    TInt delay_ys = iWaitDelay_ys;
    iWaitDelay_ys = 0;
    iLock.Signal();

    if (delay_ys) 
        {
        User::After(TTimeIntervalMicroSeconds32(delay_ys));
        iLock.Wait();
        iDebugData.iCountLeadTooFast++; // Only for debug
        iLock.Signal();
        }
    }

void CAudioSynchronizer::DoCancel()
    {
    /*!! TODO*/
    }


void CAudioSynchronizer::MaoscOpenComplete(TInt aError)
    {
	if (aError!=KErrNone)
        {
            TRACE2_A(_L("CAudioSynchronizer::MaoscOpenComplete, aError = "), aError);
        }
    else
        {
        TRACE_A(_L("CAudioSynchronizer::MaoscOpenComplete"));
#ifdef SYMBIAN_CRYSTAL
	    iAStream->SetAudioPropertiesL(TMdaAudioDataSettings::ESampleRate8000Hz, 
		    TMdaAudioDataSettings::EChannelsMono);
#else
	    iAStream->SetAudioPropertiesL(TMdaAudioDataSettings::ESampleRate16000Hz, 
		    TMdaAudioDataSettings::EChannelsMono);
#endif
        iDspState=EAudioSyncReady;
        if (iVolume==0)
#ifdef __WINS__
            SetVolumeL(90);
#else
            SetVolumeL(5);
#endif
	    iAStream->SetPriority(EPriorityNormal, EMdaPriorityPreferenceNone);
        TRACE_A(_L("CAudioSynchronizer::MaoscOpenComplete: iSemReadyForNextWrite"));
        
	    iLockDuringStartup.Signal(); // Startup done
        }
    }


void CAudioSynchronizer::MaoscBufferCopied(TInt /*aError*/, const TDesC8& /*aBuffer*/)
    {
    iLock.Wait();
    TRACE(_L("CAudioSynchronizer::MaoscBufferCopied"));

	// Check if writing a sample has taken longer than assumed. It indicates
	// that LDD buffer is full and the server have to wait until there
	// is free space(?). In practice, normally (buffer is not full) copying takes 0 or 1 tics.
	// If buffer is full, there comes about 10-50 copies that are 0 or 1 ticks and then
	// one copy that takes about 50 ticks(!), then again 10-15 normal copies and one
	// big one, etc.
	//
	if (iMomentOfWriteInTics)
		{
		TUint copyTime = User::TickCount() - iMomentOfWriteInTics;
		if (iDebugData.iLeadIndex < KDebugMaxLeadIndex)
			iDebugData.iLeadValues[iDebugData.iLeadIndex++] = copyTime;
		TInt duration = Max(2, iBlockDurationInTics); // 0 or 1 tics are too small duration
		if (copyTime >= duration*3)
			{
			iLddBufferFullCounter++;
			iIsLddBufferFull = ETrue;
			}
		}

    iDspState=EAudioSyncPlaying;
    iLock.Signal();

    // Send signal if another thread is waiting for it 
    if (iSemReadyForNextWrite.Count()<0)
        {
        TRACE(_L("CAudioSynchronizer::MaoscBufferCopied: iSemReadyForNextWrite.Signal"));
        iSemReadyForNextWrite.Signal();
        }
    }

void CAudioSynchronizer::MaoscPlayComplete(TInt /*aError*/)
    {
    TRACE(_L("CAudioSynchronizer::MaoscPlayComplete"));
    iStarvation = ETrue;
    }

void CAudioSynchronizer::ResetSync()
/**
 * Reset audio synchronizing
 */
    {
    TRACE_A(_L("CAudioSynchronizer::ResetSync()"));
    iLock.Wait();

    iCountSampleBlocksCopied = 0;
    iStartPlayTickCount = User::TickCount(); 
    iStarvation = EFalse;
    iWaitDelay_ys = 0; 
	iLddBufferFullCounter = 0;
	iIsLddBufferFull = EFalse;
	iMomentOfWriteInTics = 0;

    // For debugging
    iDebugData.iLeadInBlocks = 0;
    iDebugData.iTickPeriod_ys2 = iTickPeriod_ys;
    iDebugData.iCountSamplesCopied = 0;
    iDebugData.iPrevCountSamplesCopied = 0;
    iDebugData.iCountLeadTooLate = 0;
    iDebugData.iCountLeadTooFast = 0;
    iDebugData.iPrevCountLeadTooLate = 0;
    iDebugData.iPrevCountLeadTooFast = 0;
    iDebugData.iCountNormalBlocks = 0;
    iDebugData.iPrevCountNormalBlocks = 0;
    iDebugData.iCountStarvation = 0;
    iDebugData.iPlayCompleteCounter = 0;
    iDebugData.iBufferLengthTooBigCounter = 0;
    iDebugData.iPrevTickCount = 0;

    iLock.Signal();
    }


void CAudioSynchronizer::SetVolumeL(TInt aVolume)
    {
    TRACE2_A(_L("CAudioSynchronizer::SetVolumeL (%d)"), aVolume);
    iVolume = aVolume;
    if (iDspState>=EAudioSyncReady)
        {
        if(iVolume > 100)
            iVolume = iAStream->MaxVolume();
        else if(iVolume < 0)
            iVolume = 0;
        iAStream->SetVolume((iVolume*iAStream->MaxVolume())/100);
		TRACE2_A(_L("CAudioSynchronizer::SetVolumeL: SetVolume(%d)"), 
			(iVolume*iAStream->MaxVolume())/100);
        }
    }
