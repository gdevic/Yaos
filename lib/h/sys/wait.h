/******************************************************************************
*                                                                             *
*   Module:     wait.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       9/30/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        Unix standard file header

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 9/30/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _WAIT_H_
#define _WAIT_H_


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

typedef int pid_t;                      // Define type for process ID

#define NULL                0L          // Define NULL number


// Macros that extract information from the wait status

#define _HI(stat)               ((stat>>8)&0xFF)
#define _LO(stat)               (stat&0xFF)

#define WIFEXITED(stat)         (_LO(stat)==0)
#define WEXITSTATUS(stat)       _HI(stat)

#define WIFSIGNALED(stat)       ((_HI(stat)==0)&&(_LO(stat)>0))
#define WTERMSIG(stat)          _LO(stat)

#define WIFSTOPPED(stat)        (_LO(stat)==0x7F)
#define WSTOPSIG(stat)          _HI(stat)


// Options for the waitpid() call

#define WNOHANG                 1
#define WUNTRACED               2       // (unused)


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern pid_t wait( int *status );
extern pid_t waitpid( pid_t pid, int *status, int options );



#endif //  _WAIT_H_
