#ifndef __AGI_COMMANDS_H__
#define __AGI_COMMANDS_H__

/*
 *  AGI Commands
* Based on GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 * For details: https://github.com/Davidebyzero/GBAGI.git
*/

#define MAX_TESTCMD		19
#define MAX_AGICMD		256//182

void InvalidBox(void);


void cEvalZ(void);
void cEqualn(void);
void cEqualv(void);
void cLessn(void);
void cLessv(void);
void cGreatern(void);
void cGreaterv(void);
void cIsset(void);
void cIssetv(void);
void cHas(void);
void cObjInRoom(void);
void cPosn(void);
void cController(void);
void cHaveKey(void);
void cSaid(void);
void cCompareStrings(void);
void cObjInBox(void);
void cCenterPosn(void);
void cRightPosn(void);

void cReturn(void);
void cIncrement(void);
void cDecrement(void);
void cAssignn(void);
void cAssignv(void);
void cAddn(void);
void cAddv(void);
void cSubn(void);
void cSubv(void);
void cLindirectv(void);
void cRindirect(void);
void cLindirectn(void);
void cSet(void);
void cReset(void);
void cToggle(void);
void cSetV(void);
void cResetV(void);
void cToggleV(void);
void cNewRoom(void);
void cNewRoomV(void);
void cLoadLogics(void);
void cLoadLogicsV(void);
void cCall(void);
void cCallV(void);
void cLoadPic(void);
void cDrawPic(void);
void cShowPic(void);
void cDiscardPic(void);
void cOverlayPic(void);
void cShowPriScreen(void);
void cLoadView(void);
void cLoadViewV(void);
void cDiscardView(void);
void cAnimateObj(void);
void cUnanimateAll(void);
void cDraw(void);
void cErase(void);
void cPosition(void);
void cPositionV(void);
void cGetPosn(void);
void cReposition(void);
void cSetView(void);
void cSetViewV(void);
void cSetLoop(void);
void cSetLoopV(void);
void cFixLoop(void);
void cReleaseLoop(void);
void cSetCel(void);
void cSetCelV(void);
void cLastCel(void);
void cCurrentCel(void);
void cCurrentLoop(void);
void cCurrentView(void);
void cNumberOfLoops(void);
void cSetPriority(void);
void cSetPriorityV(void);
void cReleasePriority(void);
void cGetPriority(void);
void cStopUpdate(void);
void cStartUpdate(void);
void cForceUpdate(void);
void cIgnoreHorizon(void);
void cObserveHorizon(void);
void cSetHorizon(void);
void cObjectOnWater(void);
void cObjectOnLand(void);
void cObjectOnAnything(void);
void cIgnoreObjs(void);
void cObserveObjs(void);
void cDistance(void);
void cStopCycling(void);
void cStartCycling(void);
void cNormalCycle(void);
void cEndOfLoop(void);
void cReverseCycle(void);
void cReverseLoop(void);
void cCycleTime(void);
void cStopMotion(void);
void cStartMotion(void);
void cStepSize(void);
void cStepTime(void);
void cMoveObj(void);
void cMoveObjV(void);
void cFollowEgo(void);
void cWander(void);
void cNormalMotion(void);
void cSetDir(void);
void cGetDir(void);
void cIgnoreBlocks(void);
void cObserveBlocks(void);
void cBlock(void);
void cUnblock(void);
void cGet(void);
void cGetV(void);
void cDrop(void);
void cPut(void);
void cPutV(void);
void cGetRoomV(void);
void cLoadSound(void);
void cSound(void);
void cStopSound(void);
void cPrint(void);
void cPrintV(void);
void cDisplay(void);
void cDisplayV(void);
void cClearLines(void);
void cTextScreen(void);
void cGraphics(void);
void cSetCursorChar(void);
void cSetTextAttribute(void);
void cShakeScreen(void);
void cConfigureScreen(void);
void cStatusLineOn(void);
void cStatusLineOff(void);
void cSetString(void);
void cGetString(void);
void cWordToString(void);
void cParse(void);
void cGetNum(void);
void cPreventInput(void);
void cAcceptInput(void);
void cSetKey(void);
void cAddToPic(void);
void cAddToPicV(void);
void cStatus(void);
void cSaveGame(void);
void cRestoreGame(void);
void cInitDisk(void);
void cRestartGame(void);
void cShowObj(void);
void cRandom(void);
void cProgramControl(void);
void cPlayerControl(void);
void cObjStatusV(void);
void cQuit(void);
void cShowMem(void);
void cPause(void);
void cEchoLine(void);
void cCancelLine(void);
void cInitJoy(void);
void cToggleMonitor(void);
void cVersion(void);
void cScriptSize(void);
void cSetGameId(void);
void cLog(void);
void cSetScanStart(void);
void cResetScanStart(void);
void cRepositionTo(void);
void cRepositionToV(void);
void cTraceOn(void);
void cTraceInfo(void);
void cPrintAt(void);
void cPrintAtV(void);
void cDiscardViewV(void);
void cClearTextRect(void);
void cSetUpperLeft(void);
void cSetMenu(void);
void cSetMenuItem(void);
void cSubmitMenu(void);
void cEnableItem(void);
void cDisableItem(void);
void cMenuInput(void);
void cShowObjV(void);
void cOpenDialogue(void);
void cCloseDialogue(void);
void cMulN(void);
void cMulV(void);
void cDivN(void);
void cDivV(void);
void cCloseWindow(void);
void cSetSimple(void);
void cPushScript(void);
void cPopScript(void);
void cHoldKey(void);
void cSetPriBase(void);
void cDiscardSound(void);
void cHideMouse(void);
void cAllowMenu(void);
void cShowMouse(void);
void cFenceMouse(void);
void cMousePosn(void);
void cReleaseKey(void);
void cAdjEgoMoveToXY(void);

void ExecuteGoto(void);
void ExecuteIF(void);
void SkipORTrue(void);
void SkipANDFalse(void);

typedef struct {
	#ifdef AGI_COMMANDS_INCLUDE_NAMES
	char *name;
	#else
	// no name field to safe space
	#endif
	void (*func)(void);
	U8 nParams;
	U8 pMask;
} AGICMD;

typedef struct {
	#ifdef AGI_COMMANDS_INCLUDE_NAMES
	char *name;
	#else
	// no name field to safe space
	#endif
	void (*func)(void);
	U8 nParams;
	U8 pMask;
} AGITEST;

extern const AGITEST testCommands[MAX_TESTCMD];
extern const AGICMD agiCommands[MAX_AGICMD];


#endif