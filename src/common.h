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

// generic types

// Boolean values
typedef uint8_t bool_t;

#define true	1
#define false	0

// System time
typedef uint32_t time_t;

// Status return values
typedef int32_t status_t;

// success!
#define S_OK	0

// generic failure
#define S_ERR	(-1)

// other failures
#define S_NOMEM	(-2)
#define S_EMPTY	(-3)

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
