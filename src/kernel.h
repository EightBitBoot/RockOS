/**
** @file	kernel.h
**
** @author	Numerous CSCI-452 classes
**
** @brief	Miscellaneous OS routines
*/

#ifndef KERNEL_H_
#define KERNEL_H_

#include "common.h"
#include "stacks.h"

#include "x86arch.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

// default contents of EFLAGS register for new processes
#define DEFAULT_EFLAGS	(EFLAGS_MB1 | EFLAGS_IF)

// PID for the init process is defined in params.h

/*
** Types
*/

/*
** Globals
*/

// character buffers, usable throughout the OS
// not guaranteed to retain their contents across an exception return
extern char _b256[256];
extern char _b512[512];

// Other system variables (see kernel.c for possible names)

// table of state name strings
extern const char *_state_str[][2];
// indices for short and long names
#define ST_L_NAME		0
#define ST_S_NAME		1

// table of priority name strings
extern const char *_prio_str[];

/*
** Prototypes
*/

/**
** _kinit - system initialization routine
**
** Called by the startup code immediately before returning into the
** first user process.
*/
void _kinit( void );

/**
** _kpanic - kernel-level panic routine
**
** usage:  _kpanic( msg )
**
** Prefix routine for __panic() - can be expanded to do other things
** (e.g., printing a stack traceback)
**
** @param msg  String containing a relevant message to be printed, or NULL
*/
void _kpanic( const char *msg );

#endif
/* SP_ASM_SRC */

#endif
