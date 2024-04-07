/**
** @file	sched.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Scheduler implementation
*/

#define	SP_KERNEL_SRC

#include "common.h"

#include "kernel.h"
#include "sched.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

// the ready queue
queue_t _ready[N_PRIOS];

// the currently-executing process
pcb_t *_current;

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/**
** Name:	_sch_init()
**
** Initialize the scheduler.
*/
void _sch_init( void )
{
	// create all the ready queues as FIFO queues
	for( int i = 0; i < N_PRIOS; ++i ) {
		_que_create( &_ready[i], NULL );
	}

	// there is no current process (yet)
	_current = NULL;

	__cio_puts( " SCH" );
}

/**
** Name:	_schedule(pcb)
**
** Schedule a process. Adds the supplied PCB to the ready queue.
**
** @param pcb   The PCB to be scheduled
**
** @return Status of the schedule attempt.
*/
status_t _schedule( pcb_t *pcb )
{
	// sanity check?
	assert1( pcb != NULL );

	// if this process has been killed, zombify it
	if( pcb->state == Killed ) {
		_pcb_zombify( pcb );
		return S_OK;
	}
	
	// get the scheduling priority for this process
	prio_t n = pcb->priority;

	// check the priority for validity?
	assert( n >= SysPrio && n < N_PRIOS );

	// mark the process as ready to execute
	pcb->state = Ready;

	// add the process to the relevant queue
	return _que_insert( &_ready[n], pcb );
}

/**
** Name:	_dispatch()
**
** Select the next process to run from the ready queue.
*/
void _dispatch( void )
{
	pcb_t *pcb = NULL;

	do {

		// find an available process to dispatch
		int n = SysPrio;
		while( n < N_PRIOS && QUE_IS_EMPTY(&_ready[n]) ) {
			++n;
		}

		// if we don't have one, we are in deep trouble!
		assert( n < N_PRIOS );

		// found one; pull it off the queue, but blow up
		// if that fails
		assert( _que_remove(&_ready[n],(void **)&pcb) == S_OK );

		// if this process has been killed, zombify it
		if( pcb->state == Killed ) {
			// get rid of it
			_pcb_zombify( pcb );
			// keep looking
			pcb = NULL;
		}

	} while( pcb == NULL );

	// found one - make it the current process
	_current = pcb;

	// now a running process with the standard quantum
	_current->state = Running;
	_current->ticks_left = Q_STD;
}
