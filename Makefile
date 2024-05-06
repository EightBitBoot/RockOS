#
# SCCS ID: @(#)Makefile	2.2	11/28/22
#
# Makefile to control the compiling, assembling and linking of standalone
# programs in the DSL.  Used for both individual interrupt handling
# assignments and the SP baseline OS (with appropriate tweaking).
#

#
# Fancy, fast dependency generation taken from
# https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# (specifically the "Combining Compilation and Dependency Generation" section)
#

##################
#  FILE SECTION  #
##################

SRC_DIR = src

BUILD_DIR = build
DEP_DIR = $(BUILD_DIR)/dep
OBJ_DIR = $(BUILD_DIR)/obj
ASM_DIR = $(BUILD_DIR)/asm
LST_DIR = $(BUILD_DIR)/lst
BIN_DIR = $(BUILD_DIR)/bin

BUILD_DIRS = $(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR) $(ASM_DIR) $(LST_DIR) $(BIN_DIR)

VPATH ::= $(subst " ",:,$(shell find $(SRC_DIR) -type d))

#
# OS files
#

OS_C_SRC = clock.c kernel.c kmem.c procs.c queues.c sched.c sio.c stacks.c \
	   syscalls.c vgatext.c acpi/acpi.c acpi/aml.c acpi/checksum.c \
	   acpi/tables/rsdp.c acpi/tables/sdt.c vga.c vgaconst.c

OS_S_SRC =

OS_HDRS  = clock.h common.h compat.h kdefs.h kernel.h kmem.h offsets.h \
	   	   params.h procs.h queues.h sched.h sio.h stacks.h syscalls.h \
	   	   vgatext.h acpi/acpi.h vga.h

OS_LIBS =

OS_C_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(OS_C_SRC:.c=.o)))
OS_S_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(OS_S_SRC:.S=.o)))


OS_SRCS = $(OS_C_SRC) $(OS_S_SRC)
OS_OBJS = $(OS_C_OBJ) $(OS_S_OBJ)

#
# "Userland" files
#

USR_C_SRC = ulibc.c users.c

USR_S_SRC = ulibs.S

USR_HDRS  = udefs.h ulib.h users.h

USR_LIBS  =

USR_C_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(USR_C_SRC:.c=.o)))
USR_S_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(USR_S_SRC:.S=.o)))

USR_SRCS  = $(USR_C_SRC) $(USR_S_SRC)
USR_OBJS  = $(USR_C_OBJ) $(USR_S_OBJ)

#
# Framework files
#

FMK_S_SRC = startup.S isr_stubs.S libs.S

FMK_C_SRC = cio.c libc.c support.c

FMK_HDRS = bootstrap.h cio.h lib.h support.h uart.h \
	   	   x86arch.h x86pic.h x86pit.h

FMK_S_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(FMK_S_SRC:.S=.o)))
FMK_C_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(FMK_C_SRC:.c=.o)))

FMK_SRCS = $(FMK_S_SRC) $(FMK_C_SRC)
FMK_OBJS = $(FMK_S_OBJ) $(FMK_C_OBJ)

BOOT_SRC = bootstrap.S
BOOT_OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(BOOT_SRC:.S:.o)))

# Collections of files

OBJECTS = $(FMK_OBJS) $(OS_OBJS) $(USR_OBJS)

SOURCES = $(BOOT_SRC) $(FMK_SRCS) $(OS_SRCS) $(USR_SRCS)

DEPENDENCIES = $(OBJECTS:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)
# Temp workaround for bootstrap.S because it goes .S -> .s -> .d
# and never becomes an object: therefore it isn't included in
# $(OBJECTS) or $(DEPENDENCIES).
#
# Without it being in $(DEPENDENCIES), the second to last line
# $(DEPENDENCIES): doesn't include it as an empty target.  Finally,
# without the empty target, the dependency for %.s: %.S ($(DEP_DIR)/%.d),
# is missing, has no rule to be built, and therefore the entire
# pattern rule is skipped.  Instead make uses a builtin, default
# implicit rule (along the lines of to .S.s) which doesn't include
# $(DEP_FLAGS) and therefore doesn't build the dependency file.
#
# TODO(Adin): Add an explicit rule for bootstrap.b (a la prog.b)
# and change bootstrap to be built as a regular object file (.o)
DEPENDENCIES += $(DEP_DIR)/bootstrap.d

#####################
#  OPTIONS SECTION  #
#####################

#
# Compilation/assembly definable options
#
# General options:
#	CLEAR_BSS		include bootstrap code to clear all BSS space
#	GET_MMAP		get BIOS memory map via int 0x15 0xE820
#	SP_CONFIG		enable SP OS-specific startup variations
#
# OS behavior:
#	STATIC_STACKS		statically allocate all stack space
#	USER_SHELL		have 'init' spawn the user-level shell
#
# Debugging options:
#	RPT_INT_UNEXP		report any 'unexpected' interrupts
#	RPT_INT_MYSTERY		report interrupt 0x27 specifically
#	TRACE_CX		include context restore trace code
#	TRACE=n			bitmask of internal tracing options
#	SANITY=n		enable "sanity check" level 'n'
#	  0			absolutely critical errors only
#	  1			important consistency checking
#	  2			less important consistency checking
#	  > 2			currently unused
#	CONSOLE_STATS		print statistics on console kbd input
#	SYSTEM_STATUS=n         dump queue & process info every 'n' seconds
#
# Define SANITY as 0 for minimal runtime checking (critical errors only).
# If not defined, SANITY defaults to 9999.
#
# TRACE is a bitmask; see kdefs.h for definitions of the individual bits
# in the mask and the names of the available tracing macros.  Currently,
# these bits are defined:
#
#	PCB      0001	STACK    0002	QUEUE    0040	SCHED  0008
#	SYSCALLS 0010	SYSRETS  0020	EXIT     0040	DISP   0080
#	CONSOLE  0100	KMEM     0200	KM_FREE  0400	SPAWN  0800
#	SIO STAT 1000	SIO_ISR  2000	SIO_RD   4000	SIO_WR 8000
#
# You can add compilation options "on the fly" by using EXTRAS=thing
# on the command line.  For example, to compile with -H (to show the
# hierarchy of #includes):
#
# 	make EXTRAS=-H
#

GEN_OPTIONS = -DCLEAR_BSS -DGET_MMAP -DSP_CONFIG
DBG_OPTIONS = -DTRACE_CX -DSTATUS=3

USER_OPTIONS = $(GEN_OPTIONS) $(DBG_OPTIONS)

##############################################################
# YOU SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS POINT!!! #
##############################################################

#
# Compilation/assembly control
#

#
# We only want to include from the current directory
#
# INCLUDES = -I. -I/home/fac/wrc/include
INCLUDES = -I$(SRC_DIR)

#
# Compilation/assembly/linking commands and options
#
DBG_FLAGS = -g
DEP_FLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.d

CPP = cpp
CPPFLAGS = $(DBG_FLAGS) $(USER_OPTIONS) -nostdinc $(INCLUDES)

#
# Compiler/assembler/etc. settings for 32-bit binaries
#
CC = gcc
CFLAGS = -m32 -fno-pie -std=c99 -fno-stack-protector -fno-builtin -Wall -Wstrict-prototypes $(CPPFLAGS) $(EXTRAS)

AS = as
ASFLAGS = $(DBG_FLAGS) --32

LD = ld
LDFLAGS = -melf_i386 -no-pie

#
# Transformation rules - these ensure that all compilation
# flags that are necessary are specified
#
# Note use of 'cpp' to convert .S files to temporary .s files: this allows
# use of #include/#define/#ifdef statements. However, the line numbers of
# error messages reflect the .s file rather than the original .S file.
# (If the .s file already exists before a .S file is assembled, then
# the temporary .s file is not deleted.  This is useful for figuring
# out the line numbers of error messages, but take care not to accidentally
# start fixing things by editing the .s file.)
#
# The .c.X rule produces a .X file which contains the original C source
# code from the file being compiled mixed in with the generated
# assembly language code.  Very helpful when you need to figure out
# exactly what C statement generated which assembly statements!
#

#
# offsets.h is included as a prerequisite for all compilation targets
# because of the shift to "fancy" dependency rule generation.  The first
# time make is run now, there are no dependency targets.  Theoretically
# all should be well because everything will be regenerated, including
# offsets.h.  Indeed this does happen as offset.h is a prerequisite of
# disk.img.
#
# However, this is a race condition when running on multiple # processes
# with -j.  make has no idea that certian .o files also rely on offsets.h
# and can schedule them to be built at the same time as or even before
# offsets.h is built.  This results in gcc not finding the file and the
# build failing.
#
# Adding it as a prerequisite here fixes the issue and only causes a full
# recompile in an unlikely edge case.  common.h is auto generated and should
# never be modified by hand.  In the off chance it is changed by hand (or any
# of its prerequisites are changed), a full recompilation will be triggered.
# However, in all other cases it will sit unchanged and will not have any
# effect on the out of date status of any targets.
#

.SUFFIXES: .X .i
.PRECIOUS: $(ASM_DIR)/%.s

.c.X:
	$(CC) $(CFLAGS) -g -c -Wa,-adhln $*.c > $*.X

.c.s:
	$(CC) $(CFLAGS) -S $*.c

$(ASM_DIR)/%.s: %.S offsets.h $(DEP_DIR)/%.d | $(DEP_DIR) $(ASM_DIR)
	$(CPP) $(DEP_FLAGS) $(CPPFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.S offsets.h $(DEP_DIR)/%.d | $(DEP_DIR) $(OBJ_DIR) $(ASM_DIR) $(LST_DIR)
	$(CPP) $(DEP_FLAGS) $(CPPFLAGS) -o $(ASM_DIR)/$*.s $<
	$(AS) $(ASFLAGS) -o $@ $(ASM_DIR)/$*.s -a=$(LST_DIR)/$*.lst

$(BIN_DIR)/%.b: $(ASM_DIR)/%.s | $(LST_DIR) $(OBJ_DIR) $(BIN_DIR)
	$(AS) $(ASFLAGS) -o $(OBJ_DIR)/$*.o $< -a=$(LST_DIR)/$*.lst
	$(LD) $(LDFLAGS) -Ttext 0x0 -s --oformat binary -e begtext -o $@ $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%.o: %.c offsets.h $(DEP_DIR)/%.d | $(DEP_DIR) $(OBJ_DIR)
	$(CC) $(DEP_FLAGS) $(CFLAGS) -c -o $@ $<

.c.i:
	$(CC) -E $(CFLAGS) -c $*.c > $*.i

#
# -- DO NOT UNCOMMENT --
# This is the wrong way to generate dependencies and
# is here for documentation purposes only
#

# $(DEP_DIR)/%.d: %.c | $(DEP_DIR)
# 	$(CC) $(CFLAGS) -MM -o $@ $<

#
# Targets for remaking bootable image of the program
#
# Default target:  disk.img
#

.PHONY: disk.img floppy.img prog.out prog.o

disk.img: $(BUILD_DIR)/disk.img

floppy.img: $(BUILD_DIR)/floppy.img

prog.out: $(BUILD_DIR)/prog.out

prog.o: $(OBJ_DIR)/prog.o

prog.b: $(BIN_DIR)/prog.b

$(BUILD_DIR)/disk.img: $(SRC_DIR)/offsets.h $(BIN_DIR)/bootstrap.b $(BIN_DIR)/prog.b $(BUILD_DIR)/BuildImage $(BUILD_DIR)/prog.nl $(BUILD_DIR)/prog.dis | $(BUILD_DIR)
	$(BUILD_DIR)/BuildImage -d usb -o $(BUILD_DIR)/disk.img -b $(BIN_DIR)/bootstrap.b $(BIN_DIR)/prog.b 0x10000

$(BUILD_DIR)/floppy.img: $(SRC_DIR)/offsets.h $(BIN_DIR)/bootstrap.b $(BIN_DIR)/prog.b $(BUILD_DIR)/BuildImage $(BUILD_DIR)/prog.nl $(BUILD_DIR)/prog.dis | $(BUILD_DIR)
	$(BUILD_DIR)/BuildImage -d floppy -o $(BUILD_DIR)/floppy.img -b $(BIN_DIR)/bootstrap.b $(BIN_DIR)/prog.b 0x10000

$(BUILD_DIR)/prog.out: $(OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/prog.out $(OBJECTS)

$(OBJ_DIR)/prog.o: $(OBJECTS) | $(OBJ_DIR)
	$(LD) $(LDFLAGS) -o $(OBJ_DIR)/prog.o -Ttext 0x10000 $(OBJECTS) $(A_LIBS)

$(BIN_DIR)/prog.b: $(OBJ_DIR)/prog.o | $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $(BIN_DIR)/prog.b -s --oformat binary -Ttext 0x10000 $(OBJ_DIR)/prog.o
#
# Targets for copying bootable image onto boot devices
#

.PHONY: floppy usb

floppy: floppy.img
	dd if=floppy.img of=/dev/fd0

usb: disk.img
	/usr/local/dcs/bin/dcopy $(BUILD_DIR)/disk.img

#
# Special rule for creating the modification and offset programs
#
# These are required because we don't want to use the same options
# as for the standalone binaries.
#

.PHONY: BuildImage Offsets

BuildImage:	$(BUILD_DIR)/BuildImage

Offsets: $(BUILD_DIR)/Offsets

offsets.h: $(SRC_DIR)/offsets.h

$(SRC_DIR)/offsets.h: $(BUILD_DIR)/Offsets
	$(BUILD_DIR)/Offsets -h $(SRC_DIR)/offsets.h

$(BUILD_DIR)/BuildImage: BuildImage.c | $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/BuildImage $(SRC_DIR)/prog/BuildImage.c

$(BUILD_DIR)/Offsets: Offsets.c procs.h stacks.h queues.h common.h | $(BUILD_DIR)
	$(CC) -mx32 -std=c99 $(INCLUDES) -I../framework -o $(BUILD_DIR)/Offsets $(SRC_DIR)/prog/Offsets.c

$(BUILD_DIRS):
	mkdir -p $@

#
# Clean out this directory
#

.PHONY: clean realclean

clean:
	rm -f *.nl *.nll *.lst *.b *.i *.o *.X *.dis

realclean: clean
	rm -f offsets.h *.img BuildImage Offsets
	rm -rf src/offsets.h $(BUILD_DIR)

#
# Create a printable namelist from the prog.o file
#

.PHONY: prog.nl prog.nll

prog.nl: $(BUILD_DIR)/prog.nl

prog.nll: $(BUILD_DIR)/prog.nll

$(BUILD_DIR)/prog.nl: $(OBJ_DIR)/prog.o | $(BUILD_DIR)
	nm -Bng $(OBJ_DIR)/prog.o | pr -w80 -3 > $(BUILD_DIR)/prog.nl

$(BUILD_DIR)/prog.nll: $(OBJ_DIR)/prog.o | $(BUILD_DIR)
	nm -Bn $(OBJ_DIR)/prog.o | pr -w80 -3 > $(BUILD_DIR)/prog.nll

#
# Generate a disassembly
#

.PHONY: prog.dis

prog.dis: $(BUILD_DIR)/prog.dis

$(BUILD_DIR)/prog.dis: $(OBJ_DIR)/prog.o | $(BUILD_DIR)
	objdump -d $(OBJ_DIR)/prog.o > $(BUILD_DIR)/prog.dis

#
# Include all dependency files generated from the last run(s) of make
#

$(DEPENDENCIES):
-include $(DEPENDENCIES)
