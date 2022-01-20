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
 * $Header: /cvsroot/scummvm/scummvm/sound/mididrv.cpp,v 1.50 2004/02/05 13:56:39 chrilith Exp $
 *
 */

#include "stdafx.h"
#include "sound/mididrv.h"
#include "common/str.h"
#include "common/util.h"

/** Internal list of all available 'midi' drivers. */
static const struct MidiDriverDescription midiDrivers[] = {
	{"auto", "Default", MD_AUTO},
	{"null", "No music", MD_NULL},

#if defined(WIN32A) && !defined(_WIN32_WCE)
	{"windows", "Windows MIDI", MD_WINDOWS},
#endif

#if defined(UNIX) && !defined(__BEOS__) && !defined(MACOSX)
	{"seq", "SEQ", MD_SEQ},
#endif

#if defined(MACOSX)
	{"qt", "QuickTime", MD_QTMUSIC},
	{"core", "CoreAudio", MD_COREAUDIO},
#endif

#if defined(__MORPHOS__)
	{"etude", "Etude", MD_ETUDE},
#endif

#if defined(UNIX) && defined(USE_ALSA)
	{"alsa", "ALSA", MD_ALSA},
#endif

#if !defined(__PALM_OS__)
	{"adlib", "Adlib", MD_ADLIB},
#endif
	{"towns", "FM Towns", MD_TOWNS},
	{"pcspk", "PC Speaker", MD_PCSPK},
	{"pcjr", "IBM PCjr", MD_PCJR},

#if defined(__PALM_OS__)
	{"ypa1", "Yamaha Pa1", MD_YPA1},
	{"zodiac", "Tapwave Zodiac", MD_ZODIAC},
#endif
	{0, 0, 0}
};


const MidiDriverDescription *getAvailableMidiDrivers() {
	return midiDrivers;
}

int parseMusicDriver(const Common::String &str) {
	if (str.isEmpty())
		return -1;

	const char *s = str.c_str();
	const MidiDriverDescription *md = getAvailableMidiDrivers();

	while (md->name) {
		if (!scumm_stricmp(md->name, s)) {
			return md->id;
		}
		md++;
	}

	return -1;
}

const char *MidiDriver::getErrorName(int error_code) {
	static const char *const midi_errors[] = {
		"No error",
		"Cannot connect",
		"Streaming not available",
		"Device not available",
		"Driver already open"
	};

	if ((uint) error_code >= ARRAYSIZE(midi_errors))
		return "Unknown Error";
	return midi_errors[error_code];
}
