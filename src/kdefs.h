/**
** @file kdefs.h
**
** @author Numerous CSCI-452 classes
**
** Kernel-only definitions for the baseline system.
**
*/

#ifndef KDEFS_H_
#define KDEFS_H_

// Standard system headers

#include "cio.h"
#include "support.h"

// Kernel library

#include "lib.h"

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Types
*/

/*
** Macros and pseudo-functions
*/

// bit patterns for modulus checking of (e.g.) sizes and addresses

#define MOD4_BITS        0x3
#define MOD4_MASK        0xfffffffc

#define MOD16_BITS       0xf
#define MOD16_MASK       0xfffffff0

/*
** Utility macros
*/

//
// macros to clear data structures
//
// these are usable for clearing single-valued data items (e.g.,
// a PCB, a Queue, a QNode, etc.)
#define CLEAR(v)        __memclr( &v, sizeof(v) )
#define CLEAR_PTR(p)    __memclr( p, sizeof(*p) )

// macros for access registers and system call arguments

// REG(pcb,x) -- access a specific register in a process context
#define REG(pcb,x)  ((pcb)->context->x)

// RET(pcb) -- access return value register in a process context
#define RET(pcb)    ((pcb)->context->eax)

// ARG(pcb,n) -- access argument #n from the indicated process
//
// ARG(pcb,0) --> return address
// ARG(pcb,1) --> first parameter
// ARG(pcb,2) --> second parameter
// etc.
//
// ASSUMES THE STANDARD 32-BIT ABI, WITH PARAMETERS PUSHED ONTO THE
// STACK.  IF THE PARAMETER PASSING MECHANISM CHANGES, SO MUST THIS!
#define ARG(pcb,n)  ( ( (uint32_t *) (((pcb)->context) + 1) ) [(n)] )

#endif
/* SP_ASM_SRC */

#endif
