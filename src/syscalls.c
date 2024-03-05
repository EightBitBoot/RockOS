/**
** @file	syscalls.c
**
** @author	CSCI-452 class of 20235
**
** @brief	System call implementations
*/

#define SP_KERNEL_SRC

#include "common.h"

#include "x86arch.h"
#include "x86pic.h"
#include "uart.h"

#include "support.h"
#include "bootstrap.h"

#include "syscalls.h"
#include "sched.h"
#include "procs.h"
#include "stacks.h"
#include "clock.h"
#include "cio.h"
#include "sio.h"

/*
** PRIVATE DEFINITIONS
*/

// macros to simplify tracing a bit
//
// TRACING_SYSCALLS and TRACING_SYSRETS are defined in kdefs.h
// If not tracing syscalls, SYSCALL_ENTER is a no-op, and
// SYSCALL_EXIT just does a return.

#if TRACING_SYSCALLS
#define SYSCALL_ENTER(x)	do { \
		__cio_printf( "--> %s, pid %08x", __func__, (uint32_t) (x) ); \
	} while(0)
#else
#define SYSCALL_ENTER(x)	/* */
#endif

#if TRACING_SYSRETS
#define SYSCALL_EXIT(x)	do { \
		__cio_printf( "<-- %s %08x\n", __func__, (uint32_t) (x) ); \
		return; \
	} while(0)
#else
#define SYSCALL_EXIT(x)	return
#endif

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// prototypes for the entry point functions

// a macro to simplify syscall entry point specification
#define	SYSIMPL(x)		static void _sys_##x( void )

SYSIMPL(exit);		SYSIMPL(sleep);		SYSIMPL(read);		SYSIMPL(write);
SYSIMPL(waitpid);	SYSIMPL(getdata);	SYSIMPL(setdata);	SYSIMPL(kill);
SYSIMPL(fork);		SYSIMPL(exec);

// The system call jump table
//
// Initialized using designated initializers to ensure the entries
// are correct even if the syscall code values should happen to change.
//
// TODO: fix the parameter specification if needed

static void (* const _syscalls[N_SYSCALLS])( void ) = {
	[ SYS_exit    ] = _sys_exit,
	[ SYS_sleep   ] = _sys_sleep,
	[ SYS_read    ] = _sys_read,
	[ SYS_write   ] = _sys_write,
	[ SYS_waitpid ] = _sys_waitpid,
	[ SYS_getdata ] = _sys_getdata,
	[ SYS_setdata ] = _sys_setdata,
	[ SYS_kill    ] = _sys_kill,
	[ SYS_fork    ] = _sys_fork,
	[ SYS_exec    ] = _sys_exec
};

/*
** PUBLIC GLOBAL VARIABLES
*/

// queue of sleeping processes
queue_t _sleeping;

/*
** PRIVATE FUNCTIONS
*/

/**
** Name:  _sys_isr
**
** System call ISR
**
** @param vector    Vector number for the clock interrupt
** @param code      Error code (0 for this interrupt)
*/
static void _sys_isr( int vector, int code ) {

	// Keep the compiler happy.
	(void) vector;
	(void) code;

	// TODO: replace this comment with the real implementation
	
	// Tell the PIC we're done.
	__outb( PIC_PRI_CMD_PORT, PIC_EOI );
}

/**
** Second-level syscall handlers
**
** All have this prototype:
**
**		static void _sys_NAME( void );
**
** Values being returned to the user are placed into the EAX
** field in the context save area for that process.
*/

/**
** _sys_exit - terminate the calling process
**
** implements:
**		void exit( int32_t status );
**
** does not return
*/
SYSIMPL(exit)
{
	// TODO: implement
}

/**
** _sys_sleep - put the current process to sleep for some length of time
**
** implements:
**		void sleep( uint32_t ms );
**
** if ms == 0, just yields the CPU
*/
SYSIMPL(sleep)
{
	// TODO: implement this
}

/**
** _sys_read - read into a buffer from a stream
**
** implements:
**		int32_t read( uint32_t chan, void *buffer, uint32_t length );
**
** returns:
**		input data (in 'buffer')
**		number of bytes read, or an error code (intrinsic)
*/
SYSIMPL(read)
{
	// TODO: implement this
}

/**
** _sys_write - write from a buffer to a stream
**
** implements:
**		int32_t write( uint32_t chan, const void *buffer, uint32_t length );
**
** returns:
**		number of bytes written, or an error code (intrinsic)
*/
SYSIMPL(write)
{
	// TODO: implement this
}

/**
** _sys_waitpid - wait for a child process to terminate
**
** implements:
**		int32_t waitpid( pid_t id, int32_t *status );
**
** returns:
**		pid of the terminated child, or E_NO_CHILDREN (intrinsic)
**		exit status of the child via a non-NULL 'status' parameter
*/
SYSIMPL(waitpid)
{
	// TODO: implement this
}

/**
** _sys_getdata - retrieve some OS or process information
**
** implements:
**		int32_t getdata( datum_t which );
**
** returns:
**		the requested information, or -1 on error
*/
SYSIMPL(getdata)
{
	// TODO: implement this
}

/**
** _sys_setdata - modify some OS or process value
**
** implements:
**		int32_t setdata( datum_t which, int32_t value );
**
** returns:
**		the old value, or -1 on error
*/
SYSIMPL(setdata)
{
	// TODO: implement this
}

/**
** _sys_kill - terminate a process with extreme prejudice
**
** implements:
**		int32_t kill( pid_t victim );
**
** returns:
**		status of the kill attempt
*/
SYSIMPL(kill)
{
	// TODO: implement this
}

/**
** _sys_fork - create a new process
**
** implements:
**		int32_t fork( void );
**
** returns:
**		parent - PID of new child, or -1 on error
**		child  - 0
*/
SYSIMPL(fork)
{
	// TODO: implement this
}

/**
** _sys_exec - replace the memory image of a process
**
** implements:
**		void exec( userfcn_t entry, char *args[] );
**
** returns:
**		only on failure
*/
SYSIMPL(exec)
{
	// TODO: implement this
}

/*
** PUBLIC FUNCTIONS
*/

/**
** Name:  _sys_init
**
** Syscall module initialization routine
**
** Dependencies:
**    Must be called after _cio_init()
*/
void _sys_init( void ) {

	__cio_puts( " Sys" );

	// allocate a queue for sleeping processes
	// TODO create the queue
	// TODO write the ordering function for the sleep queue

	// install the second-stage ISR
	__install_isr( INT_VEC_SYSCALL, _sys_isr );
}
