/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header: /cvsroot/scummvm/scummvm/backends/sdl/sdl.cpp,v 1.56.2.2 2004/02/28 13:01:43 fingolfin Exp $
 *
 */

#include "sdl-common.h"
#include "common/scaler.h"
#include "common/util.h"

#include "audioconvert.h"
//#include"sound/linearrate.h"


extern void AudioLibPanic(TInt aPanic);


class OSystem_SDL : public OSystem_SDL_Common {
public:
	OSystem_SDL();
    	// Set a parameter
#if 0 //MM 
    uint32 property(int param, Property *value);

	// Update the dirty areas of the screen
	void update_screen();

protected:
	SDL_Surface *_hwscreen;    // hardware screen

	ScalerProc *_scaler_proc;

	virtual void load_gfx_mode();
	virtual void unload_gfx_mode();
	virtual bool save_screenshot(const char *filename);
	void hotswap_gfx_mode();
#endif //if 0 MM
};

#if 0 //MM
OSystem_SDL_Common *OSystem_SDL_Common::create_intern() {
	return new OSystem_SDL();
}
#endif// if 0 MM

OSystem_SDL::OSystem_SDL()
#if 0 //MM
	 : _hwscreen(0), _scaler_proc(0)
#endif //if 0 MM
{
}
#include <Mda\Common\Audio.h>
#include <MdaAudioOutputStream.h>
class OSystem_SDL_Symbian:public OSystem_SDL,public MMdaAudioOutputStreamCallback, public CBase
{
public:
	OSystem_SDL_Symbian();
	~OSystem_SDL_Symbian();

    void setCompleteListener(MCompleteListener* listener);

	static OSystem *create(int gfx_mode);

	static TInt Sound_and_music_thread(TAny *params);
	void  sound_and_music_thread(TAny *params);

	void MaoscOpenComplete(TInt aError) ;
	void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);
	void MaoscPlayComplete(TInt aError);
	// Set function that generates samples 
	bool set_sound_proc( SoundProc proc, void *param, SoundFormat format);
	void clear_sound_proc();

     
    void setSystemAudio(const TSystemAudioCaps& aCaps);

private:

	CMdaAudioOutputStream* iStream;
	CActiveScheduler* iScheduler ;
	TBool iTerminateSoundThread;
    enum {EStop, ERun};
	TInt iScStatus;
	TBool FM_high_quality;

    MCompleteListener* iListener;

protected:
	static OSystem_SDL_Symbian *create_intern();
private:
    TSystemAudioCaps iAudioCaps;
    TThreadId iSndThreadId;
};

#if 0 // MM
void OSystem_SDL::load_gfx_mode() {
	_forceFull = true;
	_mode_flags |= DF_UPDATE_EXPAND_1_PIXEL;

	_tmpscreen = NULL;
	_tmpScreenWidth = (_screenWidth + 3);
	
	switch(_mode) {
	case GFX_NORMAL:
		_scaleFactor = 1;
		_scaler_proc = Normal1x;
		break;
		/*
	case GFX_DOUBLESIZE:
		_scaleFactor = 2;
		_scaler_proc = Normal2x;
		break;
	case GFX_TRIPLESIZE:
		_scaleFactor = 3;
		_scaler_proc = Normal3x;
		break;

	case GFX_2XSAI:
		_scaleFactor = 2;
		_scaler_proc = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		_scaleFactor = 2;
		_scaler_proc = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		_scaleFactor = 2;
		_scaler_proc = SuperEagle;
		break;
	case GFX_ADVMAME2X:
		_scaleFactor = 2;
		_scaler_proc = AdvMame2x;
		break;
	case GFX_ADVMAME3X:
		_scaleFactor = 3;
		_scaler_proc = AdvMame3x;
		break;
	case GFX_HQ2X:
		_scaleFactor = 2;
		_scaler_proc = HQ2x;
		break;
	case GFX_HQ3X:
		_scaleFactor = 3;
		_scaler_proc = HQ3x;
		break;
	case GFX_TV2X:
		_scaleFactor = 2;
		_scaler_proc = TV2x;
		break;
	case GFX_DOTMATRIX:
		_scaleFactor = 2;
		_scaler_proc = DotMatrix;
		break;
*/
	default:
		error("unknown gfx mode %d", _mode);
	}

	//
	// Create the surface that contains the 8 bit game data
	//
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _screenWidth, _screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//

	_hwscreen = SDL_SetVideoMode(_screenWidth * _scaleFactor, (_adjustAspectRatio ? 240 : _screenHeight) * _scaleFactor, 16, 
		_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
	);
	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		// FIXME: We should be able to continue the game without
		// shutting down or bringing up the debug console, but at
		// this point we've already screwed up all our member vars.
		// We need to find a way to call SDL_VideoModeOK *before*
		// that happens and revert to all the old settings if we
		// can't pull off the switch to the new settings.
		//
		// Fingolfin says: the "easy" way to do that is not to modify
		// the member vars before we are sure everything is fine. Think
		// of "transactions, commit, rollback" style... we use local vars
		// in place of the member vars, do everything etc. etc.. In case
		// of a failure, rollback is trivial. Only if everything worked fine
		// do we "commit" the changed values to the member vars.
		warning("SDL_SetVideoMode says we can't switch to that mode");
		quit();
	}

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);
	
	// Need some extra bytes around when using 2xSaI
	uint16 *tmp_screen = (uint16 *)calloc(_tmpScreenWidth * (_screenHeight + 3), sizeof(uint16));
	_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen,
						_tmpScreenWidth, _screenHeight + 3, 16, _tmpScreenWidth * 2,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen failed");

	// keyboard cursor control, some other better place for it?
	km.x_max = _screenWidth * _scaleFactor - 1;
	km.y_max = (_adjustAspectRatio ? 240 : _screenHeight) * _scaleFactor - 1;
	km.delay_time = 25;
	km.last_time = 0;
}

void OSystem_SDL::unload_gfx_mode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL; 
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen); 
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		free(_tmpscreen->pixels);
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

void OSystem_SDL::hotswap_gfx_mode() {
	if (!_screen)
		return;

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch.
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen); 

	// Setup the new GFX mode
	load_gfx_mode();

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);
	
	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	free(old_tmpscreen->pixels);
	SDL_FreeSurface(old_tmpscreen);

	// Blit everything to the screen
	update_screen();
	
	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;
}

void OSystem_SDL::update_screen() {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _screenWidth * _scaleFactor, _newShakePos * _scaleFactor};

		if (_adjustAspectRatio)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Make sure the mouse is drawn, if it should be drawn.
	draw_mouse();
	
	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart, 
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);
		
		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_num_dirty_rects = 1;

		_dirty_rect_list[0].x = 0;
		_dirty_rect_list[0].y = 0;
		_dirty_rect_list[0].w = _screenWidth;
		_dirty_rect_list[0].h = _screenHeight;
	}

	// Only draw anything if necessary
	if (_num_dirty_rects > 0) {

		SDL_Rect *r; 
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirty_rect_list + _num_dirty_rects;

		if (_scaler_proc == Normal1x && !_adjustAspectRatio) {
			SDL_Surface *target = _overlayVisible ? _tmpscreen : _screen;
			for (r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;
				
				if (_overlayVisible) {
					// FIXME: I don't understand why this is necessary...
					dst.x--;
					dst.y--;
				}
				dst.y += _currentShakePos;
				if (SDL_BlitSurface(target, r, _hwscreen, &dst) != 0)
					error("SDL_BlitSurface failed: %s", SDL_GetError());
			}
		} else {
			if (!_overlayVisible) {
				for (r = _dirty_rect_list; r != last_rect; ++r) {
					dst = *r;
					dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
					dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
					if (SDL_BlitSurface(_screen, r, _tmpscreen, &dst) != 0)
						error("SDL_BlitSurface failed: %s", SDL_GetError());
				}
			}

			SDL_LockSurface(_tmpscreen);
			SDL_LockSurface(_hwscreen);

			srcPitch = _tmpscreen->pitch;
			dstPitch = _hwscreen->pitch;

			for (r = _dirty_rect_list; r != last_rect; ++r) {
				register int dst_y = r->y + _currentShakePos;
				register int dst_h = 0;
				register int orig_dst_y = 0;

				if (dst_y < _screenHeight) {
					dst_h = r->h;
					if (dst_h > _screenHeight - dst_y)
						dst_h = _screenHeight - dst_y;

					dst_y *= _scaleFactor;

					if (_adjustAspectRatio) {
						orig_dst_y = dst_y;
						dst_y = real2Aspect(dst_y);
					}

					_scaler_proc((byte *)_tmpscreen->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
						(byte *)_hwscreen->pixels + r->x * 2 * _scaleFactor + dst_y * dstPitch, dstPitch, r->w, dst_h);
				}

				r->x *= _scaleFactor;
				r->y = dst_y;
				r->w *= _scaleFactor;
				r->h = dst_h * _scaleFactor;

				//if (_adjustAspectRatio && orig_dst_y / _scaleFactor < _screenHeight)
				//	r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y);
			}
			SDL_UnlockSurface(_tmpscreen);
			SDL_UnlockSurface(_hwscreen);
		}

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = (_adjustAspectRatio ? 240 : _screenHeight) * _scaleFactor;
		}

		// Finally, blit all our changes to the screen
		SDL_UpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list);
	}

	_num_dirty_rects = 0;
	_forceFull = false;
}



uint32 OSystem_SDL::property(int param, Property *value) {
#if 0 //MM
	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends

	if (param == PROP_TOGGLE_FULLSCREEN) {
		assert(_hwscreen != 0);
		_full_screen ^= true;

		if (_mouseDrawn)
			undraw_mouse();

#ifdef MACOSX
		// On OS X, SDL_WM_ToggleFullScreen is currently not implemented. Worse,
		// it still always returns -1. So we simply don't call it at all and
		// use hotswap_gfx_mode() directly to switch to fullscreen mode.
		hotswap_gfx_mode();
#else
		if (!SDL_WM_ToggleFullScreen(_hwscreen)) {
			// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode
			hotswap_gfx_mode();
		}
#endif
		return 1;
	} else if (param == PROP_SET_GFX_MODE) {
		if (value->gfx_mode > 11)	// FIXME! HACK, hard coded threshold, not good
			return 0;

		_mode = value->gfx_mode;
		hotswap_gfx_mode();

		return 1;
	} else if (param == PROP_TOGGLE_ASPECT_RATIO) {
		if (_screenHeight == 200) {
			assert(_hwscreen != 0);
			_adjustAspectRatio ^= true;
			hotswap_gfx_mode();
		}
	} else if (param == PROP_HAS_SCALER) {
		if (value->gfx_mode <= 11)	// FIXME: Hardcoded
			return 1;
		return 0;
	}
#endif //if 0 MM
	return OSystem_SDL_Common::property(param, value);
}

#endif //if 0 MM
#if 0 //MM

bool OSystem_SDL::save_screenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	SDL_SaveBMP(_hwscreen, filename);
	return true;
}
#endif //if 0 MM


#include "sdl-common.h"



#define FRAG_SIZE 4096
typedef struct {
	OSystem::SoundProc sound_proc;
	void *param;
	byte format;
	OSystem_SDL_Symbian* iSystem;
} THREAD_PARAM;


OSystem *OSystem_SDL_Symbian::create(int gfx_mode) 
{
	OSystem_SDL_Symbian *syst = OSystem_SDL_Symbian::create_intern();
#if 0 //MM
	syst->init_intern(gfx_mode);
#endif //if 0 MM
	return syst;
}



OSystem_SDL_Symbian *OSystem_SDL_Symbian::create_intern() {
	return new OSystem_SDL_Symbian();
}

//#include "config-manager.h"

//extern Common::ConfigManager *g_config;

OSystem_SDL_Symbian::OSystem_SDL_Symbian() : iListener(NULL)
{
/*	const char *dir = NULL;
	dir = g_config->get("savepath");

	// If SCUMMVM_SAVEPATH was not specified, try to use general path from config
	if (!dir || dir[0] == 0)
		dir = g_config->get("savepath", "scummvm");
	if (!dir || dir[0] == 0)
	{
		setenv("SCUMMVM_SAVEPATH","C:\\documents\\",true);
		setenv("savepath","C:\\documents\\",true);
	}
	else
	{
		setenv("SCUMMVM_SAVEPATH",dir,true);
		setenv("savepath",dir,true);
	}*/
#pragma message("Set save path here")
}

OSystem_SDL_Symbian::~OSystem_SDL_Symbian()
{
}

TSystemAudioCaps::TSystemAudioCaps() : iRate(22050), iChannels(2) {}


LOCAL_C void Panic(TInt aPanic)
    {
    User::Panic(_L("OSystem_SDL_Symbian"), aPanic);
    }

LOCAL_C TInt GetSOSRate(TInt aRate)
    {
    switch(aRate)
        {
        case 8000: return TMdaAudioDataSettings::ESampleRate8000Hz;
        case 16000: return TMdaAudioDataSettings::ESampleRate16000Hz;
        case 11025: return TMdaAudioDataSettings::ESampleRate11025Hz;
        case 22050: return TMdaAudioDataSettings::ESampleRate22050Hz;
        case 44100: return TMdaAudioDataSettings::ESampleRate44100Hz;
        }
    Panic(KErrNotSupported);
    return 0;
    }

LOCAL_C TInt GetSOSChannels(TInt aChannels)
    {
    switch(aChannels)
        {
        case 1: return TMdaAudioDataSettings::EChannelsMono;
        case 2: return TMdaAudioDataSettings::EChannelsStereo;
        }
    Panic(KErrNotSupported);
    return 0;
    }

/*
extern TUint8* gg;
extern TInt ss;
*/


void OSystem_SDL_Symbian::setSystemAudio(const TSystemAudioCaps& aCaps)
    {
    iAudioCaps.iRate = aCaps.iRate;
    iAudioCaps.iChannels = aCaps.iChannels;
    }

//#include<E32SVR.H>

LOCAL_C TBool StopIdle(TAny*)
    {
    CActiveScheduler::Stop();
    return EFalse;
    }

LOCAL_C void FlushL()
    {
    CIdle* ii = CIdle::NewL(CActive::EPriorityIdle);
    ii->Start(TCallBack(StopIdle));
    CActiveScheduler::Start();
    delete ii;
    }

#define PRINT(x, y) Debug_print(_L(x), y);
#include<flogger.h>
LOCAL_C void Debug_print(const TDesC& aDes, TInt aValue)
    {
    RFileLogger::WriteFormat(_L("doom"), _L("audio.txt"), EFileLoggingModeAppend, _L("%S : %d"), &aDes, aValue);
    }

void  OSystem_SDL_Symbian::sound_and_music_thread(TAny *params)
{
	iTerminateSoundThread=EFalse;

    delete iScheduler;
    iScheduler = NULL;
    CActiveScheduler::Install(NULL);
	iScheduler = new (ELeave) CActiveScheduler;
	CActiveScheduler::Install(iScheduler);
    
    delete iStream;
    iStream = NULL;
	iStream = CMdaAudioOutputStream::NewL(*this);

	TMdaAudioDataSettings audioSettings;
	audioSettings.Query();
    audioSettings.iSampleRate = GetSOSRate(iAudioCaps.iRate);
    audioSettings.iChannels = GetSOSChannels(iAudioCaps.iChannels);
//  audioSettings.iSampleRate = TMdaAudioDataSettings::ESampleRate16000Hz;
//	audioSettings.iSampleRate = TMdaAudioDataSettings::ESampleRate22050Hz;
//	audioSettings.iChannels = TMdaAudioDataSettings::EChannelsStereo;
	audioSettings.iFlags = 0;
	audioSettings.iVolume = 0;

   
    iStream->Open(&audioSettings);
	iScStatus = ERun;
	CActiveScheduler::Start();
        
    if(iScStatus != EStop)
        User::Leave(iScStatus);
               

	int  param, frag_size;
    uint8* sound_buffer=(uint8*)User::AllocL(FRAG_SIZE);
	OSystem::SoundProc sound_proc = ((THREAD_PARAM *) params)->sound_proc;
	void *proc_param = ((THREAD_PARAM *) params)->param;

    CleanupStack::PushL(sound_buffer);
    
    uint8* swapbuf = NULL;

    CleanupStack::PushL(swapbuf);

    if(iAudioCaps.iRate != 22050)
        {
        swapbuf = (uint8*) User::AllocL(FRAG_SIZE);
        }

    if(iAudioCaps.iChannels != 2)
        {
        if(swapbuf == NULL)
            swapbuf = (uint8*) User::AllocL(FRAG_SIZE);
        }

    PRINT("start audio", 0);

	for(;;)
        {
	//	uint8 *buf = (uint8 *)sound_buffer;
		int size, written;
		sound_proc(proc_param, (byte *)sound_buffer, FRAG_SIZE);
		size = FRAG_SIZE;

        if(iAudioCaps.iRate != 22050)
            {
            size = BitRateConverter::Convert(
                (TUint16*) sound_buffer,
                (TUint16*) swapbuf,
                22050,
                iAudioCaps.iRate,
                size);
        
            byte* tmp = sound_buffer;
            sound_buffer = (byte*) swapbuf;
            swapbuf = tmp;
            }

        if(iAudioCaps.iChannels != 2)
            {
            size = StereoToMonoConverter::Convert((uint16*)sound_buffer, (uint16*)swapbuf, size);
            byte* tmp = sound_buffer;
            sound_buffer = (byte*) swapbuf;
            swapbuf = tmp;
            }

        /*
        static int c = 0;
        int ok = 0;
        for(int rr = 0; !ok && rr < size; rr++)
            {
            if(sound_buffer[rr] != 0)
                ok = 1;
            }
        if(ok)
            c += size;
        */
		const TPtrC8 ptr (sound_buffer ,size);

		iStream->WriteL(ptr);
		iScStatus = ERun;
		CActiveScheduler::Start();

        if(iScStatus != EStop)
            {
            PRINT("iScStatus", iScStatus);
            FlushL();
            User::Leave(iScStatus);
            return;//for integrity
            }
		
		if(iTerminateSoundThread)
		    {
            PRINT("iTerminateSoundThread", iScStatus);
            //if(iScStatus == EStop)
            //    iStream->Stop();
            FlushL();
          //  RDebug::Print(_L("suze: %d"), c);
			break;
		    }
	    }

//    delete rateconverter;
//    delete stream;
    
     PRINT("cleanup2", iScStatus);
     PRINT("cleanup2", iListener == NULL);

    if(iListener)
        iListener->completeListen(KErrNone, 0xFFFFFFF);

	
	delete iStream;
	iStream = NULL;
    
    CleanupStack::PopAndDestroy(2); //swap and sound buffer 
    
	delete iScheduler;
	iScheduler = NULL;
	iTerminateSoundThread =EFalse;
}

TInt OSystem_SDL_Symbian::Sound_and_music_thread(TAny *params)
{
	CTrapCleanup* cleanup = CTrapCleanup::New();
    OSystem_SDL_Symbian* s = ((THREAD_PARAM *) params)->iSystem;
    TInt err = KErrNone;
    for(;;)
        {
	    TRAP(err,s->sound_and_music_thread(params));
        if(err == KErrNone)
            break;

        if(s->iScStatus == KErrDied ||
           s->iScStatus == KErrInUse ||
           /*s->iScStatus == KErrSessionClosed ||*/
           s->iScStatus == KErrServerTerminated)
            {
            User::After(100000); //just wait here that audio revivids?
            }
        else
            break;
        }
    delete cleanup;
	return err;
}

bool OSystem_SDL_Symbian::set_sound_proc(SoundProc proc, void *param, SoundFormat format)
{
	static THREAD_PARAM thread_param;

	/* And finally start the music thread */
	thread_param.param = param;
	thread_param.sound_proc = proc;
	thread_param.format = format;
	thread_param.iSystem=this;

	const TInt minStack = 8192;

	RThread thread;
    const TInt err = thread.Create(_L("SCUSND"),Sound_and_music_thread,minStack,NULL,(void *) &thread_param);
    __ASSERT_ALWAYS(err == KErrNone, AudioLibPanic(err));
	iSndThreadId = thread.Id();
    thread.SetPriority(EPriorityMore);
	thread.Resume();
    thread.Close();
	return true;
}


#define HANDLE_ERROR(x) if(x != KErrNone) {iScStatus = x; CActiveScheduler::Stop(); return;}


void OSystem_SDL_Symbian::MaoscOpenComplete(TInt aError) 
{
    HANDLE_ERROR(aError)
    //__ASSERT_ALWAYS(aError == KErrNone, User::Panic(_L("OpenComplete"), aError));
	//iStream->SetAudioPropertiesL( TMdaAudioDataSettings::ESampleRate22050Hz,TMdaAudioDataSettings::EChannelsStereo); // Set 11025 stereo sound, 16 bit only supported
	//iStream->SetAudioPropertiesL( TMdaAudioDataSettings::ESampleRate16000Hz,TMdaAudioDataSettings::EChannelsMono);
    iStream->SetAudioPropertiesL(GetSOSRate(iAudioCaps.iRate), GetSOSChannels(iAudioCaps.iChannels));
    iStream->SetVolume(iStream->MaxVolume());
	if(iScStatus != EStop)
	{
	CActiveScheduler::Stop();
	iScStatus = EStop;
	}
}


void OSystem_SDL_Symbian::setCompleteListener(MCompleteListener* listener)
    {
    iListener = listener;
    }


void OSystem_SDL_Symbian::MaoscBufferCopied(TInt aError, const TDesC8& /*aBuffer*/)
{
    HANDLE_ERROR(aError)
	
    if(iScStatus != EStop)
	{
	CActiveScheduler::Stop();
	iScStatus = EStop;
//    gg = NULL;
    if(iListener != NULL)
        {
        MCompleteListener* listener = iListener;
        iListener = NULL;
        listener->completeListen(aError, FRAG_SIZE);
        }
	}
}

void OSystem_SDL_Symbian::MaoscPlayComplete(TInt aError)
    { //called after stop - no error resolving needed
    if(iScStatus !=  EStop)
	    {
	    CActiveScheduler::Stop();
	    iScStatus = EStop;
        if(iListener != NULL)
            {
            MCompleteListener* listener = iListener;
            iListener = NULL;
            listener->completeListen(aError, FRAG_SIZE);
            }
	    }
    }

void OSystem_SDL_Symbian::clear_sound_proc()
    {
    RThread thread;
    if(thread.Open(iSndThreadId) == KErrNone)
        {
        if(thread.ExitType() == EExitPending)
            {
            TRequestStatus logon;
            thread.Logon(logon);
		    iTerminateSoundThread = ETrue;
            User::WaitForRequest(logon);
	        }
        thread.Close();
        }
    }

OSystem *OSystem_SDL_create(int gfx_mode) {
	#ifdef __SYMBIAN32__
	return OSystem_SDL_Symbian::create(gfx_mode);
	#else
	return OSystem_SDL_Common::create(gfx_mode);
	#endif
}

