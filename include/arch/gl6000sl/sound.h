#ifndef __VGL_SOUND_H
#define __VGL_SOUND_H
/*
V-Tech Genius Leader Sound

Re-directing to speech.h

2020-05-19 Bernhard "HotKey" Slawik
*/

// Use the speech synthesizer
#include "speech.h"


void vgl_sound_off() {
	speech_stop();
}

void vgl_sound(word frq, word len) {
	// Perform a beep (frq is actually a delay...)
	(void)frq;
	(void)len;
	
	//@TODO: Implement
	// The stock firmware must be modifying the base frequency parameter of the LPC10 packets on-the-fly!
	speech_play((byte *)SPEECH_ASSET_BEEP_ADDR, SPEECH_ASSET_BEEP_LEN);	//, 6d131 = Bing (invalid key)
}


void vgl_sound_note(word n, word len) {
	(void)n;
	(void)len;
	
	//@TODO: Implement
	speech_play((byte *)SPEECH_ASSET_BEEP_ADDR, SPEECH_ASSET_BEEP_LEN);	//, 6d131 = Bing (invalid key)
}


void beep() {
	speech_play((byte *)SPEECH_ASSET_BEEP_ADDR, SPEECH_ASSET_BEEP_LEN);	//, 6d131 = Bing (invalid key)
}

#endif // __VGL_SOUND_H