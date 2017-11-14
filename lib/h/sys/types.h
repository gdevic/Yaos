/******************************************************************************
*
*   Module:     sys/types.h
*
*   Revision:   1.00
*
*   Date:       09/02/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          Basic data types

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 09/02/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Important defines/undefines
******************************************************************************/
#ifndef _TYPES_H_
#define _TYPES_H_

/******************************************************************************
*   Include Files
******************************************************************************/

/******************************************************************************
*   Local Defines, Variables and Macros
******************************************************************************/
typedef unsigned short dev_t;
typedef unsigned short gid_t;
typedef unsigned short ino_t;
typedef unsigned short mode_t;
typedef unsigned short nlink_t;
typedef long off_t;
typedef int pid_t;

#ifndef _SIZE_T_DEF_
#define _SIZE_T_DEF_
 typedef unsigned size_t;
#endif

#ifndef _SSIZE_T_DEF_
#define _SSIZE_T_DEF_
 typedef unsigned ssize_t;
#endif

typedef unsigned short uid_t;


/******************************************************************************
*   Global Functions
******************************************************************************/


#endif // _TYPES_H_
