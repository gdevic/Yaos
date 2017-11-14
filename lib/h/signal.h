/******************************************************************************
*                                                                             *
*   Module:     signal.h                                                      *
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
#ifndef _SIGNAL_H_
#define _SIGNAL_H_


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

#define _NSIG                   22      // Number of signals used

#define SIGHUP                  1       // Hangup
#define SIGINT                  2       // Interrupt
#define SIGQUIT                 3       // Quit
#define SIGILL                  4       // Illegal instruction
#define SIGTRAP                 5       // Trace trap
#define SIGABRT                 6       // IOT instruction
#define SIGIOT                  6       // PDP-11 Abort
#define SIGUNUSED               7       // Unused code
#define SIGFPE                  8       // Floating point exception
#define SIGKILL                 9       // Kill (cannot be caught or ignored
#define SIGUSR1                 10      // User defined signal #1
#define SIGSEGV                 11      // Segmentation violation
#define SIGUSR2                 12      // User defined signal #2
#define SIGPIPE                 13      // Write on a pipe with no reader
#define SIGALRM                 14      // Alarm clock
#define SIGTERM                 15      // Software termination from kill
#define SIGUNUSED2              16      // Unused code
#define SIGCHLD                 17      // Child process terminated or stopped
#define SIGCONT                 18      // Continue if stopped
#define SIGSTOP                 19      // Stop signal
#define SIGTSTP                 20      // Interactive stop signal
#define SIGTTIN                 21      // Bg process wants to read
#define SIGTTOU                 22      // Bg process wants to write


typedef unsigned int sigset_t;

typedef void (*__sighandler_t)(int);


#define SIG_ERR     ((__sighandler_t) -1 )      // Error return
#define SIG_DFL     ((__sighandler_t) 0 )       // Default signal handler
#define SIG_IGN     ((__sighandler_t) 1 )       // Ignore signal
#define SIG_HOLD    ((__sighandler_t) 2 )       // Block signal
#define SIG_CATCH   ((__sighandler_t) 3 )       // Catch signal


struct sigaction
{
    __sighandler_t sa_handler;          // SIG_DFL, SIG_IGN or ptr to function
    sigset_t sa_mask;                   // Signals to be blocked during handler
};


// Values for sigprocmask()

#define SIG_BLOCK               0       // For blocking signals
#define SIG_UNBLOCK             1       // For unblocking signals
#define SIG_SETMASK             2       // For setting the signal mask


/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern int raise( int sig );
extern int kill( pid_t pid, int sig );
extern int sigaction( int sig, const struct sigaction *act, struct sigaction *oact);
extern int sigaddset( sigset_t *set, int sig );
extern int sigdelset( sigset_t *set, int sig );
extern int sigemptyset( sigset_t *set );
extern int sigfillset( sigset_t *set );
extern int sigismember( sigset_t *set, int sig );
extern int sigpending( sigset_t *set );
extern int sigprocmask( int how, const sigset_t *set, sigset_t *oset );
extern int sigsuspend( const sigset_t *sigmask );


#endif //  _SIGNAL_H_
