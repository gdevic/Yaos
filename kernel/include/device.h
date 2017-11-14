/******************************************************************************
*                                                                             *
*   Module:     Device.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/17/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.-
    Module Description:

        This is a header file for the device module.  The device control block
        structure is defined here.  It defines functions that a driver exports

        Also the device request buffer is defined here.  It is used for all 
        device driver communication.
-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/17/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _DEVICE_H_
#define _DEVICE_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define MAX_DEVMAJOR        8           // Number of major devices supported

//.-
//----------------------------------------------------------------------------
// Device driver blocks - if this changes, every device definition block must
// change, e.g. in tty.c, DosFS.c,...
//----------------------------------------------------------------------------

typedef struct
{
    int nMinors;                        // Number of minor instances

    int (* dev_init)();                 // Device initialization as whole
                                        // Calls to a device driver: --
    int (* dev_creat)();                //  Create a file
    int (* dev_open)();                 //  Open a file
    int (* dev_close)();                //  Close a file
    int (* dev_read)();                 //  Read from a file
    int (* dev_write)();                //  Write to a file
    int (* dev_lseek)();                //  Seek into a file
    int (* dev_ioctl)();                //  Control aspects of a device
    int (* dev_stat)();                 //  Get stat of a node

} TDev;


extern TDev Dev[ MAX_DEVMAJOR ];


//----------------------------------------------------------------------------
// Define device request buffer - this buffer is passed to a device driver
// containing optional parameters for a call.
//----------------------------------------------------------------------------

typedef struct
{
    int n;                              // Integer parameter
    char *p;                            // Pointer parameter
    char *s;                            // Stat pointer parameter

} TDevRq;

//-.
/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern void Init_Dev();

//----------------------------------------------------------------------------
// Device functions that are not applicable but not illegal should use
// this function (such is dev_lseek for terminals)
//----------------------------------------------------------------------------
extern int Dev_null();


#endif //  _DEVICE_H_
