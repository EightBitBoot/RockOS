/**
** @file	common.h
**
** @author	CSCI-452 class of 20235
** @author	Warren R. Carithers
**
** @brief	Common definitions for the baseline system.
**
** This header file pulls in the standard header information
** needed by all parts of the system (OS and user levels).
**
** Things which are kernel-specific go in the kdefs.h file;
** things which are user-specific go in the udefs.h file.
** The appropriate 'defs' file is included here based on the
** SP_KERNEL_SRC macro.
*/

#ifndef COMMON_H_
#define COMMON_H_

#include "params.h"

/*
** General (C and/or assembly) definitions
**
** This section of the header file contains definitions that can be
** used in either C or assembly-language source code.
*/

// NULL pointer value
//
// we define this the traditional way so that
// it's usable from both C and assembly

#define NULL			0

// predefined I/O channels

#define CHAN_CIO		0
#define CHAN_SIO		1

// maximum number of arguments that can be passed to a user process

#define	MAX_ARGS		10

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
**
** Anything that should not be visible to something other than
** the C compiler should be put here.
*/

// halves of various data sizes

#define UI16_UPPER		0xff00
#define UI16_LOWER		0x00ff

#define UI32_UPPER		0xffff0000
#define UI32_LOWER		0x0000ffff

#define UI64_UPPER		0xffffffff00000000LL
#define UI64_LOWER		0x00000000ffffffffLL

// Simple conversion pseudo-functions usable by everyone

// convert seconds to ms

#define SEC_TO_MS(n)	((n) * 1000)

/*
** Types
*/

// standard integer sized types
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;

// Boolean values
typedef uint8_t bool_t;

#define true	1
#define false	0

// ----------------------------------------------------------
// System time
typedef uint32_t time_t;

// ----------------------------------------------------------
// User main() function signature
typedef int32_t (*userfcn_t)(int32_t,char*[]);

// ----------------------------------------------------------
// Get/set syscall options
typedef int32_t datum_t;

enum datum_e {
	Pid = 0, PPid = 1, Prio = 2, Time = 3,
	// sentinel
	N_DATUMS
	// yes, that's valid as the plural of 'datum', according
	// to both Merriam-Webster and the Britannia Dictionary
};

// ----------------------------------------------------------
// System call error return values

// success!
#define E_SUCCESS		0

// generic failure
#define E_FAILURE		(-1)

// other failures
#define E_BAD_PARAM		(-2)
#define E_BAD_CHAN		(-3)
#define	E_NO_CHILDREN	(-4)
#define	E_NO_DATA		(-5)
#define	E_NO_PROCS		(-6)
#define	E_NOT_FOUND		(-7)

// ----------------------------------------------------------
// Predefined user process exit status values

// normal exit status
#define	EXIT_SUCCESS	0

// error indicators
#define	EXIT_FAILURE	(-1)
#define	EXIT_ABORTED	(-2)
#define	EXIT_KILLED		(-3)

// ----------------------------------------------------------
// PIDs are 16-bit unsigned ints
typedef uint16_t pid_t;

// ----------------------------------------------------------
// Minimal space for priority representation
typedef uint8_t prio_t;

// Process priorities
enum prio_e {
    SysPrio = 0, UserPrio, DeferredPrio,
    // sentinel
    N_PRIOS
};

/*
** Additional OS-only or user-only things
*/

#ifdef SP_KERNEL_SRC
#include "kdefs.h"
#else
#include "udefs.h"
#endif

#endif
// !SP_ASM_SRC

#endif
