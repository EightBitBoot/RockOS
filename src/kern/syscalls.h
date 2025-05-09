/**
** @file	syscalls.h
**
** @author	CSCI-452 class of 20235
**
** @brief	System call declarations
*/

#ifndef SYSCALLS_H_
#define SYSCALLS_H_

/*
** General (C and/or assembly) definitions
**
** This section of the header file contains definitions that can be
** used in either C or assembly-language source code.
*/

#include "common.h"

// system call codes
//
// these are used in the user-level C library stub functions,
// and are defined here as CPP macros instead of as an enum
// so that they can be used from assembly
#define  SYS_exit                   0
#define  SYS_sleep                  1
#define  SYS_read                   2
#define  SYS_write                  3
#define  SYS_waitpid                4
#define  SYS_getdata                5
#define  SYS_setdata                6
#define  SYS_kill                   7
#define  SYS_fork                   8
#define  SYS_exec                   9
#define  SYS_vgatextclear           10
#define  SYS_vgatextgetactivecolor  11
#define  SYS_vgatextsetactivecolor  12
#define  SYS_acpicommand            13
#define SYS_vgatextgetblinkenabled  14
#define SYS_vgatextsetblinkenabled  15
#define SYS_vgagetmode              16
#define SYS_vgasetmode              17
#define SYS_vgaclearscreen          18
#define SYS_vgatest                 19
#define SYS_vgadrawimage            20
#define SYS_vgawritepixel           21
#define  SYS_ciogetcursorpos        22
#define  SYS_ciosetcursorpos        23
#define  SYS_ciogetspecialdown      24

#define  SYS_fopen                  25
#define  SYS_fclose                 26
#define  SYS_fread                  27
#define  SYS_fwrite                 28
#define  SYS_flistdir               29
#define  SYS_fcreate                30
#define  SYS_fdelete                31
#define  SYS_fioctl                 32
#define  SYS_fseek                  33
#define SYS_fchdir                  34
#define SYS_fgetcwd                 35


// UPDATE THIS DEFINITION IF MORE SYSCALLS ARE ADDED!
#define N_SYSCALLS      36

// dummy system call code for testing our ISR
#define SYS_bogus       0xbad

// interrupt vector entry for system calls
#define INT_VEC_SYSCALL 0x80

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
**
** Anything that should not be visible to something other than
** the C compiler should be put here.
*/

#include "util/queues.h"

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

/**
** Name:  _sys_init
**
** Syscall module initialization routine
*/
void _sys_init( void );

#endif
// SP_ASM_SRC

#endif
