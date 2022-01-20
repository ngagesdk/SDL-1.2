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
    SDL_epocvideo.h
    Epoc based SDL video driver implementation

    Epoc version by Hannu Viitala (hannu.j.viitala@mbnet.fi)
*/

#ifndef _SDL_epocvideo_h
#define _SDL_epocvideo_h

extern "C" {
#include "SDL_mouse.h"
#include "SDL_sysvideo.h"
};

#include <e32std.h>
#include <bitdev.h> 
#include <w32std.h>

#ifndef SYMBIAN_CRYSTAL
#include <bitdraw.h> // CFbsDrawDevice
#endif

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *_this
#define Private	_this->hidden

#ifdef SYMBIAN_CRYSTAL
#define SDL_NUMMODES	5
#else // SYMBIAN_SERIES60
#define SDL_NUMMODES	23
#endif

/* Private display data */
struct SDL_PrivateVideoData {

    SDL_Rect            *SDL_modelist[SDL_NUMMODES+1];

	/* Epoc window server info */
    
    RWsSession			EPOC_WsSession;
	RWindowGroup		EPOC_WsWindowGroup;
    TInt                EPOC_WsWindowGroupID;
	RWindow				EPOC_WsWindow;
	CWsScreenDevice*	EPOC_WsScreen;
	CWindowGc*			EPOC_WindowGc;
	TRequestStatus		EPOC_WsEventStatus;
	TRequestStatus		EPOC_RedrawEventStatus;
	TWsEvent			EPOC_WsEvent;
	TWsRedrawEvent		EPOC_RedrawEvent;

#if defined(__WINS__) || defined(TEST_BM_DRAW)
    CWsBitmap*          EPOC_Bitmap;
#else
#endif

#ifndef SYMBIAN_CRYSTAL
    CFbsDrawDevice*		EPOC_DrawDevice; // drawing device for some drawing modes
#endif

    TBool               EPOC_IsWindowFocused; //!!Not used for anything yet!

    /* Screen hardware frame buffer info */

   	TBool				EPOC_HasFrameBuffer;
	TInt				EPOC_BytesPerPixel;
	TInt				EPOC_BytesPerScanLine;
	TInt				EPOC_BytesPerScreen;
	TDisplayMode		EPOC_DisplayMode;
	TSize				EPOC_ScreenSize;
	TUint8*				EPOC_FrameBuffer;		/* if NULL in HW we can't do direct screen access */
    TPoint              EPOC_ScreenOffset;
	CFbsBitGc::TGraphicsOrientation EPOC_ScreenOrientation;

    /* Simulate double screen height */
	TInt				EPOC_ScreenXScaleValue;
	TInt				EPOC_ScreenYScaleValue;
};

extern "C" {
extern void RedrawWindowL(_THIS);
};


#endif /* _SDL_epocvideo_h */

