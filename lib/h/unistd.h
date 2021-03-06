/******************************************************************************
*                                                                             *
*   Module:     Unistd.h                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       09/02/96
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the UNIX standard functions

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/8/97    Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _UNISTD_H_
#define _UNISTD_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include <sys/types.h>                  // Include basic types

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

//----------------------------------------------------------------------------
// Names used with sysconf() function
//----------------------------------------------------------------------------

#define _SC_ARG_MAX             0
#define _SC_CHILD_MAX           1
#define _SC_CLK_TCK             2
#define _SC_NGROUPS_MAX         3
#define _SC_STREAM_MAX          4
#define _SC_TZNAME_MAX          5
#define _SC_OPEN_MAX            6
#define _SC_JOB_CONTROL         7
#define _SC_SAVED_IDS           8
#define _SC_VERSION             9


//----------------------------------------------------------------------------
// Names used with pathconf() function
//----------------------------------------------------------------------------

#define _PC_LINK_MAX            0
#define _PC_MAX_CANON           1
#define _PC_MAX_INPUT           2
#define _PC_NAME_MAX            3
#define _PC_PATH_MAX            4
#define _PC_PIPE_BUF            5
#define _PC_CHOWN_RESTRICTED    6
#define _PC_NO_TRUNC            7
#define _PC_VDISABLE            8


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

//extern void  _exit( int status );
extern int   access( const char *pathname, int mode );
//extern long  alarm( long seconds );
//extern int   chdir( const char *path );
//extern char *cuserid( char *s );
//extern int   dup2( int oldfd );
//extern int   dup( int oldfd );
extern int   execl( const char *path, const char *arg, ... );
extern int   execle( const char *path, const char *arg, ... );
extern int   execlp( const char *file, const char *arg, ... );
extern int   execv( const char *path, char *const argv[] );
extern int   execve( const char *path, char *const argv[], char *const *envp );
extern int   execvp( const char *file, char *const argv[] );
extern pid_t fork( void );
extern long  fpathconf( int filedes, int name );
//extern char *getcwd( char *buf, size_t size );
extern gid_t getegid( void );
extern uid_t geteuid( void );
extern uid_t getuid( void );
extern gid_t getgid( void );
extern pid_t getpgrp( void );
extern pid_t getpid( void );
extern pid_t getppid( void );
//extern int   getgroups( int gidsetsize, gid_t grouplist[] );
//extern char *getlogin( void );
//extern int   isatty( int filedes );
//extern int   link( const char *existing, const char *new );
//extern off_t lseek( int filedes, off_t offset, int whence );
extern long   pathconf( const char *path, int name );
//extern int   pause( void );
//extern int   pipe( int filedes[2] );
extern int   read( int filedes, void *buf, unsigned int nbyte );
//extern int   rmdir( const char *path );
//extern int   setgid( gid_t gid );
//extern int   setpgid( pid_t pid, pid_t pgid );
//extern pid_t setsid( void );
//extern int   setuid( uid_t uid );
//extern unsigned int sleep( unsigned int seconds );
extern long  sysconf( int name );
//extern pid_t tcgetpgrp( int filedes );
//extern int   tcsetpgrp( int filedes, pid_t pgrpid );
//extern char *ttyname( int filedes );
//extern int   unlink( const char *path );
//extern int   write( int filedes, const void *buf, unsigned int nbyte );


#endif //  _UNISTD_H_
