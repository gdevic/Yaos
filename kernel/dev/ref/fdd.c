/******************************************************************************
*                                                                             *
*   Module:     fdd.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       08/11/96                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This file implements the low-level driver for the floppy disk access.

        Only operations on the whole tracks are supported.
        * NEC PD765 chip is supported
        * The default port base address is 0x3F0

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 08/11/96   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/
#define DEB     printf

#define TEST                1

#include "doscomp.h"                    // Include DOS test compatibility
#include "ufs.c"
#include "fdd.h"                        // Include floppy driver header

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/


// Digital-Output Register
#define DOR_CONTROL         0x3F2       // Digital Output Register
#define   DOR_SELECT        0x0C        // Select the drive and motor state
#define   DOR_RESET         0x00        // Reset sequence
#define   DOR_RESET_ACK     0xC0        // Good value after the reset

// Floppy Configuration Register
#define FDC_CONFIG          0x3F7       // Transfer rate/digital input register

// Floppy Disk Controller registers
#define FDC_STATUS          0x3f4       // FDC Status register:
#define   STAT_RESET        0x80        // Used as a new style reset
#define   STAT_READY        0x80        // Also request for master
#define   STAT_DIRECTION    0x40        // Direction bit
#define   STAT_DMA          0x20        // Non-DMA mode
#define   STAT_BUSY         0x10        // Controller is busy
#define FDC_DATA            0x3f5       // FDC Data register

// Commands for the Data port of the Floppy Disk Controller
#define READ                0x66        // Single track MFM Read operation
#define WRITE               0x45        // Single track MFM Write operation
#define   BAD_SECTOR        0x05        // Bad sector, needs recalibration
#define   BAD_TRACK         0x1F        // Union of bad access errors
#define   WRITE_PROT        0x02        // Write protected disk
#define   TRANS_ST0         0x00        // Top 5 bits of status 0 reg after R/W
#define RECAL               0x07        // Calibrate operation
#define   CALIBRATED        1           // Calibrated flag
#define   UNCALIBRATED      0           // Uncalibrated flag
#define SENSE               0x08        // Interrupt sense
#define SPECIFY             0x03        // Specify parameters
#define   SPEC2             0x02        // H. Load time  | DMA mode(0),not used
#define SEEK                0x0F        // Seek operation
#define   ST0_BITS          0xF8        // Healthy seek mask
#define   SEEK_ST0          0x20        // Healthy seek result

// Command status registers and specific masks
#define ST0                 0          // Status 0 register
#define ST1                 1          // Status 1 register
#define ST2                 2          // Status 2 register
#define ST_SECTOR           5          // Sector number report
#define ST_TRACK            1          // Present track

// DMA registers and commands
#define DMA_ADDR            0x0004
#define DMA_COUNT           0x0005
#define DMA_TOP             0x0081
#define DMA_INIT            0x000A
#define DMA_M1              0x000B
#define DMA_M2              0x000C
#define DMA_READ            0x46
#define DMA_WRITE           0x4A
#define DMA_ARM             2

#define NUM_INTWAIT         2000000     // Time to wait for an interrupt
#define NUM_DELAY           40          // Reasonable delay for reading
#define NUM_FDC_RETRY       40          // Number of retries when talking
#define NUM_RESULT          8           // Number of resulting bytes from data
#define NUM_ERRORS          20          // Number of trials to read/write

#define NUM_HEADS           2           // Number of heads, constant for fdd's
#define RW_BPS              0x02        // R/W bytes per sector (512 bytes)
#define RW_SECTOR           0xFF        // R/W sector size

#define IODELAY(X)      { int i; for(i=0;i<(X);i++,inp(0x80));  }

#define RW_GAP3         drives[fd->bDriveType].bGap
#define TRANSFER_RATE   drives[fd->bDriveType].bRate
#define NUM_SECTORS     drives[fd->bDriveType].bSectors
#define NUM_TRACKS      drives[fd->bDriveType].bTracks
#define SPEC1           drives[fd->bDriveType].bStep
#define TOTAL_SECTORS   drives[fd->bDriveType].wTotalSectors


// Define different drives
typedef struct drives
{
    BYTE bGap;                          // Gap size
    BYTE bRate;                         // Transfer rate
    BYTE bSectors;                      // Sectors per track
    BYTE bTracks;                       // Number of tracks on one side
    BYTE bStep;                         // Stepping rate & Head unload time
    WORD wTotalSectors;                 // Total sectors per diskette
} Tdrives;


// The following types are supported:
#define NUM_DRIVES          3           // Number of drives described
#define   DRIVE_720         0           // 3.5", 720 Kb
#define   DRIVE_1P44        1           // 3.5", 1.44 Mb
#define   DRIVE_1P2         2           // 5.25", 1.2 Mb

static Tdrives drives[ NUM_DRIVES ] =
{
//    Gap
//     |   Transfer rate: 2=250, 1=300, 0=500 Kbps
//     |    |  Sectors per track
//     |    |   |  Number of tracks on one side
//     |    |   |   |   Stepping rate [7:4] and Head unload time [3:0]
//     |    |   |   |    |    Total number of sectors
//     |    |   |   |    |     |
    { 0x2A, 2,  9,  80, 0xDF, 1440 },
    { 0x1B, 0, 18,  80, 0xCF, 2880 },
    { 0x1B, 0, 15,  80, 0xCF, 2400 }
};


// Define all accessible floppy devices
static struct floppy
{
   BYTE bOperation;                     // Current operation
   BYTE *pbAddress;                     // Memory buffer address
   WORD wCount;                         // Number of bytes to transfer

   BYTE bTrack;                         // Destination track to access
   BYTE bHead;                          // The head number to use
   int  nInterruptAck;                  // Interrupt counter
   BYTE bCalibrated;                    // Flag to request calibration
   BYTE bCurrentTrack;                  // Current track that the head is on
   BYTE bReset;                         // FDC needs reset
   BYTE bRunning;                       // Motor is running or not
   BYTE bDriveType;                     // Index into drive types
   BYTE bDrive;                         // Drive number US1 US0
} floppy [MAX_MINOR_FLOPPY];

static struct floppy *fd;               // Current floppy

static BYTE sbResult[ NUM_RESULT ];     // FDC result string

// Local error codes
#define ERR_TIMEOUT         -128        // Wait for interrupt had timed out
#define ERR_SEEK            -129
#define ERR_STATUS          -130
#define ERR_TRANSFER        -131
#define ERR_RECALIBRATE     -132
#define ERR_WRITE_PROTECT   -133


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   BYTE FDC_Init()                                                           *
*                                                                             *
*******************************************************************************
*
*   Initializes floppy driver
*
*   Returns:
*       Number of minor devices that can be managed
*
******************************************************************************/
BYTE FDC_Init()
{
    fd = &floppy[ 0 ];

    fd->bReset = 1;
    fd->bRunning = 0;
    fd->bCalibrated = UNCALIBRATED;
    fd->bCurrentTrack = 0xFF;
    fd->bDriveType = DRIVE_1P44;
    fd->bDrive = 0;                 // Floppy 0 (A:)


    fd = &floppy[ 1 ];

    fd->bReset = 1;
    fd->bRunning = 0;
    fd->bCalibrated = UNCALIBRATED;
    fd->bCurrentTrack = 0xFF;
    fd->bDriveType = DRIVE_1P2;
    fd->bDrive = 1;                 // Floppy 1 (B:)

    return( 2 );
}


/******************************************************************************
*
*   DWORD FDC_GetTrackLen( BYTE bMinor )
*
*       Returns the length of a track of the specific floppy device.  Use this
*       value to allocate a buffer for read/write operations.
*
*   Where:
*       bMinor is the minor device number
*
*   Returns:
*       length of the track in bytes
*       0 if device is not assigned or invalid
*
******************************************************************************/
DWORD FDC_GetTrackLen( BYTE bMinor )
{
    if( bMinor >= MAX_MINOR_FLOPPY ) return( 0 );
    if( floppy[bMinor].bDriveType >= NUM_DRIVES ) return( 0 );
    return( drives[ floppy[bMinor].bDriveType ].bSectors * 512 );
}


/******************************************************************************
*
*   int FDC_WaitForInterrupt()
*
*      Waits for floppy controller interrupt.  Number of interrupts should
*      be set by the operation that schedules an interrupt.  This function
*      just waits until the correct number of interrupts has occurred (count
*      becomes zero).
*
*   Returns:
*      OK if interrupt occurred
*      ERR_TIMEOUT if timed-out
*
******************************************************************************/
static int FDC_WaitForInterrupt()
{
   int i;

   DEB("Collecting interrupt...");

   // Arm the counter
   i = NUM_INTWAIT;

   while( fd->nInterruptAck > 0 )
   {
       inp( 0x80 );
       if( i-- == 0 )
       {
           DEB("Wait for interrupt timed-out!\n");
           return( ERR_TIMEOUT );
       }
   }

   DEB("OK\n");
   return( OK );
}


/******************************************************************************
*
*   void FDC_StartMotor()
*
*      Starts the floppy disk motor.  This command does not generate interrupt
*      from the fdc.
*
******************************************************************************/
static void FDC_StartMotor()
{
    BYTE bValue;

    DEB("Start motor\n");
    if( fd->bRunning ) return;

    bValue = (1 << (fd->bDrive+4)) + DOR_SELECT + fd->bDrive;

    DEB("Actually starting the motor\n");
    outp( DOR_CONTROL, bValue );

    // Delay 1/4 seconds
    delay(250);

    fd->bRunning = 1;
}


/******************************************************************************
*
*   void FDC_StopMotor()
*
*      Stops the floppy disk motor.  This command does not generate interrupt
*      from the fdc.
*
******************************************************************************/
static void FDC_StopMotor()
{
    BYTE bValue;

    DEB("Stop motor\n");

    bValue = (0 << (fd->bDrive+4)) + DOR_SELECT + fd->bDrive;

    outp( DOR_CONTROL, bValue );
    fd->bRunning = 0;
}


/******************************************************************************
*
*   void FDC_reset()
*
*      Resets the Floppy Disk Controller.  Reprograms it with the current
*      working characteristics.
*
******************************************************************************/
static void FDC_reset()
{
   int res, i;

   fd->bReset = 0;
   fd->bRunning = 0;

   DEB("FDC Reset\n");
   DisableInterrupts();

   // Reset the FDC using the old style and new style:
   outp( FDC_STATUS, STAT_RESET );      // New style reset

   fd->nInterruptAck = 1;               // Expect an interrupt
   outp( DOR_CONTROL, DOR_RESET );
   outp( DOR_CONTROL, DOR_SELECT + fd->bDrive );

   EnableInterrupts();
   FDC_WaitForInterrupt();

   // Get the status
   sbResult[0] = 0;
   FDC_out( SENSE );
   res = FDC_GetResult();
   if( res != OK )
   {
       DEB("FDC Wont reset!\n");
       return;
   }

   res = sbResult[0];
   if( res != DOR_RESET_ACK )
   {
       DEB("FDC did not become ready after reset!\n");
       return;
   }

   // Program the Floppy Drive
   outp( FDC_CONFIG, TRANSFER_RATE );

   FDC_out( SPECIFY );
   FDC_out( SPEC1 );
   FDC_out( SPEC2 );

   // Some FDC/drive combinations need a stabilization period of several
   // miliseconds after data rate set and before R/W operation.
   //
   delay(35);

   fd->bCalibrated = UNCALIBRATED;
}


/******************************************************************************
*
*   int FDC_Recalibrate()
*
*      Recalibrates the floppy drive.  This will position the head on the
*      track 0.
*
******************************************************************************/
static int FDC_Recalibrate()
{
    int res;

    DEB("Recalibrate\n");
    FDC_StartMotor();

    // Force seek next time
    fd->bCurrentTrack = 0xFF;

    // Recalibrate the motor
    fd->nInterruptAck = 1;              // Schedule an interrupt
    FDC_out( RECAL );
    FDC_out( fd->bDrive );

    if( fd->bReset ) return( ERR_SEEK );

    // Wait for interrupt after the operation
    FDC_WaitForInterrupt();

    // Get the status
    FDC_out( SENSE );
    res = FDC_GetResult();

    if( res != OK
    || (sbResult[ST0]&ST0_BITS) != SEEK_ST0
    || (sbResult[ST_TRACK] != 0) )
    {
       // Recalibration failed
       DEB("Recalibration failed\n");

       fd->bReset = 1;
       fd->bCalibrated = UNCALIBRATED;
       return( ERR_RECALIBRATE );
    }

    fd->bCalibrated = CALIBRATED;
    return( OK );
}


/******************************************************************************
*
*   void FDC_out( BYTE bValue )
*
*      Outputs a byte to the Floppy Disk Controller
*
*   Where:
*      bValue - byte to be sent
*
******************************************************************************/
static void FDC_out( BYTE bValue )
{
   DWORD dwRetry;
   BYTE bIn;

//   DEB("FDC_out( %2X )\n", bValue );

   // Check if controller needs reset - no need to output then
   if( fd->bReset )
   {
       DEB("returning due to reset\n");
       return;
   }

   dwRetry = NUM_FDC_RETRY;

   // It may take several tries to get the fdc to accept the command
   while( dwRetry-- > 0 )
   {
       bIn = inp( FDC_STATUS );
       bIn &= (STAT_READY | STAT_DIRECTION | STAT_DMA);
       if( bIn != STAT_READY ) continue;

       // FDC is ready and listening, so send a command and return
       outp( FDC_DATA, bValue );
       return;
   }

   DEB("FDC does not listen!\n");
   fd->bReset = 1;
}


/******************************************************************************
*
*   int FDC_GetResult()
*
*      Reads the result byte stream from the controller data port and stores
*   the resulting stream to the sbResult[].
*
******************************************************************************/
static int FDC_GetResult()
{
    BYTE bValue;
    int i, j;

    DEB("FDC_GetResult: ");

    // Little delay
    IODELAY( NUM_DELAY );

    // Loop and read the resulting byte stream
    for( i=0; i<NUM_RESULT; i++ )
    {
        // Read the status port
        bValue = inp( FDC_STATUS );
        if( (bValue & STAT_READY)==0 )
        {
           DEB("ERR_STATUS\n");
           return( ERR_STATUS );
        }

        bValue = inp( FDC_STATUS );
        if( (bValue & STAT_DIRECTION)==0 )
        {
           DEB("ERR_STATUS\n");
           return( ERR_STATUS );
        }

        // Now we are talking... Get a byte and store it
        bValue = inp( FDC_DATA );
        DEB("%02X ", bValue );
        sbResult[i] = bValue;

        // Little delay
        IODELAY( NUM_DELAY );

        // Check if that was all the controller has to say
        bValue = inp( FDC_STATUS );
        if( (bValue & STAT_BUSY)==0 )
        {
           DEB(" .. OK\n");
           return( OK );
        }
    }

    // Too many results, controller reset required
    DEB("Reset needed!\n");

    fd->bReset = 1;
    return( ERR_STATUS );
}


/******************************************************************************
*
*   int FDC_Transfer()
*
*      Performs the transfer between floppy disk and memory.
*
******************************************************************************/
static int FDC_Transfer()
{
    WORD wTotal;
    int res;

    // Check if calibration is needed
    if( fd->bCalibrated == UNCALIBRATED )
        return( ERR_TRANSFER );

    DEB("Transfer\n");

    fd->nInterruptAck = 1;             // Schedule an interrupt

    // Output 9 bytes to the controller chip
    FDC_out( fd->bOperation );          // FDC_READ or FDC_WRITE
    FDC_out( (fd->bHead << 2) + fd->bDrive );
    FDC_out( fd->bTrack );
    FDC_out( fd->bHead );
    FDC_out( 1 );                       // Starting is the sector 1
    FDC_out( RW_BPS );                  // Bytes per sector
    FDC_out( NUM_SECTORS );             // Sectors/track
    FDC_out( RW_GAP3 );                 // Gap size
    FDC_out( RW_SECTOR );               // Sector size

    if( fd->bReset ) return( ERR_TRANSFER );

    // Wait for interrupt after the operation
    FDC_WaitForInterrupt();

    // Get the status
    res = FDC_GetResult();
    DEB("res: %d\n", res );

    if( res != OK ) return( res );

    // Examine the return status for errors
    if((sbResult[ST1]&BAD_SECTOR) || (sbResult[ST2]&BAD_TRACK) )
    {
       DEB("* Bad sector/track\n");
       fd->bCalibrated = UNCALIBRATED;
    }

    if( sbResult[ST1]&WRITE_PROT )
    {
       DEB("* Write protected disk!\n");
       return( ERR_WRITE_PROTECT );
    }

    if( (sbResult[ST0]&ST0_BITS) != TRANS_ST0 ) return( ERR_TRANSFER );
    if( sbResult[ST1] | sbResult[ST2] ) return( ERR_TRANSFER );

    // Confirm the data length
    DEB("sbRes[ST_SECTOR]: %d\n", sbResult[ST_SECTOR] );
    wTotal = (sbResult[ST_SECTOR] - 1) * 512;
    printf("TOTAL: %d\n", wTotal );
//    if( wTotal != fd->wCount ) return( ERR_TRANSFER );

    return( OK );
}


/******************************************************************************
*
*   int FDC_Seek()
*
*      Positions a floppy head onto the given track
*
******************************************************************************/
static int FDC_Seek()
{
    int res;

    DEB("FDC_Seek\n");

    if( fd->bCalibrated == UNCALIBRATED )
       if( FDC_Recalibrate() != OK ) return( ERR_SEEK );

    // No need to issue seek command if the head is already on the right track
    if( fd->bCurrentTrack == fd->bTrack ) return( OK );

    fd->nInterruptAck = 1;              // Schedule an interrupt

    // Issue the seek command
    FDC_out( SEEK );
    FDC_out( (fd->bHead << 2) + fd->bDrive );
    FDC_out( fd->bTrack );

    if( fd->bReset ) return( ERR_SEEK );

    // Wait for interrupt and read the drive status after the seek command
    FDC_WaitForInterrupt();

    FDC_out( SENSE );
    res = FDC_GetResult();

    // Check the seek bits for error
    if((sbResult[ST0]&ST0_BITS) != SEEK_ST0 ) res = ERR_SEEK;
    if( sbResult[ST1] != fd->bTrack ) res = ERR_SEEK;
    if( res != OK )
       if( FDC_Recalibrate() != OK ) return( ERR_SEEK );
    if( res==OK )
        fd->bCurrentTrack = fd->bTrack;

    return( res );
}


/******************************************************************************
*
*   void dma_setup()
*
*      Sets the DMA for the transfer
*
******************************************************************************/
static void dma_setup()
{
   int mode, low_addr, high_addr, top_addr, low_ct, high_ct;

   mode = (fd->bOperation==READ)? DMA_READ : DMA_WRITE;
   low_addr  = ((int)fd->pbAddress >> 0 ) & 255;
   high_addr = ((int)fd->pbAddress >> 8 ) & 255;
   top_addr  = ((int)fd->pbAddress >> 16) & 255;
   low_ct    = ((fd->wCount - 1) >> 0) & 255;
   high_ct   = ((fd->wCount - 1) >> 8) & 255;

   DisableInterrupts();

   // Set up the DMA registers
   outp( DMA_M2, mode );
   outp( DMA_M1, mode );
   outp( DMA_ADDR, low_addr );
   outp( DMA_ADDR, high_addr );
   outp( DMA_TOP, top_addr );
   outp( DMA_COUNT, low_ct );
   outp( DMA_COUNT, high_ct );

   EnableInterrupts();

   // Initialize DMA
   outp( DMA_INIT, DMA_ARM );
}


/******************************************************************************
*
*   int FDC_Operation( BYTE bMinor, BYTE bOperation, BYTE bTrack, BYTE * pbBuf)
*
*       Performs the operation of reading or writing a track.
*
*   Where:
*       bMinor is the minor device number
*       bOperation is FDC_READ or FDC_WRITE
*       bTrack is the track number to read or to write.
*       pbBuf is a pointer to a sufficiently large buffer to where to store
*           the track. Buffer must not cross 64K boudary!
*
*   Returns:
*       OK      for success
*       EIO     if an IO error occurred or disk is write-protected
*       EINVAL  if the argument is out of range
*
******************************************************************************/
int FDC_Operation( BYTE bMinor, BYTE bOperation, BYTE bTrack, BYTE * pbBuf)
{
   int errors, ret;

   // Set the minor device pointer
   if( bMinor >= MAX_MINOR_FLOPPY ) return( EINVAL );
   fd = &floppy[ bMinor ];

   // Set the operation code and destination address
   fd->bOperation = bOperation==FDC_READ? READ : WRITE;
   fd->pbAddress = pbBuf;

   fd->wCount = NUM_SECTORS * 512;
//   fd->wCount = 2 * 512;
//   fd->wCount = 1 * 512;     // !!!

   // Calculate the effective track and head numbers
   fd->bHead = bTrack % NUM_HEADS;
   fd->bTrack = bTrack / NUM_HEADS;
   if( fd->bHead >= NUM_HEADS || fd->bTrack >= NUM_TRACKS ) return( EINVAL );

   DEB("Effective track: %d, head: %d\n", fd->bTrack, fd->bHead );

   errors = 0;
   while( errors++ < NUM_ERRORS )
   {
       // First check if reset is needed
       if( fd->bReset ) FDC_reset();

       // Start the DMA chip
       dma_setup();

       // Start the motor
       FDC_StartMotor();

       ret = FDC_Seek();
       if( ret != OK ) continue;

       // Perform the transfer
       ret = FDC_Transfer();
       if( ret == OK )
       {
            FDC_StopMotor();
            return( OK );
       }
       if( ret == ERR_WRITE_PROTECT ) break;
   }

   // Stop the motor
   FDC_StopMotor();

   return( ret == OK? OK : EIO );
}


#if TEST

/******************************************************************************
*   DOS Compatibility Section
******************************************************************************/

void (__interrupt __far *prev_int_E)();

#pragma off (check_stack)
#pragma aux FDCHandler parm;
interrupt FDCHandler()
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
   fd->nInterruptAck--;

   outp(0x20, 0x20);
}
void HandlerEnd() {}
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


BYTE FDDInit()
{
   int error, res;
#if 1
   if( ( !lock_region ( &fd->nInterruptAck, 4) ) ||
       ( !lock_region ((void near *) FDCHandler,
       (char *) HandlerEnd - (char near *) FDCHandler)) )
   {
       printf( "locks failed\n" );
       exit(1);
   }
#endif
   prev_int_E = _dos_getvect( 0x0e );
   _dos_setvect( 0x0E, FDCHandler );

//*******************************************************************

#define ADDR    0xB8000

    FDC_Init();

    printf("Track len A: %d, B: %d\n",
        FDC_GetTrackLen( 0 ), FDC_GetTrackLen( 1 ) );

#if 0
    error = FDC_Operation( 0, FDC_READ, 1, (BYTE *) ADDR );
    printf("Result of read operation: %d\n", error );
    if( error != OK ) goto exit;
    getch();

    *(BYTE *)ADDR = 'Q';

    error = FDC_Operation( 0, FDC_WRITE, 1, (BYTE *) ADDR );
    printf("Result of write operation: %d\n", error );
    *(BYTE *)ADDR = ' ';

    if( error != OK ) goto exit;
    getch();

    error = FDC_Operation( 0, FDC_READ, 1, (BYTE *) ADDR );
    printf("Result of read back operation: %d\n", error );

//*******************************************************************
#endif

    return( 2 );

exit:;
//   _dos_setvect( 0x0E, prev_int_E );
}

FDC_Close()
{
    _dos_setvect( 0x0E, prev_int_E );
}

#endif // TEST


