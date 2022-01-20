#ifndef __EAUDIOLIB_H__
#define __EAUDIOLIB_H__

#include<e32base.h>

class RFs;

const TUid KMidiChannel = {0};

class CAudioPlayer;

class MMidiPlayer
    {
    public:
        virtual void PlayMidiFromFileL(RFs& aFs, const TDesC& aFileName, TBool aLooping) = 0; //from file
        virtual void PlayMidiL(const TDesC8& aMidiData, TBool aCopy, TBool aLooping) = 0; //from memory, copies the data
        virtual void PlayMidiL(HBufC8* aData, TBool aLooping) = 0; //from memory and takes data ownership
    };

class TChannelType
            {
            public:
                TChannelType(); //default
                TChannelType(TBool aStereo, TInt aRate);
                TChannelType(TBool aStereo, TInt aRate, TInt aBufferSize);
                TChannelType(const TChannelType& aType);
                TChannelType& operator=(const TChannelType& aType);
                ~TChannelType();      
            private:
                void Copy(const TChannelType& aType);
            private:
                friend class CAudioPlayer;
                RArray<TInt> iData;
            };

class MChannelPlayer
    {
    public:
        enum TCompleteType {EBufferEvent, EBufferComplete, ESynchronized};
        virtual void OpenChannelL(const TUid& aChannel, const TChannelType& aType) = 0; //open an audio channel
        virtual void WriteSound(const TDesC8& aData, const TUid& aChannel) = 0; //write data into it
        virtual void WaitCompleted(const TUid& aChannel, TCompleteType aType) = 0;
        virtual TInt SyncTime(const TUid& aChannel) const = 0;
    };



class CEAudioLib : public CBase
    {
    public:
        static CEAudioLib* NewL(const TChannelType& aType);
        ~CEAudioLib();
        MMidiPlayer& MidiPlayer();
        MChannelPlayer& ChannelPlayer();
        TInt MaxVolume();
        void SetVolume(TInt aVolume, const TUid& aChannel);
        TInt Volume(const TUid&  aChannel);
        void SetPause(TBool aPause);
        void SetPause(TBool aPause, const TUid& aChannel);
        void Stop();
        void Stop(const TUid& aChannel);
        TBool IsPlaying( const TUid& aChannel) const;
    private:
        CAudioPlayer* iPlayer;
    };

#endif