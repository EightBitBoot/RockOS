/**
** @file	debug.h
**
** @author	Numerous CSCI-452 classes
**
** @brief	Debugging macros
**
*/

#ifndef DEBUG_H_
#define DEBUG_H_

// Standard system headers

#include "io/cio.h"
#include "kern/support.h"
#include "libc/lib.h"

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Debugging and sanity-checking macros
*/

// Warning messages to the console
// m: message (condition, etc.)
#define WARNING(m)  do { \
        __cio_printf( "WARN %s (%s @ %d): ", __func__, __FILE__, __LINE__ ); \
        __cio_puts( m ); \
        __cio_putchar( '\n' ); \
    } while(0)

// Panic messages to the console
// n: severity level
// m: message (condition, etc.)
#define PANIC(n,m)  do { \
        __sprint( b512, "ASSERT %s (%s @ %d), %d: %s\n", \
                  __func__, __FILE__, __LINE__, n, # m ); \
        _kpanic( b512 ); \
    } while(0)

/*
** Assertions are categorized by the "sanity level" being used in this
** compilation; each only triggers a fault if the sanity level is at or
** above a specific value.  This allows selective enabling/disabling of
** debugging checks.
**
** The sanity level is set during compilation with the CPP macro
** "SANITY".  A sanity level of 0 disables conditional assertions,
** but not the basic assert() version.
*/

#ifndef SANITY
// default sanity check level: check everything!
#define SANITY  9999
#endif

// Always-active assertions
#define assert(x)   if( !(x) ) { PANIC(0,x); }

// only provide these macros if the sanity check level is positive

#if SANITY > 0

#define assert1(x)  if( SANITY >= 1 && !(x) ) { PANIC(1,x); }
#define assert2(x)  if( SANITY >= 2 && !(x) ) { PANIC(2,x); }
#define assert3(x)  if( SANITY >= 3 && !(x) ) { PANIC(3,x); }
#define assert4(x)  if( SANITY >= 4 && !(x) ) { PANIC(4,x); }

#else

#define assert1(x)  // do nothing
#define assert2(x)  // do nothing
#define assert3(x)  // do nothing
#define assert4(x)  // do nothing

#endif
/* if SANITY > 0 */

/*
** Tracing options are enabled by defining the TRACE macro in the
** Makefile.  The value of that macro is a bit mask.
*/

#ifdef TRACE

// bits for selecting the thing(s) to be traced
#define TRACE_PCB           0x0001
#define TRACE_STACK         0x0002
#define TRACE_QUEUE         0x0004
#define TRACE_SCHED         0x0008
#define TRACE_SYSCALLS      0x0010
#define TRACE_SYSRETS       0x0020
#define TRACE_EXIT          0x0040
#define TRACE_DISPATCH      0x0080
#define TRACE_CONSOLE       0x0100
#define TRACE_KMEM          0x0200
#define TRACE_KMEM_FREE     0x0400
#define TRACE_SPAWN         0x0800
#define TRACE_SIO_STAT      0x1000
#define TRACE_SIO_ISR       0x2000
#define TRACE_SIO_RD        0x4000
#define TRACE_SIO_WR        0x8000

// compile-time expressions for testing trace options
// usage:  #if TRACING_thing
#define TRACING_PCB         ((TRACE & TRACE_PCB) != 0)
#define TRACING_STACK       ((TRACE & TRACE_STACK) != 0)
#define TRACING_QUEUE       ((TRACE & TRACE_QUEUE) != 0)
#define TRACING_SCHED       ((TRACE & TRACE_SCHED) != 0)
#define TRACING_SYSCALLS    ((TRACE & TRACE_SYSCALLS) != 0)
#define TRACING_SYSRETS     ((TRACE & TRACE_SYSRETS) != 0)
#define TRACING_EXIT        ((TRACE & TRACE_EXIT) != 0)
#define TRACING_DISPATCH    ((TRACE & TRACE_DISPATCH) != 0)
#define TRACING_CONSOLE     ((TRACE & TRACE_CONSOLE) != 0)
#define TRACING_KMEM        ((TRACE & TRACE_KMEM) != 0)
#define TRACING_KMEM_FREE   ((TRACE & TRACE_KMEM_FREE) != 0)
#define TRACING_SPAWN       ((TRACE & TRACE_SPAWN) != 0)
#define TRACING_SIO_STAT    ((TRACE & TRACE_SIO_STAT) != 0)
#define TRACING_SIO_ISR     ((TRACE & TRACE_SIO_ISR) != 0)
#define TRACING_SIO_RD      ((TRACE & TRACE_SIO_RD) != 0)
#define TRACING_SIO_WR      ((TRACE & TRACE_SIO_WR) != 0)

#define TRACING_SOMETHING   (TRACE != 0)

#else

// TRACE is undefined, so just define these all as "false"

#define TRACING_PCB         0
#define TRACING_STACK       0
#define TRACING_QUEUE       0
#define TRACING_SCHED       0
#define TRACING_SYSCALLS    0
#define TRACING_SYSRET      0
#define TRACING_EXIT        0
#define TRACING_DISPATCH    0
#define TRACING_CONSOLE     0
#define TRACING_KMEM        0
#define TRACING_KMEM_FREE   0
#define TRACING_SPAWN       0
#define TRACING_SI_STAT     0
#define TRACING_SIO_ISR     0
#define TRACING_SIO_RD      0
#define TRACING_SIO_WR      0

#define TRACING_SOMETHING   0

#endif
// TRACE

#endif
// !SP_ASM_SRC

#endif
