#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include"eaudiolib.h"

class RFs;
class SoundMixer;
class MidiDriver;
class MidiParser;

#include"sound/mixer.h"

class CMidiPlayer;
class CStreamPlayer;

class MParameterReader
    {
    public:
        enum TParameter {EBits16, ERate, EStereo, ESize, KLastOne};
        virtual TInt ReadParameter(TParameter aParameter, const TChannelType& aType) const = 0;
        virtual TInt ReadGlobalParameter(TParameter aParameter) const = 0;
    };

class CAudioPlayer : public CBase,
 public MChannelPlayer,
 public MCompleteListener,
 public MParameterReader,
 public MChannelReadObserver
    {
    public:
        CAudioPlayer(const TChannelType& aType);
        void ConstructL();
        ~CAudioPlayer();
        CMidiPlayer& Midi();
        CStreamPlayer& Channel(TInt aChannel);
        TInt ChannelIndex(TInt aChannel) const;
        void OpenChannelL(const TUid& aChannel,  const TChannelType& aType);
        void WriteSound(const TDesC8& aData, const TUid& aChannel);
        void WaitCompleted(const TUid& aChannel, MChannelPlayer::TCompleteType aType);
        TInt MaxVolume() const;
        TInt Volume(const TUid& aChannel);
        void SetVolume(TInt aVolume, const TUid&  aChannel);
        void SetPause(TBool aPause, const TUid&  aChannel);
        void SetPause(TBool aPause);
        void StopAll();
        void Stop(const TUid& aChannel);
        TBool IsPlaying(const TUid&  aChannel) const;
    private:
        void completeListen(TInt aErr, TInt aBytesWritten);
        void channelRead(const PlayingSoundHandle& aHandle, int aAmount);
    private:
        TInt ReadParameter(TParameter aParameter, const TChannelType& aType) const;
        TInt ReadGlobalParameter(TParameter aParameter) const;
        TInt SyncTime(const TUid& aChannel) const;
        void ResetSyncTime(const TUid& aChannel);
    private:
        const TChannelType iType;
        SoundMixer* iMixer;
        CMidiPlayer* iMidiPlayer;
        TFixedArray<CStreamPlayer*, 16> iStreamPlayers;
    };



class CMidiPlayer : public CBase, public MMidiPlayer
    {
    public:
        CMidiPlayer(SoundMixer& aMixer);
        void ConstructL();
        ~CMidiPlayer();
        void PlayMidiFromFileL(RFs& aFs, const TDesC& aFileName, TBool aLooping); //from file
        void PlayMidiL(const TDesC8& aMidiData, TBool aCopy, TBool aLooping); //from memory, copies the data
        void PlayMidiL(HBufC8* aData, TBool aLooping); //from memory and takes data ownership
        void SetVolume(TInt aVolume);
        TInt Volume();
        TBool IsPlaying() const;
        void Stop();
        void SetPause(TBool aPause);
    private:
        SoundMixer& iMixer;
        MidiDriver* iDriver;
        MidiParser* iParser;
        const TDesC8* iMidiData;
        HBufC8* iMidiDataBuffer;
    };

class CStreamPlayer : public CBase
    {
    public:
        CStreamPlayer(SoundMixer& aMixer, TInt aChannel);
        void Activate(const MParameterReader& aReader, const TChannelType& aType);
        TBool Write(const TDesC8& aData);
        TBool IsActive() const;
        inline TInt Channel() const;
        void Stop();
        void SetVolume(TInt aVolume);
        TInt Volume() const;
        ~CStreamPlayer();
        void GetCompleted(TRequestStatus& aStatus, TBool aAllWritten);
        TBool Complete(TInt aErr);
        TBool IsStreamEmpty() const;
        TInt SyncTime() const;
       // void ResetSyncPoint();
       // void SetSyncPoint(TInt aDataSize);
        void SetPause(TBool aPause);
        inline TBool operator==(const PlayingSoundHandle& aHandle) const;
        void Draining();
        void WaitStream();
        void StreamRead(TInt anAmount);
    private:
        void TuneUp();
        void TuneDown();
    private:
        SoundMixer& iMixer;
        TInt iChannel;
        PlayingSoundHandle iHandle;
        RThread iWaitThread;
        TRequestStatus* iWaitStatus;
        TBool iAllWritten;
        TInt iStreamTreshold;
        TInt iStreamPos;
        TInt iSyncTime;
        //TInt iTuning;
        //TInt iTendency;
        TInt iRate;
        TInt iSampleSize;
    };

inline TInt CStreamPlayer::Channel() const
    {
    return iChannel;
    }

inline TBool CStreamPlayer::operator==(const PlayingSoundHandle& aHandle) const
    {
    return aHandle == iHandle;
    }


#endif