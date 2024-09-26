/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

#ifndef __AGI_VARS_H__
#define __AGI_VARS_H__


#define MAX_VARS			256
extern U8 vars[MAX_VARS];

#define MAX_FLAGS			256
extern U8 flags[MAX_FLAGS/8];
extern U8 toggleFlags[MAX_FLAGS];

#define MAX_CONTROLLERS		50
extern U8 controllers[MAX_CONTROLLERS];

typedef struct {
	U16 key;
	U8 num;
} CTLMAP;

extern CTLMAP ctlMap[MAX_CONTROLLERS];

#define MAX_STRINGS			24
#define MAX_STRINGS_LEN		40
extern char strings[MAX_STRINGS][MAX_STRINGS_LEN+1];


enum {
	vROOMNUM,
	vROOMPREV,
	vEGOBORDER,
	vSCORE,
	vOBJECT,
	vOBJBORDER,
	vEGODIR,
	vSCOREMAX,
	vMEMORY,
	vUNKWORD,
	vDELAY,
	vSECONDS,
	vMINUTES,
	vHOURS,
	vDAYS,
	vJOYSTICK,
	vEGOVIEWNUM,
	vERROR1,
	vERROR2,
	vKEYPRESSED,
	vCOMPUTER,
	vPRINTDURATION,
	vSOUNDTYPE,
	vSOUNDVOL,
	vMAXINPUT,
	vINVITEM,
	vMONTIOR
};

enum {
	fEGOONWATER,
	fEGOHIDDEN,
	fPLAYERCOMMAND,
	fEGOONSIGNAL,
	fSAIDOK,
	fNEWROOM,
	fRESTART,
	fSCRIPTDISABLED,
	fJOYSTICK,
	fSOUND,
	fDEBUG,
	fFIRSTLOGIC0,
	fRESTORE,
	fINVSELECT,
	fMENU,
	fPRINTMODE,
	fRESTARTMODE,
	fDUMMY_17,
	fDUMMY_18,
	fDUMMY_19,
	fLOOPMODE
};


U16 ABS(int x);

void SetFlag(U8 num);
void ResetFlag(U8 num);
void ToggleFlag(U8 num);
BOOL TestFlag(U8 num);

void ClearVars(void);
void ClearFlags(void);
void ClearControllers(void);
void ClearControlKeys(void);


#endif