#ifndef __BINT_H
#define __BINT_H

// BINT - Global Interrupt Handler


volatile word bint_timer;

// __interrupt
//void bint() __naked
void bint();

#endif	// __BINT_H