###############################################################################
#                                                                             #
#       makefile of C Library for YAOS - Yet Another Operating System         #
#                                                                             #
#                           (c) Goran Devic                                   #
#                                                                             #
#                                                                             #
# Use: wmake clibs | wmake clean | wmake all                                  #
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake clibs    - Makes libraries.  Must use this call.                     #
#                                                                             #
#  wmake clean    - cleans all the objects (forces complete                   #
#                   compilation)                                              #
#                                                                             #
#  wmake all      - cleans all the objects                                    #
#                   and then makes all anew                                   #
#                                                                             #
###############################################################################

SRC              = Source\
INC              = H\
OBJ              = Objects\

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

WCC_OPTIONS      = -4s -s -W2 -d2 -zp4 -zl -fpi87
WASM_OPTIONS     =
WLIB_OPTIONS     = -b -c -l
WCC_ROUTE        = -Fo=$^*.obj -i=$(INC)

###############################################################################
#                                                                             #
# List of dependencies                                                        #
#                                                                             #
###############################################################################

$(LIB1)             :   $(OBJ)ctype.obj         &
                        $(OBJ)printf.obj        &
                        $(OBJ)string.obj        &

                    wlib $(WLIB_OPTIONS) $(LIB1) -+$?


$(LIB2)             :   $(OBJ)startup.obj       &

                    wlib $(WLIB_OPTIONS) $(LIB2) -+$?

###############################################################################

$(OBJ)ctype.obj     :   $(SRC)ctype.c           &
                        $(INC)ctype.h           &

                    wcc386 $(WCC_ROUTE) $(SRC)ctype $(WCC_OPTIONS)


$(OBJ)printf.obj    :   $(SRC)printf.c          &
                        $(INC)ctype.h           &
                        $(INC)stdio.h           &
                        $(INC)stdlib.h          &
                        $(INC)stdarg.h          &

                    wcc386 $(WCC_ROUTE) $(SRC)printf $(WCC_OPTIONS)


$(OBJ)string.obj    :   $(SRC)string.c          &
                        $(INC)string.h          &


                    wcc386 $(WCC_ROUTE) $(SRC)string $(WCC_OPTIONS)


###############################################################################

$(OBJ)startup.obj   :   $(SRC)startup.asm       &

                    wasm $(WCC_ROUTE) $(SRC)startup.asm $(WASM_OPTIONS)


###############################################################################
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake clibs    - Makes libraries.  Must use this call.                     #
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

clean               :   .SYMBOLIC
                    @del $(LIB1)
                    @del $(LIB2)
                    @del $(OBJ)*.obj
                    @del $(OBJ)*.lst
                    @del $(SRC)*.obj
                    @del $(SRC)*.exe
                    @del $(SRC)*.err
                    @del *.err
                    @del *.lst
                    @del *.map
                    @del err

all                 :   .SYMBOLIC
                    @del $(LIBRARY)
                    wmake clean
                    wmake clibs
