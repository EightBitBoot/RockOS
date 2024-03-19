/*
** @file	procs.h
**
** @author	CSCI-452 class of 20235
**
** @brief	Process-related declarations
*/

#ifndef PROCS_H_
#define PROCS_H_

#include "common.h"

/*
** General (C and/or assembly) definitions
*/

// standard quantum for user processes is 5 clock ticks
#define	Q_STD		5

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Types
**
** Note: some process-related types (e.g., priorities) are defined
** in common.h because they are used by both the OS and user-level
** code.
*/

// process states may need to be moved to common.h
// if they are going to be visible to userland

// Process states
enum state_e {
	// ordinary states
	Unused = 0, New,
	// "active" states
	Ready, Running, Sleeping, Blocked, Waiting,
	// "dead" states
	Killed, Zombie,
	// sentinel - value equals the number of states
	N_STATES
};

// our process state type
typedef uint8_t state_t;

/*
** Process context structure
**
** NOTE:  the order of data members here depends on the
** register save code in isr_stubs.S!!!!
**
** This will be at the top of the user stack when we enter
** an ISR.  In the case of a system call, it will be followed
** by the return address and the system call parameters.
*/

typedef struct context_s {
	uint32_t ss;		// pushed by isr_save
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t vector;
	uint32_t code;
	uint32_t eip;		// pushed by the hardware
	uint32_t cs;
	uint32_t eflags;
} context_t;

#define SZ_CONTEXT	sizeof(context_t)

// Now we can include stacks.h (it needs to know what a
// context_t is). It also needs to know what a pcb_t is,
// so we'll add a forward type declaration for that.

typedef struct pcb_s pcb_t;

#include "stacks.h"

/*
** the process control block
**
** fields are ordered by size to avoid padding
**
** ideally, its size should divide evenly into 1024 bytes;
** currently, 32 bytes
*/

struct pcb_s {

	// four-byte fields

	// start with these eight bytes, for easy access in assembly
	context_t *context;		// pointer to context save area on stack
	stack_t *stack;			// pointer to process stack

	status_t exit_status;	// termination status, for parent's use
	time_t wakeup;			// wakeup time, for sleeping processes
	
	// two-byte fields
	//
	pid_t pid;				// PID of this process
	pid_t ppid;				// PID of our parent process
	
	// one-byte fields
	//
	state_t state;			// process state
	uint8_t ticks_left;		// ticks remaining in the current time slice
	prio_t priority;		// process priority
	
	// filler, to round us up to 32 bytes
	// adjust this as fields are added/removed/changed
	uint8_t filler[9];	

};

#define	SZ_PCB	sizeof(pcb_t)

/*
** Globals
*/

// the process table
extern pcb_t _processes[N_PROCS];

// next available PID
extern pid_t _next_pid;

// pointer to the PCB for the 'init' process
extern pcb_t *_init_pcb;

/*
** Prototypes
*/

/**
** Name:	_pcb_init
**
** Initializes the process module.
**
** Dependencies:
**	Must be called before any process creation is done.
*/
void _pcb_init( void );

/**
** Name:	_pcb_alloc
**
** Allocates a PCB structure
**
** @return A pointer to a "clean" PCB, or NULL
*/
pcb_t *_pcb_alloc( void );

/**
** Name:	_pcb_dealloc
**
** Returns a PCB to the "available" list
**
** @param pcb  The PCB to be deallocated
*/
void _pcb_dealloc( pcb_t *pcb );

/**
** Name:	_pcb_find
**
** Locate a PCB by PID
**
** @param pid   The PID we want to locate
**
** @return A pointer to the desired PCB, or NULL
*/
pcb_t *_pcb_find( pid_t pid );

/**
** Name:	_pcb_cleanup
**
** Reclaim a process' data structures
**
** @param pcb   The PCB to reclaim
*/
void _pcb_cleanup( pcb_t *pcb );

/**
** Name:	_pcb_zombify
**
** Do the real work for exit() and some kill() calls
**
** Also called from the scheduler and the dispatcher.
**
** @param victim  Pointer to the PCB for the exiting process
*/
void _pcb_zombify( pcb_t *victim );

/*
** Debugging/tracing routines
*/

/**
** Name:	_pcb_dump
**
** Dumps the contents of this PCB to the console
**
** @param msg   An optional message to print before the dump
** @param p     The PCB to dump out
*/
void _pcb_dump( const char *msg, register pcb_t *p );

/**
** Name:	_ctx_dump
**
** Dumps the contents of this process context to the console
**
** @param msg   An optional message to print before the dump
** @param c     The context to dump out
*/
void _ctx_dump( const char *msg, register context_t *c );

/**
** Name:	_ctx_dump_all
**
** dump the process context for all active processes
**
** @param msg  Optional message to print
*/
void _ctx_dump_all( const char *msg );

/**
** Name:	_ptable_dump
**
** dump the contents of the "active processes" table
**
** @param msg  Optional message to print
** @param all  Dump all or only part of the relevant data
*/
void _ptable_dump( const char *msg, bool_t all );

#endif
// !SP_ASM_SRC

#endif
