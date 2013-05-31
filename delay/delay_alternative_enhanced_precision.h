/*

lowlevel delay routines

Designed by Shane Tolmie of www.microchipC.com corporation.  Freely distributable.
Questions and comments to webmaster@microchipC.com.

Completely re-written by Matthew Swabey and Paul Hoy of Southampton University, July 2005.
Questions and comments to mas@ecs.soton.ac.uk

Freely distributable, as above. Please add any other clock frequencies you want.

This uses no memory resources, and is precise.

Hi-Tech C and

Example C:

#define PIC_CLK 8000000
#include "delay.h"

void main(void)
{
	TRISC = 0x00;
	while(1)
	{
		PORTC = 0x00;
		DelayUs(x);
		PORTC = 0xFF;
	}
}	

*/

#ifndef __DELAY_H
#define __DELAY_H

#if (PIC_CLK == 4000000)
	// #define dly125n please remove; for 32Mhz+ only
	// #define dly250n please remove; for 16Mhz+ only
	// #define dly500n please remove; for 8Mhz+ only
	#define dly1u asm("nop")
	#define dly2u dly1u;dly1u
#elif (PIC_CLK == 8000000)
	// #define dly125n please remove; for 32Mhz+ only
	// #define dly250n please remove; for 16Mhz+ only
	#define dly500n asm("nop")
	#define dly1u dly500n;dly500n
	#define dly2u dly1u;dly1u
#elif (PIC_CLK == 10000000)
	#define dly400n asm("nop")
#elif (PIC_CLK == 16000000)
	// #define dly125n please remove; for 32Mhz+ only
	#define dly250n asm("nop")
	#define dly500n dly250n;dly250n
	#define dly1u dly500n;dly500n
	#define dly2u dly1u;dly1u
#elif (PIC_CLK == 20000000)
	#define dly200n asm("nop")
	#define dly400n dly200n;dly200n
	#define dly1u dly400n;dly400n;dly200n
	#define dly2u dly1u;dly1u
#elif (PIC_CLK == 32000000)
	#define dly125n asm("nop")
	#define dly250n dly125n;dly125n
	#define dly500n dly250n;dly250n
	#define dly1u dly500n;dly500n
	#define dly2u dly1u;dly1u
#else
	#error please define PIC_CLK correctly
	
#endif

/* ======================================== */
/* Delay Routines for less than 255us       */
/* These give a precise delay.              */
/* DelayUs(10) is 10us exactly!             */
/* Only multiples of 2 available at slower  */
/* speeds (16Mhz or less)					*/

#if PIC_CLK == 4000000
	#define DelayUs(x)\
	asm("\tMOVLW "___mkstr(x>>2));\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 2");
#elif PIC_CLK == 8000000
	#define DelayUs(x)\
	asm("\tMOVLW "___mkstr(x>>1));\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 2");
#elif PIC_CLK == 10000000
	#define DelayUs(x)\
	asm("\tMOVLW "___mkstr(x>>1));\
	asm("\tNOP");\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 3");
#elif PIC_CLK == 16000000
	#define DelayUs(x)\
	asm("\tMOVLW "___mkstr(x));\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 2");
#elif PIC_CLK == 20000000
	#define DelayUs(x)\
	asm("\tMOVLW "___mkstr(x));\
	asm("\tNOP");\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 3");
#elif PIC_CLK == 32000000
	#define DelayUs(x)\
	asm("\tMOVLW "___mkstr(x));\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 6");
#else
	#error please define PIC_CLK correctly
#endif

/* ======================================== */
/* NDelay Routines for less than 255us      */
/* These give a precise delay to the Next   */
/* Instructions ACTIVATION, so NDelayUs(10) */
/* is 10us - 1 instruction!                 */

#if PIC_CLK == 4000000
	#define NDelayUs(x)\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tMOVLW "___mkstr((x>>2)-1) );\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 2");
#elif PIC_CLK == 8000000
	#define NDelayUs(x)\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tMOVLW "___mkstr((x>>1)-1) );\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 2");
#elif PIC_CLK == 10000000
	#define NDelayUs(x)\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tMOVLW "___mkstr((x>>1)-1) );\
	asm("\tNOP");\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 3");
#elif PIC_CLK == 16000000
	#define NDelayUs(x)\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tMOVLW "___mkstr(x-1));\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 2");
#elif PIC_CLK == 20000000
	#define NDelayUs(x)\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tMOVLW "___mkstr(x-1));\
	asm("\tNOP");\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 3");
#elif PIC_CLK == 32000000
	#define NDelayUs(x)\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tMOVLW "___mkstr(x-1));\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tNOP");\
	asm("\tADDLW 0xFF");\
	asm("\tBTFSS _STATUS, 2");\
	asm("\tGOTO $ - 6");
#else
	#error please define PIC_CLK correctly
#endif

#endif
