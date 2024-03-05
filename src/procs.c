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
