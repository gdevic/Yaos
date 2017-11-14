/******************************************************************************
*                                                                             *
*   Module:     TTY.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is the terminal device driver.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <errno.h>                      // Include error definitions

#include "device.h"                     // Include device driver header file

#include "kernel.h"                     // Include kernel header

#include "v86.h"                        // Include V86 module header

#include "display.h"                    // Include debug display header

#include "inline.h"                     // Include inline functions

#include "tty.h"                        // Include its own header

#include "process.h"                    // Include process header

#include "intel.h"                      // Include Intel-specific defines

#include "mm.h"                         // Include memory management header

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

TTY_Struct *pCurTTY = NULL;             // Current (controlling) terminal


/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define DEFAULT_ATTRIB      7           // Default terminal attribute


//----------------------------------------------------------------------------
// Define a device control block for the terminal device
//----------------------------------------------------------------------------

int TTY_open( BYTE bMinor, TDevRq *pRq );
int TTY_close( BYTE bMinor, TDevRq *pRq );
int TTY_read( BYTE bMinor, TDevRq *pRq );
int TTY_write( BYTE bMinor, TDevRq *pRq );

//----------------------------------------------------------------------------

TDev DevTTY =                           // Device control block template
{
    MINOR_TTY,                          // Number of minor instances

    Dev_null,                           // Initialize terminal device

    Dev_null,                           //  Create a file
    TTY_open,                           //  Open a file
    TTY_close,                          //  Close a file
    TTY_read,                           //  Read from a file
    TTY_write,                          //  Write to a file
    Dev_null,                           //  Seek into a file
    Dev_null,                           //  Control aspects of a device
    Dev_null                            //  Get stat of a node
};

//----------------------------------------------------------------------------

// Define the array of pointers to terminal device instances data

TTY_Struct *TTY[ MINOR_TTY ] =
{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

void term_out( TTY_pStruct *pTTY, char c );
void term_in ( TTY_pStruct *pTTY, char c );


/******************************************************************************
*                                                                             *
*   Keyboard Queue Functions                                                  *
*                                                                             *
*******************************************************************************
*
*   Keyboard queue functions:
*
#   int kIsEmpty( TTY_Queue *pQ );
#   int kIsFull( TTY_Queue *pQ );
#   int kSearch( TTY_Queue *pQ, BYTE c1, BYTE c2 );
#   void kEnqueue( TTY_Queue *pQ, BYTE c );
#   BYTE kDequeue( TTY_Queue *pQ );
#   BYTE kPeek( TTY_Queue *pQ );
#   BYTE kDequeueHead( TTY_Queue *pQ );
#
#   Before enqueue and dequeue you need to check that the queue is not
#   full/empty.
*
******************************************************************************/

int kIsEmpty( TTY_Queue *pQ )
{
    return( pQ->head == pQ->tail );
}

int kIsFull( TTY_Queue *pQ )
{
    int i;

    i = (pQ->head+1 == TTY_BUFLEN) ? 0 : pQ->head + 1;

    return( i == pQ->head );
}

int kSearch( TTY_Queue *pQ, BYTE c1, BYTE c2 )
{
    int i = pQ->tail;

    while( i != pQ->head )
    {
        if( (pQ->buf[i] == c1) || (pQ->buf[i] == c2) )
            return( 1 );

        if( i+1 == TTY_BUFLEN )
            i = 0;
        else
            i++;
    }

    return( 0 );
}

void kEnqueue( TTY_Queue *pQ, BYTE c )
{
    pQ->buf[ pQ->head ] = c;

    if( pQ->head+1 == TTY_BUFLEN )
        pQ->head = 0;
    else
        pQ->head++;
}

BYTE kDequeue( TTY_Queue *pQ )
{
    BYTE c;

    c = pQ->buf[ pQ->tail ];

    if( pQ->tail+1 == TTY_BUFLEN )
        pQ->tail = 0;
    else
        pQ->tail++;

    return( c );
}

BYTE kPeek( TTY_Queue *pQ )
{
    return( pQ->buf[ pQ->tail ] );
}

BYTE kDequeueHead( TTY_Queue *pQ )
{
    if( pQ->head == 0 )
        pQ->head = TTY_BUFLEN - 1;
    else
        pQ->head--;

    return( pQ->buf[ pQ->head ] );
}


/******************************************************************************
*                                                                             *
*   TTY_Struct * CreateTTY()                                                  *
*                                                                             *
*******************************************************************************
*
*   This function creates a terminal structure for the use by a process.
*
*   Returns:
*       pointer to dynamically allocated terminal structure
#       NULL if the memory could not be allocated
*
******************************************************************************/
TTY_Struct * CreateTTY()
{
    TTY_Struct *pTTY;
    int i, j;


    // Allocate memory for the terminal structure

    pTTY = (TTY_Struct *) MALLOC( sizeof(TTY_Struct) );

    if( pTTY != NULL )
    {
        // Fill in the values of a terminal structure:

        // Set the initial flags

        pTTY->c_iflag = ICRNL + IXON;
        pTTY->c_oflag = IGNCR;
        pTTY->c_lflag = ECHO + ECHOE + ECHOK + ECHONL + ICANON;
        pTTY->c_cflag = 0;

        // Fill in the control characters in the c_cc array

        pTTY->c_cc[VEOF]  = EOF_CHAR;
        pTTY->c_cc[VEOL]  = EOL_CHAR;
        pTTY->c_cc[VERASE]= ERASE_CHAR;
        pTTY->c_cc[VINTR] = INTR_CHAR;
        pTTY->c_cc[VKILL] = KILL_CHAR;
        pTTY->c_cc[VQUIT] = QUIT_CHAR;
        pTTY->c_cc[VSUSP] = SUSP_CHAR;
        pTTY->c_cc[VSTART]= START_CHAR;
        pTTY->c_cc[VSTOP] = STOP_CHAR;

        // Set private fields of the terminal structure

        pTTY->x = 0;
        pTTY->y = 0;
        pTTY->stop = 0;
        pTTY->tabs = 8;
        pTTY->attrib = DEFAULT_ATTRIB;

        // Set the input and output queue indices

        pTTY->in.head = 0;
        pTTY->in.tail = 0;
        pTTY->out.head = 0;
        pTTY->out.tail = 0;

        // Set the minor number to free

        pTTY->bMinor = -1;

        // Set the input and output functions

        pTTY->fnOut = term_out;
        pTTY->fnIn  = term_in;

        // Reset the screen area

        for( i=0; i<TTY_LINES; i++)
        {
            for( j=0; j<80; j++ )
            {
                pTTY->screen[i][j] = (DEFAULT_ATTRIB << 8) + ' ';
            }
        }

        // If the current controlling terminal does not point to any of the
        // terminal structures, set it to this one.  That will show the term
        // by default upon creation, and we wont have to switch windows

        for( i=0; i<MINOR_TTY; i++ )
            if( pCurTTY == TTY[i] )
                break;

        if( (pCurTTY==NULL) || (i==MINOR_TTY) )
            pCurTTY = pTTY;

        // Return a pointer to a new terminal structure

        return( pTTY );
    }

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   void DestroyTTY( TTY_Struct *pTTY )                                       *
*                                                                             *
*******************************************************************************
*
*   This function destroys a given terminal structure.
*
*   Where:
*       pTTY is a pointer to a terminal structure to be destroyed
*
*   Returns:
*       void
*
******************************************************************************/
void DestroyTTY( TTY_Struct *pTTY )
{
    FREE( pTTY );
}


/******************************************************************************
*                                                                             *
*   int TTY_open( BYTE bMinor, TDevRq *pRq )                                  *
*                                                                             *
*******************************************************************************
*
*   This function opens (creates if it not already created) a minor terminal
#   device.
*
*   Where:
*       bMinor is a minor number
#       pRq is the device request block structure (unused)
#
*   Returns:
#       bMinor number (>=0) if device opened
#       Error code < 0 if device could not be opened
*
******************************************************************************/
int TTY_open( BYTE bMinor, TDevRq *pRq )
{
//dprintf("\nTTY_open %d %08X", bMinor, TTY[bMinor] );

    // Basic range check

    if( bMinor >= MINOR_TTY ) return( EBADF );

    if( TTY[bMinor] == NULL )
    {
        // Create a terminal structure

        if( (TTY[bMinor] = CreateTTY()) == NULL )
            return( EMFILE );
    }

    // Set the minor number in the terminal structure

    TTY[bMinor]->bMinor = bMinor;

pCurTTY = TTY[bMinor];

    return( bMinor );
}


/******************************************************************************
*                                                                             *
*   int TTY_close( BYTE bMinor, TDevRq *pRq )                                 *
*                                                                             *
*******************************************************************************
*
*   This function closes and deallocates a terminal device
*
*   Where:
*       bMinor is a minor number
#       pRq is the device request block structure
*
*   Returns:
#       0 if device was successfully closed
#       Error code if error
*
******************************************************************************/
int TTY_close( BYTE bMinor, TDevRq *pRq )
{
//dprintf("\nTTY_close %d", bMinor );

    // Basic range check

    if( bMinor >= MINOR_TTY ) return( EBADF );

    // Check if the attempt is made to close an unopened minor device

    if( TTY[bMinor] == NULL ) return( EBADF );

    DestroyTTY( TTY[bMinor] );

    TTY[bMinor] = NULL;

    return( 0 );
}


/******************************************************************************
*                                                                             *
*   int TTY_read( BYTE bMinor, TDevRq *pRq )                                  *
*                                                                             *
*******************************************************************************
*
*   This function reads a character from a terminal device.
#
#   This call can block a process if a character is not available.
*
*   Where:
*       bMinor is a minor number
#       pRq is the device request block structure
*
*   Returns:
*       number of characters read
#       error code
#       EAGAIN if a process needs to be blocked for this device
*
******************************************************************************/
int TTY_read( BYTE bMinor, TDevRq *pRq )
{
    TTY_Struct *pTTY;
    BYTE c;
    int count = 0;

//dprintf("\nTTY_read %d (%08X,%d)", bMinor, pRq->p, pRq->n );

    // Basic range check

    if( bMinor >= MINOR_TTY ) return( EBADF );
    pTTY = TTY[bMinor];

    // Check if the attempt is made to use unopened minor device

    if( pTTY == NULL ) return( EBADF );

    // If a character is not available, return error

    if( kIsEmpty(&pTTY->in) )
    {
        // Signal device does not have data ready for the request

        // If the process should block, we send error; if not, we send 0

//        if( non-blocking ) return 0;

        return( EAGAIN );
    }

    // In the canonical mode, look for the presence of queued newline and
    // block if it is not yet there

    if( pTTY->c_lflag & ICANON )
    {
        // We will block if there is no newline or end-of-file character

        if( !kSearch(&pTTY->in, pTTY->c_cc[VEOL], pTTY->c_cc[VEOF]) )
            return( EAGAIN );
    }

    // Send the characters, all available or requested

    while( pRq->n && !kIsEmpty(&pTTY->in) )
    {
        c = *pRq->p++ = kDequeue(&pTTY->in);
        count++;
        pRq->n--;

        // If the process request count becomes zero,
        // we have satisfy the request

        if( pRq->n == 0 )  return( count );

        // In the canonical mode we copy only characters up to and
        // including the newline or end-of-file

        if( (pTTY->c_lflag & ICANON)
        && ((c == pTTY->c_cc[VEOL]) || (c == pTTY->c_cc[VEOF])) )
        {
            return( count );
        }
    }

    // We did not satisfy the request - block the process

    return( EAGAIN );
}


/******************************************************************************
*                                                                             *
*   void term_in( TTY_pStruct *pTTY, char c )                                 *
*                                                                             *
*******************************************************************************
*
*   This function accepts a character from the keyboard and updates the
#   terminal buffer.  It is called when a key has been depressed.
#
#   It can unblock a waiting process.
*
*   Where:
*       pTTY is a pointer to a terminal
#       c is the character to write into a terminal structure
*
*   Returns:
*       void
*
******************************************************************************/
void term_in( TTY_pStruct *pTTY, char c )
{
    TDevRq Wq, *pRq;
    int pid, count = 0, stop = 0;


    // In the canonical mode we need to do some more processing

    if( pTTY->c_lflag & ICANON )
    {
        // Ster start/stop character flags

        if( c == pTTY->c_cc[VSTOP] )
        {
            pTTY->stop = 1;
            return;
        }

        if( c == pTTY->c_cc[VSTART] )
        {
            pTTY->stop = 0;
            return;
        }

        // If the character is backspace, erase the last queued character

        if( c == pTTY->c_cc[VERASE] )
        {
            // If the queue is empty, just return

            if( kIsEmpty(&pTTY->in) )  return;

            kDequeueHead(&pTTY->in);
            stop = 1;
        }
        else

        // If a character is a kill line, null the queue

        if( c == pTTY->c_cc[VKILL] )
        {
            pTTY->in.head = pTTY->in.tail;
            stop = 1;
        }
        else

        // If '\n' and INLCR, set CR instead

        if( (c == '\n') && (pTTY->c_iflag & INLCR) )
            c = '\r';
        else

        // If '\r' and IGNCR, ignore character

        if( (c == '\r') && (pTTY->c_iflag & IGNCR) )
            return;
        else

        // If '\r' and ICRNL, set LF instead

        if( (c == '\r' ) && (pTTY->c_iflag & ICRNL) )
            c = '\n';
    }

    // If the echo is enabled, form the device request buffer and echo a char

    if( (pTTY->c_lflag & ECHO)
     || ((pTTY->c_lflag & ECHONL) && (pTTY->c_lflag & ICANON) && (c==pTTY->c_cc[VEOL])) )
    {
        Wq.p = &c;                      // Character at `c'
        Wq.n = 1;                       // Length of 1

        TTY_write( pTTY->bMinor, &Wq );
    }

    // Stop processing if signalled or queue the character

    if( stop )
        return;
    else
        if( !kIsFull( &pTTY->in) )  kEnqueue(&pTTY->in, c);


    // If a process is waiting on a terminal read, send it a character

    if( pid = IsBlocked_Read( &pRq ) )
    {
        // In a raw mode, send the current character

        if( !(pTTY->c_lflag & ICANON) )
        {
            *pRq->p++ = kDequeue(&pTTY->in);
            pRq->n--;

            // If the process request count becomes zero,
            // unblock the process

            if( pRq->n == 0 )
            {
                Unblock_Read();
                DevUnblock( pid, 1 );
            }
        }
        else
        {
            // In a canonical mode, if we have hit newline or end-of-file
            // copy down all queued characters up to and including the
            // terminating one

            if( (c == pTTY->c_cc[VEOL]) || (c == pTTY->c_cc[VEOF]) )
            {
                while( pRq->n && !kIsEmpty(&pTTY->in) )
                {
                    c = *pRq->p++ = kDequeue(&pTTY->in);
                    pRq->n--;
                    count++;

                    if( (c == pTTY->c_cc[VEOL]) || (c == pTTY->c_cc[VEOF]) )
                        break;
                }

                Unblock_Read();
                DevUnblock( pid, count );
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   int TTY_write( BYTE bMinor, TDevRq *pRq )                                 *
*                                                                             *
*******************************************************************************
*
*   This function writes to a terminal device.  It uses terminal's current
#   setting (raw/cooked mode).
#
*       If ICANON
*           if ERASE_CHAR, erase the last char from the display
*           if KILL_CHAR, erase the last line from the display
*
*   Where:
*       bMinor is a minor number
#       pRq is the device request block structure:
#           n - number or characters to write
#           p - pointer to a character string to write
*
*   Returns:
*       number of characters written
*
******************************************************************************/
int TTY_write( BYTE bMinor, TDevRq *pRq )
{
    TTY_Struct *pTTY;
    char c;
    int nWritten = 0;

//dprintf("\nTTY_write %d (%08X,%d)", bMinor, pRq->p, pRq->n );

    // Basic range check

    if( bMinor >= MINOR_TTY ) return( EBADF );
    pTTY = TTY[bMinor];

    // Check if the attempt is made to use unopened minor device

    if( pTTY == NULL ) return( EBADF );

    while( pRq->n-- > 0 )
    {
        c = *pRq->p++;
        nWritten++;

        // If in cooked mode, process erase and kill characters

        if( pTTY->c_lflag & ICANON )
        {
            if( (pTTY->c_lflag & ECHOE) && (c == pTTY->c_cc[VERASE]) )
            {
                // Erase the last character

                pTTY->fnOut( pTTY, pTTY->c_cc[VERASE] );
                pTTY->fnOut( pTTY, ' ');
                c = pTTY->c_cc[VERASE];
            }
            else
            if( (pTTY->c_lflag & ECHOK) && (c == pTTY->c_cc[VKILL]) )
            {
                // Kill line character, simulated with a CR code

                c = pTTY->c_cc[VEOL];
            }
            else

            // If '\r' and ICRNL, set LF instead

            if( (c == '\r' ) && (pTTY->c_oflag & ICRNL) ) c = '\n';

            // If '\n' and INLCR, set CR instead

            if( (c == '\n') && (pTTY->c_oflag & INLCR) )  c = '\r';

            // If '\r' and IGNCR, ignore character

            if( (c == '\r') && (pTTY->c_oflag & IGNCR) )  continue;

            // If the stop flag has been set, do not print character

            if( pTTY->stop )  continue;
        }

        // Printable character

        pTTY->fnOut( pTTY, c );
    }

    // If the writing terminal is controlling, position the cursor

    if( pTTY == pCurTTY )
    {
        SetCursor( pTTY->x + 80 * pTTY->y );
    }

    return( nWritten );
}


/******************************************************************************
*                                                                             *
*   void term_out( TTY_pStruct *pTTY, char c )                                *
*                                                                             *
*******************************************************************************
*
*   This function performs actual writing of a character to the terminal
#   buffer and to the screen, if the terminal is the controlling.
*
*   Where:
*       pTTY is a pointer to a terminal
#       c is the character to write
*
*   Returns:
*       void
*
******************************************************************************/
/******************************************************************************
*                                                                             *
*   Helper function that scrolls the display                                  *
*                                                                             *
******************************************************************************/
void new_line( TTY_Struct *pTTY )
{
    //------------------------------------------------------------------------
    // Scroll up TTY_LINES-1 lines
    //------------------------------------------------------------------------

    abs_memcpy( (DWORD) &pTTY->screen[0] + LIN_KERNEL,
                (DWORD) &pTTY->screen[1] + LIN_KERNEL, (TTY_LINES-1) * 160 );

    // Clear the last line

    abs_memsetw( (DWORD) &pTTY->screen[TTY_LINES-1] + LIN_KERNEL,
                 (pTTY->attrib << 8) + ' ', 80 );

    // Do the same thing to the screen if a terminal is controlling
    // For now, Just copy the whole buffer

    if( pTTY == pCurTTY )
    {
        abs_memcpy( LIN_SCREEN,
                    LIN_KERNEL + (DWORD) pTTY->screen,
                    (TTY_LINES) * 160 );
    }
}

/******************************************************************************
*                                                                             *
*   void term_out( TTY_pStruct *pTTY, char c )                                *
*                                                                             *
******************************************************************************/
void term_out( TTY_pStruct *pTTY, char c )
{
    BYTE x, y;


    // Check for few special codes:
    // End-of-line moves the cursor to the beginning of a new line

    if( c == pTTY->c_cc[VEOL] )
    {
        pTTY->x = 0;

        if( ++pTTY->y >= TTY_LINES )
        {
            pTTY->y--;
            new_line( pTTY );
        }

        return;
    }

    // Erase character moves the cursor one place to the left

    if( c == pTTY->c_cc[VERASE] )
    {
        if( pTTY->x )  pTTY->x--;

        return;
    }

    // Get the output coordinates

    x = pTTY->x;
    y = pTTY->y;

    // Store the character into the terminal buf

    pTTY->screen[y][x] = c | (pTTY->attrib << 8);

    // If a terminal is controlling, put the character to the screen as well

    if( pTTY == pCurTTY )
    {
        abs_pokew( LIN_SCREEN + y * 160 + x * 2, c | (pTTY->attrib << 8) );
    }

    // Advance the cursor position

    if( ++x >= 80 )
    {
        // New line

        x = 0;

        if( ++y >= TTY_LINES )
        {
            y--;
            new_line( pTTY );
        }
    }

    pTTY->x = x;
    pTTY->y = y;
}

