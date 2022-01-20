

#include "audioplayer.h"
#include "common/system.h"
#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include <f32file.h>

#define Panic(x) AudioLibPanic(x)
extern void AudioLibPanic(TInt);



extern MidiParser *MidiParser_createRO();
extern MidiParser *MidiParser_createEUP();

extern OSystem* createSystem();

static OSystem* oss;

OSystem* const OSystem::instance()
    {
    if(oss == NULL)
        {
        oss = createSystem();
        }
    return oss;
    }

enum
    {
    ENone, ERo, ESo, EXmidi, ESmf 
    };

LOCAL_C TInt RegocMidi(const TDesC8& aData)
    {
    const TInt KHheadersz = 52;
    if(aData.Length() > KHheadersz)
        {
        if(aData.Mid(4, 3).Compare(_L8("ROL")) == 0)
            return ERo;
        if(aData.Mid(8, 2).Compare(_L8("SO")) == 0)
            return ERo;
        for(TInt i = 0; i < KHheadersz - 4; i++)
            {
            TPtrC8 h = aData.Mid(i, 4);
            if(h.Compare(_L8("MThd")) == 0)
                return ESmf;
            if(h.Compare(_L8("FORM")) == 0)
                return EXmidi;
            }
        }
    return ENone;
    }

LOCAL_C MidiParser* CreateMidiParser(const TDesC8& aData)
    {
    switch(RegocMidi(aData))
        {
        case ERo:
            return MidiParser_createRO();
        case ESo:
            return MidiParser_createEUP();
        case EXmidi:
            return MidiParser::createParser_XMIDI();
        case ESmf:
             return MidiParser::createParser_SMF();
        default:
            return NULL;
        }
    }

CMidiPlayer::CMidiPlayer(SoundMixer& aMixer) : iMixer(aMixer) 
    {
    }

void CMidiPlayer::ConstructL()
    {
    iDriver = MidiDriver_ADLIB_create(&iMixer);

    const TInt err = iDriver->open();
    
    if(err != 0)
        {
        TBuf<64> errs;
        errs.Copy(TPtrC8((const TUint8*)iDriver->getErrorName(err)).Left(errs.MaxLength()));
        User::InfoPrint(errs);
        User::Leave(err);
        }
    }

void CMidiPlayer::PlayMidiFromFileL(RFs& aFs, const TDesC& aFileName, TBool aLooping)
    {
    TEntry entry;
    User::LeaveIfError(aFs.Entry(aFileName, entry));
    HBufC8* buf = HBufC8::NewLC(entry.iSize);
    RFile file;
    User::LeaveIfError(file.Open(aFs, aFileName, EFileRead));
    TPtr8 ptr = buf->Des();
    file.Read(ptr);
    file.Close();
    CleanupStack::Pop(); //buf
    PlayMidiL(buf, aLooping);
    }

void CMidiPlayer::PlayMidiL(HBufC8* aData, TBool aLooping)
    {
    Stop();
    delete iMidiDataBuffer;
    iMidiDataBuffer = aData;
    PlayMidiL(*iMidiDataBuffer, EFalse, aLooping);
    }

TBool CMidiPlayer::IsPlaying() const
    { 
    if(iParser != NULL)
        return iParser->getTick() > 0;
    return EFalse;
    }

void CMidiPlayer::PlayMidiL(const TDesC8& aMidiData, TBool aCopy, TBool aLooping)
    {
    if(aCopy)
        {
        delete iMidiDataBuffer;
        iMidiData = NULL;
        iMidiDataBuffer = aMidiData.AllocL();
        iMidiData = iMidiDataBuffer;
        }
    else
        {
        iMidiData = &aMidiData;
        }

    Stop();   

    iParser = CreateMidiParser(*iMidiData);

    __ASSERT_ALWAYS(iParser != NULL, Panic(KErrNotFound));

    iParser->setMidiDriver(iDriver);

    byte* data = const_cast<byte*>(iMidiData->Ptr());

    iParser->loadMusic(data, iMidiData->Size());

    iParser->setTimerRate((iDriver->getBaseTempo() * 128) >> 7);
    iParser->property(MidiParser::mpAutoLoop, aLooping);
    iDriver->setTimerCallback(iParser, MidiParser::timerCallback);
    }

void CMidiPlayer::SetVolume(TInt aVolume)
    {
    iMixer.setMusicVolume(aVolume);
    }

void CMidiPlayer::SetPause(TBool aPause)
    {
    if(iParser != NULL)
        {
        if(aPause)
            iDriver->setTimerCallback(NULL, NULL);
        else
            iDriver->setTimerCallback(iParser, MidiParser::timerCallback);
        }
    }

TInt CMidiPlayer::Volume()
    {
    return iMixer.getMusicVolume();
    }

void CMidiPlayer::Stop()
    {
     if(iParser!= NULL)
         iParser->unloadMusic();
    delete iParser;
    iParser = NULL;
    iDriver->setTimerCallback(NULL, NULL);
    }

CMidiPlayer::~CMidiPlayer()
    {
    if(iParser != NULL)
        iParser->unloadMusic();
    delete iParser;
    if(iDriver != NULL)
        iDriver->close();
    delete iDriver;
    delete iMidiDataBuffer;
    }



/////////////////////////////////////////////////////////////////////////////////////////////////////////

CStreamPlayer::CStreamPlayer(SoundMixer& aMixer, TInt aChannel) : iMixer(aMixer), iChannel(aChannel)
    {
    iWaitThread.Open(RThread().Id());
    }

void CStreamPlayer::Activate(const MParameterReader& aReader, const TChannelType& aParameters)
    {
    const TInt rate = aReader.ReadParameter(MParameterReader::ERate, aParameters);
    TInt flags = 
        aReader.ReadParameter(MParameterReader::EStereo, aParameters)
        ? SoundMixer::FLAG_STEREO : 0;
    flags |= aReader.ReadParameter(MParameterReader::EBits16, aParameters)
        ? SoundMixer::FLAG_16BITS : 0;
    
  
    iSampleSize = 0; //8 bit no stereo, one byte / sample - as offset
  
    if(flags & SoundMixer::FLAG_STEREO)
        iSampleSize += 1;
    if(flags & SoundMixer::FLAG_16BITS)
        iSampleSize += 1;
    
    iRate = 1000000 / rate; //get one sample time in microseconds;
    

    const TInt size = aReader.ReadParameter(MParameterReader::ESize, aParameters);
    
    iStreamTreshold = size / 8; //
    iMixer.newStream(&iHandle, rate, /*SoundMixer::FLAG_LITTLE_ENDIAN |*/ flags, (uint32) size);
    }


TBool CStreamPlayer::Write(const TDesC8& aData)
    {
    //calculate size and rate every time for accuracy
    const TInt sz = aData.Size();
    if(iMixer.appendStream(iHandle, const_cast<TUint8*>(aData.Ptr()), sz) <= 0)
        {
        iStreamTreshold = sz;
        iSyncTime = (sz  * iRate) >> iSampleSize; //that many samples
        iStreamPos += sz;
        return ETrue;
        }
  //  TuneUp(); //the stream is filled too quickly!
    User::InfoPrint(_L("Full"));
    return EFalse;
    }

TInt CStreamPlayer::SyncTime() const
    {
    return iSyncTime;
    }



TBool CStreamPlayer::IsActive() const
    {
    return (TBool) iHandle.isActive();
    }


void CStreamPlayer::Stop()
    {
    iMixer.stopHandle(iHandle);
    __ASSERT_DEBUG(!IsActive(), Panic(KErrNotReady));
    }

void CStreamPlayer::SetVolume(TInt aVolume)
    {
    iMixer.setChannelVolume(iHandle, aVolume);
    }

TInt CStreamPlayer::Volume() const
    {
    return iMixer.getChannelVolume(iHandle);
    }

CStreamPlayer::~CStreamPlayer()
    {
    iMixer.stopHandle(iHandle);
    iWaitThread.Close();
    }


/** */
void CStreamPlayer::GetCompleted(TRequestStatus& aStatus, TBool aAllWritten)
    {
    iAllWritten = aAllWritten;
    __ASSERT_DEBUG(iWaitStatus == NULL, Panic(KErrNotReady));
    aStatus = KRequestPending;
    iWaitStatus = &aStatus;
    if(iWaitThread.Id() != RThread().Id())
        {
        iWaitThread.Close();
        iWaitThread.Open(RThread().Id());
        }
    }



TBool CStreamPlayer::Complete(TInt aErr)
    {
    if(iWaitStatus != NULL)
        {
        if(iAllWritten && !IsStreamEmpty()) //if iAllWritten, then waited until stream is empty
            return ETrue; //not yet complete
       __ASSERT_DEBUG(iWaitThread.Id() != RThread().Id(), Panic(KErrGeneral));
        iWaitThread.RequestComplete(iWaitStatus, aErr);
        }
    return EFalse;
    }


void CStreamPlayer::StreamRead(TInt anAmount)
    {
    iStreamPos -= anAmount;
    }

void CStreamPlayer::WaitStream()
    {
    if(iStreamPos > iStreamTreshold)
        User::After(iSyncTime);
    if(iStreamPos <= 0)
        {
        iStreamPos = 1; //next drain, set zero again
        User::After(1); //just hold busy loops
        }
    }

TBool CStreamPlayer::IsStreamEmpty() const
    {
    return iMixer.isDrained(iHandle);
    }

void CStreamPlayer::SetPause(TBool aPause)
    {
    iMixer.pauseHandle(iHandle, (bool)aPause);
    }


void CStreamPlayer::Draining()
    {
   // User::InfoPrint(_L("Drain"));
    const TThreadPriority p = RThread().Priority();
    RThread().SetPriority(p);
    iStreamPos = 0;
    }

////////////////////////////////////////////////////////////////////////////////////////////

CAudioPlayer::CAudioPlayer(const TChannelType& aType) : iType(aType)
    {
    }



TInt CAudioPlayer::ReadParameter(TParameter aParameter, const TChannelType& aType) const
    {
    return aType.iData[aParameter];
    }

TInt CAudioPlayer::ReadGlobalParameter(TParameter aParameter) const
    {
    return iType.iData[aParameter];
    }

void CAudioPlayer::ConstructL()
    {

    OSystem::instance();

    TSystemAudioCaps caps;
    caps.iRate = ReadGlobalParameter(MParameterReader::ERate);
    caps.iChannels = ReadGlobalParameter(MParameterReader::EStereo) ? 2 : 1;
    OSystem::instance()->setSystemAudio(caps);

    iMixer = new (ELeave) SoundMixer();

    iMidiPlayer = new (ELeave) CMidiPlayer(*iMixer);
    
    iMidiPlayer->ConstructL();

    iMixer->setMusicVolume(MaxVolume());
    iMixer->setVolume(MaxVolume());

    iMixer->setReadObserver(this);

    OSystem::instance()->setCompleteListener(NULL);

    }


CMidiPlayer& CAudioPlayer::Midi()
    {
    return *iMidiPlayer;
    }

TInt CAudioPlayer::ChannelIndex(TInt aChannel) const
    {
    for(TInt i = 0;i < iStreamPlayers.Count(); i++)
        {
        if(iStreamPlayers[i] == NULL) //there are no nulls between
            return -1;
        if(iStreamPlayers[i]->Channel() == aChannel)
           return i;
        }
    return -1;
    }

void CAudioPlayer::OpenChannelL(const TUid& aChannel,  const TChannelType& aType)
    {
    __ASSERT_ALWAYS(aChannel != KMidiChannel, Panic(KErrArgument));
    TInt index = ChannelIndex(aChannel.iUid);
    if(index < 0)
        {
        CStreamPlayer* s = new (ELeave) CStreamPlayer(*iMixer, aChannel.iUid);
        index = 0;
        while(iStreamPlayers[index] != NULL)
            index++;
        iStreamPlayers[index] = s;
        }
    
    if(iStreamPlayers[index]->IsActive())
        iStreamPlayers[index]->Stop();

    iStreamPlayers[index]->Activate(*this, aType); 
    }

void CAudioPlayer::SetPause(TBool aPause, const TUid& aChannel)
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    __ASSERT_DEBUG(index >= 0, Panic(KErrNotFound));
    CStreamPlayer* player = iStreamPlayers[index];
    player->SetPause(aPause);
    }

void CAudioPlayer::StopAll()
    {
     for(TInt i = 0; i < iStreamPlayers.Count(); i++)
        {
        if(iStreamPlayers[i] != NULL)
            {
            Stop(TUid::Uid(iStreamPlayers[i]->Channel()));
            }
        }
    }

void CAudioPlayer::Stop(const TUid& aChannel)
    {
     for(TInt i = 0; i < iStreamPlayers.Count(); i++)
        {
        if(iStreamPlayers[i] == NULL)
            break; //no nulls between
        iStreamPlayers[i]->Stop();
        delete iStreamPlayers[i];
        iStreamPlayers[i] = NULL;
        for(TInt j = i; j < iStreamPlayers.Count() - 1; j++)
            iStreamPlayers[j] = iStreamPlayers[j + 1];
        }
    }

void CAudioPlayer::channelRead(const PlayingSoundHandle& aHandle, TInt anAmount)
    {
    for(TInt i = 0;i < iStreamPlayers.Count(); i++)
        {
        CStreamPlayer* player = iStreamPlayers[i];
        if(player == NULL) //there are no nulls between
            return;
        if(*player == aHandle)
            {
            if(anAmount < 0)
                player->Draining(); //we are writing too slowly!
            else
                player->StreamRead(anAmount);
            return;
            }
        }
            
    }

void CAudioPlayer::WriteSound(const TDesC8& aData, const TUid& aChannel)
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    __ASSERT_DEBUG(index >= 0, Panic(KErrNotFound));

    CStreamPlayer* player = iStreamPlayers[index];

    while(!player->Write(aData))
        {
        TRequestStatus status;
        player->GetCompleted(status, EFalse);
        OSystem::instance()->setCompleteListener(this);
        User::WaitForRequest(status);
        }

    }


TInt CAudioPlayer::SyncTime(const TUid& aChannel) const
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    __ASSERT_DEBUG(index >= 0, Panic(KErrNotFound));
    return iStreamPlayers[index]->SyncTime();
    }

void CAudioPlayer::ResetSyncTime(const TUid& aChannel)
    {
    Panic(KErrNotSupported);
    /*const TInt index = ChannelIndex(aChannel.iUid);
    __ASSERT_DEBUG(index >= 0, Panic(KErrNotFound));
    iStreamPlayers[index]->ResetSyncPoint();*/
    }

void CAudioPlayer::WaitCompleted(const TUid& aChannel, MChannelPlayer::TCompleteType aType)
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    __ASSERT_DEBUG(index >= 0, Panic(KErrNotFound));
    CStreamPlayer* const player = iStreamPlayers[index];

    if(aType == MChannelPlayer::ESynchronized)
        {
        OSystem::instance()->setCompleteListener(this);
        player->WaitStream();
        }
    else
        {
        const TBool waitAll = (aType == MChannelPlayer::EBufferComplete);
        TRequestStatus status;
        player->GetCompleted(status, waitAll);
        OSystem::instance()->setCompleteListener(this);
        User::WaitForRequest(status);
        }
    }

void CAudioPlayer::completeListen(TInt aErr, TInt /*aBytesWritten*/)
    {
    TBool complete = EFalse;
    for(TInt i = 0;i < iStreamPlayers.Count(); i++)
        {
        if(iStreamPlayers[i] == NULL)
            break; //no nulls between
        complete = iStreamPlayers[i]->Complete(aErr);
        }
    if(complete)
        OSystem::instance()->setCompleteListener(this);
    }

CAudioPlayer::~CAudioPlayer()
    {
    if(iMixer != NULL)
        iMixer->stopAll();

    if(oss != NULL)
        oss->clear_sound_proc();

    for(TInt i = 0;i < iStreamPlayers.Count(); i++)
        {
        if(iStreamPlayers[i] != NULL)
            iStreamPlayers[i]->Stop();
        }
    
    completeListen(KErrNone, 0);

    iStreamPlayers.DeleteAll();
   
    delete iMidiPlayer;
    
    delete iMixer;
   
    delete oss;
    }

void CAudioPlayer::SetPause(TBool aPause)
    {
    iMixer->pauseAll((bool)aPause);
    }


void CAudioPlayer::SetVolume(TInt aVolume, const TUid& aChannel)
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    iStreamPlayers[index]->SetVolume(aVolume);
    }

TInt CAudioPlayer::MaxVolume() const
    {
    return TInt(255);
    }

TInt CAudioPlayer::Volume(const TUid& aChannel)
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    return iStreamPlayers[index]->Volume();
    }

TBool CAudioPlayer::IsPlaying(const TUid& aChannel) const
    {
    const TInt index = ChannelIndex(aChannel.iUid);
    return iStreamPlayers[index]->IsActive();
    }