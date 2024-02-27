/**
** @file	sched.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Scheduler implementation
*/

#define	SP_KERNEL_SRC

#include "common.h"

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
	// TODO anything else we need to do here?

	// create all the ready queues as FIFO queues
	for( int i = 0; i < N_PRIOS; ++i ) {
		_que_create( &_ready[i], NULL );
	}

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
	// TODO sanity check?
	
	// get the scheduling priority for this process
	prio_t n = pcb->prio;

	// TODO check the priority for validity?

	// mark the process as ready to execute
	pcb->state = Ready;

	// add the process to the relevant queue
	return _que_insert( &_ready[n], pcb );

/**
** Name:	_dispatch()
**
** Select the next process to run from the ready queue.
*/
status_t _dispatch( void )
{
	// TODO
}

#endif
// !SP_ASM_SRC

#endif
