###############################################################################
#                                                                             #
#            makefile for YAOS - Yet Another Operating System                 #
#                                                                             #
#                        (c) 1996 Goran Devic                                 #
#                                                                             #
#                                                                             #
# Use: wmake | wmake clean | wmake all                                        #
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

SRC              = Source\
INC              = Include\
INC_CLIB         = ..\Lib\H\
OBJ              = Objects\

CHECKSUM         = ..\Tools\Build\checksum.exe

KERNEL_FILE_NAME = _KERNEL_

OBJECT_FILES1    = debugger.obj,dis386.obj,display.obj,eval.obj,kernel.obj,kernela.obj,
OBJECT_FILES2    = keyboard.obj,interrpt.obj,mem.obj,mm.obj,printk.obj,sched.obj,
OBJECT_FILES3    = timers.obj,tss.obj,ttys.obj,v86a.obj

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
TEST_OPTIONS     = -4s -s -W2 -od -d2 -fpi87
WASM_OPTIONS     =
WLINK_OPTIONS    = form phar option map library ..\lib\clib.lib
WCC_ROUTE        = -Fo=$^*.obj -i=$(INC) -i=$(INC_CLIB)

###############################################################################
#                                                                             #
# List of dependencies                                                        #
#                                                                             #
###############################################################################

test.exe           : $(KERNEL_FILE_NAME)  test.obj

test.obj           : test.c

                   wcl386 $(WCC_ROUTE) test.c $(TEST_OPTIONS) /"library ..\lib\clib.lib" 



$(KERNEL_FILE_NAME) :   $(OBJ)debugger.obj      &
                        $(OBJ)dis386.obj        &
                        $(OBJ)display.obj       &
                        $(OBJ)eval.obj          &
                        $(OBJ)kernel.obj        &
                        $(OBJ)kernela.obj       &
                        $(OBJ)keyboard.obj      &
                        $(OBJ)interrpt.obj      &
                        $(OBJ)mem.obj           &
                        $(OBJ)mm.obj            &
                        $(OBJ)printk.obj        &
                        $(OBJ)timers.obj        &
                        $(OBJ)sched.obj         &
                        $(OBJ)tss.obj           &
                        $(OBJ)ttys.obj          &
                        $(OBJ)v86a.obj

                    echo Name $^* Path $(OBJ) > $^*.lnk
                    echo File >> $^*.lnk
                    echo $(OBJECT_FILES1) >> $^*.lnk
                    echo $(OBJECT_FILES2) >> $^*.lnk
                    echo $(OBJECT_FILES3) >> $^*.lnk

                    @del $(KERNEL_FILE_NAME)
                    wlink $(WLINK_OPTIONS) @$^*
                    @del $^*.lnk
                    $(CHECKSUM) $(KERNEL_FILE_NAME).EXP $(KERNEL_FILE_NAME)
                    @del $(KERNEL_FILE_NAME).EXP

###############################################################################

$(OBJ)debugger.obj  :   $(SRC)debugger.c        &

                    wcc386 $(WCC_ROUTE) $(SRC)debugger $(WCC_OPTIONS)

$(OBJ)dis386.obj    :   $(SRC)dis386.c          &

                    wcc386 $(WCC_ROUTE) $(SRC)dis386   $(WCC_OPTIONS)

$(OBJ)display.obj   :   $(SRC)display.c          &

                    wcc386 $(WCC_ROUTE) $(SRC)display  $(WCC_OPTIONS)

$(OBJ)eval.obj      :   $(SRC)eval.c             &

                    wcc386 $(WCC_ROUTE) $(SRC)eval     $(WCC_OPTIONS)

$(OBJ)kernel.obj    :   $(SRC)kernel.c          &
                        $(INC)kernel.h

                    wcc386 $(WCC_ROUTE) $(SRC)kernel   $(WCC_OPTIONS)

$(OBJ)kernela.obj   :   $(SRC)kernela.asm

                    wasm $(WCC_ROUTE) $(SRC)kernela    $(WASM_OPTIONS)

$(OBJ)keyboard.obj  :   $(SRC)keyboard.c

                    wcc386 $(WCC_ROUTE) $(SRC)keyboard $(WCC_OPTIONS)

$(OBJ)interrpt.obj  :   $(SRC)interrpt.c 

                    wcc386 $(WCC_ROUTE) $(SRC)interrpt $(WCC_OPTIONS)

$(OBJ)mem.obj       :   $(SRC)mem.c    

                    wcc386 $(WCC_ROUTE) $(SRC)mem      $(WCC_OPTIONS)

$(OBJ)mm.obj        :   $(SRC)mm.c              &
                        $(INC)mm.h     

                    wcc386 $(WCC_ROUTE) $(SRC)mm       $(WCC_OPTIONS)

$(OBJ)printk.obj     :   $(SRC)printk.c         &

                    wcc386 $(WCC_ROUTE) $(SRC)printk   $(WCC_OPTIONS)

$(OBJ)sched.obj     :   $(SRC)sched.c           &

                    wcc386 $(WCC_ROUTE) $(SRC)sched    $(WCC_OPTIONS)

$(OBJ)timers.obj    :   $(SRC)timers.c          &

                    wcc386 $(WCC_ROUTE) $(SRC)timers   $(WCC_OPTIONS)

$(OBJ)tss.obj       :   $(SRC)tss.c             &

                    wcc386 $(WCC_ROUTE) $(SRC)tss      $(WCC_OPTIONS)

$(OBJ)ttys.obj      :   $(SRC)ttys.c            &

                    wcc386 $(WCC_ROUTE) $(SRC)ttys     $(WCC_OPTIONS)


$(OBJ)v86a.obj      :   $(SRC)v86a.asm

                    wasm $(WCC_ROUTE) $(SRC)v86a       $(WASM_OPTIONS)


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
                    @del $(OBJ)*.obj
                    @del $(OBJ)*.lst
                    @del $(SRC)*.obj
                    @del $(SRC)*.exe
                    @del $(SRC)*.err
                    @del *.err
                    @del *.lst
                    @del *.map
                    @del ..\Tools\*.obj
                    @del err

all                 :   .SYMBOLIC
                    del $(KERNEL_FILE_NAME)
                    wmake clean
                    wmake

