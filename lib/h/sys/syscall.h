/******************************************************************************
*
*   Module:     sys\syscall.h
*
*   Revision:   1.00
*
*   Date:       09/05/96
*
*   Author:     Goran Devic
*
*******************************************************************************

    Module Description:

          This is a header file for the system calls support

*******************************************************************************
*
*   Changes:
*
*   DATE     REV   DESCRIPTION OF CHANGES                          AUTHOR
* --------   ----  ---------------------------------------------   -----------
* 09/05/96   1.00  Original                                        Goran Devic
* --------   ----  ---------------------------------------------   -----------
*******************************************************************************
*   Include Files
******************************************************************************/
#ifndef _SYSCALL_H_
#define _SYSCALL_H_


/******************************************************************************
*   Global Variables, Defines and Macros
******************************************************************************/

#define WRITE_BUF_LEN       1024        // Length of a write buffer


#define NUM_SYSCALLS        18          // Number of system calls


#define SYS_NULL            0           // (unused)
#define SYS_FORK            1           // Fork a process
#define SYS_EXEC            2           // Execute a new process
#define SYS_WAITPID         3           // Wait for a process
#define SYS_EXIT            4           // Exit a process
#define SYS_KILL            5           //

#define SYS_CREAT           6           // Create a new file or rewrite old one
#define SYS_OPEN            7           // Open a file
#define SYS_CLOSE           8           // Close a file
#define SYS_READ            9           // Read from a file
#define SYS_WRITE           10          // Write to a file
#define SYS_LSEEK           11          // Repositions read/write offset
#define SYS_IOCTL           12          // I/O Control interface

#define SYS_STAT            13          // Get info about a file
#define SYS_FSTAT           14          // Get file status

#define SYS_OPENDIR         15          // Opens a process directory stream
#define SYS_READDIR         16          // Reads a directory
#define SYS_CLOSEDIR        17          // Closes a process directory stream

#if 0

#define SYS_RENAME          38          //
#define SYS_MKDIR           39          //
#define SYS_RMDIR           40          //

#define SYS_DUP             41          //
#define SYS_DUP2            62          //

#define SYS_LINK            9           //
#define SYS_UNLINK          10          //
#define SYS_CHDIR           12          //
#define SYS_TIME            13          //
#define SYS_MKNOD           14          //
#define SYS_CHMOD           15          //
#define SYS_CHOWN           16          //
#define SYS_BREAK           17          //
#define SYS_STAT            18          //
#define SYS_GETPID          20          //
#define SYS_MOUNT           21          //
#define SYS_UMOUNT          22          //
#define SYS_SETUID          23          //
#define SYS_GETUID          24          //
#define SYS_STIME           25          //
#define SYS_PTRACE          26          //
#define SYS_ALARM           27          //
#define SYS_FSTAT           28          //
#define SYS_PAUSE           29          //
#define SYS_UTIME           30          //
#define SYS_STTY            31          //
#define SYS_GTTY            32          //
#define SYS_ACCESS          33          //
#define SYS_NICE            34          //
#define SYS_FTIME           35          //
#define SYS_SYNC            36          //
#define SYS_PIPE            42          //
#define SYS_TIMES           43          //
#define SYS_PROF            44          //
#define SYS_BRK             45          //
#define SYS_SETGID          46          //
#define SYS_GETGID          47          //
#define SYS_SIG             48          //
#define SYS_GETEUID         49          //
#define SYS_GETEGID         50          //
#define SYS_ACCT            51          //
#define SYS_PHYS            52          //
#define SYS_LOCK            53          //
#define SYS_FCNTL           55          //
#define SYS_MPX             56          //
#define SYS_SETPGID         57          //
#define SYS_ULIMIT          58          //
#define SYS_UNAME           59          //
#define SYS_UMASK           60          //
#define SYS_CHROOT          61          //
#define SYS_GETPPID         63          //
#define SYS_GETPGRP         64          //




#define NUM_SYSCALLS        65          // Number of system calls


#define SYS_NULL            0           // (unused)
#define SYS_EXIT            1           // Exit a process
#define SYS_FORK            2           // Fork a process
#define SYS_READ            3           // Read from a file
#define SYS_WRITE           4           // Write to a file
#define SYS_OPEN            5           // Open a file
#define SYS_CLOSE           6           // Close a file
#define SYS_WAITPID         7           // Wait for a process
#define SYS_CREAT           8           // Create a new file or rewrite old one
#define SYS_LINK            9           //
#define SYS_UNLINK          10          //
#define SYS_EXEC            11          // Execute a new process
#define SYS_CHDIR           12          //
#define SYS_TIME            13          //
#define SYS_MKNOD           14          //
#define SYS_CHMOD           15          //
#define SYS_CHOWN           16          //
#define SYS_BREAK           17          //
#define SYS_STAT            18          //
#define SYS_LSEEK           19          // Repositions read/write offset
#define SYS_GETPID          20          //
#define SYS_MOUNT           21          //
#define SYS_UMOUNT          22          //
#define SYS_SETUID          23          //
#define SYS_GETUID          24          //
#define SYS_STIME           25          //
#define SYS_PTRACE          26          //
#define SYS_ALARM           27          //
#define SYS_FSTAT           28          //
#define SYS_PAUSE           29          //
#define SYS_UTIME           30          //
#define SYS_STTY            31          //
#define SYS_GTTY            32          //
#define SYS_ACCESS          33          //
#define SYS_NICE            34          //
#define SYS_FTIME           35          //
#define SYS_SYNC            36          //
#define SYS_KILL            37          //
#define SYS_RENAME          38          //
#define SYS_MKDIR           39          //
#define SYS_RMDIR           40          //
#define SYS_DUP             41          //
#define SYS_PIPE            42          //
#define SYS_TIMES           43          //
#define SYS_PROF            44          //
#define SYS_BRK             45          //
#define SYS_SETGID          46          //
#define SYS_GETGID          47          //
#define SYS_SIG             48          //
#define SYS_GETEUID         49          //
#define SYS_GETEGID         50          //
#define SYS_ACCT            51          //
#define SYS_PHYS            52          //
#define SYS_LOCK            53          //
#define SYS_IOCTL           54          // I/O Control interface
#define SYS_FCNTL           55          //
#define SYS_MPX             56          //
#define SYS_SETPGID         57          //
#define SYS_ULIMIT          58          //
#define SYS_UNAME           59          //
#define SYS_UMASK           60          //
#define SYS_CHROOT          61          //
#define SYS_DUP2            62          //
#define SYS_GETPPID         63          //
#define SYS_GETPGRP         64          //

#endif

/******************************************************************************
*   Functions
******************************************************************************/

#define SYS_CALL3(num,a,b,c)        sys_call(num,a,b,c)
#define SYS_CALL2(num,a,b)          sys_call(num,a,b)
#define SYS_CALL1(num,a)            sys_call(num,a)
#define SYS_CALL0(num)              sys_call(num)



#endif // _SYSCALL_H_
