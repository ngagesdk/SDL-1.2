#ifndef __TINYSDL_H__
#define __TINYSDL_H__


typedef int (*TinySDL_TimerCallback)();
typedef unsigned short Uint16;
typedef unsigned char Uint8;
typedef unsigned int Uint32;

enum {AUDIO_S16SYS};

struct TinySDL_AudioSpec
    {
    int freq;	
	Uint16 format;
	Uint8  channels;	
	Uint8  silence;	
	Uint16 samples;		
	Uint16 padding;		
	Uint32 size;
	void (*callback)(void *userdata, Uint8 *stream, int len);
	void  *userdata;
    };

struct TinySDL_mutex
    {
    int handle;
    };

void TinySDL_SetTimer(int aHandle, TinySDL_TimerCallback aCallback);
int TinySDL_OpenAudio(TinySDL_AudioSpec* aSpec, void*);
void TinySDL_PauseAudio(int aPause);
void TinySDL_CloseAudio();
Uint32 TinySDL_GetTicks();
void TinySDL_Delay(int aDelay);
TinySDL_mutex* TinySDL_CreateMutex();
void TinySDL_mutexP(TinySDL_mutex* aMutex);
void TinySDL_mutexV(TinySDL_mutex* aMutex);
void TinySDL_DestroyMutex(TinySDL_mutex* aMutex);


#define SDL_AudioSpec TinySDL_AudioSpec
#define SDL_mutex TinySDL_mutex

#define SDL_SetTimer(x, y) TinySDL_SetTimer(x, y)
#define SDL_TimerCallback TinySDL_TimerCallback

#define SDL_OpenAudio(x, y) TinySDL_OpenAudio(x, y)
#define SDL_PauseAudio(x) TinySDL_PauseAudio(x)
#define SDL_CloseAudio() TinySDL_CloseAudio()
#define SDL_GetTicks() TinySDL_GetTicks()
#define SDL_Delay(x) TinySDL_Delay(x)
#define SDL_CreateMutex() TinySDL_CreateMutex()
#define SDL_mutexP(x) TinySDL_mutexP(x)
#define SDL_mutexV(x) TinySDL_mutexP(x)
#define SDL_DestroyMutex(x) TinySDL_DestroyMutex(x)


#endif