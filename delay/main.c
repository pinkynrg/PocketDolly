#include <pic.h>

//for Hi-Tech C only

#include "delay.h" //make sure that PIC_CLK in this file is correctly defined

main()
{
  unsigned int timeout_int;
  unsigned char timeout_char;

  dly250n;      //macro - delay constant 250ns at any clock speed
  dly1u;        //macro - delay constant 1us at any clock speed

  DelayUs(40);  //do not do DelayUs(0) or else it bombs :)
  DelayUs(255); //delay 255us (this takes an unsigned char argument)
  DelayBigUs(65535); //delay 65,535us (this takes an unsigned integer argument)

  DelayMs(255); //delay 255ms (this takes an unsigned char argument)
  DelayBigMs(65535);  //delay 65,535ms (this takes an unsigned integer argument)

  DelayMs_interrupt(254); //if you ever use a delay in an interrupt function, use this one

  DelayS(12); //delay 12 seconds

  //these following functions are useful for timeouts
  timeout_char=timeout_char_us(575);  //at 16Mhz, a value higher than this will generate 'arithmetic overflow'
  while(timeout_char-- && (RA1==0));  //wait up to 1147us for port RA1 to go high - this is the max timeout

  timeout_int=timeout_int_us(262143); //at 16Mhz, a value higher than this will generate 'arithmetic overflow'
  while(timeout_int-- && (RA1==0));   //wait up to 491512us for port RA1 to go high - this is the max timeout
}


