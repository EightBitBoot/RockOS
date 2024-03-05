/**
** @file	clock.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Clock module implementation
*/

#define SP_KERNEL_SRC

#include "x86arch.h"
#include "x86pic.h"
#include "x86pit.h"

#include "common.h"

#include "clock.h"
#include "kernel.h"
#include "procs.h"
#include "queues.h"
#include "sched.h"
#include "sio.h"
#include "syscalls.h"

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

#if defined(STATUS)
    // Periodically, dump the queue lengths and the SIO status (along
    // with the SIO buffers, if non-empty).
    //
    // Define the symbol STATUS with a value equal to the desired
    // reporting frequency, in seconds.

    // TODO whatever we want to do here

#endif

    // time marches on!
    ++_system_time;

	// TODO check sleep queue?
	// TODO deal with current process' quantum?
	// TODO dispatch new process if necessary?

    // tell the PIC we're done
    __outb( PIC_PRI_CMD_PORT, PIC_EOI );
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

    // configure the clock
    uint32_t divisor = PIT_FREQUENCY / CLOCK_FREQUENCY;
    __outb( PIT_CONTROL_PORT, PIT_0_LOAD | PIT_0_SQUARE );
    __outb( PIT_0_PORT, divisor & 0xff );        // LSB of divisor
    __outb( PIT_0_PORT, (divisor >> 8) & 0xff ); // MSB of divisor

    // register the second-stage ISR
    __install_isr( INT_VEC_TIMER, _clk_isr );

    __cio_puts( " CLK" );
}
