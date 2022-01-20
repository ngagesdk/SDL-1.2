#include"tiny_sdl.h"
#include"e32base.h"


void TinySDL_SetTimer(int aHandle, TinySDL_TimerCallback aCallback)
    {
    }

int TinySDL_OpenAudio(TinySDL_AudioSpec* aSpec, void*)
    {
    return 0;
    }

void TinySDL_PauseAudio(int aPause)
    {
    }

void TinySDL_CloseAudio()
    {
    }

Uint32 TinySDL_GetTicks()
    {
    return 0;
    }

void TinySDL_Delay(int aDelay)
    {
    }

TinySDL_mutex* TinySDL_CreateMutex()
    {
    return NULL;
    }

void TinySDL_mutexP(TinySDL_mutex* aMutex)
    {
    }

void TinySDL_mutexV(TinySDL_mutex* aMutex)
    {
    }

void TinySDL_DestroyMutex(TinySDL_mutex* aMutex)
    {
    }