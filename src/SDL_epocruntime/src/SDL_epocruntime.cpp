#include"sdl_epocruntime.h"
#include<E32SVR.H>
#include<hal_data.h>
#include<hal.h>

#define SYMBIAN_70

GLDEF_C TInt E32Dll(TDllReason)
	{
	return KErrNone;
	}

EXPORT_C void Epoc_Runtime::GetScreenInfo(TScreenInfoV01& screenInfo2)
	{
	TPckg<TScreenInfoV01> sInfo2(screenInfo2);
	UserSvr::ScreenInfo(sInfo2);
#ifdef SYMBIAN_70 //it should find out if there is a real flag!
    if(!screenInfo2.iScreenAddressValid)
        {
        TInt addr; 
        screenInfo2.iScreenAddressValid = KErrNone == HAL::Get(HALData::EDisplayMemoryAddress, addr);
        screenInfo2.iScreenAddress = (TAny*) addr;
        }
#endif
    }

#ifdef SYMBIAN_70
#pragma message("Symbian 7.x")
#else
#pragma message("Symbian 6.x")
#endif

