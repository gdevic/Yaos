/******************************************************************************
*                                                                             *
*   Module:     Errno.h                                                       *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       10/12/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file defining all of the error codes

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 10/12/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _ERRNO_H_
#define _ERRNO_H_


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

#define E2BIG        (-1)   // Arguments and environment is larger than ARG_MAX
#define EACCESS      (-2)   // Search file permission is denied
#define EAGAIN       (-3)   // O_NONBLOCK flag is set; not enough resources
#define EBADF        (-4)   // Invalid file descriptor
#define EBUSY        (-5)   // The directory is in use
#define ECHILD       (-6)   // There are no children
#define EDEADLK      (-7)   // An fcntl would cause a deadlock
#define EDOM         (-8)   // Input argument is outside math domain
#define EEXIST       (-9)   // The name file already exists
#define EFAULT       (-10)  // Invalid address in an arg to a function call
#define EFBIG        (-11)  // File exceeds maximum file size
#define EINTR        (-12)  // Function was interrupted by a signal
#define EINVAL       (-13)  // Invalid argument
#define EIO          (-14)  // Input or output error
#define EISDIR       (-15)  // A name is a directory
#define EMFILE       (-16)  // Too many file descriptors used by a process
#define EMLINK       (-17)  // The number of links exceeds LINK_MAX
#define ENAMETOOLONG (-18)  // Length of a filename exceeds PATH_MAX
#define ENFILE       (-19)  // Too many files are open in the system
#define ENODEV       (-20)  // No such device
#define ENOENT       (-21)  // A file or directory does not exist
#define ENOEXEC      (-22)  // File is not an executable
#define ENOLCK       (-23)  // No locks available
#define ENOMEM       (-24)  // No memory available
#define ENOSPC       (-25)  // No space left on disk
#define ENOSYS       (-26)  // Function not implemented
#define ENOTDIR      (-27)  // A component of a path is not a directory
#define ENOTEMPTY    (-28)  // Cant delete or rename a non-empty directory
#define ENOTTY       (-29)  // A file is not a terminal
#define ENXIO        (-30)  // No such device
#define EPERM        (-31)  // Operation is not permitted
#define EPIPE        (-32)  // Attempt to write to a pipe with no reader
#define ERANGE       (-33)  // Result is too large
#define EROFS        (-34)  // Read-only file system
#define ESPIPE       (-35)  // An lseek was issued on a pipe or fifo
#define ESRCH        (-36)  // No such process
#define EXDEV        (-37)  // Attempt to link a file to another file system

#define MAX_ERRNO    37     // Number of error codes

extern int errno;


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/


#endif //  _ERRNO_H_
