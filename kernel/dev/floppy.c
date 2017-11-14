/******************************************************************************
*                                                                             *
*   Module:     Floppy.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       5/21/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements the low-level floppy disc(s) controller.
        The driver is synchronous - that is not the best in terms of
        the overall performance, but is easy to get it work initially.

        There are two interrupts that service the driver:
        IRQ 6  - Sets the semaphore that it have occurred
        IRQ 0  - Turns the floppy motor(s) off when its counter reaches zero

        PC AT is assumed.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
*  5/21/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <stdio.h>                      // Include standard I/O header file
#include <conio.h>

#include "ufs.h"                        // Include File System defines

#define BYPASS
#include <bios.h>

#ifdef TEST

#include <dos.h>
#define ADDR   (0xA0000 - 9216)


union REGS regs;
struct SREGS sregs;

static struct rminfo {
    long EDI;
    long ESI;
    long EBP;
    long reserved_by_system;
    long EBX;
    long EDX;
    long ECX;
    long EAX;
    short flags;
    short ES,DS,FS,GS,IP,CS,SP,SS;
} RMI;


BYTE *pBuf;

int DosAlloc( int size )
{
    union REGS regs;
    short sel, seg;

    regs.w.ax = 0x100;
    regs.w.bx = size >> 4;
    int386( 0x31, &regs, &regs );
    sel = regs.w.dx;
    seg = regs.w.ax;

//    printf("Base selector: %04Xh\n", sel );
//    printf("Real mode segment: %04X\n", seg );

    return( seg << 4 );
}


#endif

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#pragma aux DisableInterrupts = "cli" parm;
#pragma aux EnableInterrupts  = "sti" parm;

//=============================================================================
//  DMA Chip Defines
//=============================================================================

#define DMA_READ            0x46        // DMA Read operation
#define DMA_WRITE           0x4A        // DMA Write operation
#define DMA_ARM             0x02        // DMA Arm operation

#define DMA_ADDRESS_LOW     0x0004      // DMA Address port for low 16 bits
#define DMA_ADDRESS_HIGH    0x0081      // DMA Address port for high 4 bits
#define DMA_COUNT           0x0005      // DMA Count port
#define DMA_INIT            0x000A      // DMA Init port
#define DMA_STATUS1         0x000B      // DMA Status 1 port
#define DMA_STATUS2         0x000C      // DMA Status 2 port

//=============================================================================
//  Floppy Controller Registers for NEC nPD765X
//=============================================================================

#define VER_PD765           0x80        // Version NEC PD765(A) compatible
#define VER_PD765B          0x90        // Version NEC PD765B compatible


#define DOR                 0x3F2       // Digital Output Register
#define   DOR_RESET         0x00        // Reset value
#define   DOR_SELECT        0x0C        // Select the drive and motor state
#define   DOR_MOTOR_A       0x10        // Motor A on

#define FDC_STATUS          0x3F4       // FDC Status Register
#define   STAT_READY        0x80        // Request for data
#define   STAT_DIRECTION    0x40        // Direction bit
#define   STAT_BUSY         0x10        // Controller is busy

#define FDC_DATA            0x3F5       // FDC Data Register

#define FDC_DIR             0x3F7       // Configuration Control Register (AT)
#define   DIR_RATE          0x00        // Data transfer rate for 1.44, 3 1/2"

// Floppy Controller Commands
#define CMD_CHECK_STATUS    0x08        // Check Interrupt Status
#define CMD_FIX_DATA        0x03        // Set Drive Data
#define   DATA1             0xCF        // Step Rate / Head Unload Time
#define   DATA2             0x02        // Head Load Time / Non-DMA
#define CMD_RECALIBRATE     0x07        // Recalibrate drive
#define CMD_SEEK            0x0F        // Seek specific track
#define CMD_READ            0x66        // Read command
#define CMD_WRITE           0x45        // Write command


#define COUNTDOWN_TIME      (18*3)      // Time in ticks (18.2 per sec) to
                                        // turn off the floppy disk motor
#define SECTORS             18          // Sectors in a track
#define TRACK_LEN        (SECTORS*512)  // Track length in bytes

//=============================================================================

// Define a floppy device
static struct floppy
{
    BOOL    fMotor;                     // Motor is running or not
    BOOL    fTurnOffMotor;              // If Int0 should count down and turn
                                        //  off the motor of this drive
    BYTE    bCountDown;                 // Motor spinoff countdown
    BYTE    bTrack;                     // Current logical track

} fd;

static BYTE sbResult[ 10 ];             // FDC result string
static volatile BOOL fSemaphore;

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void DMASetup( BYTE bOp, void *pAddress )                                 *
*                                                                             *
*******************************************************************************
*
*   Sets up DMA chip for a data transfer
*
*   Where:
*       bOp is DMA_READ or DMA_WRITE
*
*   Returns:
*
*
******************************************************************************/
static void DMASetup( BYTE bOp, void *pAddress )
{
    printf("DMA Armed\n");

    DisableInterrupts();
    outp( DMA_STATUS2, bOp );           // Set the DMA mode
    outp( DMA_STATUS1, bOp );           // Set the DMA mode
    outp( DMA_ADDRESS_LOW, (int)pAddress & 0xFF );
    outp( DMA_ADDRESS_LOW, ((int)pAddress >> 8) & 0xFF );
    outp( DMA_ADDRESS_HIGH, ((int)pAddress >> 16) & 0xFF );
    outp( DMA_COUNT, (TRACK_LEN - 1) & 0xFF );
    outp( DMA_COUNT, ((TRACK_LEN - 1) >> 8) & 0xFF );
    EnableInterrupts();

    outp( DMA_INIT, DMA_ARM );
}


/******************************************************************************
*                                                                             *
*   void StartMotor()                                                         *
*                                                                             *
*******************************************************************************
*
*   Starts the motor
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
static void StartMotor()
{
    // If the motor is running, just return
    if( fd.fMotor )
        return;

    // Start the motor
    outp( DOR, DOR_SELECT + DOR_MOTOR_A );

    // Wait 1/4 of a second for a motor to spin off
    delay( 250 );

    // Set the flags; start the timer that turns off the motor,
    // an operation will just reset the time out value
    fd.fMotor = TRUE;
    fd.bCountDown = COUNTDOWN_TIME;
    fd.fTurnOffMotor = TRUE;
}


/******************************************************************************
*
*   void FDC_writeb( BYTE bValue )
*
*      Outputs a byte to the Floppy Disk Controller
*
*   Where:
*      bValue - byte to be sent
*
******************************************************************************/
static void FDC_writeb( BYTE bValue )
{
    BYTE bIn;

    while( !kbhit() )
    {
        bIn = inp( FDC_STATUS );
        if( (bIn & (STAT_READY | STAT_DIRECTION)) == STAT_READY )
        {
            outp( FDC_DATA, bValue );
            return;
        }
    }
}


/******************************************************************************
*
*   int FDC_read( int num )
*
*      Reads result of operation from the Floppy Disk Controller
*
*   Where:
#       num is the number of bytes of the status to read
*
******************************************************************************/
static int FDC_read( int num )
{
    BYTE bIn;
    int i;


    for( i=0; i<num; i++ )
    {
        // Wait until the controller is ready to send data
        do
        {
            bIn = inp( FDC_STATUS );

            if( kbhit() )
                return( -1 );
        }
        while( (bIn & (STAT_READY | STAT_DIRECTION)) != (STAT_READY | STAT_DIRECTION)  );

        // Read a byte from the controller
        sbResult[i] = inp( FDC_DATA );
    }

#if 1
    // Read spurious data
    do
    {
        delay(1);
        bIn = inp( FDC_STATUS );

        if( kbhit() )
            return( -1 );

        if( bIn & (STAT_READY | STAT_DIRECTION) == (STAT_READY | STAT_DIRECTION) )
            printf("> %2X ", inp( FDC_DATA) );
    }
    while( bIn & (STAT_READY | STAT_DIRECTION) == STAT_READY );
#endif

    return( i );
}




/******************************************************************************
*                                                                             *
*   BOOL PollSemaphore()                                                      *
*                                                                             *
*******************************************************************************
*
*   Polls the interrupt 14 (IRQ 6) semaphore
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
static BOOL PollSemaphore()
{
    while( fSemaphore == FALSE )
    {
        if( kbhit() )
            return( TRUE );
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   int Reset()                                                               *
*                                                                             *
*******************************************************************************
*
*   Resets the floppy drive
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
static int Reset()
{
    // Turn off all the motors and reset the controller
    DisableInterrupts();
    fSemaphore = FALSE;
    outp( DOR, DOR_RESET );
    outp( DOR, DOR_SELECT );
    EnableInterrupts();
    PollSemaphore();

    // Motor is now off
    fd.fMotor        = FALSE;
    fd.fTurnOffMotor = FALSE;

    // Check the status after an interrupt
    FDC_writeb( CMD_CHECK_STATUS );
    FDC_read( 2 );

    printf("Interrupt Status ST0: %2X\n", sbResult[0] );
    printf("Cilinder:             %2X\n", sbResult[1] );

    // Fix drive data
    FDC_writeb( CMD_FIX_DATA );
    FDC_writeb( DATA1 );
    FDC_writeb( DATA2 );

    Recalibrate();

    return( 1 );
}


/******************************************************************************
*                                                                             *
*   int Recalibrate()                                                         *
*                                                                             *
*******************************************************************************
*
*   Recalibrates the floppy disk
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
static int Recalibrate()
{
    StartMotor();

    // Recalibrate drive A
    fSemaphore = FALSE;
    FDC_writeb( CMD_RECALIBRATE );
    FDC_writeb( 0 );
    PollSemaphore();

    // Check the status after an interrupt
    FDC_writeb( CMD_CHECK_STATUS );
    FDC_read( 2 );

    printf("Recalibrate\n");
    printf("Interrupt Status ST0: %2X\n", sbResult[0] );
    printf("Cilinder:             %2X\n", sbResult[1] );

    // Head is now at the track 0
    fd.bTrack = 0;

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int Seek( int nTrack )                                                    *
*                                                                             *
*******************************************************************************
*
*   Seek the specific track
*
*   Where:
*       nTrack is the logical track number
*
*   Returns:
*
*
******************************************************************************/
static int Seek( int nTrack )
{
    BYTE bPhyTrack, bHead;


    // Cant do seek if motor is not on
    StartMotor();
    fd.bCountDown = COUNTDOWN_TIME;

    // If the head is already on the specific track, just return
    if( fd.bTrack == nTrack )
        return( nTrack );

    // Find the physical track and head from the logical track
    bPhyTrack = nTrack >> 1;
    bHead     = nTrack & 1;

    // Seek to a specified track
    fSemaphore = FALSE;
    FDC_writeb( CMD_SEEK );
    FDC_writeb( bHead << 2 );
    FDC_writeb( bPhyTrack );
    PollSemaphore();

    // Check the status after an interrupt
    FDC_writeb( CMD_CHECK_STATUS );
    FDC_read( 2 );

    printf("Seek\n");
    printf("Interrupt Status ST0: %2X\n", sbResult[0] );
    printf("Cilinder:             %2X\n", sbResult[1] );

    fd.bTrack = nTrack;

    return( nTrack );
}


/******************************************************************************
*                                                                             *
*   int Transfer( BYTE bOp )                                                  *
*                                                                             *
*******************************************************************************
*
*
*
*   Where:
*       bOp is FDC_READ or FDC_WRITE
*
*   Returns:
*
*
******************************************************************************/
static int Transfer( BYTE bOp )
{
    BYTE bPhyTrack, bHead;


    // Find the physical track and head from the logical track
    bPhyTrack = fd.bTrack >> 1;
    bHead     = fd.bTrack & 1;

    // Set up the data transfer rate
    outp( FDC_DIR, DIR_RATE );

    delay(35);
    fd.bCountDown = COUNTDOWN_TIME;

    // Send the command stream
    fSemaphore = FALSE;
    FDC_writeb( bOp );                  // FDC_READ | FDC_WRITE
    FDC_writeb( bHead << 2 );           // Head [2]
    FDC_writeb( bPhyTrack );            // Track
    FDC_writeb( bHead );                // Head
    FDC_writeb( 1 );                    // First sector
    FDC_writeb( 2 );                    // Sector size
    FDC_writeb( SECTORS );              // Last sector number
    FDC_writeb( 0x1B );                 // GAP 3
    FDC_writeb( 0xFF );                 // Data Length
    PollSemaphore();

    FDC_read( 7 );
    printf("Status 0:      %2X\n", sbResult[0] );
    printf("Status 1:      %2X\n", sbResult[1] );
    printf("Status 2:      %2X\n", sbResult[2] );
    printf("Track:         %2X\n", sbResult[3] );
    printf("Head:          %2X\n", sbResult[4] );
    printf("Sector number: %2X\n", sbResult[5] );
    printf("Sector size:   %2X\n", sbResult[6] );

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int FloppyRead( TIOCtlMsg * IOCtlMsg )                                    *
*                                                                             *
*******************************************************************************
*
*
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
int FloppyRead( TIOCtlMsg * IOCtlMsg )
{
#ifndef BYPASS
    // Read a track operation

    // Seek the right track
    Seek( IOCtlMsg->nData );

    // Setup the DMA chip for transfer
    DMASetup( DMA_READ, IOCtlMsg->pBuffer );

    // Transfer the track
    Transfer( CMD_READ );
#else
    int retry=10;

    // Read sectors 1-18, nData is the starting sector number

    while( retry-- )
    {
        memset(&RMI,0,sizeof(RMI));
        RMI.EAX=0x00000212;
        RMI.EBX = 0;
        RMI.ECX = (IOCtlMsg->nData / 36) * 256 + 1;
        RMI.ES  = (int)pBuf >> 4;
        RMI.EDX = ((IOCtlMsg->nData / 18) & 1) * 256;
        RMI.DS  = 0;

        /* Use DMPI call 300h to issue the DOS interrupt */
        regs.w.ax = 0x0300;
        regs.h.bl = 0x13;
        regs.h.bh = 0;
        regs.w.cx = 0;
        sregs.es = FP_SEG(&RMI);
        regs.x.edi = FP_OFF(&RMI);
        int386x( 0x31, &regs, &regs, &sregs );
        
        if( RMI.EAX == 18 )
        {
            memcpy( IOCtlMsg->pBuffer, pBuf, 18*512 );
            return( ESUCCESS );
        }

        memset(&RMI,0,sizeof(RMI));
        RMI.EAX=0x00000000;

        /* Use DMPI call 300h to issue the DOS interrupt */
        regs.w.ax = 0x0300;
        regs.h.bl = 0x13;
        regs.h.bh = 0;
        regs.w.cx = 0;
        sregs.es = FP_SEG(&RMI);
        regs.x.edi = FP_OFF(&RMI);
        int386x( 0x31, &regs, &regs, &sregs );        
    };
    return( EFAIL );
#endif

    return( ESUCCESS );
}


/******************************************************************************
*                                                                             *
*   int FloppyWrite( TIOCtlMsg * IOCtlMsg )                                   *
*                                                                             *
*******************************************************************************
*
*
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
int FloppyWrite( TIOCtlMsg * IOCtlMsg )
{
#ifndef BYPASS
    // Write a track operation

    // Seek the right track
    Seek( IOCtlMsg->nData );

    // Setup the DMA chip for transfer
    DMASetup( DMA_WRITE, IOCtlMsg->pBuffer );

    // Transfer the track
    Transfer( CMD_WRITE );
#else
    int retry=4;

    while( retry-- )
    {
        memcpy( pBuf, IOCtlMsg->pBuffer, 18*512 );
        memset(&RMI,0,sizeof(RMI));
        RMI.EAX=0x00000312;
        RMI.EBX = 0;
        RMI.ECX =   (IOCtlMsg->nData / 36) * 256 +
                    ((IOCtlMsg->nData % 18) + 1);
        RMI.ES  = (int)pBuf >> 4;
        RMI.EDX = ((IOCtlMsg->nData / 18) % 1) * 256;
        RMI.DS  = 0;

        /* Use DMPI call 300h to issue the DOS interrupt */
        regs.w.ax = 0x0300;
        regs.h.bl = 0x13;
        regs.h.bh = 0;
        regs.w.cx = 0;
        sregs.es = FP_SEG(&RMI);
        regs.x.edi = FP_OFF(&RMI);
        int386x( 0x31, &regs, &regs, &sregs );
        
        if( RMI.EAX == 18 )
            return( ESUCCESS );
    };
    return( EFAIL );
#endif
    return( ESUCCESS );
}



#if TEST

/******************************************************************************
*   DOS Compatibility Section
******************************************************************************/

void (__interrupt __far *prev_int_E)();
void (__interrupt __far *prev_int_8)();

#pragma off (check_stack)
#pragma aux FDCHandler parm;
static interrupt FDCHandler()
{
    BYTE b;

    b = *(BYTE *)( 0xb8000 + 79*2 + 49*80*2);
    if( b==' ' )
       b='1';
    else
       if( b=='9' )
           b='A';
       else
           b++;
    *(BYTE *)( 0xb8000 + 79*2 + 49*80*2) = b;

    // Send the signal using a semaphore
    // (Gee, I've never thought I would be using this)
    fSemaphore = TRUE;


    outp(0x20, 0x20);
}

#pragma aux IRQ0Handler parm;
static interrupt IRQ0Handler()
{
    // Check if a floppy drive needs to be turned off
    if( fd.fTurnOffMotor )
        if( (fd.bCountDown--) == 0 )
        {
            outp( DOR, DOR_SELECT );

            fd.fTurnOffMotor = FALSE;
            fd.fMotor = FALSE;
        }

    // Finish up the interrupt routine
    outp(0x20, 0x20);
}
static void HandlerEnd() {}
#pragma on (check_stack)


/*********************************************************************
*
*   BYTE lock_region( void * address, DWORD length )
*
*   Where:
*       address is the starting address to lock
*       length is the length in bytes to lock
*
*   Returns:
*       0 if it can not lock the memory
*       1 if memory was successfully locked
*
**********************************************************************/
static BYTE lock_region( void *address, DWORD length )
{
    union REGS regs;

    // Lock the given memory region
    //
    regs.w.ax = 0x600;
    regs.w.bx = (WORD)( (DWORD)address >> 16 );
    regs.w.cx = (WORD)( (DWORD)address & 0xFFFF );
    regs.w.si = (WORD)( length >> 16 );
    regs.w.di = (WORD)( length & 0xFFFF );
    int386( 0x31, &regs, &regs );

    // Return 1 (cflag is 0) if succeded
    //
    return( !regs.w.cflag );
}

int FloppyInit()
{
#ifndef BYPASS

    setbuf( stdout, NULL );

    if(( !lock_region ((void near *) &fd, (int)&fSemaphore - (int)&fd ) ) ||
       ( !lock_region ((void near *) FDCHandler, (char *) HandlerEnd - (char near *) FDCHandler)) )
    {
       printf( "locks failed\n" );
       exit(1);
    }

    prev_int_E = _dos_getvect( 0x0e );
    prev_int_8 = _dos_getvect( 0x08 );
    _dos_setvect( 0x0E, FDCHandler );
    _dos_setvect( 0x08, IRQ0Handler );

    printf("At start...\n");

    fd.bTrack = 0xFF;
    Reset();

    printf("Init done\n");

    pBuf = (BYTE *) malloc( 9216 );
    memcpy( pBuf, ADDR, 9216 );

#else
Again:
    pBuf = DosAlloc( 9216 );
    if( pBuf==NULL )
        return( EFAIL );
    if( ((int)pBuf/65536) != (((int)pBuf+9216)/65536) )
    {
        printf("Boundary !!!\n");
        goto Again;
    }
#endif

    return( ESUCCESS );
}


int CloseFloppy()
{
#ifndef BYPASS
    memcpy( ADDR, pBuf, 9216 );
    free( pBuf );

    printf("At exit...\n");

   _dos_setvect( 0x0E, prev_int_E );
   _dos_setvect( 0x08, prev_int_8 );
#endif

    return( ESUCCESS );
}

#else

/******************************************************************************
*                                                                             *
*   int FloppyInit()                                                          *
*                                                                             *
*******************************************************************************
*
*   Initializes floppy controller
*
*   Where:
*
*
*   Returns:
*       Number of minor floppy devices found (will be 1 for this release)
*
******************************************************************************/
int FloppyInit()
{
    fd.bTrack = 0xFF;

    Reset();

    return( ESUCCESS );
}

#endif // TEST
