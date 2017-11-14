###############################################################################
#                                                                             #
#                      makefile for YAOS Binaries                             #
#                                                                             #
#                           (c) Goran Devic                                   #
#                                                                             #
#                                                                             #
# Use: wmake clean | wmake all                                                #
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake all      - cleans all the objects                                    #
#                   and then makes all anew                                   #
#                                                                             #
#  wmake clean    - cleans all the objects (forces complete                   #
#                   compilation)                                              #
#                                                                             #
###############################################################################

SRC              = Source\
INC              = ..\Lib\H\
OBJ              = Objects\


###############################################################################
#                                                                             #
# Compiler directives                                                         #
#                                                                             #
###############################################################################

# WCC_OPTIONS:
#  ~ Generate 486 optimum code with stack based params passing (-4s)
#  ~ Do not check stack overflow (-s)
#  ~ Warning level 2 (-W2)
#  ~ Include debug data (-d2)
#  ~ Do not optimize (-od)
#  ~ Double word align data (-zp4)
#  ~ Relax some checking (page 74 of Watcom User Guide) (-ze)
#  ~ Do not place library names in object code (-zl)
#  ~ Use Intel fp unit
#

WCC_OPTIONS      = -4s -s -W2 -od -d2 -zp4 -zl -fpi87
WASM_OPTIONS     =
WLINK_OPTIONS    = form phar option map lib ..\lib\cstart,..\lib\clib path $(OBJ)
WCC_ROUTE        = -Fo=$(OBJ)$^*.obj -i=$(INC)

###############################################################################
#                                                                             #
# List of dependencies                                                        #
#                                                                             #
###############################################################################

hello.exp          : $(SRC)hello.c

                    wcc386 $(WCC_ROUTE) $(SRC)hello $(WCC_OPTIONS)
                    wlink $(WLINK_OPTIONS) name hello.exp file hello


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

clean               :   .SYMBOLIC
                    @if exist $(OBJ)*.obj       del $(OBJ)*.obj
                    @if exist $(OBJ)*.lst       del $(OBJ)*.lst
                    @if exist *.err             del *.err
                    @if exist *.lst             del *.lst
                    @if exist *.map             del *.map
                    @if exist err               del err

all                 :   .SYMBOLIC
                    @if exist *.exp             del *.exp
                    wmake hello.exp

