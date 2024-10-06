#ifndef __AGI_CMD_AGI_C__
#define __AGI_CMD_AGI_C__

/*
 *  AGI Commands
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/


//#include "gbagi.h"
//#include "logic.h"
//#include "commands.h"
//#include "views.h"
//#include "picture.h"
//#include "screen.h"
//#include "text.h"
//#include "status.h"
//#include "menu.h"
//#include "input.h"
//#include "errmsg.h"
//#include "system.h"
//#include "invobj.h"
//#include "wingui.h"
//#include "parse.h"
//#include "gamedata.h"
//#include "saverestore.h"

#include "agi_logic.h"
#include "agi_commands.h"

//char sztmp[256];

void UnimplementedBox(U8 id) {
	//sprintf(sztmp,"Unimplemented command:\n%s()", agiCommands[id].name);
	//MessageBox(sztmp);
	(void)id;
}
void InvalidBox() {
	//sprintf(sztmp,"Invalid command:\ncmd_unknown%03d()", code[-1]);
	//MessageBox(sztmp);
}

void cReturn() {
	//NULL
	code_term();	// End current code to return?
}

//increment(vA);
//vA++;       
//	Increments the given variable vA. If vA is 255, it will not be incremented
//	and remain 255, preventing it from rolling over back to zero.
void cIncrement() {
	U8 code_0 = code_get();
	if (vars[ code_0 ] < 0xFF) vars[ code_0 ]++;
}

//decrement(vA);
//vA--;
//	Decrements the given variable vA. If vA is 0, it will not be decremented and
//	remain 0, preventing it from rolling over back to 255.
void cDecrement() {
	U8 code_0 = code_get();
	if(vars[ code_0 ]) vars[ code_0 ]--;
}

//assignn(vA,B);
//vA = B;
//	Variable vA is assigned the value of B, where B is an immediate integer
//	value between 0 and 255.
void cAssignn() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] = code_1;
}

//assignv(vA,vB);
//vA = vB;
//	Variable vA is assigned the value of variable vB.
void cAssignv() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] = vars[ code_1 ];
}

//addn(vA,B);
//vA += B;
//vA = vA + B;
//	The value of B, where B is an immediate integer value, is added to variable
//	vA. No checking takes place, so if the result is greater than 255, it will
//	wrap around to zero and above.
void cAddn() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] += code_1;
}

//addv(vA,vB);
//vA += vB;
//vA = vA + vB;
//	The value of vB, where vB is variable, is added to variable vA. No checking
//	takes place, so if the result is greater than 255, it will wrap around to
//	zero and above.
void cAddv() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] += vars[ code_1 ];
}

//subn(vA,B);
//vA -= B;
//vA = vA - B;
//	The value of B, where B is an immediate integer value, is subtracted from
//	variable vA. No checking takes place, so if the result is less than 0, it
//	will wrap around to 255 and below.
void cSubn() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] -= code_1;
}

//subv(vA,vB);
//vA -= vB;
//vA = vA - vB;
//	The value of vB, where vB is a variable, is subtracted from variable vA. No
//	checking takes place, so if the result is less than 0, it will wrap around
//	to 255 and below.
void cSubv() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] -= vars[ code_1 ];
}

//lindirectv(vA,vB);
//v[vA] = vB;
//	The value of vB, where vB is a variable, is assigned to the variable which number is specified in vA.
void cLindirectv() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ vars[ code_0 ] ] = vars[ code_1 ];
}

//rindirect(vA,vB);
//vA = v[vB];
//	Variable vA is assigned the value which is contained in the variable which
//	is specified in vB.
void cRindirect() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] = vars[ vars[ code_1 ] ];
}

//lindirectn(vA,B);
//v[vA] = B;
//	The value of vC (where C is the value of vA) is set to B.
void cLindirectn() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ vars[ code_0 ] ] = code_1;
}

//set(fA);
//fA = TRUE;
//	Flag fA is set to a boolean TRUE.
void cSet() {
	U8 code_0 = code_get();
	SetFlag( code_0 );
}

//reset(fA);
//fA = FALSE;
//	Flag fA is set to a boolean FALSE.
void cReset() {
	U8 code_0 = code_get();
	ResetFlag( code_0 );
}

//toggle(fA);
//fA = !fA;
//	Flag fA is toggled. If it was TRUE, it will now be FALSE. If it was FALSE,
//	it will now be TRUE.
void cToggle() {
	U8 code_0 = code_get();
	ToggleFlag( code_0 );
}

//set.v(vA);
//f[vA] = TRUE;
//	Flag fX, where X is the value of vA, is set to a boolean TRUE.
void cSetV() {
	U8 code_0 = code_get();
	SetFlag( vars[ code_0 ] );
}

//reset.v(vA);
//f[vA] = FALSE;
//	Flag fX, where X is the value of vA,is set to a boolean FALSE.
void cResetV() {
	U8 code_0 = code_get();
	ResetFlag( vars[ code_0 ] );
}

//toggle.v(vA);
//f[vA] = !f[vA];
//	Flag fX, where X is the value of vA, is toggled. If it was TRUE, it will
//	now be FALSE. If it was FALSE, it will now be TRUE.
void cToggleV() {
	U8 code_0 = code_get();
	ToggleFlag( vars[ code_0 ] );
}

//new.room(ROOMNO);
//	The interpreter takes the game to another "room", where roomNum is the logic
//	number of the new room to execute. Each interpreter cycle, Logic.000 is
//	executed, which then calls the current room's logic.
//	Switching to another room causes the interpreter to perform the following
//	tasks:
//		+ All of the view objs are unanimated and erased 
//		+ Any sound currently playing is stopped. 
//		+ The controllers are cleared. 
//		+ The pic views and overlays are cleared from the script stack. 
//		+ The ego's control state is set to TRUE. 
//		+ View obj blocking is disabled. 
//		+ The horizon is set back to the default 36. 
//		+ v01 (vOldRoom) is set to v00 (vCurrentRoom) 
//		+ v00 (vCurrentRoom) is set to roomNum. 
//		+ v04 (vViewObj) and v05 (vViewObjBorder) are set to zero. 
//		+ v08 (vFreeMem) is set to 10 (not important). 
//		+ v16 (vEgoView) is set to the current view number view obj #0 is set to.
//		+ If the ego is exiting a room from a border (v02, vEgoBorder), place
//		  the ego view obj (#0) at the opposite end.
//		+ The fNewRoom flag (f05) is set to TRUE. 
//		+ The status line is redrawn.
//		+ The currently active logics all return and execution resumes from the
//		  beginning of logic.000
void cNewRoom() {
	U8 code_0 = code_get();
	
	//code = NewRoom( code_0 );
	NewRoom( code_0 );	// NewRoom always returns NULL, which exits current logic
	
	code_term();	// Terminate current code
}

//new.room.v(vROOMNO);
//	Performs the exact same task as new.room(int), but the number of the new
//	room is specified in the variable vRoomNum instead of an immediate integer.
void cNewRoomV() {
	U8 code_0 = code_get();
	
	//code = NewRoom(vars[ code_0 ]);
	NewRoom(vars[ code_0 ]);	// NewRoom always returns NULL, which exits current logic
	
	code_term();	// Terminate current code
}

//load.logics(A);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded into RAM
//	as they were on the original PC interpreters. However, in the PC
//	interpreters, this command loaded the specified logic resource (logic.num)
//	from the vol file into RAM so it could be executed.
void cLoadLogics() {
	// do nothing thanks to the preload!
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
	U8 code_0 = code_get();
}

//load.logics.v(vA);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded into RAM
//	as they were on the original PC interpreters. However, in the PC
//	interpreters, this command loaded the specified logic resource (logic.vNum)
//	from the vol file into RAM so it could be executed.
void cLoadLogicsV() {
	code_skip(1);
	// do nothing thanks to the preload!
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
}

//call(A);
//	The logic specified by num is executed following the call command. When the
//	logic returns, execution of the previous logic which called it resumes.
void cCall() {
	U8 code_0 = code_get();
	if(CallLogic( code_0 )) {
		
	} else {
		//printf("call-term"); getchar();
		//code = NULL;
		code_term();	// Terminate current code
	}
}

//call.v(vA);
//	The logic specified by vNum is executed following the call command. When the
//	logic returns, execution of the previous logic which called it resumes.
void cCallV() {
	U8 code_0 = code_get();
	if(CallLogic( vars[ code_0 ] )) {
		
	} else {
		//printf("callV-term"); getchar();
		//code = NULL;
		code_term();	// Terminate current code
	}
}

//load.pic(vA);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded into RAM
//	as they were on the original PC interpreters. However, in the PC
//	interpreters, this command loaded the specified picture resource
//	(picture.vPicNum) from the vol file into RAM so it could be used.
void cLoadPic() {
	U8 code_0 = code_get();
	// do nothing thanks to the preload!
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
	
}

//draw.pic(vA);
//	The off screen picture buffer is cleared and the picture specified by
//	variable vPicNum is drawn. It does not actually update the picture on the
//	screen, show.pic() does that.
void cDrawPic() {
	U8 code_0 = code_get();
	
	U8 pic_num = vars[ code_0 ];
	
	//printf("cDrawPic(v["); printf_d(code_0); printf("] ("); printf_d(pic_num); printf("))...");
	//getchar();
	
	// Only render (and process), do not show.
	vagi_pic_draw(pic_num);
	//printf("Back from cDrawPic..."); getchar();
	//vagi_pic_show();
}

//show.pic();
//	The current off screen picture buffer is drawn on the screen filling the
//	whole graphical play area with a background.
void cShowPic() {
	ResetFlag(fPRINTMODE);
	//@TODO: Implement
	/*
	cCloseWindow();
	ShowPic();
	*/
	
	//printf("cShowPic!"); getchar();
	vagi_pic_show();
	//draw_buffer(BUFFER_BANK_VIS, 0,LCD_WIDTH, 0,LCD_HEIGHT, 0,0, true);
	
	PIC_VISIBLE = TRUE;
}

//discard.pic(vA);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded/unloaded
//	to/from RAM as they were on the original PC interpreters. However, in the
//	PC interpreters, this command unloaded the specified picture resource
//	(picture.vPicNum) from memory.
void cDiscardPic() {
	U8 code_0 = code_get();
	// do nothing thanks to the preload!
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
}

//overlay.pic(vA);
//	Performs the same function as draw.pic(var), but does not clear the picture
//	buffer prior to drawing. It simply draws the specified picture in vPicNum
//	over the current picture. Because only white areas are filled, any fills the
//	overlaid picture makes on non-white areas will be ignored.
void cOverlayPic() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	OverlayPic( vars[ code_0 ] );
	*/
	U8 pic_num = vars[ code_0 ];
	
	printf("cOverlayPic(v["); printf_d(code_0); printf("] ("); printf_d(pic_num); printf("))");
	getchar();
	
	//@TODO: Only render (and process), do not show.
	vagi_pic_draw(pic_num);
}

//show.pri.screen();
//	Displays the current picture buffer's priority picture on the screen to view
//	until a key is pressed. It then redraws the screen as normal and resumes
//	the game.
void cShowPriScreen() {
	//@TODO: Implement
	/*
	RotatePicBuf();
	RenderUpdate(0,0,PIC_MAXX,PIC_MAXY);
	
	WaitEnterEsc();
	
	RotatePicBuf();
	RenderUpdate(0,0,PIC_MAXX,PIC_MAXY);
	*/
}

//load.view(VIEWNUM);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded into RAM
//	as they were on the original PC interpreters. However, in the PC
//	interpreters, this command loaded the specified view resource (view.viewNum)
//	from the vol file into RAM so it could be used.
void cLoadView() {
	U8 code_0 = code_get();
	// do nothing thanks to the preload!
	
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
	
	//scriptCount++;
}

//load.view.v(vVIEWNUM);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded into RAM
//	as they were on the original PC interpreters. However, in the PC
//	interpreters, this command loaded the specified view resource (view.vViewNum)
//	from the vol file into RAM so it could be used.
void cLoadViewV() {
	U8 code_0 = code_get();
	// do nothing thanks to the preload!
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
}

//discard.view(VIEWNUM);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded/unloaded
//	to/from RAM as they were on the original PC interpreters. However, in the
//	PC interpreters, this command unloaded the specified view resource
//	(view.viewNum) from memory.
void cDiscardView() {
	U8 code_0 = code_get();
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
}

//animate.obj(oA);
//	If the view object specified by voNum is not currently animated, it sets the
//	view object for updating/cycling/animating, sets it's motion to mtNORMAL,
//	cycle to cyNORMAL, and direction to drNONE.
void cAnimateObj() {
	U8 code_0 = code_get();
	
	if (code_0 >= MAX_VOBJ) {
		//ErrorMessage(ERR_VOBJ_NUM, code_0);
		return;
	}
	
	VOBJ *v = &ViewObjs[code_0];
	if(!(v->flags & oANIMATE)) {
		v->flags		= oUPDATE|oCYCLE|oANIMATE;
		v->motion		= mtNONE;
		v->cycle		= cyNORMAL;
		v->direction	= dirNONE;
	}
}

//unanimate.all();
//	Deactivates and erases all the objects currently visible on the screen.
void cUnanimateAll() {
	VOBJ *v;
	int i;
	EraseBlitLists();
	
	//for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++)
	for(i=0; i < MAX_VOBJ; i++) {
		v = &ViewObjs[i];
		v->flags &= ~(oANIMATE|oDRAWN);
	}
}

//draw(oA);
//	Draws the view object specified by voNum onto the screen. It solidifies
//	it's positioning on screen, erases all the animated objects from the screen,
//	then redraws them all with the new object.
void cDraw() {
	U8 code_0 = code_get();
	DrawObj( code_0 );
}

//erase(oA);
//	Erases the view object specified by voNum from the screen. It erases all
//	the animated objects from the screen, and if the view object is flagged for
//	updating, the unanimated objects as well, then redraws them without the
//	erased object.
void cErase() {
	U8 code_0 = code_get();
	EraseObj( code_0 );
}

//position(oA,X,Y);
//	Simply sets the view object specified by voNum to the coordinates specified
//	by the X and Y parameters passed to the command.
void cPosition() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->prevX = v->x = code_1;
	v->prevY = v->y = code_2;
}

//position.v(oA,vX,vY);
//	Simply sets the view object specified by voNum to the coordinates specified
//	by the variables vX and vY passed to the command.
void cPositionV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->prevX = v->x = vars[ code_1 ];
	v->prevY = v->y = vars[ code_2 ];
}

//get.posn(oA,vX,vY);
//	The variables vX and vY are assigned the values of the the view object
//	specified by voNum's X and Y coordinates.
void cGetPosn() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	
	vars[ code_1 ] = (U8)v->x;
	vars[ code_2 ] = (U8)v->y;
}

//reposition(oA,vDX,vDY);
//	Repositions the view object specified by voNum to new coordinates. The
//	current coordinates are offset by the values of the specified variables vDX
//	and vDY. The vDX and vDY variables are signed values from -128 to +127 and
//	are added to the view object's x and y coordinates. The view object's new
//	position is then solidified.
void cReposition() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	int coord;
	
	v->flags |= oREPOSITIONING;
	
	if( ( (coord = (S8)vars[ code_1 ]) >= 0 ) || (-coord < v->x) )
		v->x += coord;
	else
		v->x = 0;
	
	if( ( (coord = (S8)vars[ code_2 ]) >= 0 ) || (-coord < v->y) )
		v->y += coord;
	else
		v->y = 0;
	
	SolidifyObjPosition(v);
}

//set.view(oA,B);
//	The view object specified by voNum's view is set to the view resource
//	specified in viewNum. The view object can then be drawn with the cels from
//	the new view resource.
void cSetView() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	SetObjView(&ViewObjs[ code_0 ], code_1);
}

//set.view.v(oA,vB);
//	The view object specified by voNum's view is set to the view resource
//	specified in variable vViewNum. The view object can then be drawn with the
//	cels from the new view resource.
void cSetViewV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	SetObjView(&ViewObjs[ code_0 ], vars[ code_1 ]);
}

//set.loop(oA,B);
//	The view object specified by voNum's current loop is set to the loop
//	specified by loopNum. If the current cel number it out of the new loop's
//	range, the current cel number is then set to zero.
void cSetLoop() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	SetObjLoop(&ViewObjs[ code_0 ], code_1);
}

//set.loop.v(oA,vB);
//	The view object specified by voNum's current loop is set to the loop
//	specified by the variable vLoopNum. If the current cel number it out of the
//	new loop's range, the current cel number is then set to zero.
void cSetLoopV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	SetObjLoop(&ViewObjs[ code_0 ], vars[ code_1 ]);
}

//fix.loop(oA);
//	The interpreter normally will set a view object's loop automatically based
//	on it's current direction. This is frequently desired and very useful,
//	however, in the case that you do not want this, a call of this command will
//	prevent the interpreter from doing this. The loop will stay what you set it
//	to no matter what direction the view object is set to.
void cFixLoop() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oFIXEDLOOP;
}

//release.loop(oA);
//	The interpreter can automatically set a view object's loop based on it's
//	current direction. This is frequently desired and very useful for view
//	objects such as people or other moving objects. By calling this command,
//	the view object's current loop will be automatically set by the interpreter.
void cReleaseLoop() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags &= ~oFIXEDLOOP;
}

//set.cel(oA,B);
//	The view object specified by voNum's current cel is set to the number
//	specified by celNum.
void cSetCel() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	SetObjCel(v, code_1);
	v->flags &= ~oSKIPUPDATE;
}

//set.cel.v(oA,vB);
//	The view object specified by voNum's current cel is set to the number
//	specified by the variable vCelNum.
void cSetCelV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	SetObjCel(v, vars[ code_1 ]);
	v->flags &= ~oSKIPUPDATE;
}

//last.cel(oA,vB);
//	The variable vCelNum is set to the number of the last cel in the view object
//	specified by voNum's current loop. This is equal to the total number of
//	cels - 1 as the first cel is 0.
void cLastCel() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[code_0].pLoop[0]-1;
}

//current.cel(oA,vB);
//	The variable vCelNum is set to the number of the current cel in the view
//	object specified by voNum.
void cCurrentCel() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[code_0].cel;
}

//current.loop(oA,vB);
//	The variable vLoopNum is set to the number of the current loop in the view
//	object specified by voNum.
void cCurrentLoop() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[code_0].loop;
}

//current.view(oA,vB);
//	The variable vViewNum is set to the number of the current view in the view
//	object specified by voNum.
void cCurrentView() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[code_0].view;
}

//number.of.loops(oA,vB);
//	The variable vTotalLoops is set to the total number of loops in the view
//	object specified by voNum's current view. The last loop is equal to the
//	total number of loops - 1 as the first loop number is 0.
void cNumberOfLoops() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[code_0].totalLoops;
}

//set.priority(oA,PRI);
//	The view object specified by voNum's priority value is set to priNum.
//	Normally a priority is set automatically based on the Y coordinate of the
//	view object. However, with this command, you can set it to be fixed on any
//	priority value regardless of coordinate.
//
//	The priority value can be anywhere from 0 to 15 (though controls are 0-3
//	and priorities are 4-15). If it is 15, the object will be drawn above
//	everything and ignore all boundaries specified by control lines.
void cSetPriority() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->flags	|= oFIXEDPRIORITY;
	v->priority = code_1;
}

//set.priority.v(oA,vPRI);
//	The view object specified by voNum's priority value is set to vPriNum.
//	Normally a priority is set automatically based on the Y coordinate of the
//	view object. However, with this command, you can set it to be fixed on any
//	priority value regardless of coordinate.
//
//	The priority value can be anywhere from 0 to 15 (though controls are 0-3
//	and priorities are 4-15). If it is 15, the object will be drawn above
//	everything and ignore all boundaries specified by control lines.
void cSetPriorityV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->flags	|= oFIXEDPRIORITY;
	v->priority = vars[ code_1 ];
}

//release.priority(oA);
//	The view object specified by voNum's priority value reset to automatically
//	be set based on the current Y coordinate of the view object.
void cReleasePriority() {
	U8 code_0 = code_get();
	ViewObjs[code_0].flags &= ~oFIXEDPRIORITY;
}

//get.priority(oA,vPRI);
//	vPRI is set to the priority of object oA.
void cGetPriority() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[code_0].priority;
}

//stop.update(oA);
//	If the view object specified by voNum is flagged for updating, all view
//	objects on screen are erased, view object specified is unflagged to not be
//	updated, then the view objects are redrawn.
void cStopUpdate() {                 
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	if(v->flags & oUPDATE) {
		EraseBlitLists();
		v->flags &= ~oUPDATE;
		DrawBlitLists();
	}
}

//start.update(oA);
//	If the view object specified by voNum is not flagged for updating, all view
//	objects on screen are erased, view object specified is flagged to be updated,
//	then the view objects are redrawn.
void cStartUpdate() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	if(!(v->flags & oUPDATE)) {
		EraseBlitLists();
		v->flags |= oUPDATE;
		DrawBlitLists();
	}
}

//force.update(oA);
//	Immediately erases all the current view objects on screen, redraws them,
//	then updates them. The parameter is unused in the majority of interpreters.
//	It was only used in the very early versions when they only updated the
//	specified view object.
void cForceUpdate() {
	U8 code_0 = code_get();
	EraseBlitLists();
	DrawBlitLists();
	//UpdateBlitLists();
}

//ignore.horizon(oA);
//	The view object specified by voNum is set to ignore the horzon, that is, it
//	will be allowed to move past the top horizon line normally a boundary for
//	moving objects.
void cIgnoreHorizon() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oINGOREHORIZON;
}

//observe.horizon(oA);
//	The view object specified by voNum is set to react to the horzon, that is,
//	it will be not be allowed to move past the top horizon line.
void cObserveHorizon() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags &= ~oINGOREHORIZON;
}

//set.horizon(Y);
//	The horizon boundary line is set to the coordinate Y. Objects observing the
//	horizon will not be allowed to move past this line. The default coordinate
//	is 36 and it is reset to the default on each call to new.room(int) or
//	new.room.v(var).
void cSetHorizon() {
	U8 code_0 = code_get();
	horizon = code_0;
}

//object.on.water(oA);
//	The view object specified by voNum is set to be constrained to only move
//	around in areas of the screen which have the water control bit set (any
//	pixels on the priority screen with a value of 3).
void cObjectOnWater() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oWATER;
}

//object.on.land(oA);
//	The view object specified by voNum is set to be constrained to only move
//	around in areas of the screen which do not completely have the water control
//	bit set (any pixels on the priority screen not with a value of 3).
void cObjectOnLand() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oONLAND;
}

//object.on.anything(oA);
//	The view object specified by voNum will be allowed to move around on any
//	type of priority region including water as long as it does not bump into a
//	control boundary.
void cObjectOnAnything() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags &= ~(oONLAND|oWATER);
}

//ignore.objs(oA);
//	The view object specified by voNum will be set to ignore other objects. That
//	is, when a collision check is made, no check between this and another view
//	object will return true. Both view objects need to have their observe objs
//	flag set or they will be unable to collide.
void cIgnoreObjs() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oIGNOREVOBJS;
}

//observe.objs(oA);
//	The view object specified by voNum will be set to observe other objects.
//	That is, when a collision check is made, it will be check between this and
//	another view object. If either view object is not set to observe other view
//	objects, they will be unable to collide.
void cObserveObjs() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags &= ~oIGNOREVOBJS;
}

//distance(oA,oB,vD);
//	The variable vDistance will be set to the absolute disance between the view
//	object specified by voNumA and the view object specified by voNumA. If either
//	of the view objects is not visible, the vDistance is set to 255. Otherwise,
//	it is set to the absolute disance, which will be a maximum of 254.
void cDistance() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	
	VOBJ *v1 = &ViewObjs[ code_0 ], *v2 = &ViewObjs[ code_1 ];
	int  distance;
	
	vars[code_2] =
		(  (!(v1->flags & oDRAWN)) || (!(v2->flags & oDRAWN))  )?
			255:
			((distance =
			  ABS(v2->y - v1->y) +
			  ABS( (v2->x + (v2->width >> 1)) - (v1->x - (v1->width >> 1) )) ) > 254)?
				254:
				distance;
}

//stop.cycling(oA);
//	The view object specified by voNum will no longer cycle, that is, the cels
//	will no longer be animating.
void cStopCycling() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags &= ~oCYCLE;
}

//start.cycling(oA);
//	The view object specified by voNum will be set to cycle, that is, the cels
//	will animate according to their cycle type (cyNORMAL, cyENDOFLOOP,
//	cyREVERSELOOP, cyREVERSECYCLE).
void cStartCycling() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oCYCLE;
}

//normal.cycle(oA);
//	The view object specified by voNum's cycle mode it set to cyNORMAL. With
//	this cycle, the view object will animate looping the cels from first to last.
void cNormalCycle() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->cycle	= cyNORMAL;
	v->flags	|= oCYCLE;
}

//end.of.loop(oA,fB);
//	The flag fNotify is set to FALSE, then the view object specified by voNum's
//	cycle mode it set to cyENDOFLOOP. It will cycle normally from beginning to
//	end. When the last cel of the loop is reached, the flag fNotify will be set.
void cEndOfLoop() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->cycle		= cyENDOFLOOP;
	v->flags		|= oUPDATE|oCYCLE|oSKIPUPDATE;
	ResetFlag(v->loopFlag = code_1);
}

//reverse.cycle(oA);
//	The view object specified by voNum's cycle mode it set to cyREVERSECYCLE.
//	With this cycle, the view object will animate looping the cels from last to
//	first.
void cReverseCycle() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->cycle	= cyREVERSECYCLE;
	v->flags	|= oCYCLE;
}

//reverse.loop(oA,fB);
//	Object oA is cycled in reverse order until the first cel in the loop is
//	reached. Flag fB is reset when the command is issued, and when the first
//	cel is displayed fB is set.
void cReverseLoop() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->cycle	= cyREVERSELOOP;
	v->flags	|= oUPDATE|oCYCLE|oSKIPUPDATE;
	ResetFlag(v->loopFlag = code_1);
}

//cycle.time(oA,vB);
//	The view object specified by voNum's cycle time will be set to the value of
//	variable vTime. The cycle time specifies how frequently to update the view
//	object's animation cycle, that is how often to move to the next frame. This
//	time is measured in interpreter cycles. If the cycle time is set to zero, the
//	view object will not cycle at all.
void cCycleTime() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	
	VOBJ *v = &ViewObjs[ code_0 ];
	v->cycleTime = v->cycleCount = vars[ code_1 ];
}

//stop.motion(oA);
//	The view object specified by voNum's will stop movement. It's direction will
//	be set to dirNONE, and motion back to mtNONE. If the view object specified
//	is the ego, the variable vEgoDirection (v6) is updated and the player control
//	is disabled.
void cStopMotion() {
	U8 code_0 = code_get();
	
	VOBJ *v = &ViewObjs[ code_0 ];
	
	v->direction	= dirNONE;
	v->motion		= mtNONE;
	
	// if it's the ego, update the sys vars
	if(v == &ViewObjs[0]) {
		vars[vEGODIR]	= dirNONE;
		PLAYER_CONTROL	= FALSE;
	}
	
}

//start.motion(oA);
//	The view object specified by voNum's motion will be set to mtNONE and if the
//	view object specified is the ego, the variable vEgoDirection (v6) is updated
//	and the player control is enabled.
void cStartMotion() {
	U8 code_0 = code_get();
	
	VOBJ *v = &ViewObjs[ code_0 ];
	
	v->motion		= mtNONE;
		   
	// if it's the ego, update the sys vars
	if(v == &ViewObjs[0]) {
		vars[vEGODIR]	= dirNONE;
		PLAYER_CONTROL	= TRUE;
	}
}

//step.size(oA,vB);
//	The view object specified by voNum's step size will be set to the value of
//	variable vStepSize. The step size specifies how many pixels to move the
//	view object per movement step.
void cStepSize() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	ViewObjs[ code_0 ].stepSize = vars[ code_1 ];
}

//step.time(oA,vB);
//	The view object specified by voNum's step time will be set to the value of
//	variable vStepTime. The step time specifies how many interpreter cycles to
//	wait before updating the view object's movement.
void cStepTime() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	
	VOBJ *v = &ViewObjs[ code_0 ];
	v->stepCount = v->stepTime = vars[ code_1 ];
}

//move.obj(oA,X,Y,STEPSIZE,fDONEFLAG);
//	Resets the flag fDone, then the view object specified by voNum will be set
//	up to move from it's current coordinate to the new coordinate specified by
//	the X and Y parameters. If the StepSize parameter is nonzero, the view
//	object's step size will be set to it's value. Upon the view object reaching
//	the destination, the flag fDone will be set to TRUE.
void cMoveObj() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	U8 code_4 = code_get();
	
	VOBJ *v = &ViewObjs[ code_0 ];
	
	v->motion			= mtMOVE;
	v->move.x			= code_1;
	v->move.y			= code_2;
	v->move.stepSize	= v->stepSize;
	
	if(code_3)
		v->stepSize = code_3;
	
	ResetFlag(v->move.flag = code_4);
	
	v->flags |= oUPDATE;
	
	if(v == &ViewObjs[0]) // for the ego
		PLAYER_CONTROL = FALSE;
	
	UpdateObjMove(v);
	
}

//move.obj.v(oA,vX,vY,vSTEPSIZE,fDONEFLAG);
//	Resets the flag fDone, then the view object specified by voNum will be
//	set up to move from it's current coordinate to the new coordinate specified
//	by the variables vX and vY parameters. If the vStepSize parameter's value is
//	nonzero, the view object's step size will be set to it's value. Upon the
//	view object reaching the destination, the flag fDone will be set to TRUE.
void cMoveObjV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	U8 code_4 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	
	v->motion			= mtMOVE;
	v->move.x			= vars[ code_1 ];
	v->move.y			= vars[ code_2 ];
	v->move.stepSize	= v->stepSize;
	
	if(vars[ code_3 ])
		v->stepSize = vars[ code_3 ];
	
	ResetFlag(v->move.flag = code_4);
	
	v->flags |= oUPDATE;
	
	if(v == &ViewObjs[0]) // for the ego
		PLAYER_CONTROL = FALSE;
	
	UpdateObjMove(v);
}

//follow.ego(oA,STEPSIZE,fDONEFLAG);
//	Resets the flag fDone, then the view object specified by voNum will be set
//	up to move from it's current coordinate to the new coordinate to the ego's
//	(view object #0) position. If the vStepSize parameter's value is nonzero,
//	the view object's follow step size will be set to it's value, otherwise it
//	will be set to the view object's step size. Upon the view object reaching
//	the destination, the flag fDone will be set to TRUE.
void cFollowEgo() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	
	v->motion 					= mtFOLLOW;
	v->follow.stepSize 			= (code_1 <= v->stepSize)?v->stepSize:code_1;
	v->follow.count 			= 255;
	v->flags 					|= oUPDATE;
	
	ResetFlag(v->follow.flag = code_2);
	
}

//wander(oA);
//	Sets the view object specified by voNum's motion to mtWANDER and causes the
//	view object to wander aimlessly and randomly around the screen. If the view
//	object specified is the ego (view object #0), the player control is disabled.
void cWander() {
	U8 code_0 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	
	v->motion		 = mtWANDER;
	v->flags		|= oUPDATE;
	
	if(v == &ViewObjs[0]) // for the ego
		PLAYER_CONTROL = FALSE;
}

//normal.motion(oA);
//	Sets the view object specified by voNum's motion to mtNONE, in that, it will
//	stop any special movement. If the view object was following, wandering or
//	whatnot, it will now resume normal movement.
void cNormalMotion() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].motion |= mtNONE;
}

//set.dir(oA,vDIR);
//	Sets the view object specified by voNum's direction to the value of
//	variable vDirection, which is a value between 0 and 8.
void cSetDir() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	ViewObjs[ code_0 ].direction = vars[ code_1 ];
}

//get.dir(oA,vDIR);
//	Sets the value of variable vDirection to the view object specified by
//	voNum's current direction.
void cGetDir() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = ViewObjs[ code_0 ].direction;
}

//ignore.blocks(oA);
//	If the view object blocking parameter has been set up, it will allows the
//	view object specified by voNum to ignore this contraint region and move
//	freely.
void cIgnoreBlocks() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags |= oIGNORECTL;
}

//observe.blocks(oA);
//	OIf the view object blocking parameter has been set up, it will force the
//	view object specified by voNum to be contrained to the block region.
void cObserveBlocks() {
	U8 code_0 = code_get();
	ViewObjs[ code_0 ].flags &= ~oIGNORECTL;
}

//block(X1,Y1,X2,Y2);
//	Turns on view object blocking and sets the block to the region specified by
//	the X1, Y1, X2 and Y2 coordinates. Objects that observe blocks will be
//	constrained to this area.
void cBlock() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	objBlock.left	= code_0;
	objBlock.top	= code_1;
	objBlock.right	= code_2;
	objBlock.bottom	= code_3;
	
	VOBJ_BLOCKING	= TRUE;
	
}

//unblock();
//	Turns off view object blocking. Objects will no longer be constrained to
//	the block region, regardless of whether they observe blocks or not.
void cUnblock() {
	VOBJ_BLOCKING = FALSE;
}

//get(iITEM);
//	The inventory item specified by iNum will be placed in the player's
//	inventory. This is done by simply setting the item's current room to 255.
void cGet() {
	U8 code_0 = code_get();
	invObjRooms[ code_0 ]			= 255;
}

//get.v(vITEM);
//	The inventory item specified by the value of variable vInvNum will be placed
//	in the player's inventory. This is done by simply setting the item's current
//	room to 255.
void cGetV() {
	U8 code_0 = code_get();
	invObjRooms[ vars[ code_0 ] ]	= 255;
}

//drop(iITEM);
//	The inventory item specified by iNum will be removed from the player's
//	inventory. This is done by simply setting the item's current room to 0.
void cDrop() {
	U8 code_0 = code_get();
	invObjRooms[ code_0 ] = 0;
}

//put(iITEM,vROOM);
//	The inventory item specified by iNum's current room will be set to the value
//	of RoomNum. If it was in the player's inventory before, it no longer will be.
void cPut() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	invObjRooms[ code_0 ]	= vars[ code_1 ];
}

//put.v(vITEM,vROOM);
//	The inventory item specified by iNum's current room will be set to the value
//	of variable vRoomNum. If it was in the player's inventory before, it no
//	longer will be.
void cPutV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	invObjRooms[ vars[ code_0 ] ]	= vars[ code_1 ];
}

//get.room.v(vITEM,vROOM);
//	The variable vRoomNum will be set to the room number of the inventory item
//	specified by iNum.
void cGetRoomV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_1 ] = invObjRooms[ vars[ code_0 ] ];
}

//load.sound(SOUNDNO);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded into RAM
//	as they were on the original PC interpreters. However, in the PC
//	interpreters, this command loaded the specified sound resource (sound.num)
//	from the vol file into RAM so it could be executed.
void cLoadSound() {
	// do nothing thanks to the preload!

#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
	//code++;
	code_skip(1);
}

//sound(SOUNDNO,fDONEFLAG);
//	Sound SOUNDNO is played. fDONEFLAG is reset when the command is issued, and
//	set when the sound finishes playing or is stopped with the stop.sound
//	command.
//	The sound must be loaded before it is played. This can be done with the
//	load.sound command.
void cSound() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	//@TODO: Implement
	/*
	StartSound(code_0,code_1);
	*/
}

//stop.sound();
//	If a sound is currently playing, it stops, and the sound flag is set to
//	TRUE to indicate the sound is done.
void cStopSound() {
	//@TODO: Implement
	/*
	StopSound();
	*/
}

//print(mA);
//	Displays a message box on the screen with the string of Text as it's message.
//
//	Flag fPrintMode (f15) controls whether or not to automatically close the
//	message box. If set, the box will remain until the close.window() command
//	is called, and the game execution will continue with the message box on the
//	screen.
//
//	If fPrintMode is not set, variable vPrintTime (v21) is used to control the
//	timing of the message box. If nonzero, the message box will remain on the
//	screen for vPrintTime*0.5 seconds, or until the player presses a button.
//	Otherwise, it will simply stay on until the player presses a button.
void cPrint() {
	U8 code_0 = code_get();
	MessageBox(GetMessage(curLog, code_0));
}

//print.v(vA);
//	Displays a message box on the screen with the string of message number
//	vMessage as it's message.
//
//	Flag fPrintMode (f15) controls whether or not to automatically close the
//	message box. If set, the box will remain until the close.window() command
//	is called, and the game execution will continue with the message box on the
//	screen.
//
//	If fPrintMode is not set, variable vPrintTime (v21) is used to control the
//	timing of the message box. If nonzero, the message box will remain on the
//	screen for vPrintTime*0.5 seconds, or until the player presses a button.
//	Otherwise, it will simply stay on until the player presses a button.
void cPrintV() {
	U8 code_0 = code_get();
	MessageBox(GetMessage(curLog, vars[ code_0 ]));
}

//display(ROW,COLUMN,mMESSAGE);
//	Displays a string of text (message number Message) on the screen using the
//	current text color attributes and positions it at Row and Col.
void cDisplay() {
	U8 code_0 = code_get();	// row
	U8 code_1 = code_get();	// col
	U8 code_2 = code_get();	// msg
	/*
	SET_ROWCOL(code_0, code_1);
	DrawAGIString(GetMessage(curLog,code_2));
	*/
	MessageBoxXY(GetMessage(curLog, code_2), code_1, code_0, 32);
}

//display.v(vROW,vCOLUMN,vMESSAGE);
//	Displays a string of text (message number vMessage) on the screen using the
//	current text color attributes and positions it at vRow and vCol.
void cDisplayV() {
	U8 code_0 = code_get();	// row
	U8 code_1 = code_get();	// col
	U8 code_2 = code_get();	// msg
	/*
	SET_ROWCOL(vars[ code_0 ], vars[ code_1 ]);
	DrawAGIString(GetMessage(curLog,vars[ code_2 ]));
	*/
	MessageBoxXY(GetMessage(curLog, vars[ code_2 ]), code_1, code_0, 32);
}

//clear.lines(TOP,BOTTOM,COLOUR);
//	Clears the specified rows of text to the specified color. It clears from
//	RowTop to RowBottom with Color. Each row is eight pixels high.
void cClearLines() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	//@TODO: Implement
	/*
	ClearTextRect(0, code_0, 39, code_1, (code_2)?15:0);
	*/
}

//text.screen();
//	Places the game in text mode with a 40x25 character text screen. The
//	background is cleared to the text background colour attribute, and the text
//	written by default will be in the text foreground colour attribute. It also
//	resets the current text row and columns to 0,0. In text mode, no menus,
//	message boxes, views or other graphics can be used.
void cTextScreen() {
	TEXT_MODE	= TRUE;
	//@TODO: Implement
	/*
	SET_ROWCOL(0,0);
	ClearScreen(((textColour>>4)|(textColour&0xF0))&0x77);
	*/
	lcd_clear();
	
#ifdef _WINDOWS
	SystemUpdate();
#endif
}

//graphics();
//	Places the game back into graphics mode. The screen is redrawn and now
//	menus, message boxes, views or other graphics will be able to be used.
void cGraphics() {
	TEXT_MODE	= FALSE;
	//@TODO: Implement
	//RedrawScreen();
	
	
#ifdef _WINDOWS
	SystemUpdate();
#endif
}

//set.cursor.char(mCHAR);
//	Sets the current cursor character to the first character in the Char message
//	string. The cursor character is the character which trails the line of input
//	on the PC.
void cSetCursorChar() {
	U8 code_0 = code_get();
	cursorChar = GetMessage(curLog,vars[ code_0 ])[0];
}

// set.text.attribute(FG,BG);
//	Sets the text colours. The Foreground can be any colour (0-15), but the
//	Background must be either black (0) or white (non zero).
void cSetTextAttribute() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	textColour = (code_0|(code_1<<4))&0xFF;
}

//shake.screen(A);
//	Shakes the screen num amount of times then continues exection of the game.
//	This is used for earthquake scenes and the like.
void cShakeScreen() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	ShakeScreen(code_0);
	*/
}

//configure.screen(PLAYTOP,INPUTLINE,STATUSLINE);
//	Sets up the screen's elements to specified coordinates. The PlayTop
//	specifies how many pixels from the top the graphical area should be
//	displayed. InputLine, unused on the PC MDA/Hercules and Apple Macintosh
//	AGIs (as well as GBAGI), specifies where to position the input line.
//	StatusLine specifies where to draw the status bar. All are in text rows
//	(ie. 8 pixels high).
void cConfigureScreen() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	minRow		= code_0;
	//minRowY		= (minRow*(SCREEN_WIDTH*CHAR_HEIGHT));
	inputPos	= code_1;
	statusRow	= code_2;
}

//status.line.on();
//	Turns the status bar on making it visible at it's configured coordinate.
void cStatusLineOn() {
	STATUS_VISIBLE = TRUE;
	WriteStatusLine();
}

//status.line.off();
//	Turns the status bar off making it invisible.
void cStatusLineOff() {
	STATUS_VISIBLE = FALSE;
	WriteStatusLine();
}

//set.string(sA,mB);
//	Copies the contents of MessageSrc to StringDest. If the message is longer
//	than 40 bytes, only the first 40 bytes or copied.


void cSetString() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	
	strncpy(strings[ code_0 ], GetMessage(curLog,code_1), MAX_STRINGS_LEN);
	
	// just to make sure--some strncpys may not null it if it reaches maxLen
	strings[ code_0 ][MAX_STRINGS_LEN] = '\0';
}

//get.string(sA,mB,Y,X,L);
//	Retreives a string of input from the player. On GBAGI and other AGI versions
//	such as the PC MDA/Hercules and Apple Macintosh versions, it pops up an
//	input dialog with mCaption as it's caption and does not use the Row/Col
//	parameters. On other versions such as the PC, it places the input line at
//	Row,Col. The string entered will be limited to maxLen, and then stored in
//	sDest.
void cGetString() {
	U8 code_0 = code_get();	// sDest
	U8 code_1 = code_get();	// msg
	U8 code_2 = code_get();	// y
	U8 code_3 = code_get();	// x
	U8 code_4 = code_get();	// maxLen
	//@TODO: Implement
	//printf("cGetString!\n");
	/*
	ExecuteGetStringDialog(FALSE,code_0,GetMessage(curLog,code_1),code_4+1);
	*/
	
	MessageBoxXY(GetMessage(curLog,code_1), code_3, code_2, 80);
	//printf("GetString: ["); printf(GetMessage(curLog,code_1)); printf("]: ");
	putchar(cursorChar);	// Use the defined char (there's a AGI command to set that!)
	gets(strings[code_0]);
}

//word.to.string(wA,sB);
//	This command is supposed to convert a word to a string
void cWordToString() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	strncpy(strings[ code_0 ], wordStrings[ code_1 ], MAX_STRINGS_LEN);
}

//parse(sA);
//	Parses the string contained in sInput as it would a normal line of input
//	by the player. If the parse results in success it will set the flag
//	fInputReceived (f02) to TRUE. It is commonly used to verify that a string
//	entered is valid.
void cParse() {
	U8 code_0 = code_get();
	ResetFlag(fPLAYERCOMMAND);
	ResetFlag(fSAIDOK);
	
	//@TODO: Implement
	//printf("cParse!\n");
	if(code_0 < MAX_STRINGS) {
		//printf("cParse: \""); printf(strings[code_0]); printf("\"");
		ParseInput(strings[code_0]);
	}
}

//get.num(mPROMPT,vNUM);
//	Prompts the player to enter a number with mCaption as the request text. On
//	GBAGI and other AGI versions such as the PC MDA/Hercules and Apple Macintosh
//	versions, it pops up an input dialog with mCaption as it's caption. On other
//	versions such as the PC, it requests the input on the standard input line.
//	The result is stored in variable vNum.
void cGetNum() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	//@TODO: Implement
	/*
	ExecuteGetStringDialog(TRUE,code_1,GetMessage(curLog,code_0),3);
	*/
	printf("cGetNum!");
	printf(GetMessage(curLog,code_0));
	
	//@FIXME: Implement number entry!
	//gets(sztmp);
	//vars[code_1] = atoi(sztmp);
	vars[code_1] = 1;
	
}

//prevent.input();
//	Disables input from the player. The player will not be able to enter input
//	until the accept.input() command is called.
void cPreventInput() {
	INPUT_ENABLED = FALSE;
}

//accept.input();
//	Enables input from the player. The player will now be able to enter game
//	input.
void cAcceptInput() {
	INPUT_ENABLED = TRUE;
}

//set.key(nCODE1,CODE2,cA);
//	Sets the controller specified by cController to be activated by the key
//	specified by codeA and codeB. The combination of the two codes allow the use
//	of ALT+KEY, CTRL+KEY combinations among others. After this is called, when
//	the player presses the set key, the controller will be set to TRUE.
void cSetKey() {
	U8 code_0w = code_get_word();
	U8 code_2 = code_get();
	int i;
	
	//printf("ctrl["); printf_d(code_2); printf("]="); printf_x2(code_0w >> 8); printf_x2(code_0w & 0xff);
	for(i=0; i<MAX_CONTROLLERS-1; i++) {
		if(!ctlMap[i].key) {
			ctlMap[i].key = code_0w;	//bGetW(code);
			ctlMap[i].num = code_2;
			break;
		}
	}
}

//add.to.pic(VIEWNO,LOOPNO,CELNO,X,Y,PRI,MARGIN);
//	Adds a view to the picture as if it were orignally part of the picture
//	including priority and control.
//	The specified View is drawn onto to the background picture with the
//	specified Loop and Cel at the given coordinates, X and Y with Pri as it's
//	priority. Because of the priority, it can be drawn behind other parts of
//	the picture making it seamless. The Margin parameter is the priority control
//	number for the bottom control box. It can be between 0 and 3. If the value is
//	above 3, it will not have a control base. The control box will be a minimum
//	of one pixel high, and as high as the view or the next priority line (which
//	ever is shorter).
void cAddToPic() {
	U8 code_0 = code_get();	// view
	U8 code_1 = code_get();	// loop
	U8 code_2 = code_get();	// cell
	U8 code_3 = code_get();	// x
	U8 code_4 = code_get();	// y
	U8 code_5 = code_get();	// pri
	U8 code_6 = code_get();	// margin
	AddToPic( code_0 , code_1 , code_2 , code_3 , code_4 , code_5 | (code_6<<4) );
}

//add.to.pic.v(vVIEWNO,vLOOPNO,vCELNO,vX,vY,vPRI,vMARGIN);
//	Adds a view to the picture as if it were orignally part of the picture
//	including priority and control.
//	The specified vView is drawn onto to the background picture with the
//	specified vLoop and vCel at the given coordinates, vX and vY with vPri as it's
//	priority. Because of the priority, it can be drawn behind other parts of
//	the picture making it seamless. The vMargin parameter is the priority control
//	number for the bottom control box. It can be between 0 and 3. If the value is
//	above 3, it will not have a control base. The control box will be a minimum
//	of one pixel high, and as high as the view or the next priority line (which
//	ever is shorter).
void cAddToPicV() {
	U8 code_0 = code_get();	// view
	U8 code_1 = code_get();	// loop
	U8 code_2 = code_get();	// cell
	U8 code_3 = code_get();	// x
	U8 code_4 = code_get();	// y
	U8 code_5 = code_get();	// pri
	U8 code_6 = code_get();	// margin
	
	AddToPic( vars[code_0] , vars[code_1] , vars[code_2] , vars[code_3] , vars[code_4] , vars[code_5]|(vars[code_6]<<4) );
}

//status();
//	Brings up the inventory item status screen. In GBAGI, it brings up a dialog
//	box with the list of items the player currently has in their inventory. In
//	the original AGI, it brought up a simple text screen with the names of the
//	objects.
//	If flag fInvSelect (f13) is set, it allows you to select an item to view it
//	and it's description. Otherwise it's simply a list of the items.
void cStatus() {
	//@TODO: Implement
	
	printf("cStatus");
	//ExecuteInvDialog();
}

//save.game();
//	Saves the current game in progress to a file so it can be restored (resumed)
//	at a later time. It executes a save dialog for the user to select a game
//	and/or (re)name it. It then saves all the information needed to a file (or
//	in the case of GBAGI, SRAM).
//	It saves the current variables, flags, strings, inventory information, and
//	significant interpreter variables. As well, in the PC AGIs, it saves the
//	"script", which contains the information regarding which resources have been
//	loaded, which pictures and picviews have been drawn, etc. Because GBAGI
//	already has all the resources in ROM, and therefore, they need not be loaded
//	into RAM to be used, this is not needed. GBAGI only needs to keep track of
//	the pictures drawn and the picviews.
void cSaveGame() {
	//@TODO: Implement
	//SaveGame();
}

//restore.game();
//	Executes the game selection dialog and resumes the selected saved game.
void cRestoreGame() {
	//@TODO: Implement
	//RestoreGame();
	//EnableAllMenuItems();
}

//init.disk();
//	An obsolete command from the original days of AGI on the AppleII and other
//	older versions. It was used to allows the user to format (initialize) a
//	floppy disk for saved games. It is not implemented in newer versions of AGI.
void cInitDisk() {
	// Not to be implemented
}

//restart.game();
//	Restarts the game, discarding any current progress in the game. It restarts
//	the interpreter then sets the fRestart flag (f06) and begins execution of the
//	game from logic.000. The game's logics then process the flag accordingly,
//	generally using it to know whether to start in the first room or from the
//	title screen, whether to set up the menu, assign the controller keys, etc.
void cRestartGame() {
	//@TODO: Implement
	/*
	if(	TestFlag(fRESTARTMODE) ||
		MessageBox("Restart?")
	) {
		cCancelLine();
		AGIInit(TRUE);
		ticks = 0;
		EnableAllMenuItems();
		SetFlag(fRESTART);
		//code = NULL;
		code_term();
	}
	*/
}

//show.obj(VIEWNO);
//	Displays a view resource, specified by ViewNum on the screen along with it's
//	embedded description in a message box. Performs the same task as
//	show.obj.v(var), which is generally used in conjunction with the status()
//	command and f13 to display the inventory items.
void cShowObj() {
	U8 code_0 = code_get();
	ShowObj(code_0);
}

//random(A,B,vC);
//	Generates a random number between Start and End and places it into variable
//	vDest.
void cRandom() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	
	vars[ code_2 ] = (rand() % (((code_1-code_0)+1))) + code_0;
}

//program.control();
//	Releases control of the ego from the player back to the computer. The player
//	will no longer be able to control the ego, only the logic can manipulate the
//	view object. To give control back to the player, a simple call to
//	player.control(); is all that is needed.
void cProgramControl() {
	PLAYER_CONTROL		= FALSE;
}

//player.control();
//	Gives control of the ego back to the player back. The player will now be
//	able to control and move the ego.
void cPlayerControl() {
	PLAYER_CONTROL		= TRUE;
	ViewObjs[0].motion	= mtNONE;
}

//obj.status.v(oA);
//	Displays information regarding the view object specified by variable
//	vVObjNum. It displays the view object number, coordinates, size, priority
//	and step size.
void cObjStatusV() {
	U8 code_0 = code_get();
	
	VOBJ *v = &ViewObjs[ vars[code_0] ];
	//@TODO: Implement
	/*
	sprintf(sztmp,
		"Object %d:\n"
		"x: %d  xsize: %d\n"
		"y: %d  ysize: %d\n"
		"pri: %d\n"
		"stepsize: %d",
		(int)(v-ViewObjs),v->x,v->width,v->y,v->height,v->priority,v->stepSize
	);
	MessageBox(sztmp);
	*/
}

//quit(nCONFIRM);
//	Exits the game. In the case of PC AGIs, it will exit back to the OS such as
//	DOS. In GBAGI, it will exit back to the game selection menu.
//	If the parameter QuitMode is nonzero, it quits immidately. Otherwise it
//	gives the player a dialog box asking whether or not to quit. If the player
//	presses Enter (A) it will quit, otherwise if they press ESC (B) it will not.
void cQuit() {
	U8 code_0 = code_get();
	
	#ifdef AGI_LOGIC_DEBUG
	printf("cQuit!"); getchar();
	#endif
	
	if(code_0 || MessageBox("Really quit?")) {
		//AGIExit();
		QUIT_FLAG = TRUE;
		//code = NULL;
	}
}

//show.mem();
//	Displays the current memory information such as how much memory is being
//	used, the maximum that has been used, etc.
void cShowMem() {
	//MessageBox("Memory cool!");
}

//pause();
//	Simply pauses the game by displaying a message box and waits for the player
//	to press a button or key to close it.
void cPause() {
	//MessageBox("Game paused.");
}

//echo.line();
//	Recalls the last line of input the player made allowing them to modify it
//	and submit it again.
void cEchoLine() {
	//@TODO: Implement
	/*
	ExecuteInputDialog(FALSE);
	*/
}

//cancel.line();
//	Would clear any text currently entered on the input line. This is not needed
//	on AGIs with popup input dialogs such as GBAGI, the PC MDA/Hercules, and
//	Apple Macintosh ports.
void cCancelLine() {
	// no need to implement as the game pauses for input and thus no logic would be executed!
	//code=code;
}

//init.joy();
//	Would calibrate the joystick if one was connected to the computer. It is not
//	needed on GBAGI because the joystick is already fully calibrated.
void cInitJoy() {
	//	MessageBox("Joystick cool!");
	//code=code;
}

//toggle.monitor();
//	An obsolete command used in the early versions of AGI. It would allows the
//	player to toggle between different video modes.
void cToggleMonitor() {
	// Not to be implemented
	//code=code;
}

//version();
//	Displays a message box with the interpreter's name and version information.
void cVersion() {
	MessageBox(
		"VAGI based on\n"
		"GBAGI v"BUILD_VERSION"\n\n"
		"By Brian Provinciano\n"
		"http://www.bripro.com"
	);
}

//script.size(SIZE);
//	Would set the game's script size to the value specified by Size. The script
//	was used by save games to store information regarding which resources were
//	loaded, pictures were drawn, picviews were drawn, etc. It is not used by
//	GBAGI as only picture and picview information is needed.
void cScriptSize() {
	// Not to be implemented
	
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
	U8 code_0 = code_get();
	//printf("cScriptSize="); printf_d(code_0);
}

//set.game.id(mID);
//	This command was used to set the game ID of the game data. Each interpreter
//	contained an embedded ID such as "KQ1". Sierra's games would then run this
//	command with the game id such as "set.game.id("KQ1");" and in initialization
//	logic. If the mID string did not match the interpreter's game id, it would
//	exit. This was likely used to prevent games from being run on an incorrect
//	version of interpreter.
void cSetGameId() {
	U8 code_0 = code_get();
	strncpy(szGameID, GetMessage(curLog,code_0),MAX_ID_LEN);
	szGameID[MAX_ID_LEN] = '\0';
	
	//printf("Game ID: \""); printf(szGameID); printf("\"\n");
}

//log(mLOGMESSAGE);
//	Used for debugging purpouses, this would write current game information to
//	a file named "logfile", such as the current room number, player input, and
//	the message specified by mNote.
void cLog() {
	// Not to be implemented
	//code++;
	code_skip(1);
}

//set.scan.start();
//	Bookmarks the current position in the current logic for later execution.
//	When the logic is called again, it will begin execution from just after
//	this command was called rather than the beginning of the logic.
void cSetScanStart() {
	//logScan[curLog->num] = (U16)(code-curLog->code);
	
	//@TODO: Check if correct!
	#ifdef AGI_LOGIC_DEBUG
	printf("cSetScanStart");getchar();
	#endif
	U16 code_now = vagi_res_tell(curLog->res_h);
	logScan[curLog->num] = (U16)(code_now - curLog->ofs_code);
}

//reset.scan.start();
//	Clears the bookmark in the current logic. When the logic is called again,
//	it will begin execution from the beginning of the logic as it would normally.
void cResetScanStart() {
	
	//@TODO: Check if correct!
	#ifdef AGI_LOGIC_DEBUG
	printf("cResetScanStart");getchar();
	#endif
	
	logScan[curLog->num] = 0;
}

//reposition.to(oA,X,Y);
//	Erases and repositions the view object specified by voNum to new
//	coordinates X,Y.
void cRepositionTo() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	VOBJ *v = &ViewObjs[ code_0 ];
	v->x		= code_1;
	v->y		= code_2;
	v->flags	|= oREPOSITIONING;
	
	SolidifyObjPosition(v);
}

//reposition.to.v(oA,vX,vY);
//	Erases and repositions the view object specified by voNum to new coordinates
//	specified by the values in variables vX and vY.
void cRepositionToV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	
	VOBJ *v = &ViewObjs[ code_0 ];
	v->x		= vars[ code_1 ];
	v->y		= vars[ code_2 ];
	v->flags	|= oREPOSITIONING;
	
	SolidifyObjPosition(v);
	
}

//trace.on();
//	Turns the trace mode on for debugging purpouses. It will only work if
//	fDebug (f10) is set. It allows the user to trace through the logic code as it is
//	executed.
void cTraceOn() {
	// no debugging would be needed--the games are already done, and it would
	// just slow down performance
	//code=code;
}

//trace.info(LOGNUM,TOP,HEIGHT);
//	Configures the trace mode. LogNum specifies the logic which contains the
//	logic command names. They are stored as messages. The Top parameter
//	specifies where to draw the message box (the Y coordinate). The Height
//	parameter specifies how high the message box will be.
void cTraceInfo() {
	// no debugging would be needed--the games are already done, and it would
	// just slow down performance
	//code += 3;
	code_skip( 3);
}

//print.at(mA,X,Y,W);
//	Displays a message box with mText as it's message at the coordinates
//	specified in X and Y. Width specifies the maximum width of a message box,
//	which by default is 30. It operates identical to the normal print command
//	otherwise.
void cPrintAt() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	MessageBoxXY(GetMessage(curLog,code_0),code_1,code_2,code_3);
}

//print.at.v(vA,vX,vY,vW);
//	Displays a message box with the text of the message number specified in the
//	variable vMessage as it's message at the coordinates specified in X and Y.
//	Width specifies the maximum width of a message box, which by default is 30.
//	It operates identical to the normal print command otherwise.
void cPrintAtV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	MessageBoxXY(GetMessage(curLog,vars[ code_0 ]),code_1,code_2,code_3);
}

//discard.view.v(vVIEWNUM);
//	In GBAGI, all resources are in ROM, thus do not need to be loaded/unloaded
//	to/from RAM as they were on the original PC interpreters. However, in the
//	PC interpreters, this command unloaded the specified view resource
//	(view.vViewNum) from memory.
void cDiscardViewV() {
#ifdef _FULL_BLIT_RESFRESHES
	EraseBlitLists();
	DrawBlitLists();
#endif
	//code++;
	code_skip(1);
}

//clear.text.rect(Y1,X1,Y2,X2,COLOUR);
//	Clears the specified text region to the specified color. It clears from
//	RowTop to RowBottom from the ColTop to the ColBottom with Color. Each row
//	and column is eight pixels high/wide (in the case of the GBA, 8x6). The
//	only colors available are black (0) and white (non zero).
void cClearTextRect() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
	U8 code_4 = code_get();
	//@TODO: Implement
	/*
	ClearTextRect(code_1,code_0,code_3,code_2,(code_4)?15:0);
	*/
}

// set.upper.left(A,B);
//	An obsolete command from the early versions of AGI. It does nothing in the
//	later versions.
void cSetUpperLeft() {
	// does nothing
	//code += 2;
	code_skip( 2);
}

//set.menu(mNAME);
//	Adds a menu to the menubar for placing items under with mText as it's
//	caption. All items added following will be added under this menu.
void cSetMenu() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	SetMenu(GetMessage(curLog,code_0));
	*/
}

//set.menu.item(mNAME,cA);
//	Adds a menu item to the current menu with mText as it's caption. It's
//	controller is set to cController, and every time the player selects the
//	item, the controller will be set to TRUE.
void cSetMenuItem() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	//@TODO: Implement
	/*
	SetMenuItem(GetMessage(curLog,code_0),code_1);
	*/
}

//submit.menu();
//	Finalizes the menubar with it's menus and menu items. It will now be ready to be accessed through menu.input().
void cSubmitMenu() {
	//@TODO: Implement
	/*
	SubmitMenu();
	*/
}

//enable.item(cA);
//	Enables all menu items which have cController assigned to them.
void cEnableItem() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	ToggleMenuItem(code_0,TRUE);
	*/
}

//disable.item(cA);
//	Disables all menu items which have cController assigned to them.
void cDisableItem() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	ToggleMenuItem(code_0,FALSE);
	*/
}

//menu.input();
//	Activates the menubard to item selection. If fMenuInput (f14) is set to
//	FALSE or the allow.menu(int) is called with a FALSE value, it will not be
//	activated. Once activated, if the player selects and item, it's controller
//	will be set.
void cMenuInput() {
	//@TODO: Implement
	/*
	MenuInput();
	*/
}

//show.obj.v(vVIEWNO);
//	Displays a view resource, specified by the value of variable vViewNum on
//	the screen along with it's embedded description in a message box. It is
//	generally used in conjunction with the status() command and f13 to display
//	the inventory items.
void cShowObjV() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	ShowObj(vars[code_0]);
	*/
}

//open.dialogue();
//	If the close.dialogue() command has been called and a new message box is
//	displayed, the previous one will not be cleared. However, if open.dialogue()
//	is called, it will close the message box and refresh the screen before
//	drawing the new one.
void cOpenDialogue() {
	WINDOW_OPEN = TRUE;
}

//close.dialogue();
//	The close.dialogue() command causes the currently displayed message box to
//	be closed, but not erased, and still remain on the screen, and thus, when a
//	new message box is drawn, it could be drawn over the old one.
void cCloseDialogue() {
	WINDOW_OPEN = FALSE;
}

//mul.n(vA,B);
//vA *= B;
//vA = vA * B;
//	The variable vA will be multiplied by the value of B, where B is an
//	immediate integer value. No checking takes place, so if the result is
//	greater than 255, it will wrap around to zero and above.
void cMulN() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] *= code_1;
}

//mul.v(vA,vB);
//vA *= vB;
//vA = vA * vB;
//	The variable vA will be multiplied by the value of vB, where vB is variable.
//	No checking takes place, so if the result is greater than 255, it will wrap
//	around to zero and above.
void cMulV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	vars[ code_0 ] *= vars[ code_1 ];
}

//div.n(vA,B);
//vA /= B;
//vA = vA / B;
//	The variable vA will be divided by the value of B, where B is an immediate
//	integer value.
void cDivN() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	if(code_1) // the real interpreter would crash upon dividing by zero, but I don't need to be _that_ accurate!
		vars[ code_0 ] /= code_1;
}

//div.n(vA,vB);
//vA /= vB;
//vA = vA / vB;
//	The variable vA will be divided by the value of vB, where vB is variable.
void cDivV() {
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	if(vars[ code_1 ]) // the real interpreter would crash upon dividing by zero, but I don't need to be _that_ accurate!
		vars[ code_0 ] /= vars[ code_1 ];
}

//close.window();
//	The close.window() command causes the currently displayed message box to be
//	closed and erased from the screen.
void cCloseWindow() {
	WINDOW_OPEN = FALSE;
	//@TODO: Implement
	/*
	RenderUpdate(0,0,PIC_MAXX,PIC_MAXY);
	*/
}

//	The set.simple() command sets the save.game mode to automatic by giving a
//	predefined name for a save game in sAutoSave. Saving a game will
//	automatically save to the game named sAutoSave.
void cSetSimple() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	strncpy(szAutoSave, strings[ code_0 ], sizeof(szAutoSave));
	*/
}

//	The push.script command saves the current position in the game script for
//	later reteival via the pop.script() command. It does not actually push any
//	value onto the stack though, so if you use it twice, the previous saved
//	value will be lost.
void cPushScript() {
	#ifdef AGI_LOGIC_DEBUG
	printf("cPushScript");
	#endif
	//pushedScriptCount = scriptCount;
	//code=code;
}

//	The pop.script command will retreive the script position saved by
//	push.script() and set it to the current position, discarding any elements
//	added to the script after the push.script() command has been called.
void cPopScript() {
	#ifdef AGI_LOGIC_DEBUG
	printf("cPopScript");
	#endif
	//scriptCount = pushedScriptCount;
	//code=code;
}

//	The hold.key command, used in such games as Mixed-Up Mother Goose sets the
//	interpreter in a mode where the player must hold down the arrow key or
//	joystick to move the ego. Once they release the button, the ego will stop
//	moving. In normal mode, the player would press it once to move the ego, and
//	again to stop the ego.
void cHoldKey() {
	WALK_HOLD	= TRUE;
}

//	The set.pri.base command sets up a new priority base. Normally the first 48
//	rows of pixels are priority 4, and the following are 5, 6, 7, etc.,
//	incrementing the priority every 12 rows. This allows the player to have a
//	more priorities in a smaller region.
void cSetPriBase() {
	int y,db;
	
	U8 code_0 = code_get();
	PRI_FIXED = FALSE;
	
	for(y=0; y<PIC_HEIGHT; y++) {
		if((db = y-code_0) >= 0) {
			if((priTable[y] = (db*10)/(PIC_HEIGHT-code_0)+5) > 15)
				priTable[y] = 15;
		} else
			priTable[y] = 4;
	}
}

//	In GBAGI, all resources are in ROM, thus do not need to be loaded/unloaded
//	to/from RAM as they were on the original PC interpreters. However, in the PC
//	interpreters, this command unloaded the specified sound resource
//	(sound.soundNum) from memory.
void cDiscardSound() {
	// not to be implemented, not needed
	U8 code_0 = code_get();
}

//	The PC and AppleII AGIs were actually the only two not to have mouse support.
//	The Amiga, Atari ST, Macintosh and Apple IIgs versions all had support for
//	the mouse. This function makes the mouse cursor invisible.
void cHideMouse() {
	// not to be implemented, the GBA has no mouse, heh
}

//	This command sets whether or not the menubar is enabled. If mode is 0, it
//	will be disabled, otherwise, it will be enabled.
void cAllowMenu() {
	U8 code_0 = code_get();
	//@TODO: Implement
	/*
	MENU_SELECTABLE = code_0;
	*/
}

//	In the versions of AGI which had mouse support, the Amiga, Atari ST,
//	Macintosh and Apple IIgs versions, this function made the mouse cursor
//	visible if it were previously hidden.
void cShowMouse() {
	// not to be implemented, the GBA has no mouse, heh
	//code=code;
}

//	In the versions of AGI which had mouse support, the Amiga, Atari ST,
//	Macintosh and Apple IIgs versions, this function set up an invisible barrier
//	in which the mouse cursor was restricted to.
void cFenceMouse() {
	// not to be implemented, the GBA has no mouse, heh
	U8 code_0 = code_get();
	U8 code_1 = code_get();
	U8 code_2 = code_get();
	U8 code_3 = code_get();
}

//	In the versions of AGI which had mouse support, the Amiga, Atari ST,
//	Macintosh and Apple IIgs versions, this function assigned the two given
//	variables, vX and vY to the current coordinates of the mouse.
void cMousePosn() {
	// not to be implemented, the GBA has no mouse, heh
	U8 code_0 = code_get();
	U8 code_1 = code_get();
}

//	The release.key command sets the interpreter back to normal ego movement
//	mode. In this mode, the player presses the directional key or button once to
//	move the ego, and again to stop the ego.
void cReleaseKey() {
	WALK_HOLD	= FALSE;
}

//	Toggles the movement of the ego. Only used in the last versions of AGI
//	version 3.
void cAdjEgoMoveToXY() {
	//UnimplementedBox(code[-1]);
	//code=code;
}

#endif