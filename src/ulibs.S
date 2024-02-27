/**
** @file	ulibs.S
**
** @author	Numerous CSCI-452 classes
**
** @brief	assembly-language user-level library functions
*/

#define	SP_ASM_SRC

// get the system call codes

#include "syscalls.h"

/**
** System call stubs
**
** All have the same structure:
**
**      move a code into EAX
**      generate the interrupt
**      return to the caller
**
** As these are simple "leaf" routines, we don't use
** the standard enter/leave method to set up a stack
** frame - that takes time, and we don't really need it.
*/

#define	SYSCALL(name) \
	.globl	name			; \
name:					; \
	movl	$SYS_##name, %eax	; \
	int	$INT_VEC_SYSCALL	; \
	ret

/*
** "real" system calls
*/

SYSCALL(exit)
SYSCALL(read)
SYSCALL(write)
SYSCALL(sleep)
/*
** TODO remaining system calls
** TODO any other assembly-language library functions
*/
