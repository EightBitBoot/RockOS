/**
** @file	sched.h
**
** @author	CSCI-452 class of 20235
**
** @brief	Scheduler declarations
*/

#ifndef SCHED_H_
#define SCHED_H_

#include "common.h"

#include "procs.h"
#include "queues.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Types
*/

/*
** Globals
*/

// the ready queue
extern queue_t _ready[N_PRIOS];

// the currently-executing process
extern pcb_t *_current;

/*
** Prototypes
*/

/**
** Name:	_sch_init()
**
** Initialize the scheduler.
*/
void _sch_init( void );

/**
** Name:	_schedule(pcb)
**
** Schedule a process. Adds the supplied PCB to the ready queue.
**
** @param pcb   The PCB to be scheduled
**
** @return Status of the schedule attempt.
*/
status_t _schedule( pcb_t *pcb );

/**
** Name:	_dispatch()
**
** Select the next process to run from the ready queue.
*/
void _dispatch( void );

#endif
// !SP_ASM_SRC

#endif
