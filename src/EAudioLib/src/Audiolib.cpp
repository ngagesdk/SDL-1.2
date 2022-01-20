#include "eaudiolib.h"
#include "audioplayer.h"

void AudioLibPanic(TInt aPanic)
    {
    User::Panic(_L("Audio lib"), aPanic);
    }

CEAudioLib* CEAudioLib::NewL(const TChannelType& aType)
    {
    CEAudioLib* lb = new (ELeave) CEAudioLib();
    CleanupStack::PushL(lb);
    lb->iPlayer = new (ELeave) CAudioPlayer(aType);
    lb->iPlayer->ConstructL();
    CleanupStack::Pop(); //lb
    return lb;
    }

MMidiPlayer& CEAudioLib::MidiPlayer()
    {
    return iPlayer->Midi();
    }

MChannelPlayer& CEAudioLib::ChannelPlayer()
    {
    return *iPlayer;
    }

LOCAL_C void Resize(RArray<TInt>& aArray, TInt aSize)
    {
    for(TInt i = 0; i < aSize; i++)
        {
        aArray.Append(0);
        }
    }

#define _ADJUST Resize(iData, CAudioPlayer::KLastOne);

TChannelType::TChannelType()
    {
    _ADJUST
    *this =  TChannelType(ETrue, 22050);
    }

TChannelType::TChannelType(TBool aStereo, TInt aRate)
    {
    _ADJUST
    const TInt defaultSize = 1024 * 64;
    *this = TChannelType(aStereo, aRate, defaultSize);
    }

TChannelType::TChannelType(TBool aStereo, TInt aRate, TInt aSize)
    {
    _ADJUST
    iData[MParameterReader::EBits16] =  ETrue;
    iData[MParameterReader::EStereo] = aStereo;
    iData[MParameterReader::ERate] = aRate;
    iData[MParameterReader::ESize] = aSize;
    }


TChannelType::TChannelType(const TChannelType& aType)
    {
    _ADJUST
    Copy(aType);
    }

TChannelType& TChannelType::operator=(const TChannelType& aType)
    {
    Copy(aType);
    return *this;
    }

void TChannelType::Copy(const TChannelType& aType)
    {
    for(TInt i = 0; i < MParameterReader::KLastOne; i++)
        iData[i] =  aType.iData[i];
    }

TChannelType::~TChannelType()
    {
    iData.Close();
    }

TInt CEAudioLib::MaxVolume()
    {
    return iPlayer->MaxVolume();
    }
   
void CEAudioLib::SetVolume(TInt aVolume, const TUid& aChannel)
    {
    aChannel == KMidiChannel ?
        iPlayer->Midi().SetVolume(aVolume) : iPlayer->SetVolume(aVolume, aChannel);
    }
TInt CEAudioLib::Volume(const TUid& aChannel)
    {
    return aChannel == KMidiChannel ?
        iPlayer->Midi().Volume() : iPlayer->Volume(aChannel);
    }

CEAudioLib::~CEAudioLib()
    {
    delete iPlayer;
    }

void CEAudioLib::SetPause(TBool aPause)
    {
    iPlayer->SetPause(aPause);
    }

void CEAudioLib::SetPause(TBool aPause, const TUid& aChannel)
    {
    aChannel == KMidiChannel ?
        iPlayer->Midi().SetPause(aPause) : iPlayer->SetPause(aPause, aChannel);
    }

void CEAudioLib::Stop()
    {
    Stop(KMidiChannel);
    iPlayer->StopAll();
    }

void CEAudioLib::Stop(const TUid& aChannel)
    {
    aChannel == KMidiChannel ?
        iPlayer->Midi().Stop() : iPlayer->Stop(aChannel);
    }

TBool CEAudioLib::IsPlaying( const TUid& aChannel) const
    {
    return aChannel == KMidiChannel ?
        iPlayer->Midi().IsPlaying() : iPlayer->IsPlaying(aChannel);
    }

void warning(char const* aStr, ...)
    {
#ifdef WARNINGS_AND_DEBUGS
    VA_LIST list;
    VA_START(list, aStr);
    TBuf8<1024> data;
    const TPtrC8 txt((const TUint8*)aStr);
    data.FormatList(txt, list);
    TBuf<256> warn;
    warn.Copy(data);
    User::InfoPrint(warn);
    VA_END(list);
#else
    aStr;
#endif
    }

void debug(int val, char const* aStr, ...)
    {
#ifdef WARNINGS_AND_DEBUGS
    VA_LIST list;
    VA_START(list, aStr);
    TBuf8<1024> data;
    const TPtrC8 txt((const TUint8*)aStr);
    data.FormatList(txt, list);
    TBuf<256> warn;
    warn.Copy(data);
    warn.AppendFormat(_L(" %d"), val);
    User::InfoPrint(warn);
    VA_END(list);
#else
    val;
    aStr;
#endif
    }

void error(char const* aStr, ...)
    {
    VA_LIST list;
    VA_START(list, aStr);
    TBuf8<1024> data;
    const TPtrC8 txt((const TUint8*)aStr);
    data.FormatList(txt, list);
    TBuf<256> panic;
    panic.Copy(data);
    User::Panic(panic , KErrGeneral);
    VA_END(list);
    }