/*
** @file	procs.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Process-related implementations
*/

#define	SP_KERNEL_SRC

#include "common.h"

#include "procs.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// the number of available PCBs
static uint32_t _avail_pcbs;


/*
** PUBLIC GLOBAL VARIABLES
*/

// the process table
pcb_t _processes[N_PROCS];

// next available PID
pid_t _next_pid;

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/**
** Name:	_pcb_init
**
** Initializes the process module.
**
** Dependencies:
**	Must be called before any process creation is done.
*/
void _pcb_init( void )
{
	// clear out all the PCBs
	__memclr( _processes, sizeof(_processes) );

	// reset the "free" count
	_avail_pcbs = N_PROCS;

	// reset the PID counter
	_next_pid = FIRST_USER_PID;

	// report that we're done
	__cio_puts( " PCB" );
}

/**
** Name:	_pcb_alloc
**
** Allocates a PCB structure
**
** @return A pointer to a "clean" PCB, or NULL
*/
pcb_t *_pcb_alloc( void )
{
	// can't allocate one if there aren't any available
	if( _avail_pcbs < 1 ) {
		return NULL;
	}
	
	// find the first available PCB structure
	register int i = 0;
	while( i < N_PROCS && _processes[i].state != Unused ) {
		++i;
	}

	// sanity check - if we ran off the end of the
	// PCB array without finding a free one, there's
	// a consistency problem because _active indicated
	// that there should have been at least one available
	assert( i < N_PROCS );
	
	// zero out the memory and mark this PCB as "in use"
	__memclr( &_processes[i], sizeof(pcb_t) );
	_processes[i].state = New;
	
	// one fewer PCB in the pool
	_avail_pcbs -= 1;
	
	// return the PCB to the caller
	return &_processes[i];
}

/**
** Name:	_pcb_dealloc
**
** Returns a PCB to the "available" list
**
** @param pcb  The PCB to be deallocated
*/
void _pcb_dealloc( pcb_t *pcb )
{
	// sanity check?
	assert1( pcb != NULL );
	
	// one more PCB we can allocate
	_avail_pcbs += 1;
		
	// we do the least amount of work necessary here;
	// if/when this PCB is re-used, we'll clear the
	// rest of it when it's allocated
	pcb->state = Unused;
}
