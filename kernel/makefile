###############################################################################
#                                                                             #
#            makefile for YAOS - Yet Another Operating System                 #
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
SRC_DEB          = Source\Debugger\
SRC_DEV          = Source\Dev\
INC              = Include\
OBJ              = Object\

CC               = wcc386
ASM              = masm
LINK             = wlink

CLIB             = ..\Lib\CLib.Lib
CINC             = ..\Lib\H\

KERNEL_FILE_NAME = Kernel.exp

###############################################################################
#                                                                             #
# Compiler directives                                                         #
#                                                                             #
###############################################################################
# WCC_OPTIONS:
#  ~ Generate 486 optimum code with stack based params passing (-4s)
#  ~ Do not check stack overflow (-s)
#  ~ Warning level 3 (-W3)
#  ~ Include debug data (-d2)
#  ~ Do not optimize (-od)
#  ~ Double word align data (-zp4)
#  ~ Relax some checking (page 74 of Watcom User Guide) (-ze)
#  ~ Do not place library names in object code (-zl)
#

CC_OPTIONS      = -4s -s -W2 -od -d2 -zp4 -zl
ASM_OPTIONS     = /DMASM6 /c /DIS_32
LINK_OPTIONS    = form phar option map
CC_ROUTE        = -Fo=$^*.obj -i=$(INC) -i=$(CINC)

###############################################################################
#                                                                             #
# List of Object Files                                                        #
#                                                                             #
###############################################################################

OBJECT_KERNEL1   = Init.obj,Inita.obj,Device.obj,DosX.obj,Dpmi.obj,Exec.obj,
OBJECT_KERNEL2   = File.obj,Gdt.obj,Int.obj,Inta.obj,Keyboard.obj,PageMM.obj,PF.obj,
OBJECT_KERNEL3   = Printf.obj,Queue.obj,Sched.obj,TSS.obj,V86.obj,V86a.obj,VM.obj,

OBJECT_SYSCALL1  = SysCall.obj,Path.obj,Exit.obj,Open.obj,Read.obj,Seek.obj,Stats.obj,
OBJECT_DEV       = Write.obj,TTY.obj,DosFS.obj,

OBJECT_DEBUG1    = Break.obj,Command.obj,Debug.obj,Dis386.obj,Display.obj,Dump.obj,
OBJECT_DEBUG2    = Eval.obj,Info.obj,Map.obj,Register.obj,Set.obj,Trap.obj

###############################################################################
#                                                                             #
# List of Dependencies                                                        #
#                                                                             #
###############################################################################

$(KERNEL_FILE_NAME) :                           &
                        $(OBJ)Init.obj          &
                        $(OBJ)Inita.obj         &
                        $(OBJ)Device.obj        &
                        $(OBJ)DosX.obj          &
                        $(OBJ)Dpmi.obj          &
                        $(OBJ)Exec.obj          &
                        $(OBJ)File.obj          &
                        $(OBJ)Gdt.obj           &
                        $(OBJ)Int.obj           &
                        $(OBJ)Inta.obj          &
                        $(OBJ)Keyboard.obj      &
                        $(OBJ)PageMM.obj        &
                        $(OBJ)PF.obj            &
                        $(OBJ)Printf.obj        &
                        $(OBJ)Queue.obj         &
                        $(OBJ)SysCall.obj       &
                        $(OBJ)Sched.obj         &
                        $(OBJ)TSS.obj           &
                        $(OBJ)V86.obj           &
                        $(OBJ)V86a.obj          &
                        $(OBJ)VM.obj            &
                                                &
                        $(OBJ)SysCall.obj       &
                        $(OBJ)Path.obj          &
                        $(OBJ)Exit.obj          &
                        $(OBJ)Open.obj          &
                        $(OBJ)Read.obj          &
                        $(OBJ)Seek.obj          &
                        $(OBJ)Stats.obj         &
                        $(OBJ)Write.obj         &
                                                &
                        $(OBJ)TTY.obj           &
                        $(OBJ)DosFS.obj         &
                                                &
                        $(OBJ)Break.obj         &
                        $(OBJ)Command.obj       &
                        $(OBJ)Debug.obj         &
                        $(OBJ)Dis386.obj        &
                        $(OBJ)Display.obj       &
                        $(OBJ)Dump.obj          &
                        $(OBJ)Eval.obj          &
                        $(OBJ)Info.obj          &
                        $(OBJ)Map.obj           &
                        $(OBJ)Register.obj      &
                        $(OBJ)Set.obj           &
                        $(OBJ)Trap.obj          &

                    echo Name $^* Path $(OBJ) > $^*.lnk
                    echo Lib $(CLIB) >> $^*.lnk
                    echo File >> $^*.lnk
                    echo $(OBJECT_KERNEL1) >> $^*.lnk
                    echo $(OBJECT_KERNEL2) >> $^*.lnk
                    echo $(OBJECT_KERNEL3) >> $^*.lnk
                    echo $(OBJECT_SYSCALL1) >> $^*.lnk
                    echo $(OBJECT_DEV) >> $^*.lnk
                    echo $(OBJECT_DEBUG1) >> $^*.lnk
                    echo $(OBJECT_DEBUG2) >> $^*.lnk

                    @del $(KERNEL_FILE_NAME) > NUL
                    $(LINK) $(LINK_OPTIONS) @$^*

###############################################################################
#                                                                             #
# Kernel Source Files                                                         #
#                                                                             #
###############################################################################

$(OBJ)Device.obj    :   $(SRC)Device.c          &
                        $(INC)Device.h          &
                        $(INC)TTY.h             &
                        $(INC)DosFS.h           &

                    $(CC)  $(CC_ROUTE) $(SRC)Device     $(CC_OPTIONS)

$(OBJ)DosX.obj      :   $(SRC)DosX.c            &
                        $(INC)DosX.h            &
                        $(INC)Display.h         &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)V86.h             &

                    $(CC)  $(CC_ROUTE) $(SRC)DosX       $(CC_OPTIONS)

$(OBJ)Dpmi.obj      :   $(SRC)Dpmi.c            &
                        $(INC)Dpmi.h            &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)MM.h              &

                    $(CC)  $(CC_ROUTE) $(SRC)Dpmi       $(CC_OPTIONS)

$(OBJ)Gdt.obj       :   $(SRC)Gdt.c             &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)MM.h              &

                    $(CC)  $(CC_ROUTE) $(SRC)Gdt        $(CC_OPTIONS)

$(OBJ)Exec.obj      :   $(SRC)Exec.c            &
                        $(INC)Dosx.h            &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)MM.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &
                        $(INC)Break.h           &

                    $(CC)  $(CC_ROUTE) $(SRC)Exec       $(CC_OPTIONS)

$(OBJ)File.obj      :   $(SRC)Exec.c            &
                        $(INC)File.h            &

                    $(CC)  $(CC_ROUTE) $(SRC)File       $(CC_OPTIONS)

$(OBJ)Init.obj      :   $(SRC)Init.c            &
                        $(INC)Device.h          &
                        $(INC)DosX.h            &
                        $(INC)Display.h         &
                        $(INC)File.h            &
                        $(INC)Inline.h          &
                        $(INC)Int.h             &
                        $(INC)Kernel.h          &
                        $(INC)Keyboard.h        &
                        $(INC)MM.h              &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &
                        $(INC)V86.h             &
                        $(INC)Assertk.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)Init       $(CC_OPTIONS)

$(OBJ)Inita.obj     :   $(SRC)Init.asm          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &

                    $(ASM) $(SRC)Init  $(OBJ)Inita.obj  $(ASM_OPTIONS)

$(OBJ)Int.obj       :   $(SRC)Int.c             &
                        $(INC)Inline.h          &
                        $(INC)PC.h              &

                    $(CC)  $(CC_ROUTE) $(SRC)Int        $(CC_OPTIONS)

$(OBJ)Inta.obj      :   $(SRC)Int.asm           &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)PC.h              &

                    $(ASM) $(SRC)Int   $(OBJ)Inta.obj   $(ASM_OPTIONS)

$(OBJ)Keyboard.obj  :   $(SRC)Keyboard.c        &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)Keyboard.h        &
                        $(INC)PC.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)Keyboard   $(CC_OPTIONS)

$(OBJ)PageMM.obj    :   $(SRC)PageMM.c          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)MM.h              &
                        $(INC)assertk.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)PageMM     $(CC_OPTIONS)

$(OBJ)PF.obj        :   $(SRC)PF.c              &
                        $(INC)Display.h         &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)MM.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)PF         $(CC_OPTIONS)



$(OBJ)Printf.obj    :   $(SRC)Printf.c          &
                        $(INC)Printf.h          &

                    $(CC)  $(CC_ROUTE) $(SRC)Printf     $(CC_OPTIONS)

$(OBJ)Queue.obj     :   $(SRC)Queue.c           &
                        $(INC)Intel.h           &
                        $(INC)MM.h              &
                        $(INC)Queue.h           &

                    $(CC)  $(CC_ROUTE) $(SRC)Queue      $(CC_OPTIONS)

$(OBJ)Sched.obj     :   $(SRC)Sched.c           &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)PC.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)Sched      $(CC_OPTIONS)

$(OBJ)TSS.obj       :   $(SRC)TSS.c             &
                        $(INC)Display.h         &
                        $(INC)Inline.h          &
                        $(INC)Int.h             &
                        $(INC)Kernel.h          &

                    $(CC)  $(CC_ROUTE) $(SRC)TSS        $(CC_OPTIONS)

$(OBJ)V86.obj       :   $(SRC)V86.c             &
                        $(INC)Inline.h          &
                        $(INC)Int.h             &
                        $(INC)Kernel.h          &
                        $(INC)V86.h             &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)V86        $(CC_OPTIONS)

$(OBJ)V86a.obj      :   $(SRC)V86.asm           &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &

                    $(ASM) $(SRC)V86   $(OBJ)V86a.obj   $(ASM_OPTIONS)

$(OBJ)VM.obj        :   $(SRC)VM.c              &
                        $(INC)Display.h         &
                        $(INC)Bios.h            &
                        $(INC)Dpmi.h            &
                        $(INC)Inline.h          &
                        $(INC)Display.h         &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)Info.h            &
                        $(INC)MM.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &
                        $(INC)V86.h             &
                        $(INC)Assertk.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)VM         $(CC_OPTIONS)

###############################################################################
#                                                                             #
# System Call Source Files                                                    #
#                                                                             #
###############################################################################

$(OBJ)SysCall.obj   :   $(SRC)SysCall.c         &
                        $(INC)Inline.h          &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)SysCall    $(CC_OPTIONS)

$(OBJ)Path.obj      :   $(SRC)Path.c            &

                    $(CC)  $(CC_ROUTE) $(SRC)Path       $(CC_OPTIONS)

$(OBJ)Exit.obj      :   $(SRC)Exit.c            &
                        $(INC)Inline.h          &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)Exit       $(CC_OPTIONS)

$(OBJ)Open.obj      :   $(SRC)Open.c            &
                        $(INC)Device.h          &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &
                        $(INC)DosX.h            &

                    $(CC)  $(CC_ROUTE) $(SRC)Open       $(CC_OPTIONS)

$(OBJ)Read.obj      :   $(SRC)Read.c            &
                        $(INC)Device.h          &
                        $(INC)File.h            &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &
                        $(INC)QQueue.h          &

                    $(CC)  $(CC_ROUTE) $(SRC)Read       $(CC_OPTIONS)

$(OBJ)Seek.obj      :   $(SRC)Seek.c            &
                        $(INC)Device.h          &
                        $(INC)File.h            &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)Seek       $(CC_OPTIONS)

$(OBJ)Stats.obj     :   $(SRC)Stats.c           &
                        $(INC)Device.h          &
                        $(INC)File.h            &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &

                    $(CC)  $(CC_ROUTE) $(SRC)Stats      $(CC_OPTIONS)

$(OBJ)Write.obj     :   $(SRC)Write.c           &
                        $(INC)Device.h          &
                        $(INC)File.h            &
                        $(INC)Kernel.h          &
                        $(INC)kSysCall.h        &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &
                        $(INC)QQueue.h          &

                    $(CC)  $(CC_ROUTE) $(SRC)Write      $(CC_OPTIONS)

###############################################################################
#                                                                             #
# Device Drivers Source Files                                                 #
#                                                                             #
###############################################################################

$(OBJ)TTY.obj       :   $(SRC_DEV)TTY.c         &
                        $(INC)Device.h          &
                        $(INC)Kernel.h          &
                        $(INC)V86.h             &
                        $(INC)Display.h         &
                        $(INC)Inline.h          &
                        $(INC)TTY.h             &
                        $(INC)Process.h         &
                        $(INC)Intel.h           &
                        $(INC)TTY.h             &
                        $(INC)MM.h              &

                    $(CC)  $(CC_ROUTE) $(SRC_DEV)TTY     $(CC_OPTIONS)

$(OBJ)DosFS.obj     :   $(SRC_DEV)DosFS.c       &
                        $(INC)Device.h          &
                        $(INC)Intel.h           &
                        $(INC)MM.h              &
                        $(INC)DosFS.h           &

                    $(CC)  $(CC_ROUTE) $(SRC_DEV)DosFS   $(CC_OPTIONS)

###############################################################################
#                                                                             #
# Debugger Source Files                                                       #
#                                                                             #
###############################################################################

$(OBJ)Break.obj     :   $(SRC_DEB)Break.c       &
                        $(INC)Debug.h           &
                        $(INC)Queue.h           &
                        $(INC)Break.h           &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)MM.h              &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Break    $(CC_OPTIONS)

$(OBJ)Command.obj   :   $(SRC_DEB)Command.c     &
                        $(INC)Command.h         &
                        $(INC)Display.h         &
                        $(INC)Keyboard.h        &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Command  $(CC_OPTIONS)

$(OBJ)Debug.obj     :   $(SRC_DEB)Debug.c       &
                        $(INC)Command.h         &
                        $(INC)Debug.h           &
                        $(INC)Dis386.h          &
                        $(INC)Dump.h            &
                        $(INC)Eval.h            &
                        $(INC)Info.h            &
                        $(INC)Inline.h          &
                        $(INC)Int.h             &
                        $(INC)Intel.h           &
                        $(INC)Keyboard.h        &
                        $(INC)Kernel.h          &
                        $(INC)Map.h             &
                        $(INC)MM.h              &
                        $(INC)PC.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &
                        $(INC)Register.h        &
                        $(INC)Set.h             &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Debug    $(CC_OPTIONS)

$(OBJ)Dis386.obj    :   $(SRC_DEB)Dis386.c      &
                        $(INC)Dis386.h          &
                        $(INC)Dis386p.h         &
                        $(INC)Dump.h            &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Dis386   $(CC_OPTIONS)

$(OBJ)Display.obj   :   $(SRC_DEB)Display.c     &
                        $(INC)Display.h         &
                        $(INC)Inline.h          &
                        $(INC)Printf.h          &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Display  $(CC_OPTIONS)

$(OBJ)Dump.obj      :   $(SRC_DEB)Dump.c        &
                        $(INC)Debug.h           &
                        $(INC)Dump.h            &
                        $(INC)Inline.h          &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &
                        $(INC)MM.h              &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Dump     $(CC_OPTIONS)

$(OBJ)Eval.obj      :   $(SRC_DEB)Eval.c        &
                        $(INC)Eval.h            &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Eval     $(CC_OPTIONS)

$(OBJ)Info.obj      :   $(SRC_DEB)Info.c        &
                        $(INC)Debug.h           &
                        $(INC)Display.h         &
                        $(INC)Dpmi.h            &
                        $(INC)Info.h            &
                        $(INC)Inline.h          &
                        $(INC)Kernel.h          &
                        $(INC)Keyboard.h        &
                        $(INC)MM.h              &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &
                        $(INC)V86.h             &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Info     $(CC_OPTIONS)

$(OBJ)Map.obj       :   $(SRC_DEB)Map.c         &
                        $(INC)Intel.h           &
                        $(INC)MM.h              &
                        $(INC)Debug.h           &
                        $(INC)Display.h         &
                        $(INC)DosX.h            &
                        $(INC)Keyboard.h        &
                        $(INC)Queue.h           &
                        $(INC)Map.h             &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Map      $(CC_OPTIONS)

$(OBJ)Register.obj  :   $(SRC_DEB)Register.c    &
                        $(INC)Debug.h           &
                        $(INC)Intel.h           &
                        $(INC)TTY.h             &
                        $(INC)Device.h          &
                        $(INC)Process.h         &
                        $(INC)Register.h        &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Register $(CC_OPTIONS)

$(OBJ)Set.obj       :   $(SRC_DEB)Set.c         &
                        $(INC)Intel.h           &
                        $(INC)MM.h              &
                        $(INC)Eval.h            &
                        $(INC)Set.h             &

                    $(CC)  $(CC_ROUTE) $(SRC_DEB)Set      $(CC_OPTIONS)

$(OBJ)Trap.obj      :   $(SRC_DEB)Trap.asm      &
                        $(INC)Intel.h           &
                        $(INC)Kernel.h          &

                    $(ASM) $(SRC_DEB)Trap  $(OBJ)Trap.obj $(ASM_OPTIONS)

###############################################################################
#                                                                             #
# Symbolic Commands                                                           #
#                                                                             #
###############################################################################

clean               :   .SYMBOLIC
                    @del $(OBJ)*.obj  > NUL
                    @del $(OBJ)*.lst  > NUL
                    @del *.err        > NUL
                    @del *.lst        > NUL
                    @del *.map        > NUL
                    @del *.lnk        > NUL
                    @del *.sbr        > NUL
                    @del err          > NUL

all                 :   .SYMBOLIC
                    @del $(KERNEL_FILE_NAME) > NUL
                    wmake clean
                    wmake

