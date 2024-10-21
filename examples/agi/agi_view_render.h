#ifndef __AGI_VIEW_RENDER_H__
#define __AGI_VIEW_RENDER_H__

#include "agi_view.h"

#define CHECK_PIC_PRI()\
	if((pri = *pBuf & 0xF0) < 0x30) {\
		pPtr = pBuf;\
		lastPri = 0;\
		while((pPtr < pBufEnd) && (lastPri < 0x30) ) {\
			pPtr += PIC_WIDTH;\
			lastPri = *pPtr & 0xF0;\
		}\
		if(lastPri > celPri)\
			pri = -1;\
	} else\
		pri = (pri>celPri)?-1:celPri /*
int CheckPicPri(U8 *pBuf, U8 *pBufEnd, int celPri) {
	int lastPri=0, pri;
	if((pri = *pBuf & 0xF0) < 0x30) {
		while((pBuf < pBufEnd) && (lastPri < 0x30) ) {
			pBuf += PIC_WIDTH;
			lastPri = *pBuf & 0xF0;
		}
		if(lastPri > celPri)
			pri = -1;
	} else
		pri = (pri>celPri)?-1:celPri;
	return pri;
}
*/


void UnBlitVObj(VOBJ *v);	// htk
void BlitVObj(VOBJ *v);

void AddObjPicPri(VOBJ *v);


#endif