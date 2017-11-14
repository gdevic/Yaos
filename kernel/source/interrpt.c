/*********************************************************************
*                                                                    *
*   Module:     interrpt.c
*                                                                    *
*   Revision:   1.00                                                 *
*                                                                    *
*   Date:       06/24/95                                             *
*                                                                    *
*   Author:     Goran Devic                                          *
*                                                                    *
**********************************************************************
*                                                                    *
*   Module Description:                                              *

          This module contains the code for the interrupt handling
          
**********************************************************************
*                                                                    *
*   Changes:                                                         *
*                                                                    *
*   DATE     REV   DESCRIPTION OF CHANGES                 AUTHOR     *
* --------   ----  -----------------------------------   -----------
  06/24/95   1.00  Original                              Goran Devic

* --------   ----  -----------------------------------   ----------- *
**********************************************************************
*   Important defines/undefines
**********************************************************************/

/*********************************************************************
*   Include Files
**********************************************************************/
#include "debugger.h"           // Include debugger types
#include "interrpt.h"           // Include interrupt header
#include "kernel.h"             // Include kernel types
#include "ports.h"              // Include basic ports I/O
#include "mm.h"

#define NULL 0

/*********************************************************************
*   Global Variables
**********************************************************************/
// Interrupt descriptor table for 256 interrupts
//
IntGateStruct IDT [256];

// Interrupt handlers for dual interrupts, contains the CPU interrupt
// and software interrupt handler
//
void (*Ints_CPU[16])() = 
{  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// Interrupt handlers for dual interrupts, contains IRQ handlers
//
void (*Ints_IRQ[16])() =
{  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };


/*********************************************************************
*   Local Variables
**********************************************************************/
static BYTE IrqHi = 0xFF,        // State of the interrupt control
            IrqLo = 0xFB;        // bits

#define DUAL_INT(intnum) \
   (((intnum)>=0x08 && (intnum)<=0x0f) || ((intnum)>=0x70 && (intnum)<=0x77))

#define DUAL_INDEX(intnum) \
   ((intnum)<=0x0f? (intnum)-0x08 : (intnum)-0x70+0x08)

/*********************************************************************
*   External Functions
**********************************************************************/
extern IRQ_0_Startup();
extern IRQ_1_Startup();
extern IRQ_2_Startup();
extern IRQ_3_Startup();
extern IRQ_4_Startup();
extern IRQ_5_Startup();
extern IRQ_6_Startup();
extern IRQ_7_Startup();
extern IRQ_8_Startup();
extern IRQ_9_Startup();
extern IRQ_A_Startup();
extern IRQ_B_Startup();
extern IRQ_C_Startup();
extern IRQ_D_Startup();
extern IRQ_E_Startup();
extern IRQ_F_Startup();

/*********************************************************************
*   Local Functions
**********************************************************************/

/*********************************************************************
*
*   interrupt DummyInt()
*
*   This interrupts stands for all uninitialized interrupts and should
*   never happen.
*
*   Returns:
*
**********************************************************************/
interrupt DummyInt()
{
   printk("DummyInt!\n");
   INT1();
}

interrupt DummyInt0(){ printk("DummyInt: 0\n"); INT1();}
interrupt DummyInt1(){ printk("DummyInt: 1\n"); INT1();}
interrupt DummyInt2(){ printk("DummyInt: 2\n"); INT1();}
interrupt DummyInt3(){ printk("DummyInt: 3\n"); INT1();}
interrupt DummyInt4(){ printk("DummyInt: 4\n"); INT1();}
interrupt DummyInt5(){ printk("DummyInt: 5\n"); INT1();}
interrupt DummyInt6(){ printk("DummyInt: 6\n"); INT1();}
interrupt DummyInt7(){ printk("DummyInt: 7\n"); INT1();}
interrupt DummyInt8(){ printk("DummyInt: 8\n"); INT1();}
interrupt DummyInt9(){ printk("DummyInt: 9\n"); INT1();}
interrupt DummyInt10(){ printk("DummyInt: 10\n"); INT1();}
interrupt DummyInt11(){ printk("DummyInt: 11\n"); INT1();}
interrupt DummyInt12(){ printk("DummyInt: 12\n"); INT1();}
interrupt DummyInt13(){ printk("DummyInt: 13\n"); INT1();}
interrupt DummyInt14(){ printk("DummyInt: 14\n"); INT1();}
interrupt DummyInt15(){ printk("DummyInt: 15\n"); INT1();}
interrupt DummyInt16(){ printk("DummyInt: 16\n"); INT1();}
interrupt DummyInt17(){ printk("DummyInt: 17\n"); INT1();}
interrupt DummyInt18(){ printk("DummyInt: 18\n"); INT1();}
interrupt DummyInt19(){ printk("DummyInt: 19\n"); INT1();}
interrupt DummyInt20(){ printk("DummyInt: 20\n"); INT1();}
interrupt DummyInt21(){ printk("DummyInt: 21\n"); INT1();}
interrupt DummyInt22(){ printk("DummyInt: 22\n"); INT1();}
interrupt DummyInt23(){ printk("DummyInt: 23\n"); INT1();}
interrupt DummyInt24(){ printk("DummyInt: 24\n"); INT1();}
interrupt DummyInt25(){ printk("DummyInt: 25\n"); INT1();}
interrupt DummyInt26(){ printk("DummyInt: 26\n"); INT1();}
interrupt DummyInt27(){ printk("DummyInt: 27\n"); INT1();}
interrupt DummyInt28(){ printk("DummyInt: 28\n"); INT1();}
interrupt DummyInt29(){ printk("DummyInt: 29\n"); INT1();}
interrupt DummyInt30(){ printk("DummyInt: 30\n"); INT1();}
interrupt DummyInt31(){ printk("DummyInt: 31\n"); INT1();}
interrupt DummyInt32(){ printk("DummyInt: 32\n"); INT1();}
interrupt DummyInt33(){ printk("DummyInt: 33\n"); INT1();}
interrupt DummyInt34(){ printk("DummyInt: 34\n"); INT1();}
interrupt DummyInt35(){ printk("DummyInt: 35\n"); INT1();}
interrupt DummyInt36(){ printk("DummyInt: 36\n"); INT1();}
interrupt DummyInt37(){ printk("DummyInt: 37\n"); INT1();}
interrupt DummyInt38(){ printk("DummyInt: 38\n"); INT1();}
interrupt DummyInt39(){ printk("DummyInt: 39\n"); INT1();}


/*********************************************************************
*
*   InitInterrupts()
*
*   Initializes interrupt subsystem
*
**********************************************************************/
InitInterrupts()
{
   int i;


   Outpb( 0x21, IrqLo );        // Mask off interrupts except cascade
   Outpb( 0xA1, IrqHi );        // Mask off interrupts


   // Fill in the Interrupt Descriptor Table with dummy interrupts
   //
   for( i=0; i<256; i++ )
       SetIDTEntry( i, (void(*)()) DummyInt, IDT_INTERRUPT );

   SetIDTEntry( 0, (void(*)()) DummyInt0, IDT_INTERRUPT );
   SetIDTEntry( 1, (void(*)()) DummyInt1, IDT_INTERRUPT );
   SetIDTEntry( 2, (void(*)()) DummyInt2, IDT_INTERRUPT );
   SetIDTEntry( 3, (void(*)()) DummyInt3, IDT_INTERRUPT );
   SetIDTEntry( 4, (void(*)()) DummyInt4, IDT_INTERRUPT );
   SetIDTEntry( 5, (void(*)()) DummyInt5, IDT_INTERRUPT );
   SetIDTEntry( 6, (void(*)()) DummyInt6, IDT_INTERRUPT );
   SetIDTEntry( 7, (void(*)()) DummyInt7, IDT_INTERRUPT );
   SetIDTEntry( 8, (void(*)()) DummyInt8, IDT_INTERRUPT );
   SetIDTEntry( 9, (void(*)()) DummyInt9, IDT_INTERRUPT );
   SetIDTEntry( 10, (void(*)()) DummyInt10, IDT_INTERRUPT );
   SetIDTEntry( 11, (void(*)()) DummyInt11, IDT_INTERRUPT );
   SetIDTEntry( 12, (void(*)()) DummyInt12, IDT_INTERRUPT );
   SetIDTEntry( 13, (void(*)()) DummyInt13, IDT_INTERRUPT );
   SetIDTEntry( 14, (void(*)()) DummyInt14, IDT_INTERRUPT );
   SetIDTEntry( 15, (void(*)()) DummyInt15, IDT_INTERRUPT );
   SetIDTEntry( 16, (void(*)()) DummyInt16, IDT_INTERRUPT );
   SetIDTEntry( 17, (void(*)()) DummyInt17, IDT_INTERRUPT );
   SetIDTEntry( 18, (void(*)()) DummyInt18, IDT_INTERRUPT );
   SetIDTEntry( 19, (void(*)()) DummyInt19, IDT_INTERRUPT );
   SetIDTEntry( 20, (void(*)()) DummyInt20, IDT_INTERRUPT );
   SetIDTEntry( 21, (void(*)()) DummyInt21, IDT_INTERRUPT );
   SetIDTEntry( 22, (void(*)()) DummyInt22, IDT_INTERRUPT );
   SetIDTEntry( 23, (void(*)()) DummyInt23, IDT_INTERRUPT );
   SetIDTEntry( 24, (void(*)()) DummyInt24, IDT_INTERRUPT );
   SetIDTEntry( 25, (void(*)()) DummyInt25, IDT_INTERRUPT );
   SetIDTEntry( 26, (void(*)()) DummyInt26, IDT_INTERRUPT );
   SetIDTEntry( 27, (void(*)()) DummyInt27, IDT_INTERRUPT );
   SetIDTEntry( 28, (void(*)()) DummyInt28, IDT_INTERRUPT );
   SetIDTEntry( 29, (void(*)()) DummyInt29, IDT_INTERRUPT );
   SetIDTEntry( 30, (void(*)()) DummyInt30, IDT_INTERRUPT );
   SetIDTEntry( 31, (void(*)()) DummyInt31, IDT_INTERRUPT );
   SetIDTEntry( 32, (void(*)()) DummyInt32, IDT_INTERRUPT );
   SetIDTEntry( 33, (void(*)()) DummyInt33, IDT_INTERRUPT );
   SetIDTEntry( 34, (void(*)()) DummyInt34, IDT_INTERRUPT );
   SetIDTEntry( 35, (void(*)()) DummyInt35, IDT_INTERRUPT );
   SetIDTEntry( 36, (void(*)()) DummyInt36, IDT_INTERRUPT );
   SetIDTEntry( 37, (void(*)()) DummyInt37, IDT_INTERRUPT );
   SetIDTEntry( 38, (void(*)()) DummyInt38, IDT_INTERRUPT );
   SetIDTEntry( 39, (void(*)()) DummyInt39, IDT_INTERRUPT );

   // Set IDT entries of dual interrupts to special intermediate 
   // handling functions
   //
   SetIDTEntry( 0x08, (void(*)()) IRQ_0_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x09, (void(*)()) IRQ_1_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x0A, (void(*)()) IRQ_2_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x0B, (void(*)()) IRQ_3_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x0C, (void(*)()) IRQ_4_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x0D, (void(*)()) IRQ_5_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x0E, (void(*)()) IRQ_6_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x0F, (void(*)()) IRQ_7_Startup, IDT_INTERRUPT );

   SetIDTEntry( 0x70, (void(*)()) IRQ_8_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x71, (void(*)()) IRQ_9_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x72, (void(*)()) IRQ_A_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x73, (void(*)()) IRQ_B_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x74, (void(*)()) IRQ_C_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x75, (void(*)()) IRQ_D_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x76, (void(*)()) IRQ_E_Startup, IDT_INTERRUPT );
   SetIDTEntry( 0x77, (void(*)()) IRQ_F_Startup, IDT_INTERRUPT );

   printk("Interrupts\n");
}


/*********************************************************************
*
*   void SetIDTEntry( BYTE bIntNum, void (*fn)(void), WORD wType )
*
*   Sets the Interrupt Descriptor entry to point to a certain interrupt
*   function and to be of the certain interrupt type.
*
*   Where:
*       bIntNum  - interrupt number to be set
*       fn       - function to be called on that interrupt
*       wType    - interrupt type: IDT_TRAP or IDT_INTERRUPT
*
**********************************************************************/
static void SetIDTEntry( BYTE bIntNum, void (*fn)(void), WORD wType )
{
   IDT[bIntNum].wOffsetLow  = (DWORD)fn & 0x0000ffff;
   IDT[bIntNum].wOffsetHigh = (DWORD)fn >> 16;
   IDT[bIntNum].wSelector   = CODESEL;
   IDT[bIntNum].wType       = wType;
}                                               


/******************************************************************************
*
*   BOOL RegisterInterruptHandler( BYTE bIntNum, void (*fn)(void), WORD wType )
*
*      This function is used to register a kernel interrupt handler.
*      The CPU internal interrupts as well as software traps will be forwarded
*      to a given function.
*
*   Where:
*      bIntNum is the number of interrupt to register, 0 to 255
*      fn is the kernel-level function to be called on interrupt
*      wType is:
*          IDT_TRAP - software/CPU interrupts
*          IDT_INTERRUPT - software/CPU interrupts, disables further interrupts
*
*   Returns:
*      TRUE if registration succedded
*      FALSE if interrupt was already registered
*      
******************************************************************************/
BOOL RegisterInterruptHandler( BYTE bIntNum, void (*fn)(void), WORD wType )
{
   // Is the interrupt dual?
   if( DUAL_INT(bIntNum) )
   {
       // Dual interrupts must be registered only in the Ints_CPU table
       if( Ints_CPU[ DUAL_INDEX(bIntNum) ] )
           return( FALSE );        // Entry already exists

       Ints_CPU[ DUAL_INDEX(bIntNum) ] = fn;
   }
   else
   {
      // Set the IDT entry
      SetIDTEntry( bIntNum, fn, wType );
   }

   return( TRUE );
}


/******************************************************************************
*
*   BOOL UnregisterInterruptHandler( BYTE bIntNum )
*
*      This function unregisters (frees) interrupt handler for a given
*      interrupt.  CPU internal interrupt as well as software traps handlers
*      may be released in this way.
*
*   Where:
*      bIntNum is the number of the interrupt to be freed
*
*   Returns:
*      TRUE if interrupt was freed
*      FALSE if interrupt cannot be freed
*
******************************************************************************/
BOOL UnregisterInterruptHandler( BYTE bIntNum )
{
   // If it is dual interrupt, unregister it from the table
   if( DUAL_INT(bIntNum) )
   {
       // Dual interrupts must be unregistered only from the Ints_CPU table
       if( Ints_CPU[ DUAL_INDEX(bIntNum) ] == 0 )
           return( FALSE );        // Entry is already unregistered

       Ints_CPU[ DUAL_INDEX(bIntNum) ] = 0;
   }
   else
   {
      // Set the IDT entry with the new dummy interrupt handler
      SetIDTEntry( bIntNum, (void(*)()) DummyInt, IDT_INTERRUPT );
   }

   return( TRUE );
}


/******************************************************************************
*
*   BOOL RegisterIRQHandler( BYTE bIRQNum, void (*fn)(void) )
*
*      This function should be used to register a kernel external interrupt
*      handler (for an IRQ line).
*
*   Where:
*      bIRQNum is the number of external interrupt to register, one of IRQ_*
*      fn is the kernel-level function to be called on external interrupt
*
*   Returns:
*      TRUE if registration succedded
*      FALSE if interrupt was already registered
*      
******************************************************************************/
BOOL RegisterIRQHandler( BYTE bIRQNum, void (*fn)(void) )
{
   // Dual interrupts must be registered only in the Ints_IRQ table
   if( Ints_IRQ[ bIRQNum ] )
       return( FALSE );        // Entry already exists

   Ints_IRQ[ bIRQNum ] = fn;
   
   return( TRUE );
}


/******************************************************************************
*
*   BOOL UnregisterIRQHandler( BYTE bIRQNum )
*
*      This function unregisters (frees) interrupt handler for a given
*      external interrupt (IRQ line).
*
*   Where:
*      bIntNum is the number of the external interrupt to be freed
*
*   Returns:
*      
******************************************************************************/
BOOL UnregisterIRQHandler( BYTE bIRQNum )
{
   // Dual interrupts must be unregistered only in the Ints_IRQ table
   if( Ints_IRQ[ bIRQNum ] == 0 )
       return( FALSE );        // Entry does not exist

   Ints_IRQ[ bIRQNum ] = 0;

   return( TRUE );
}


/******************************************************************************
*
*   void IRQControl( BYTE bEnable, BYTE bIRQNum )
*
*      Enables/Disables certain interrupt
*
*   Where:
*       bEnable is IRQ_ENABLE or IRQ_DISABLE
*       bIRQNum is the interrupt number to which the IRQ is mapped
*
*   Returns:
*
******************************************************************************/
void IRQControl( BYTE bEnable, BYTE bIRQNum )
{
    BYTE b;
     

    // Check if an interrupt requested is controlled by the first or
    // the second controller
    //
    if( bIRQNum <= IRQ_7 )
    {
        // Shift a mask for lower 8 interrupts
        //
        b = 1 << bIRQNum;

        if( bEnable == IRQ_ENABLE )
            IrqLo &= ~b;
        else
            IrqLo |= b;

        // Enable cascade
        //
        IrqLo &= ~4;

        Outpb( 0x21, IrqLo );
    }
    else
    {
        // Upper 8 interrupts
        //
        b = 1 << (bIRQNum-8);

        if( bEnable == IRQ_ENABLE )
            IrqHi &= ~b;
        else
            IrqHi |= b;

        Outpb( 0xA1, IrqHi );
    }
}

