/******************************************************************************
*
*   Module:     timer.c
*
*   Revision:   1.00
*
*   Date:       08/26/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This module contains the code for the timer and delays

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 08/26/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/
#include "interrpt.h"                   // Include interrupt header
#include "ports.h"                      // Include basic ports I/O
#include "types.h"                      // Include basic data types

/******************************************************************************
*   Global Variables
******************************************************************************/

DWORD dwDelay;                          // 1 ms in terms of inner delay loops

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/

#define DELAY_LOOP      10              // Inner loop of a delay loop
static volatile BYTE bCalibrate;        // Calibration flag


/******************************************************************************
*   Functions
******************************************************************************/
interrupt CalibrateHandler();

/******************************************************************************
*
*   void InitTimers()
*
*      Initializes timer subsystem
*
******************************************************************************/
void InitTimers()
{
    int i;

    // Initialize delay timer
    // Check the speed of a host computer by setting timer interrupt that has
    // been programmed by BIOS to tick 18.2 times per second with our handler
    // and count the number of inp(0x80) operations that can be performed 
    // in between two ticks
    //
    // TO DO:  What the heck...Program my own timer interrupt!
    //
    if( !RegisterIRQHandler( IRQ_TIMER, (void(*)()) CalibrateHandler ) )
       Halt("Unable to register timer handler");

    bCalibrate = 3;
    dwDelay = 0;
    IRQControl( IRQ_ENABLE, IRQ_TIMER );
    EnableInterrupts();

    // Wait until calibration count turns 1
    while( bCalibrate != 1 );

    // Count the number of inp's until the calibration count turns 0
    while( bCalibrate )
    {
        for( i=0; i<DELAY_LOOP; i++ ) Inpb(0x80);
        dwDelay++;
    }

    // 1 ms is so many loops
    dwDelay = dwDelay / 54;

    DisableInterrupts();
    UnregisterIRQHandler( IRQ_TIMER );

    printk("Delay loop: %d\n", dwDelay );
}


/******************************************************************************
*
*   CalibrateHandler()
*
*      This function is used during the initialization to calibrate delays
*
******************************************************************************/
interrupt CalibrateHandler()
{
    // Decrement the calibration flag
    bCalibrate--;

    // Ack the interrupt controller
    Outpb( 0x20, 0x20 );
}


/******************************************************************************
*
*   void kdelay( DWORD dwMilisec )
*
*      Delays the execution.
*
*   Where:
*      dwMilisec is the number of miliseconds to delay
*      
******************************************************************************/
void kdelay( DWORD dwMilisec )
{
    int i, j;

    // According to the delay counter, delay dwMilisec miliseconds
    //
    for( i = dwMilisec * dwDelay; i>0; i--)
        for( j=0; j<DELAY_LOOP; j++) Inpb(0x80);
}

