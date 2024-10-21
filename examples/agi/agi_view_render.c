#ifndef __AGI_VIEW_RENDER_C__
#define __AGI_VIEW_RENDER_C__

void BlitVObj(VOBJ *v) {
	vagi_res_handle_t h;
	
	
	//printf("BlitVObj");getchar();
	
	/*
	U8 *pBuf, *pBufStart, *pBufEnd, *celData = v->pCel, *pPtr;
	int
		run = 0, pri, col, lastPri,
		celPri = (v->priority << 4)&0xF0, transCol = celData[2]&0xF,
		celWidth = celData[0], celHeight = celData[1];
	BOOL
		VIEW_INVISIBLE	= TRUE,
		MIRRORED = (celData[2]&0x80) && (((celData[2]>>4)&7) != v->loop);
	
	pBuf	= MAKE_PICBUF_PTR(v->x,v->y-(celHeight-1));
	pBufEnd = pictureBuf+(PIC_WIDTH*PIC_MAXY);
	
	celData			+= 3;
	
		if(MIRRORED)
			pBuf+=celWidth-1;
		pBufStart = pBuf;
		// draw the rows of pixels decoding the RLE
		while(celHeight) {
			if(*celData) {
				// read the data. the byte's lo nybble is the run, the hi is the colour
				run = *celData & 0x0F;
				if((col = (*celData++ >> 4)) == transCol)
					if(MIRRORED)
						pBuf -= run;
					else
						pBuf += run;
				else {
					do {
						CHECK_PIC_PRI();
						if(pri != -1) {
							*pBuf	 		= pri|col;
							VIEW_INVISIBLE 	= FALSE;
						}
						if(MIRRORED)
							pBuf--;
						else
							pBuf++;
					} while(--run);
				}
			} else {
				// if the data is '\0', the row is done, go to the next one
				celData++;
				celHeight--;
				pBufStart += PIC_WIDTH;
				pBuf = pBufStart;
			}
		}
	
	if(!v->num) {  // 'tis the ego
		if(VIEW_INVISIBLE)
			SetFlag(fEGOHIDDEN);
		else
			ResetFlag(fEGOHIDDEN);
	}
	*/
	bool VIEW_INVISIBLE	= true;	// Keep track if the view has ANY pixel shown (for fEGOHIDDEN)
	
	h = vagi_res_open(AGI_RES_KIND_VIEW, v->view);
	vagi_res_seek_to(h, v->oCel);
	vagi_res_skip(h, 3);	// Skip header: U8 width, height, settings
	
	//U8 cell_mirroring = v->settings >> 4;
	U8 cell_mirroring =  (v->settings & 0x80) && (((v->settings >> 4) & 7) != v->loop);
	U8 cell_transparency = v->settings & 0x0f;
	
	// Draw cell!
	
	/*
	// Test: draw a sprite!
	int y = v->y - v->height;
	if (y < 0) return;
	
	draw_buffer_sprite_priority(
		BUFFER_BANK_PRI,
		
		&sprite_data[0], sprite_width, sprite_height,
		sprite_transparency,	// trans
		
		v->x, y,	//v->y - v->height,	//sprite_height,
		
		v->priority,
		
		0,0	//, true
	);
	*/
	U8 b;
	U8 col;
	U8 c_prio;
	U8 gx;
	U8 gy;
	U8 count;
	U8 i;
	U8 ix;
	U8 iy;
	U8 sx;
	U8 sy;
	U8 bx;
	U8 by;
	
	U8 cv;
	
	gx = v->x + (cell_mirroring ? (v->width - 1) : 0);
	gy = v->y - v->height;
	
	sy = game_to_screen_y((word)gy);
	if (sy >= LCD_HEIGHT) return;
	by = screen_to_buffer_x(sy);
	
	ix = 0;
	iy = 0;
	
	buffer_switch(BUFFER_BANK_PRI);	// We need to check priority a lot!
	
	while(true) {
		b = vagi_res_read(h);
		
		// If 0: go to next row
		if (b == 0) {
			iy ++;
			if (iy >= v->height) break;
			
			// Prepare next row of pixels
			ix = 0;
			gy ++;
			gx = v->x + (cell_mirroring ? (v->width - 1) : 0);
			
			sy = game_to_screen_y((word)gy);
			//if (sy >= LCD_HEIGHT) break;
			by = screen_to_buffer_y((word)sy);
			if (by >= BUFFER_HEIGHT) break;
			
			#ifdef VAGI_DRAW_DITHER
				// Reset dither error
				//v_err = 0;
				//v_err = ((ix * iy) * 0x77) & 0x1f;
				//v_err = (int)(rand() & 0x7f) - 64;
				v_err = (int)(rand() & 0x1f) - 0x0f;
			#endif
			continue;
		}
		col = b >> 4;
		count = b & 0x0f;
		
		if (col == cell_transparency) {
			// Skip transparent pixels in cel
			ix += count;
			if (cell_mirroring) gx -= count; else gx += count;
			
		} else {
			cv = AGI_PALETTE_TO_LUMA[col];
			// RLE
			for(i = 0; i < count; i ++) {
				
				// Scale and check priority...
				sx = game_to_screen_x((word)gx);
				bx = screen_to_buffer_x((word)sx);
				if (bx < BUFFER_WIDTH) {
					
					// Get priority value at current position
					// If we detect a control priority (< 3): scan down until we hit a "normal" priority!
					byte bby = by;
					do {
						c_prio = buffer_get_pixel_4bit(bx, bby);
						if (c_prio >= 3) break;	// Normal priority found (GBAGI searches further down if "priority < 3")
						bby++;	// Go down
					} while (bby < BUFFER_HEIGHT);
					
					if (c_prio <= v->priority) {
						
						VIEW_INVISIBLE = false;	// A pixel was drawn, so the view is visible in some form!
						
						//lcd_set_pixel_4bit(sx, sy, cv);
						#if VAGI_SCREEN_W > 160
							// If screen resolution is horizontally scaled: Fill gaps by doubling up!
							for(byte xx = sx; xx < sx+2; xx++) {
								vagi_set_pixel_8bit(xx, sy, cv);
							}
						#else
							vagi_set_pixel_8bit(sx, sy, cv);
						#endif
					} // end of "priority"
				} // end of screen clip
				
				ix ++;
				if (cell_mirroring) gx --; else gx ++;
			}
		}
		
	}
	
	vagi_res_close(h);
	
	/*
	// Since we put this into a different code segment, we have no access to Set/ResetFlag
	if(v->num == 0) {  // 'tis the ego
		if (VIEW_INVISIBLE)
			SetFlag(fEGOHIDDEN);
		else
			ResetFlag(fEGOHIDDEN);
	}
	*/
	v->invisible = VIEW_INVISIBLE;	// Flags will be set in vagi_loop() according to this
	
	#ifdef VIEW_SHOW_NUM
	// Show VObj number as a label
	lcd_draw_glypth_at(game_to_screen_x(v->x), game_to_screen_y(v->y), ('0' + v->num));
	#endif
}


void UnBlitVObj(VOBJ *v) {
	//htk: Erase object by drawing background buffer over it
	
	// Draw over its PREVIOUS position and size
	byte ssx = game_to_screen_x(v->prevX);
	byte ssy = game_to_screen_y(v->prevY);
	byte ssw = game_to_screen_x(v->prevWidth);
	byte ssh = game_to_screen_y(v->prevHeight);
	
	// Widen (and crop)
	byte b = 1;	// Over-draw by that many pixels
	if (ssx >= b) ssx -= b; else ssx = 0;
	ssw += 2*b;
	if (ssx+ssw >= LCD_WIDTH) ssw = LCD_WIDTH-ssx;
	
	
	// Top position
	int y = ssy - ssh - b;
	if (y < 0) return;
	//if (ssy >= b) ssy -= b; else ssy = 0;
	
	// Redraw object background
	draw_buffer(
		BUFFER_BANK_VIS,
		
		ssx,       ssx + ssw,
		y,
		ssy+b
		#ifdef VIEW_SHOW_NUM
		+font_char_height
		#endif
		,	//ssy - ssh, ssy,
		
		0,0	//, true
	);
	
}


void AddObjPicPri(VOBJ *v) {
	//U8 *pBuf;
	int x,y,height,celMaxX,pHigh=0;
	
	
	//printf("AddObjPicPri");getchar();
	
	if(!(v->priority & 0xF))
		v->priority |= priTable[v->y];
	
	BlitVObj(v);
	
	if(v->priority&0xC0)
		return;
	
	// get the height
	y = v->y;
	do {
		pHigh++;
		if(y <= 0)
			break;
		y--;
	} while(priTable[y] == priTable[v->y]);
	
	height = (v->height > pHigh)? pHigh:v->height;
	
	// draw  a control box for the view object
	//pBuf = MAKE_PICBUF_PTR(v->x,v->y);
	buffer_switch(BUFFER_BANK_PRI);	// We need to access priority
	
	U8 sx = game_to_screen_x((word)v->x);
	U8 bx = screen_to_buffer_x((word)sx);
	U8 sy = game_to_screen_y((word)v->y);
	U8 by = screen_to_buffer_y((word)sy);
	
	// draw a control line on the bottom
	x = screen_to_buffer_x(game_to_screen_x((word)(v->width)));
	if (x > 0)
	do {
		//*(pBuf++) = (v->priority&0xF0) | (*pBuf & 0x0F);
		buffer_set_pixel_4bit(bx, by, v->priority);
		bx ++;
	} while(--x);
	
	// now if the view is larger than 1px high, draw the rest of the box
	if(height > 1) {
		//pBuf = MAKE_PICBUF_PTR(v->x,v->y);
		bx = screen_to_buffer_x((word)sx);
		
		// draw the left and right sides of the box
		celMaxX = v->width - 1;
		U8 sx2 = game_to_screen_x((word)(v->x + celMaxX));
		U8 bx2 = screen_to_buffer_x((word)sx2);
		//y = height-1;
		y = screen_to_buffer_y(game_to_screen_y((word)(height-1)));
		if (y > 0)
		do {
			//pBuf 			-= PIC_WIDTH;
			//pBuf[0]			= (v->priority&0xF0) | (pBuf[0]&0xF);
			//pBuf[celMaxX]	= (v->priority&0xF0) | (pBuf[celMaxX]&0xF);
			buffer_set_pixel_4bit(bx, by, v->priority);
			buffer_set_pixel_4bit(bx2, by, v->priority);
			by --;
		} while(--y);
		
		// draw the top line
		//x = v->pCel[0] - 2;
		x = screen_to_buffer_x(game_to_screen_x((word)(v->width)));
		if (x > 0)
		do {
			//*++pBuf = (v->priority&0xF0) | (*pBuf & 0x0F);
			bx ++;
			buffer_set_pixel_4bit(bx, by, v->priority);
		} while(--x);
	}
	
}


#endif
