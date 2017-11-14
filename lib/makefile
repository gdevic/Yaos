###############################################################################
#                                                                             #
#       makefile for YAOS C Library - Yet Another Operating System            #
#                                                                             #
#                       (c) 1996, 1997 Goran Devic                            #
#                                                                             #
#                                                                             #
# Use: wmake | wmake clean                                                    #
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake          - Makes libraries                                           #
#                                                                             #
#  wmake clean    - cleans all the objects                                    #
#                                                                             #
###############################################################################

SRC              = Source\
INC              = H\
OBJ              = Objects\

CC               = wcc386
ASM              = masm
LINK             = wlink
LIBR             = wlib

LIB1             = clib.lib
LIB2             = cstart.lib


###############################################################################
#                                                                             #
# Compiler directives                                                         #
#                                                                             #
###############################################################################

# WCC_OPTIONS:
#  ~ Generate 486 optimum code with stack based params passing (-4s)
#  ~ Do not check stack overflow (-s)
#  ~ Warning level 2 (-W2)
#  ~ Include debug data & *do not optimize* (-d2)
#  ~ Double word align data (-zp4)
#  ~ Relax some checking (page 74 of Watcom User Guide) (-ze)
#  ~ Do not place library names in object code (-zl)
#  ~ Use Intel fp unit
#

CC_OPTIONS      = -4s -s -W2 -d2 -zp4 -zl -fpi87
ASM_OPTIONS     = /DMASM6 /c /DIS_32
LIB_OPTIONS     = -b -c -l
LINK_OPTIONS    =  form phar option map
CC_ROUTE        = -Fo=$^*.obj -i=$(INC)

###############################################################################
#                                                                             #
# List of Object Files                                                        #
#                                                                             #
###############################################################################

###############################################################################
#                                                                             #
# List of dependencies                                                        #
#                                                                             #
###############################################################################

$(LIB1)             :   $(OBJ)_Conf.obj         &
                        $(OBJ)_Convert.obj      &
                        $(OBJ)_Ctype.obj        &
                        $(OBJ)_Dir.obj          &
                        $(OBJ)_Exec.obj         &
                        $(OBJ)_FD.obj           &
                        $(OBJ)_Filbuf.obj       &
                        $(OBJ)_Flsbuf.obj       &
                        $(OBJ)_Fflush.obj       &
                        $(OBJ)_Grp.obj          &
                        $(OBJ)_Id.obj           &
                        $(OBJ)_Malloc.obj       &
                        $(OBJ)_Printf.obj       &
                        $(OBJ)_Pwd.obj          &
                        $(OBJ)_Scan.obj         &
                        $(OBJ)_Stats.obj        &
                        $(OBJ)_Stdio.obj        &
                        $(OBJ)_Stdlib.obj       &
                        $(OBJ)_String.obj       &
                        $(OBJ)_SysCall.obj      &
                        $(OBJ)_Termios.obj      &
                        $(OBJ)_Time.obj         &
                        $(OBJ)_Unistd.obj       &
                        $(OBJ)_Wait.obj         &

                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Conf.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Convert.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Ctype.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Dir.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Exec.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Fd.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Filbuf.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Flsbuf.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Fflush.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Grp.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Id.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Malloc.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Printf.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Pwd.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Scan.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Stats.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Stdio.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Stdlib.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_String.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_SysCall.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Termios.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Time.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Unistd.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB1) -+$(OBJ)_Wait.obj


$(LIB2)             :   $(OBJ)_Setup.obj        &
                        $(OBJ)_Startup.obj      &

                    $(LIBR) $(LIB_OPTIONS) $(LIB2) -+$(OBJ)_Setup.obj
                    $(LIBR) $(LIB_OPTIONS) $(LIB2) -+$(OBJ)_Startup.obj

###############################################################################

$(OBJ)_Conf.obj     :   $(SRC)_Conf.c           &
                        $(INC)unistd.h          &
                        $(INC)sys\syscall.h     &
                        $(INC)errno.h           &
                        $(INC)limits.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Conf       $(CC_OPTIONS)

$(OBJ)_Convert.obj  :   $(SRC)_Convert.c        &
                        $(INC)ctype.h           &
                        $(INC)string.h          &
                        $(INC)errno.h           &
                        $(INC)limits.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Convert    $(CC_OPTIONS)

$(OBJ)_Ctype.obj    :   $(SRC)_Ctype.c          &
                        $(INC)ctype.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Ctype      $(CC_OPTIONS)

$(OBJ)_Dir.obj      :   $(SRC)_Dir.c            &
                        $(INC)sys\clib.h        &
                        $(INC)sys\syscall.h     &
                        $(INC)dirent.h          &
                        $(INC)fcntl.h           &
                        $(INC)string.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Dir        $(CC_OPTIONS)

$(OBJ)_Exec.obj     :   $(SRC)_Exec.c           &
                        $(INC)sys\syscall.h     &
                        $(INC)errno.h           &
                        $(INC)unistd.h          &
                        $(INC)stdarg.h          &
                        $(INC)stdio.h           &
                        $(INC)stdlib.h          &
                        $(INC)string.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Exec       $(CC_OPTIONS)

$(OBJ)_Fd.obj       :   $(SRC)_Fd.c             &
                        $(INC)sys\clib.h        &
                        $(INC)string.h          &
                        $(INC)fcntl.h           &
                        $(INC)stdarg.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Fd         $(CC_OPTIONS)

$(OBJ)_Filbuf.obj   :   $(SRC)_Filbuf.c         &
                        $(INC)sys\clib.h        &
                        $(INC)fcntl.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Filbuf     $(CC_OPTIONS)

$(OBJ)_Flsbuf.obj   :   $(SRC)_Flsbuf.c         &
                        $(INC)sys\clib.h        &
                        $(INC)fcntl.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Flsbuf     $(CC_OPTIONS)

$(OBJ)_Fflush.obj   :   $(SRC)_Fflush.c         &
                        $(INC)sys\clib.h        &
                        $(INC)fcntl.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Fflush     $(CC_OPTIONS)

$(OBJ)_Grp.obj      :   $(SRC)_Grp.c            &
                        $(INC)sys\clib.h        &
                        $(INC)grp.h             &
                        $(INC)stdio.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Grp        $(CC_OPTIONS)

$(OBJ)_Id.obj       :   $(SRC)_Id.c             &
                        $(INC)sys\clib.h        &
                        $(INC)unistd.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Id         $(CC_OPTIONS)

$(OBJ)_Malloc.obj   :   $(SRC)_Malloc.c         &
                        $(INC)sys\clib.h        &
                        $(INC)stdlib.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Malloc     $(CC_OPTIONS)

$(OBJ)_Printf.obj   :   $(SRC)_Printf.c         &
                        $(INC)ctype.h           &
                        $(INC)stdarg.h          &
                        $(INC)stdlib.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Printf     $(CC_OPTIONS)

$(OBJ)_Pwd.obj      :   $(SRC)_Pwd.c            &
                        $(INC)pwd.h             &
                        $(INC)stdio.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Pwd        $(CC_OPTIONS)

$(OBJ)_Scan.obj     :   $(SRC)_Scan.c           &
                        $(INC)ctype.h           &
                        $(INC)stdio.h           &
                        $(INC)stdarg.h          &
                        $(INC)string.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Scan       $(CC_OPTIONS)

$(OBJ)_Stats.obj    :   $(SRC)_Stats.c          &
                        $(INC)sys\clib.h        &
                        $(INC)sys\stat.h        &

                    $(CC) $(CC_ROUTE) $(SRC)_Stats      $(CC_OPTIONS)

$(OBJ)_Stdio.obj    :   $(SRC)_Stdio.c          &
                        $(INC)sys\clib.h        &
                        $(INC)stdio.h           &
                        $(INC)fcntl.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Stdio      $(CC_OPTIONS)

$(OBJ)_Stdlib.obj   :   $(SRC)_Stdlib.c         &
                        $(INC)sys\clib.h        &
                        $(INC)sys\syscall.h     &
                        $(INC)stdlib.h          &
                        $(INC)string.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Stdlib     $(CC_OPTIONS)

$(OBJ)_String.obj   :   $(SRC)_String.c         &
                        $(INC)string.h          &
                        $(INC)ctype.h           &
                        $(INC)stdio.h           &
                        $(INC)errno.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_String     $(CC_OPTIONS)

$(OBJ)_SysCall.obj  :   $(SRC)_SysCall.asm      &

                    $(ASM) $(SRC)_SysCall.asm $(OBJ)_SysCall.obj  $(ASM_OPTIONS)

$(OBJ)_Termios.obj  :   $(SRC)_Termios.c        &
                        $(INC)sys\clib.h        &
                        $(INC)sys\syscall.h     &
                        $(INC)sys\ioctl.h       &
                        $(INC)termios.h         &

                    $(CC) $(CC_ROUTE) $(SRC)_Termios    $(CC_OPTIONS)

$(OBJ)_Time.obj     :   $(SRC)_Time.c           &
                        $(INC)sys\clib.h        &
                        $(INC)time.h            &

                    $(CC) $(CC_ROUTE) $(SRC)_Time       $(CC_OPTIONS)

$(OBJ)_Unistd.obj   :   $(SRC)_Unistd.c         &
                        $(INC)sys\syscall.h     &
                        $(INC)unistd.h          &

                    $(CC) $(CC_ROUTE) $(SRC)_Unistd     $(CC_OPTIONS)

$(OBJ)_Wait.obj     :   $(SRC)_Wait.c           &
                        $(INC)sys\syscall.h     &
                        $(INC)sys\wait.h        &

                    $(CC) $(CC_ROUTE) $(SRC)_Wait       $(CC_OPTIONS)

###############################################################################

$(OBJ)_Setup.obj    :   $(SRC)_Setup.c          &
                        $(INC)sys\clib.h        &
                        $(INC)stdlib.h          &
                        $(INC)errno.h           &

                    $(CC) $(CC_ROUTE) $(SRC)_Setup.c    $(CC_OPTIONS)

$(OBJ)_Startup.obj  :   $(SRC)_Startup.asm      &

                    $(ASM) $(SRC)_Startup.asm $(OBJ)_Startup.obj  $(ASM_OPTIONS)

###############################################################################
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake clean    - cleans all the objects (forces complete                   #
#                   compilation)                                              #
#                                                                             #
#  wmake all      - cleans all the objects                                    #
#                   and then makes all anew                                   #
#                                                                             #
###############################################################################

clibs               :   .SYMBOLIC
                    wmake $(LIB1)
                    wmake $(LIB2)
                    dir *.err

clean               :   .SYMBOLIC
                    @del $(OBJ)*.obj
                    @del $(OBJ)*.lst
                    @del $(SRC)*.obj
                    @del $(SRC)*.exe
                    @del $(SRC)*.err
                    @del *.err
                    @del *.sbr
                    @del *.lnk
                    @del *.lst
                    @del *.map
                    @del err

