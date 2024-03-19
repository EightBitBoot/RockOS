/**
** @file	clock.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Clock module implementation
*/

#define SP_KERNEL_SRC

#include "common.h"

#include "clock.h"

#include "procs.h"
#include "queues.h"
#include "sched.h"
#include "sio.h"
#include "stacks.h"
#include "kernel.h"
#include "syscalls.h"

#include "x86arch.h"
#include "x86pic.h"
#include "x86pit.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// pinwheel control variables
static uint32_t _pinwheel;   // pinwheel counter
static uint32_t _pindex;     // index into pinwheel string

/*
** PUBLIC GLOBAL VARIABLES
*/

// current system time
time_t _system_time;

// queue of sleeping processes
queue_t _sleeping;

/*
** PRIVATE FUNCTIONS
*/

/**
** Name:  _clk_isr
**
** The ISR for the clock
**
** @param vector    Vector number for the clock interrupt
** @param code      Error code (0 for this interrupt)
*/
static void _clk_isr( int vector, int code ) {

    // spin the pinwheel

    ++_pinwheel;
    if( _pinwheel == (CLOCK_FREQUENCY / 10) ) {
        _pinwheel = 0;
        ++_pindex;
        __cio_putchar_at( 0, 0, "|/-\\"[ _pindex & 3 ] );
    }

#if defined(SYSTEM_STATUS)
    // Periodically, dump the queue lengths and the SIO status (along
    // with the SIO buffers, if non-empty).
    //
    // Define the symbol SYSTEM_STATUS with a value equal to the desired
    // reporting frequency, in seconds.

    if( (_system_time % SEC_TO_TICKS(SYSTEM_STATUS)) == 0 ) {
		__cio_printf_at( 1, 0, " queues: RQ[%d,%d,%d] SL[%d] SIO[%d]   ",
				QUE_LENGTH(&_ready[SysPrio]),
				QUE_LENGTH(&_ready[UserPrio]),
				QUE_LENGTH(&_ready[DeferredPrio]),
				QUE_LENGTH(&_sleeping),
				QUE_LENGTH(&_sio_readq)
				);
	}

#endif

    // time marches on!
    ++_system_time;

	// wake up any sleeping processes whose time has come
	//
	// we give them preference over the current process (when
	// it is scheduled again)

	do {

		// if there isn't anyone in the sleep queue, we're done
		if( QUE_IS_EMPTY(&_sleeping) ) {
			break;
		}

		// peek at the first member of the queue
		pcb_t *pcb = NULL;
		assert(_que_peek( &_sleeping, (void **) &pcb ) == S_OK );

		// the retrieved PCB's wakeup time is the earliest time for
		// any process on the sleep queue; if that's greater than
		// the current time, there's nobody left to awaken

		if( pcb->wakeup > _system_time ) {
			break;
		}

		// OK, we need to wake someone up
		assert( _que_remove( &_sleeping, (void **) &pcb ) == S_OK );
		assert( _schedule(pcb) == S_OK );

	} while( 1 );

	// next, we decrement the current process' remaining quantum
	_current->ticks_left -= 1;

	// has it expired?
	if( _current->ticks_left < 1 ) {
		// yes! reschedule it
		assert( _schedule(_current) == S_OK );
		// pick a new current process
		_dispatch();
	}

    // tell the PIC we're done
    __outb( PIC_PRI_CMD_PORT, PIC_EOI );
}

/**
** Name:	_ord_wakeup
**
** Compare two PCB wakeup times.
**
** @param p1   first PCB
** @param p2   second PCB
**
** @return integer indicating the relationship between the wakeup times:
**   < 0 --> 'p1' <  'p2'
**   = 0 --> 'p1' == 'p2'
**   > 0 --> 'p1' >  'p2'
*/
static int _ord_wakeup( void *p1, void *p2 )
{
	uint32_t w1 = ((pcb_t *)p1)->wakeup;
	uint32_t w2 = ((pcb_t *)p2)->wakeup;

	if( w1 < w2 )
		return -1;
	else if( w1 == w2 )
		return 0;
	else
		return 1;
}

/*
** PUBLIC FUNCTIONS
*/

/**
** Name:  _clk_init
**
** Initializes the clock module
**
*/
void _clk_init( void ) {

    // start the pinwheel
    _pinwheel = (CLOCK_FREQUENCY / 10) - 1;
    _pindex = 0;

    // return to the dawn of time
    _system_time = 0;

	// configure the sleep queue
	_que_create( &_sleeping, _ord_wakeup );

    // configure the clock
    uint32_t divisor = PIT_FREQUENCY / CLOCK_FREQUENCY;
    __outb( PIT_CONTROL_PORT, PIT_0_LOAD | PIT_0_SQUARE );
    __outb( PIT_0_PORT, divisor & 0xff );        // LSB of divisor
    __outb( PIT_0_PORT, (divisor >> 8) & 0xff ); // MSB of divisor

    // register the second-stage ISR
    __install_isr( INT_VEC_TIMER, _clk_isr );

    __cio_puts( " CLK" );
}
