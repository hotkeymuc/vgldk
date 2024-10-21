#ifndef __AGI_VIEW_RENDER_C__
#define __AGI_VIEW_RENDER_C__

void UnBlitVObj(VOBJ *v) {
	//printf("UnBlitVObj");getchar();
	code_segment_call_p(2, code_segment_1__UnBlitVObj_addr, v);
}
void BlitVObj(VOBJ *v) {
	//printf("BlitVObj");getchar();
	code_segment_call_p(2, code_segment_1__BlitVObj_addr, v);
}

void AddObjPicPri(VOBJ *v) {
	//printf("AddObjPicPri");getchar();
	code_segment_call_p(2, code_segment_1__AddObjPicPri_addr, v);
}


#endif