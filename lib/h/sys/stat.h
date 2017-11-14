/******************************************************************************
*                                                                             *
*   Module:     Stat.h                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/7/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the file stat()

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/7/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _STAT_H_
#define _STAT_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <sys/types.h>                  // Include data types header file

#include <stdio.h>                      // Include standard header file

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

typedef unsigned long time_t;


struct stat
{
    dev_t   st_dev;             // Major/minor device number
    ino_t   st_ino;             // i-node number
    mode_t  st_mode;            // file mode, protection bits etc.
    nlink_t st_nlink;           // number of links
    uid_t   st_uid;             // uid of the file's owner
    gid_t   st_gid;             // gid of the file's owner
    off_t   st_size;            // file size

    // Minix types

    dev_t   st_rdev;
    long    st_blocks;

    // Time types

    time_t  st_atime;           // file last access time
    time_t  st_mtime;           // file last modification time
    time_t  st_ctime;           // file last status change time

    // DOS support section

    int     st_fd;              // DOS file handle
    int     st_attr;            // DOS file attributes
    char    st_name[PATHNAME_MAX]; // DOS file `drive:/path/name'
    void   *st_dir;             // Pointer to directory information
};


#define S_IFMT          0170000         // Type of file
#define S_IFREG         0100000         // Regular
#define S_IFBLK         0060000         // Block special file
#define S_IFDIR         0040000         // Directory
#define S_IFCHR         0020000         // Character special file
#define S_IFIFO         0010000         // File is a FIFO
#define S_ISGID         0002000         // Set group ID on execution
#define S_ISVTX         0001000         // Sticky bit


//----------------------------------------------------------------------------
// File mode bits
//----------------------------------------------------------------------------

// Owner permissions

#define S_IRWXU           0700          // Owner:  rwx------
#define S_IRUSR           0400          // Owner:  r--------
#define S_IWUSR           0200          // Owner:  -w-------
#define S_IXUSR           0100          // Owner:  --x------

// Group permissions

#define S_IRWXG           0070          // Group:  ---rwx---
#define S_IRGRP           0040          // Group:  ---r-----
#define S_IWGRP           0020          // Group:  ----w----
#define S_IXGRP           0010          // Group:  -----x---

// Other's permissions

#define S_IRWXO           0007          // Others: ------rwx
#define S_IROTH           0004          // Others: ------r--
#define S_IWOTH           0002          // Others: -------w-
#define S_IXOTH           0001          // Others: --------x


// Testing macros

#define S_ISREG(m)  (((m) & S_IFMT)==S_IFREG)   // Is it a regular file
#define S_ISDIR(m)  (((m) & S_IFMT)==S_IFDIR)   // Is it a directory
#define S_ISCHR(m)  (((m) & S_IFMT)==S_IFCHR)   // Is it a char device
#define S_ISBLK(m)  (((m) & S_IFMT)==S_IFBLK)   // Is it a block device
#define S_ISFIFO(m) (((m) & S_IFMT)==S_IFIFO)   // Is it a FIFO


#define S_ISUID             0

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int fstat( int fd, struct stat *buf );
extern int stat( const char *path, struct stat *buf );


#endif //  _STAT_H_

