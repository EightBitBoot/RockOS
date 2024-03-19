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
#include "kernel.h"
#include "stacks.h"
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

// the number of available PCBs
static uint32_t _avail_pcbs;


/*
** PUBLIC GLOBAL VARIABLES
*/

// the process table
pcb_t _processes[N_PROCS];

// next available PID
pid_t _next_pid;

// pointer to the PCB for the 'init' process
pcb_t *_init_pcb;

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
	CLEAR( _processes );

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
#if TRACING_PCB
	__cio_puts( "** _pcb_alloc(), _avail_pcbs %d\n", _avail_pcbs );
#endif

	// can't allocate one if there aren't any available
	if( _avail_pcbs < 1 ) {
#if TRACING_PCB
		__cio_puts( "** ALLOC FAILED\n" );
#endif
		return NULL;
	}
	
	// find the first available PCB structure
	register int i = 0;
	register pcb_t *pcb = _processes;
	while( i < N_PROCS && pcb->state != Unused ) {
		++i;
		++pcb;
	}

	// sanity check - if we ran off the end of the
	// PCB array without finding a free one, there's
	// a consistency problem because _avail_pcbs indicated
	// that there should have been at least one available
	assert( i < N_PROCS );
	
	// zero out the memory and mark this PCB as "in use"
	CLEAR_PTR( pcb );
	pcb->state = New;
	
	// one fewer PCB in the pool
	_avail_pcbs -= 1;

#if TRACING_PCB
	__cio_puts( "** _pcb_alloc() ret #%d,_avail_pcbs %d\n", i, _avail_pcbs );
#endif
	
	// return the PCB to the caller
	return pcb;
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
#if TRACING_PCB
	__cio_printf( "** _pcb_dealloc(%08x), avail %d\n", data, _avail_pcbs );
#endif
	// sanity check?
	assert1( pcb != NULL );
	
	// one more PCB we can allocate
	_avail_pcbs += 1;
		
	// we do the least amount of work necessary here;
	// if/when this PCB is re-used, we'll clear the
	// rest of it when it's allocated
	pcb->state = Unused;		// PCB is inactive
	pcb->pid = pcb->ppid = 0;	// guard against finding it accidentally

#if TRACING_PCB
	__cio_printf( "** _pcb_dealloc(), avail now %d\n", _avail_pcbs );
#endif
}

/**
** Name:	_pcb_find
**
** Locate a PCB by PID
**
** @param pid   The PID we want to locate
**
** @return pointer to the desired PCB, or NULL
*/
pcb_t *_pcb_find( pid_t pid )
{

#if TRACING_PCB
	__cio_printf( "** _pcb_find(%u)", pid );
#endif

	// iterate through the PCB table
	register pcb_t *pcb = _processes;

	for( int i = 0; i < N_PROCS; ++i, ++pcb ) {

		// if this is the one we want, we're done
		if( pcb->state != Unused && pcb->pid == pid ) {
#if TRACING_PCB
			__cio_printf( ", slot #%d\n", i );
#endif
			return pcb;
		}

	}

#if TRACING_PCB
	__cio_puts( ", not found\n" );
#endif
	return NULL;
}

/**
** Name:	_pcb_cleanup
**
** Reclaim a process' data structures
**
** @param pcb   The PCB to reclaim
*/
void _pcb_cleanup( pcb_t *pcb )
{

#if TRACING_PCB
	__cio_printf( "** _pcb_cleanup(0x%08x)\n", (uint32_t) pcb );
#endif

	// avoid deallocating a NULL pointer
	if( pcb == NULL ) {
		// should this be an error?
		return;
	}

	// release the stack(en?)
	if( pcb->stack != NULL ) {
		_stk_dealloc( pcb->stack );
	}

	// release the PCB
	_pcb_dealloc( pcb );
}

/**
** Name:	_pcb_zombify
**
** Do the real work for exit() and some kill() calls
**
** Also called from the scheduler and the dispatcher.
**
** @param victim  Pointer to the PCB for the exiting process
*/
void _pcb_zombify( pcb_t *victim )
{
	// should this be an error?
	if( victim == NULL ) {
		return;
	}

#if TRACING_EXIT
	__cio_printf( "** zombify(0x%08x) pid %u ppid %u\n",
				(uint32_t) victim, victim->pid, victim->ppid );
#endif

	// set its state
	victim->state = Zombie;

	/*
	** We need to locate the parent of this process.  We also need
	** to reparent any children of this process.  We do these in
	** a single loop.
	*/
	pcb_t *parent = NULL;
	pcb_t *zombie = NULL;

	// two PIDs we will look for
	pid_t vicpid = victim->pid;
	pid_t vicparent = victim->ppid;

	// speed up access to the process table entries
	register pcb_t *curr = _processes;

	for( int i = 0; i < N_PROCS; ++i, ++curr ) {

		// make sure this is a valid entry
		if( curr->state == Unused ) {
			continue;
		}

		// see if this is our parent; if it isn't, see if
		// it's a child of the terminating process
		if( curr->pid == vicparent ) {

			// found the parent; make sure we hadn't already found it!
			assert( parent == NULL );
			parent = curr;

			// NOTE: we don't break out of the 'for' loop here because
			// we want to continue iterating to find all the children
			// of this process.

		} else if( curr->ppid == vicpid ) {

			// found a child - reparent it
			curr->ppid = PID_INIT;

			// see if this child is already undead
			if( curr->state == Zombie ) {
				// if it's already a zombie, remember it, so we
				// can pass it on to 'init'; also, if there are
				// two or more zombie children, it doesn't matter
				// which one we pick here, as the others will be
				// collected as 'init' loops
				zombie = &(_processes[i]);
			}

		}
	}

	// every process must have a parent, even if it's 'init'
	assert( parent != NULL );

	/*
	** If we found a child that was already terminated, we need to
	** wake up the init process if it's already waiting.
	**
	** Note: we only need to do this for one Zombie child process -
	** init will loop and collect the others after it finishes with
	** this one.
	**
	** Also note: it's possible that the exiting process' parent is
	** also init, which means we're letting one of zombie children
	** of the exiting process be cleaned up by init before the 
	** existing process itself is cleaned up by init. This will work,
	** because after init cleans up the zombie, it will loop and
	** call waitpid() again, by which time this exiting process will
	** be marked as a zombie.
	*/
	if( zombie != NULL && _init_pcb->state == Waiting ) {

		// *****************************************************
		// This code assumes that Waiting processes are *not* in
		// a queue, but instead are just in the process table with
		// a state of 'Waiting'.  This simplifies life immensely,
		// because we don't need to dequeue it - we can just
		// schedule it and let it go.
		// *****************************************************

		// intrinsic return value is the PID
		RET(_init_pcb) = zombie->pid;

		// may also want to return the exit status
		int32_t *ptr = (int32_t *) ARG(_init_pcb,2);

		if( ptr != NULL ) {
			// *****************************************************
			// Potential VM issue here!  This code assigns the exit
			// status into a variable in the parent's address space.
			// This works in the baseline because we aren't using
			// any type of memory protection.  If address space
			// separation is implemented, this code will very likely
			// STOP WORKING, and will need to be fixed.
			// *****************************************************
			*ptr = zombie->exit_status;
		}
#if TRACING_EXIT
		__cio_printf( "** zombify zombie %d given to init\n", zombie->pid );
#endif

		// all done - schedule 'init', and clean up the zombie
		_schedule( _init_pcb );
		_pcb_cleanup( zombie );
	}

	/*
	** Now, deal with the parent of this process. If the parent is
	** already waiting, just wake it up and clean up this process.
	** Otherwise, this process becomes a zombie.
	**
	** Note: if the exiting process' parent is init and we just woke
	** init up to deal with a zombie child of the exiting process,
	** init's status won't be Waiting any more, so we don't have to
	** worry about it being scheduled twice.
	*/

	if( parent->state == Waiting ) {

		// verify that the parent is either waiting for this process
		// or is waiting for any of its children
		uint32_t target = ARG(parent,1);

		if( target == 0 || target == vicpid ) {

			// the parent is waiting for this child or is waiting
			// for any of its children, so we can wake it up.

			// intrinsic return value is the PID
			RET(parent) = vicpid;

			// may also want to return the exit status
			int32_t *ptr = (int32_t *) ARG(parent,2);

			if( ptr != NULL ) {
				/*
				********************************************************
				** Potential VM issue here!  This code assigns the exit
				** status into a variable in the parent's address space.
				** This works in the baseline because we aren't using
				** any type of memory protection.  If address space
				** separation is implemented, this code will very likely
				** STOP WORKING, and will need to be fixed.
				********************************************************
				*/
				*ptr = victim->exit_status;
			}

#if TRACING_EXIT
			__cio_printf( "** zombify victim %u given to parent %u\n",
						vicpid, vicparent );
#endif

			// all done - schedule the parent, and clean up the zombie
			_schedule( parent );
			_pcb_cleanup( victim );

			return;
		}

	}

	/*
	** The parent isn't waiting OR is waiting for a specific child
	** that isn't this exiting process, so we become a Zombie.
	**
	** This code assumes that Zombie processes are *not* in
	** a queue, but instead are just in the process table with
	** a state of 'Zombie'.  This simplifies life immensely,
	** because we won't need to dequeue it when it is collected
	** by its parent.
	*/

	victim->state = Zombie;

	/*
	** Note: we don't call _dispatch() here - we leave that for
	** the calling routine, as it's possible we don't need to
	** choose a new current process.
	*/
}

/*
** Debugging/tracing routines
*/

/**
** _pcb_dump(msg,pcb)
**
** Dumps the contents of this PCB to the console
**
** @param msg   An optional message to print before the dump
** @param p     The PCB to dump out
*/
void _pcb_dump( const char *msg, register pcb_t *p ) {

	// first, the message (if there is one)
	if( msg ) {
		__cio_puts( msg );
	}

	// the pointer
	__cio_printf( " @ %08x: ", (uint32_t) p );

	// if it's NULL, why did you bother calling me?
	if( p == NULL ) {
		__cio_puts( " NULL???\n" );
		return;
	}

	// now, the contents
	__cio_printf( " pids %d/%d state %d prio %d",
				  p->pid, p->ppid, p->state, p->priority );

	__cio_printf( "\n ticks %d xit %d wake %08x",
				  p->ticks_left, p->exit_status, p->wakeup );

	__cio_printf( "\n context %08x stack %08x",
				  (uint32_t) p->context, (uint32_t) p->stack );

	// and the filler (just to be sure)
	__cio_puts( " fill: " );
	for( int i = 0; i < sizeof(p->filler); ++i ) {
		__cio_printf( "%02x", p->filler[i] );
	}
	__cio_putchar( '\n' );
}

/**
** _ctx_dump(msg,context)
**
** Dumps the contents of this process context to the console
**
** @param msg   An optional message to print before the dump
** @param c     The context to dump out
*/
void _ctx_dump( const char *msg, register context_t *c ) {

	// first, the message (if there is one)
	if( msg ) {
		__cio_puts( msg );
	}

	// the pointer
	__cio_printf( " @ %08x: ", (uint32_t) c );

	// if it's NULL, why did you bother calling me?
	if( c == NULL ) {
		__cio_puts( " NULL???\n" );
		return;
	}

	// now, the contents
	__cio_printf( "ss %04x gs %04x fs %04x es %04x ds %04x cs %04x\n",
				  c->ss & 0xff, c->gs & 0xff, c->fs & 0xff,
				  c->es & 0xff, c->ds & 0xff, c->cs & 0xff );
	__cio_printf( "  edi %08x esi %08x ebp %08x esp %08x\n",
				  c->edi, c->esi, c->ebp, c->esp );
	__cio_printf( "  ebx %08x edx %08x ecx %08x eax %08x\n",
				  c->ebx, c->edx, c->ecx, c->eax );
	__cio_printf( "  vec %08x cod %08x eip %08x eflags %08x\n",
				  c->vector, c->code, c->eip, c->eflags );
}

/**
** _ctx_dump_all(msg)
**
** dump the process context for all active processes
**
** @param msg  Optional message to print
*/
void _ctx_dump_all( const char *msg ) {

	if( msg != NULL ) {
		__cio_puts( msg );
	}

	int n = 0;
	register pcb_t *pcb = _processes;
	for( int i = 0; i < N_PROCS; ++i, ++pcb ) {
		if( pcb->state != Unused ) {
			++n;
			__cio_printf( "%2d(%d): ", n, pcb->pid );
			_ctx_dump( NULL, pcb->context );
		}
	}
}

/**
** _ptable_dump(msg,all)
**
** dump the contents of the "active processes" table
**
** @param msg  Optional message to print
** @param all  Dump all or only part of the relevant data
*/
void _ptable_dump( const char *msg, bool_t all ) {

	if( msg ) {
		__cio_puts( msg );
	}
	__cio_putchar( ' ' );

	int used = 0;
	int empty = 0;

	register pcb_t *pcb = _processes;
	for( int i = 0; i < N_PROCS; ++i ) {
		if( pcb->state == Unused ) {

			// an empty slot
			++empty;

		} else {

			// a non-empty slot
			++used;

			// if not dumping everything, add commas if needed
			if( !all && used ) {
				__cio_putchar( ',' );
			}

			// things that are always printed
			__cio_printf( " #%d: %d/%d", i, pcb->pid, pcb->ppid );
			if( pcb->state >= N_STATES ) {
				__cio_printf( " UNKNOWN" );
			} else {
				__cio_printf( " %s", _state_str[pcb->state][ST_S_NAME] );
			}
			// do we want more info?
			if( all ) {
				__cio_printf( " wk %08x stk %08x ESP %08x EIP %08x\n",
						pcb->wakeup, (uint32_t) pcb->stack,
						pcb->context->esp,
						pcb->context->eip );
			}
		}
	}
	// only need this if we're doing one-line output
	if( !all ) {
		__cio_putchar( '\n' );
	}

	// sanity check - make sure we saw the correct number of table slots
	if( (used + empty) != N_PROCS ) {
		__cio_printf( "Table size %d, used %d + empty %d = %d???\n",
					  N_PROCS, used, empty, used + empty );
	}
}
