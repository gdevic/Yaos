###############################################################################
#                                                                             #
#            makefile for YAOS - Yet Another Operating System                 #
#                                                                             #
#                      DOS Command Line Loader                                #
#                                                                             #
#                     (c) 1996-1997 Goran Devic                               #
#                                                                             #
#                                                                             #
# Use: wmake | wmake clean | wmake all                                        #
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
#  wmake clean    - cleans all the objects                                    #
#                                                                             #
#  wmake all      - cleans all the objects and remakes the target             #
#                                                                             #
###############################################################################

SRC              = Source\
INC              = ..\Include\
OBJ              = Object\

CC               = wcc
ASM              = masm
LINK             = wlink

TARGET           = LoadOS.exe

###############################################################################
#                                                                             #
# Compiler directives                                                         #
#                                                                             #
###############################################################################
# WCC_OPTIONS:
#  ~ Warning level 3 (-W3)
#  ~ Include debug data (-d2)
#  ~ Compact memory model: small code, big data
#
#  test: embed function names

CC_OPTIONS      = -W3 -d2 -mc -en
ASM_OPTIONS     =
LINK_OPTIONS    = debug all option map name loados.exe
CC_ROUTE        = -Fo=$^*.obj -i=$(INC)

###############################################################################
#                                                                             #
# List of Object Files                                                        #
#                                                                             #
###############################################################################

OBJECT_FILES1    = Loader.obj,LoadOS.obj

###############################################################################
#                                                                             #
# List of dependencies                                                        #
#                                                                             #
###############################################################################

$(TARGET)           :   $(OBJ)Loader.obj        &
                        $(OBJ)LoadOS.obj        &

                    @echo Name $^* Path $(OBJ) > $^*.lnk
                    @echo File >> $^*.lnk
                    @echo $(OBJECT_FILES1) >> $^*.lnk

                    @del $(TARGET)
                    $(LINK) $(LINK_OPTIONS) @$^*

###############################################################################

$(OBJ)Loader.obj    :   $(SRC)Loader.asm        &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &

                    $(ASM) $(SRC)Loader $(OBJ)Loader.obj $(ASM_OPTIONS)

$(OBJ)LoadOS.obj    :   $(SRC)LoadOS.c          &

                    $(CC)  $(CC_ROUTE)  $(SRC)LoadOS     $(CC_OPTIONS)

###############################################################################
#                                                                             #
# Symbolic commands                                                           #
#                                                                             #
###############################################################################

clean               :   .SYMBOLIC
                    @del $(OBJ)*.obj
                    @del $(OBJ)*.lst
                    @del *.err
                    @del *.lst
                    @del *.map
                    @del *.lnk
                    @del err

all                 :   .SYMBOLIC
                    @del $(TARGET)
                    @wmake clean
                    @wmake

