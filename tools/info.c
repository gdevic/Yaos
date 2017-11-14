/******************************************************************************
*                                                                             *
*   Module:     Info.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       11/14/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************
.--=
    Module Description:

        This module contains the code for the collection of the information
        from the source files and making a sort of primitive documentation
        file.

    Command line arguments are:
        -f <dir/filter> use specific directory and file filter. If not
                    specified use current directory and filter `*.c'

        -q    Quiet, don't tell progress

        -l?   By default print 79-wide lines before each file name using
                the character '?'

        -h    Print help

    Text formatting codes:

        ".-" (minus-dot) starts a comment that should be collected
        "-." (dot-minus) ends a comment that should be collected

        The format codes must follow the start comment code ".-":
        "-?" Prints a 79-character wide line before the file name using
            the character `?'

-.
*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     DESCRIPTION OF CHANGES                               AUTHOR      *
* --------   ---------------------------------------------------  ----------- *
* 11/14/97   Original                                             Goran Devic *
* --------   ---------------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <ctype.h>

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/


char *sFileSpec = ".\\*.c";             // Default path/file specification

FILE *fp;                               // Input file
DIR *pDir;                              // Directory stream
struct dirent *pdirent;                 // Directory entry

int fLoud = 1;                          // Quiet = 0
int Extract = 0;                        // Number of files that were extracted
char cForceLine = '\0';                 // Force line opt from the command line

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define FORMAT_LINE         '-'         // Format line character code


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   int Process( char *sDir, char *sName )                                    *
*                                                                             *
*******************************************************************************
*
*   Process a single file
*
*   Where:
#       sDir is the directory that a file is in - if NULL, current directory
*       sName is the path/name of a file to be processed
*
*   Returns:
*       1 on success
#       0 on failure
*
******************************************************************************/
int Process( char *sDir, char *sName )
{
    int ch1, ch2;
    int fCopy = 0;
    int fDirty = 0;
    int fInString = 0;
    char cLine;
    int i;
    char sPathName[80];

    // If a line is forced, set the character, if not, set it to '\0'

    cLine = cForceLine;

    // Form the path name if the directory exists

    strcpy(sPathName, sDir);

    if( sDir != NULL && sDir[0] != '\0')
        strcat(sPathName, "/");

    strcat(sPathName, sName);

    if( fLoud )
        fprintf(stderr, "\t(%s)\n", sPathName );

    // Open a file for reading

    if( (fp = fopen(sPathName, "r"))!=NULL)
    {
        // Look for the comment to extract: starting with `. -' (no space)
        // and ending with '-.'

        ch1 = getc(fp);

        while( !feof(fp) )
        {
            ch2 = getc(fp);

            // Double quote character puts us in the string mode and we ignore
            // comments until the next quote character or a new line

            if( ch2=='\n' ) fInString = 0;
            else
            if( ch2=='"' )  fInString ^= 1;

            // We need to look ahead for the terminating pair if we are not in
            // the middle of some string

            if( ch2=='-' && !fInString )
            {
                if( (ch2=getc(fp))=='.' )
                {
                    fCopy = 0;

                    // Send a newline after the comment end code

                    putc('\n', stdout);
                }
                else
                {
                    ungetc(ch2, fp);
                    ch2 = '-';
                }
            }

            if( fCopy )
            {
                // Here we are... Need to put a comment, but first check if the
                // file header had been written so we dont do header for files
                // that are `empty' of comments

                if( fDirty==0 )
                {
                    // See if we need to put format line

                    if( cLine != '\0' )
                    {
                        putc('\n', stdout);
                        for( i=0; i<79; i++)  putc(cLine, stdout);
                        putc('\n', stdout);
                    }

                    printf("Module: %s\n\n", sName );
                    fDirty = 1;
                }

                putc(ch2, stdout);
            }

            if( !fInString )
            {
                if( ch1=='.' && ch2=='-' )
                {
                    fCopy = 1;

                    // Check for the format codes

                    if( (ch2 = getc(fp))==FORMAT_LINE)
                    {
                        cLine = getc(fp);
                        ch2 = getc(fp);
                    }
                }
            }

            ch1 = ch2;
        }

        fclose(fp);
    }
    else
    {
        perror("Unable to open file\n");

        return( 0 );
    }

    // Increase the extracted number if we had any comment in this file

    if( fDirty )  Extract++;

    return( 1 );
}



/******************************************************************************
*                                                                             *
*   int main( int argn, char *argp[] )                                        *
*                                                                             *
*******************************************************************************
*
*   This is the main function.
*
*   Where:
*       argn is the number of arguments supplied
#       argp is an array of pointers to argument strings
*
*   Returns:
*       0 on success
#       error code on failure
*
******************************************************************************/
int main( int argn, char *argp[] )
{
    int files=0;
    int processed=0;
    char sDir[64];
    char *pSlash;


    // Get the optional command line arguments

    while( --argn )
    {
        // Get new path/file specification

        if( *argp[argn]=='-' && tolower(*(argp[argn]+1))=='f' )
            sFileSpec = argp[argn + 1];
        else

        // Get the print progress disable switch

        if( *argp[argn]=='-' && tolower(*(argp[argn]+1))=='q' )
            fLoud = 0;
        else

        // Get the force line option

        if( *argp[argn]=='-' && tolower(*(argp[argn]+1))=='l' && isascii(*(argp[argn] + 2)) )
            cForceLine = *(argp[argn] + 2);
        else

        // Get the help info switch

        if( *argp[argn]=='-' )
        {
            fprintf(stderr, "\nCollects info from the source files.\n\n");
            fprintf(stderr, " -f <dir/name/pattern> File(s) specifier\n");
            fprintf(stderr, " -q                    Quiet operation\n");
            fprintf(stderr, " -l?                   Force line header\n");
            fprintf(stderr, " -h                    This help\n");

            return( 0 );
        }
    }

    // Find out the directory portion of the file specification

    strcpy(sDir, sFileSpec);
    pSlash = sDir + strlen(sDir);

    while( pSlash > sDir )
        if( *--pSlash=='/' || *pSlash=='\\' )
            break;

    *pSlash = '\0';


    // Open the directory stream

    if( (pDir = opendir(sFileSpec))!=NULL )
    {
        for( ; ; )
        {
            // Read a directory entry

            if( (pdirent = readdir(pDir))!=NULL )
            {
                // Avoid `.' and `..' entries

                if( pdirent->d_name[0]=='.' )  continue;

                ++files;

                if( fLoud )
                    fprintf(stderr, " %2d. %-15s  ", files, pdirent->d_name );

                // Process the file

                processed += Process(sDir, pdirent->d_name);
            }
            else
                break;
        }

        closedir(pDir);
    }
    else
    {
        perror(sFileSpec);

        return( errno );
    }

    if( fLoud )
        fprintf(stderr, "Total %d files, processed %d files, %d files with actual text.\n",
            files, processed, Extract );

    return( 0 );
}


