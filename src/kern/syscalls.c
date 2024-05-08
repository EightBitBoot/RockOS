/**
** @file	syscalls.c
**
** @author	CSCI-452 class of 20235
**
** @brief	System call implementations
*/

#include "params.h"
#include "util/kstring.h"
#include "vfs/vfs.h"
#define SP_KERNEL_SRC

#include "common.h"

#include "x86arch.h"
#include "x86pic.h"
#include "io/uart.h"

#include "support.h"
#include "bootstrap.h"

#include "syscalls.h"
#include "sched.h"
#include "procs.h"
#include "mem/stacks.h"
#include "clock.h"
#include "io/cio.h"
#include "io/sio.h"
#include "io/vgatext.h"
#include "acpi/acpi.h"
#include "io/vga.h"

#include "vfs/namey.h"

/*
** PRIVATE DEFINITIONS
*/

// macros to simplify tracing a bit
//
// TRACING_SYSCALLS is defined in kdefs.h and controlled by the
// TRACE macro. If not tracing syscalls, SYSCALL_ENTER is a no-op,
// and SYSCALL_EXIT just does a return.

#if TRACING_SYSCALLS

#define SYSCALL_ENTER(x)	do { \
		__cio_printf( "--> %s, pid %08x", __func__, (uint32_t) (x) ); \
	} while(0)

#define SYSCALL_EXIT(x)	do { \
		__cio_printf( "<-- %s %08x\n", __func__, (uint32_t) (x) ); \
		return; \
	} while(0)

#else

#define SYSCALL_ENTER(x)	/* */

#define SYSCALL_EXIT(x)	return

#endif

/*
** PRIVATE DATA TYPES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS AND GLOBAL VARIABLES
*/

// a macro to simplify syscall entry point specification
#define	SYSIMPL(x)		static void _sys_##x( void )

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
	// retrieve the exit status of this process
	_current->exit_status = ARG(_current,1);

	// now, we need to do the following:
	// 	reparent any children of this process and wake up init if need be
	// 	find this process' parent and wake it up if it's waiting

	_pcb_zombify( _current );

	// pick a new winner
	_dispatch();
}

/**
** _sys_sleep - put the current process to sleep for some length of time
**
** implements:
**		int32_t sleep( uint32_t ms );
**
** if ms == 0, just yields the CPU
*/
SYSIMPL(sleep)
{
	// retrieve the sleep time
	uint32_t length = ARG(_current,1);
	status_t status;

	if( length == 0 ) {

		// special case: yield the CPU
		RET(_current) = E_SUCCESS;
		status = _schedule( _current );

		// if _schedule() failed, we need to notify someone
		if( status != S_OK ) {
			__sprint( _b256, "PID %u sleep(%u) sched failed, code %d\n",
					_current->pid, length, status );
			WARNING( _b256 );
			RET(_current) = E_FAILURE;
			// must return here - don't want to dispatch a new process
			return;
		}

	} else {

		// calculate the wakeup time
		_current->wakeup = _system_time + length;

		// add to the sleep queue
		status = _que_insert( &_sleeping, (void *) _current );

		// if the insertion failed, notify someone
		if( status != S_OK ) {
			// oops!
			__sprint( _b256, "PID %u sleep(%u) failed, code %d\n",
					_current->pid, length, status );
			WARNING( _b256 );
			RET(_current) = E_FAILURE;
			return;
		}

	}

	// pick a new process to run for a while
	_dispatch();
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
	SYSCALL_ENTER( _current->pid );

	// grab the arguments
	uint32_t chan = ARG(_current,1);
	char *buf = (char *) ARG(_current,2);
	uint32_t length = ARG(_current,3);

	// the simplest case is a buffer length of 0
	if( length == 0 ) {
		RET(_current) = 0;
		SYSCALL_EXIT( 0 );
	}

	// try to get the next character(s)
	int n = 0;

	switch( chan ) {

	case CHAN_CIO:
	 	// Quick and dirty fix for a cio bug:
		//
		// __cio_gets assumes it is getting a string of at least size 2
		// (1 char + null terminator) and will return 0 if it gets
		// length < 2. Because of how read is written, if n = 0 any time
		// after the cio input queue check (in sio _OR_ cio) then it will
		// put the current process in the sio sleep queue.  However, since
		// the calling process expected input from cio, an sio input will
		// likely never come.
		//
		// The easiest fix for this issue is to just check the buffer
		// length when reading from cio and return an error if it is too
		// small.
		if(length < 2) {
			RET(_current) = E_TOO_SMALL;
			SYSCALL_EXIT(E_TOO_SMALL);
		}

		// console input is non-blocking
		if( __cio_input_queue() < 1 ) {
			RET(_current) = E_NO_DATA;
			SYSCALL_EXIT( E_NO_DATA );
		}
		// at least one character
		n = __cio_gets( buf, length );
		break;

	case CHAN_SIO:
		// SIO input is blocking, so if there are no characters
		// available, we'll block this process
		n = _sio_read( buf, length );
		break;

	default:
		// bad channel code
		RET(_current) = E_BAD_PARAM;
		SYSCALL_EXIT( E_BAD_PARAM );
	}

	// if there was data, return the byte count to the process;
	// otherwise, block the process until data is available
	if( n > 0 ) {

		RET(_current) = n;
		SYSCALL_EXIT( n );

	} else {

		// mark it as blocked
		_current->state = Blocked;

		// put it on the SIO input queue
		assert1( _que_insert(&_sio_readq,(void *)_current) == S_OK );

		// select a new current process
		_dispatch();
	}
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
	// grab the parameters
	uint32_t chan = ARG(_current,1);
	char *buf = (char *) ARG(_current,2);
	uint32_t length = ARG(_current,3);

	SYSCALL_ENTER( _current->pid );

	// this is almost insanely simple, but it does separate the
	// low-level device access fromm the higher-level syscall implementation

	switch( chan ) {
	case CHAN_CIO:
		__cio_write( buf, length );
		RET(_current) = length;
		break;

	case CHAN_SIO:
		_sio_write( buf, length );
		RET(_current) = length;
		break;

	default:
		RET(_current) = E_BAD_CHAN;
		break;
	}

	SYSCALL_EXIT( RET(_current) );
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
	SYSCALL_ENTER( _current->pid );

	/*
	** We need to do two things here:  (1) find out whether or
	** not this process has any children in the system, and (2)
	** find out whether the desired child (or any child, if the
	** target PID is 0) has terminated.
	**
	** To do this, we loop until we find a the requested PID or
	** a Zombie child process, or have gone through all of the
	** slots in the process table.
	**
	** If the target PID is 0, we don't care which child process
	** we reap here; there could be several, but we only need to
	** find one.
	*/

	// verify that we aren't looking for ourselves!
	pid_t target = ARG(_current,1);

	if( target == _current->pid ) {
		RET(_current) = E_BAD_PARAM;
		SYSCALL_EXIT( E_BAD_PARAM );
	}

	// Good.  Now, figure out what we're looking for.

	pcb_t *child = NULL;

	if( target != 0 ) {

		// we're looking for a specific child
		child = _pcb_find( target );

		if( child != NULL ) {

			// found the process; is it one of our children:
			if( child->ppid != _current->pid ) {
				// NO, so we can't wait for it
				RET(_current) = E_BAD_PARAM;
				SYSCALL_EXIT( E_BAD_PARAM );
			}

			// yes!  is this one ready to be collected?
			if( child->state != Zombie ) {
				// no, so we'll have to block for now
				child = NULL;
			}

		} else {

			// no such child
			RET(_current) = E_BAD_PARAM;
			SYSCALL_EXIT( E_BAD_PARAM );

		}

	} else {

		// looking for any child
		int i;

		// we need to find a process that is our child
		// and has already exited

		child = NULL;
		bool_t found = false;

		// save a bit of time
		register pcb_t *curr = _processes;

		for( i = 0; i < N_PROCS; ++i, ++curr ) {

			if( curr != NULL && curr->ppid == _current->pid ) {

				// found one!
				found = true;

				// has it already exited?
				if( child->state == Zombie ) {
					// yes, so we're done here
					child = curr;
					break;
				}
			}
		}

		if( !found ) {
			// got through the loop without finding a child!
			RET(_current) = E_NO_CHILDREN;
			SYSCALL_EXIT( E_NO_CHILDREN );
		}

	}

	/*
	** At this point, one of these situations is true:
	**
	**  * we are looking for a specific child and found it
	**  * we are looking for any child and found one
	**
	** Either way, 'child' will be non-NULL if the selected
	** process has already become a Zombie.  If that's the
	** case, we collect its status and clean it up; otherwise,
	** we block this process.
	*/

	// did we find one to collect?
	if( child == NULL ) {

		// no - mark the parent as "Waiting"
		_current->state = Waiting;

		// select a new current process
		_dispatch();
		SYSCALL_EXIT( (uint32_t) _current );
	}

	// found a Zombie; collect its information and clean it up
	RET(_current) = child->pid;
	int32_t *stat = (int32_t *) ARG(_current,2);

	// if stat is NULL, the parent doesn't want the status
	if( stat != NULL ) {
		*stat = child->exit_status;
	}

	// clean up the child
	_pcb_cleanup( child );

	SYSCALL_EXIT( RET(_current) );
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
	// what does the user want to know?
	int32_t what = ARG(_current,1);

	// validate it
	if( what >= N_DATUMS ) {
		// bad code
		RET(_current) = E_BAD_PARAM;
	} else {
		// valid code, so return the requested info
		switch( what ) {
		case Pid:	RET(_current) = _current->pid; break;
		case PPid:	RET(_current) = _current->ppid; break;
		case Prio:	RET(_current) = _current->priority; break;
		case Time:	RET(_current) = _system_time; break;
		default:
			// this is strange - the code is valid, but we
			// don't recognize it; probably means we haven't
			// completed the implementation for a new code yet
			__sprint( _b256, "** GD pid %u, request %d\n",
					_current->pid, what );
			WARNING( _b256 );
			RET(_current) = E_BAD_PARAM;
		}
	}
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
	// what does the user want to know?
	int32_t what = ARG(_current,1);
	int32_t data = ARG(_current,2);

	// validate it
	if( what >= N_DATUMS ) {
		// bad code
		RET(_current) = E_BAD_PARAM;
	} else {
		// at the moment, the only one we allow is Prio
		if( what == Prio ) {
			// return the old value
			RET(_current) = _current->priority;
			// update the priority
			_current->priority = data;
		} else {
			// this maybe a valid datum, but we're not allowing
			// it at the moment
			__sprint( _b256, "** SD pid %u, request (%d,%d)\n",
					_current->pid, what, data );
			WARNING( _b256 );
			RET(_current) = E_BAD_PARAM;
		}
	}
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
	// get our victim's id
	uint32_t pid = ARG(_current,1);

	// must be a valid "ordinary user"PID, and can't be the current process
	// QUESTION: what if it's the idle process?
	if( pid < FIRST_USER_PID || pid == _current->pid ) {
		RET(_current) = E_FAILURE;
		return;
	}

	// OK, this is an acceptable victim; see if it exists
	pcb_t *pcb = _pcb_find( pid );
	if( pcb == NULL ) {
		// nope!
		RET(_current) = E_NOT_FOUND;
		return;
	}

	// how we perform the kill depends on the victim's state

	// sanity check
	assert( pcb->state != Running );

	if( pcb->state == Killed || pcb->state == Zombie ) {
		// already killed, or has exited, so we have nothing to do
		RET(_current) = E_SUCCESS;
		return;
	}

	// must be active, so we need to set its exit status
	pcb->exit_status = EXIT_KILLED;

	// also need to reparent its children
}

/**
** Name:	_sys_fork
**
** Create a new process
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
#if TRACING_SYSCALLS
	__cio_printf( "--> _sys_fork, pid %d\n", _current->pid );
#endif

	// Make sure there's room for another process!
	pcb_t *pcb = _pcb_alloc();
	if( pcb == NULL ) {
		RET(_current) = E_NO_PROCS;
#if TRACING_SYSRET
		__cio_printf( "<-- %08x\n", E_NO_PROCS );
#endif
		return;
	}

	// Create the stack for the child.
	pcb->stack = _stk_alloc();
	if( pcb->stack == NULL ) {
		_pcb_dealloc( pcb );
		RET(_current) = E_NO_PROCS;
#if TRACING_SYSRET
		__cio_printf( "<-- %08x\n", E_NO_PROCS );
#endif
		return;
	}

	// Duplicate the parent's stack.
	__memcpy( (void *)pcb->stack, (void *)_current->stack, sizeof(stack_t) );

	// Set the child's identity.
	pcb->pid = _next_pid++;
	pcb->ppid = _current->pid;
	pcb->state = New;

	// replicate things inherited from the parent
	pcb->priority = _current->priority;

	pcb->cwd = _current->cwd; // TODO(Adin): Reference count the dir entry
	// TODO(Adin): Initialize stdin and out from parent here (open new files)

	/*
	** Next, we need to update the ESP and EBP values in the child's
	** stack.  The problem is that because we duplicated the parent's
	** stack, these pointers are still pointing back into that stack,
	** which will cause problems as the two processes continue to execute.
	**
	** Note: if there are other pointers to things in the parent's stack
	** (e.g., pointers to local variables), we do NOT locate and update
	** them, as that's impractical. As a result, user code that relies on
	** such pointers may behave strangely after a fork().
	*/

	// Figure out the byte offset from one stack to the other.
	int32_t offset = (void *) pcb->stack - (void *) _current->stack;

	// Add this to the child's context pointer.
	pcb->context = (context_t *) (((void *)_current->context) + offset);

	// Fix the child's ESP and EBP values IFF they're non-zero.
	if( REG(pcb,ebp) != 0 ) {
		REG(pcb,ebp) += offset;
	}
	if( REG(pcb,esp) != 0 ) {
		REG(pcb,esp) += offset;
	}

	// Follow the EBP chain through the child's stack.
	uint32_t *bp = (uint32_t *) REG(pcb,ebp);
	while( bp && *bp ) {
		*bp += offset;
		bp = (uint32_t *) *bp;
	}

	// Set the return values for the two processes.
	RET(_current) = pcb->pid;
	RET(pcb) = 0;
	// _ctx_dump( "fork: new", pcb->context );
	// __delay(400);

	// Schedule the child, and let the parent continue.
	_schedule( pcb );
#if TRACING_SYSRET
	__cio_printf( "<-- %08x\n", RET(curr) );
#endif
	// _ctx_dump( "fork: current", _current->context );
	// _ctx_dump( "fork: new", pcb->context );
	// __delay(400);
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
	uint32_t entry = ARG(_current,1);
	char **args = (char **) ARG(_current,2);

#if TRACING_SYSCALLS
	__cio_printf( "--> _sys_execp, pid %d\n", _current->pid );
#endif

	// Set up the new stack for the user.
	context_t *ctx = _stk_setup( _current->stack, entry, args );
	assert( ctx != NULL );

	// Copy the context pointer into the current PCB.
	_current->context = ctx;

	// It's also the current ESP for the process.
	_current->context->esp = (uint32_t) ctx;

	/*
	** Decision:
	**	(A) schedule this process and dispatch another,
	**	(B) let this one continue in its current time slice
	**	(C) reset this one's time slice and let it continue
	**
	** We choose option A.
	**
	** If scheduling the process fails, the exec() has failed. However,
	** all trace of the old process is gone by now, so we can't return
	** an error status to it.
	*/

	assert( _schedule(_current) == S_OK );

	_dispatch();
}

/**
** _sys_vgatextclear - clear the VGA Text Screen
**
** implements:
**		void vgatextclear( void );
*/
SYSIMPL(vgatextclear)
{
	__cio_clearscroll();
}

/**
** _sys_vgatextgetactivecolor - get the active VGA Text Mode Color
**
** implements:
**		unsigned int vgatextgetactivecolor( void );
**
** returns:
**		the active VGA Text Mode Color
*/
SYSIMPL(vgatextgetactivecolor)
{
	 RET(_current) = __vga_text_get_active_color();
}

/**
** _sys_vgatextsetactivecolor - set the active VGA Text Mode Color
**
** implements:
** 		void vgatextsetactivecolor( unsigned int vga_text_color );
**
** returns:
** 		only on failure
*/
SYSIMPL(vgatextsetactivecolor)
{
	__vga_text_set_active_color(ARG(_current,1));
}

// See acpi/acpi.h
SYSIMPL(acpicommand)
{
	enum _acpi_commands cmd = ARG(_current, 1);
	_acpi_command(cmd);
}

/**
** _sys_vgatextgetblinkenabled - get whether blink is enabled in VGA Text Mode
**
** implements:
**		unsigned int vgatextgetblinkenabled( void );
**
** returns:
**		0 if blink is disabled in VGA Text Mode
*/
SYSIMPL(vgatextgetblinkenabled)
{
	RET(_current) = __vga_text_get_blink_enabled();
}

/**
** _sys_vgatextsetblinkenabled - set whether blink is enabled in VGA Text Mode
**
** implements:
** 		void vgatextsetblinkenabled( unsigned int blink_enabled );
**
** returns:
** 		only on failure
*/
SYSIMPL(vgatextsetblinkenabled)
{
	__vga_text_set_blink_enabled(ARG(_current,1));
}

/**
** _sys_vgagetmode - get the current VGA Mode
**
** implements:
**		unsigned int vgagetmode( void );
**
** returns:
** 		0 if Text Mode
** 		1 if 16-color 640x480 Graphics Mode
** 		2 if 256-color 320x200 Graphics Mode
*/
SYSIMPL(vgagetmode)
{
	RET(_current) = __vga_get_mode();
}

/**
** _sys_vgasetmode - set the VGA Mode
** 		0 for Text Mode
** 		1 for 16-color 640x480 Graphics Mode
** 		2 for 256-color 320x200 Graphics Mode
**
** implements:
** 		void vgasetmode( unsigned mode );
**
** returns:
** 		only on failure
*/
SYSIMPL(vgasetmode)
{
	__vga_set_mode(ARG(_current,1));
}

/**
** _sys_vgaclear - clear the VGA Screen
** 
** implements:
**		void vgatextclear( void );
*/
SYSIMPL(vgaclearscreen)
{
	__vga_clear_screen();
}

/**
** _sys_vgatest - draw a test pattern for the current graphics mode on the VGA Screen
** 
** implements:
**		void vgatest( void );
*/
SYSIMPL(vgatest)
{
	__vga_draw_test_pattern();
}

/**
** _sys_vgadrawimage - draw an image on the VGA Screen
**
** implements:
** 		void vgadrawimage( uint16_t im_w, uint8_t im_h, uint8_t off_x, uint8_t off_y, uint8_t *image_data );
** 		im_w: the width of the image
**		im_h: the height of the image
**		off_x: the offset from which to draw the image on the x axis, from 0 at the left
**		off_y: the offset from which to draw the image on the y axis, from 0 at the top
**		image_data: an array of numbers corresponding to the VGA palette, arranged in rows one after another
**
** returns:
** 		only on failure
*/
SYSIMPL(vgadrawimage)
{
	__vga_draw_image(ARG(_current,1), ARG(_current,2), ARG(_current,3), ARG(_current,4), (uint8_t *) ARG(_current,5));
}

/**
** _sys_vgawritepixel - write an individual pixel on the VGA Screen
**
** implements:
** 		void vgawritepixel( unsigned x, unsigned y, unsigned c );
** 		x: the x coordinate of the pixel to plot, from 0 at the left
**		y: the y coordinate of the pixel to plot, from 0 at the top
**		c: the number corresponding to the color in the VGA palette
**
** returns:
** 		only on failure
*/
SYSIMPL(vgawritepixel)
{
	__vga_write_pixel(ARG(_current,1), ARG(_current,2), ARG(_current,3));
}

SYSIMPL(ciogetcursorpos)
{
	__cio_getpos((unsigned int *) ARG(_current, 1), (unsigned int *) ARG(_current, 2));
}

SYSIMPL(ciosetcursorpos)
{
	__cio_moveto((unsigned int) ARG(_current, 1), (unsigned int) ARG(_current, 2));
}

SYSIMPL(ciogetspecialdown)
{
	RET(_current) = __cio_getarrowdown();
}

// ----------------------------- VFS Calls -----------------------------

#define IS_BAD_FD(fd) \
	((fd) < 0 || (fd) >= VFS_MAX_OPEN_FILES || _current->open_files[(fd)] == NULL)

static int32_t __status_to_sys_ret(status_t status)
{
	switch(status) {
		case S_NOT_SUPP:
			return E_NOT_SUPPORTED;

		case S_BAD_PARAM:
			return E_BAD_PARAM;

		case S_BAD_ACTION:
			return E_BAD_ACTION;

		case S_EOF:
			return E_EOF;

		case S_NOMEM:
			return E_NO_MEM;

		case S_TOO_SMALL:
			return E_TOO_SMALL;

		case S_OK:
			return E_SUCCESS;

		default:
			return E_FAILURE;
	}

	// Should never get here but eh, what the hell
	return E_FAILURE;
}

// fd_t fopen(char *path, uint32_t mode, uint32_t flags);
SYSIMPL(fopen)
{
	char *path     = (char *) ARG(_current, 1);
	uint32_t mode  = ARG(_current, 2);
	uint32_t flags = ARG(_current, 3);
	(void) flags; // TODO(Adin): Add flags to test or remove this parameter (in ulib.h too)

	if(!path || !(mode & O_READ || mode & O_WRITE)) {
		RET(_current) = E_BAD_PARAM;
		return;
	}

	inode_t *target = NULL;
	status_t namey_status = namey(path, &target);
	if(namey_status) {
		RET(_current) = E_FAILURE;
		return;
	}

	// TODO(Adin): Should this be a reason to fail or should open be an extra
	//			   hook the fs driver can _optionally_ implement?
	if(!target->i_file_ops || !target->i_file_ops->open){
		RET(_current) = E_NOT_SUPPORTED;
		return;
	}

	/*
	 * If target file
	 * a) isn't a device _AND_
	 * b) this is a read trying to steal from a writer _OR_
	 * c) this is a write trying to steal from either a reader or a writer
	*/
	if(target->i_type != S_TYPE_DEV &&
	   ((mode & O_READ && target->i_has_writer) ||
	   (mode & O_WRITE && (target->i_has_writer || target->i_nr_readers > 0))))
	{
		RET(_current) = E_ALREADY_OPEN;
		return;
	}

	fd_t fd = _pcb_get_next_fd(_current);
	if(fd < 0) {
		RET(_current) = E_MAX_FILES_OPEN;
		return;
	}

	kfile_t *file = _vfs_allocate_file();
	file->kf_inode = target;
	file->kf_ops = target->i_file_ops;
	file->kf_mode = mode;

	status_t open_status = file->kf_ops->open(target, file, flags);
	if(open_status) {
		_vfs_free_file(file);
		RET(_current) = __status_to_sys_ret(open_status);
		return;
	}

	if(target->i_type != S_TYPE_DEV) {
		if(mode & O_READ) {
			target->i_nr_readers++;
		}

		if(mode & O_WRITE) {
			target->i_has_writer = true;
		}
	}

	_current->open_files[fd] = file;

	RET(_current) = fd;
}

// int32_t fclose(fd_t fd);
SYSIMPL(fclose)
{
	fd_t fd = ARG(_current, 1);

	if(IS_BAD_FD(fd)) {
		RET(_current) = E_BAD_PARAM;
		return;
	}

	// TODO(Adin): Reference counting

	kfile_t *file = _current->open_files[fd];
	if(file->kf_ops && file->kf_ops->close) {
		// TODO(Adin): If this fails, how should it be reported to
		// 			   userspace?
		file->kf_ops->close(file);
	}

	if(file->kf_inode->i_type != S_TYPE_DEV) {
		if(file->kf_mode & O_READ) {
			file->kf_inode->i_nr_readers--;
		}

		if(file->kf_mode & O_WRITE) {
			file->kf_inode->i_has_writer = false;
		}
	}

	// Useful debugging print: do not remove
	// __cio_printf(
	// 	"close fd %d: nr_writers: %d, has_writer %d\n",
	// 	fd, file->kf_inode->i_nr_readers, file->kf_inode->i_has_writer
	// );

	_current->open_files[fd] = NULL;
	_vfs_free_file(file);

	RET(_current) = E_SUCCESS;
}

// uint32_t fread(fd_t fd, void *buf, uint32_t num_bytes, uint32_t flags int32_t *status);
SYSIMPL(fread)
{
	fd_t fd            = ARG(_current, 1);
	void *buf          = (void *) ARG(_current, 2);
	uint32_t num_bytes = ARG(_current, 3);
	uint32_t flags     = ARG(_current, 4);
	int32_t *status    = (int32_t *) ARG(_current, 5);

	if(IS_BAD_FD(fd) || !buf) {
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_PARAM;
		}
		return;
	}

	kfile_t *target = _current->open_files[fd];
	if(!(target->kf_mode & O_READ)) {
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_ACTION;
		}
		return;
	}

	if(!target->kf_ops || !target->kf_ops->read) {
		RET(_current) = 0;
		if(status) {
			*status = E_NOT_SUPPORTED;
		}
		return;
	}

	uint32_t num_read = 0;
	uint32_t offset = (target->kf_inode->i_type == S_TYPE_DEV ? 0 : target->kf_rwhead);

	status_t read_status = target->kf_ops->read(target, buf, num_bytes, offset, flags, &num_read);

	target->kf_rwhead += num_read;

	RET(_current) = num_read;
	if(status) {
		*status = __status_to_sys_ret(read_status);
	}
}

// uint32_t fwrite(fd_t fd, void *buf, uint32_t num_bytes, uint32_t flags, int32_t *status);
SYSIMPL(fwrite)
{
	fd_t fd            = ARG(_current, 1);
	void *buf          = (void *) ARG(_current, 2);
	uint32_t num_bytes = ARG(_current, 3);
	uint32_t flags     = ARG(_current, 4);
	int32_t *status    = (int32_t *) ARG(_current, 5);

	if(IS_BAD_FD(fd) || !buf) {
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_PARAM;
		}
		return;
	}

	kfile_t *target = _current->open_files[fd];
	if(!(target->kf_mode & O_WRITE)) {
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_ACTION;
		}
		return;
	}

	if(!target->kf_ops || !target->kf_ops->write) {
		RET(_current) = 0;
		if(status) {
			*status = E_NOT_SUPPORTED;
		}
		return;
	}

	uint32_t num_written = 0;
	uint32_t offset = (target->kf_inode->i_type == S_TYPE_DEV ? 0 : target->kf_rwhead);

	status_t write_status = target->kf_ops->write(target, buf, num_bytes, offset, flags, &num_written);

	target->kf_rwhead += num_written;

	RET(_current) = num_written;
	if(status) {
		*status = __status_to_sys_ret(write_status);
	}
}

// uint32_t flistdir(fd_t fd, adinfs_dent_t *buffer, uint32_t count, int32_t *status);
SYSIMPL(flistdir)
{
	fd_t fd               = ARG(_current, 1);
	adinfs_dent_t *buffer = (adinfs_dent_t *) ARG(_current, 2);
	uint32_t count 		  = ARG(_current, 3);
	int32_t *status  	  = (int32_t *) ARG(_current, 4);
	uint32_t flags        = ARG(_current, 5);

	if(IS_BAD_FD(fd)) {
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_PARAM;
		}
		return;
	}

	kfile_t *target = _current->open_files[fd];

	if(!target->kf_ops || !target->kf_ops->iterate_shared || target->kf_inode->i_type != S_TYPE_DIR) {
		RET(_current) = 0;
		if(status) {
			*status = E_NOT_SUPPORTED;
		}
		return;
	}

	if(!(target->kf_mode & O_READ)) {
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_ACTION;
		}
	}

	uint32_t num_written = 0;
	// This is called every time because there is no guarantee the dentry cache will
	// have every child of the current inode
	status_t iterate_result = target->kf_ops->iterate_shared(target, buffer, count, &num_written, flags);

	RET(_current) = num_written;
	if(status) {
		*status = __status_to_sys_ret(iterate_result);
	}
}

// int32_t fcreate(char *path, uint32_t type, uint32_t flags);
SYSIMPL(fcreate)
{
#if 0
	char *path     = (char *) ARG(_current, 1);
	uint32_t type  = ARG(_current, 2);
	uint32_t flags = ARG(_current, 3);

	char *split_path = _km_slice_alloc();
	__memcpy(split_path, path, __strlen(path));

	// Quick and dirty last index of
	int index = __strlen(split_path) - 1;
	while(index >= 0 && split_path[index] != '/') {
		index--;
	}

	if(index < 0) {
		RET(_current) = E_BAD_PARAM;
		return;
	}

	inode_t *parent = NULL;
	status_t namey_result = namey(path, &parent);
#endif

	RET(_current) = E_NOT_SUPPORTED;
}

// int32_t fdelete(char *path, uint32_t flags);
SYSIMPL(fdelete)
{
	RET(_current) = E_NOT_SUPPORTED;
}

// int32_t fioctl(fd_t fd, uint32_t action, void *data);
SYSIMPL(fioctl)
{
	fd_t fd         = ARG(_current, 1);
	uint32_t action = ARG(_current, 2);
	void *data      = (void *) ARG(_current, 3);

	if(IS_BAD_FD(fd)) {
		RET(_current) = E_BAD_PARAM;
		return;
	}

	kfile_t *target = _current->open_files[fd];
	if(!target->kf_ops || !target->kf_ops->ioctl) {
		RET(_current) = E_NOT_SUPPORTED;
		return;
	}

	status_t ioctl_status = target->kf_ops->ioctl(target, action, data);

	RET(_current) = __status_to_sys_ret(ioctl_status);
}

// uint32_t fseek(fd_t fd, int32_t offset, uint32_t whence, int32_t *status);
SYSIMPL(fseek)
{
	fd_t fd         = ARG(_current, 1);
	int32_t offset  = ARG(_current, 2);
	uint32_t whence = ARG(_current, 3);
	int32_t *status = (int32_t *) ARG(_current, 4);

	if(IS_BAD_FD(fd)) {
		if(status) {
			*status = E_BAD_PARAM;
		}
		RET(_current) = 0;
		return;
	}

	kfile_t *target = _current->open_files[fd];

	if(target->kf_inode->i_type == S_TYPE_DEV ||
	   !target->kf_ops ||
	   !target->kf_ops->get_length)
	{
		if(status) {
			*status = E_NOT_SUPPORTED;
		}
		RET(_current) = 0;
		return;
	}

	uint32_t file_len = target->kf_ops->get_length(target);

	uint32_t new_base = 0;
	switch(whence) {
		case SEEK_CURR:
			new_base = target->kf_rwhead;
			break;

		case SEEK_SET:
			new_base = 0;
			break;

		case SEEK_END:
			new_base = file_len;
			break;

		default:
			RET(_current) = 0;
			if(status) {
				*status = E_BAD_PARAM;
			}
			return;
	}

	uint32_t new_rwhead = new_base + offset;

	// Useful debugging print: do not remove
	// __cio_printf(
	// 	"old_rwhead: %u, offset: %d, new_base: %u, new_rwhead %u\n",
	// 	target->kf_rwhead, offset, new_base, new_rwhead
	// );

	if((offset < 0 && new_rwhead > new_base) || // Underflow
	   (offset > 0 && new_rwhead < new_base) || // Overflow
	   (new_rwhead > file_len))					// rw head > file length
	{
		RET(_current) = 0;
		if(status) {
			*status = E_BAD_PARAM;
		}
		return;
	}

	target->kf_rwhead = new_rwhead;
	RET(_current) = new_rwhead;
	if(status) {
		*status = __status_to_sys_ret(S_OK);
	}
}


// int32_t chdir(char *path)
SYSIMPL(fchdir)
{
	char *path = (char *) ARG(_current, 1);

	if(!path) {
		RET(_current) = E_BAD_PARAM;
		return;
	}

	dirent_t *new_cwd = NULL;
	// __cio_printf("fchdir path %s\n", path);
	status_t resolve_status = resolve_path(path, &new_cwd);
	if(resolve_status != S_OK) {
		RET(_current) = __status_to_sys_ret(resolve_status);
		return;
	}

	if(!new_cwd) {
		RET(_current) = E_FAILURE;
		return;
	}

	_current->cwd = new_cwd;
	RET(_current) = E_SUCCESS;
}

// uint32_t getcwd(char *buffer, uint32_t buffer_len)
SYSIMPL(fgetcwd)
{
	char *buffer = (char *) ARG(_current, 1);
	uint32_t buffer_len = ARG(_current, 2);

	if(!buffer) {
		RET(_current) = 0;
		return;
	}

	__memclr(buffer, buffer_len);
	_vfs_dirent_to_pathname(_current->cwd, buffer);

	RET(_current) = __strlen(buffer);
}


// The system call jump table
//
// Initialized using designated initializers to ensure the entries
// are correct even if the syscall code values should happen to change.
// This also makes it easy to add new system call entries, as their
// position in the initialization list is irrelevant.

static void (* const _syscalls[N_SYSCALLS])( void ) = {
	[ SYS_exit    ]                = _sys_exit,
	[ SYS_sleep   ]                = _sys_sleep,
	[ SYS_read    ]                = _sys_read,
	[ SYS_write   ]                = _sys_write,
	[ SYS_waitpid ]                = _sys_waitpid,
	[ SYS_getdata ]                = _sys_getdata,
	[ SYS_setdata ]                = _sys_setdata,
	[ SYS_kill    ]                = _sys_kill,
	[ SYS_fork    ]                = _sys_fork,
	[ SYS_exec    ]                = _sys_exec,
	[ SYS_vgatextclear ]           = _sys_vgatextclear,
	[ SYS_vgatextgetactivecolor ]  = _sys_vgatextgetactivecolor,
	[ SYS_vgatextsetactivecolor ]  = _sys_vgatextsetactivecolor,
	[ SYS_acpicommand ]            = _sys_acpicommand,
	[ SYS_vgatextgetblinkenabled ] = _sys_vgatextgetblinkenabled,
	[ SYS_vgatextsetblinkenabled ] = _sys_vgatextsetblinkenabled,
	[ SYS_vgagetmode ]			   = _sys_vgagetmode,
	[ SYS_vgasetmode ]			   = _sys_vgasetmode,
	[ SYS_vgaclearscreen ]		   = _sys_vgaclearscreen,
	[ SYS_vgatest ]				   = _sys_vgatest,
	[ SYS_vgadrawimage ]		   = _sys_vgadrawimage,
	[ SYS_vgawritepixel ]		   = _sys_vgawritepixel,
	[ SYS_ciogetcursorpos ]		   = _sys_ciogetcursorpos,
	[ SYS_ciosetcursorpos ]		   = _sys_ciosetcursorpos,
	[ SYS_ciogetspecialdown ]      = _sys_ciogetspecialdown,

	[ SYS_fopen    ]               = _sys_fopen,
	[ SYS_fclose   ]               = _sys_fclose,
	[ SYS_fread    ]               = _sys_fread,
	[ SYS_fwrite   ]               = _sys_fwrite,
	[ SYS_flistdir ]               = _sys_flistdir,
	[ SYS_fcreate  ]               = _sys_fcreate,
	[ SYS_fdelete  ]               = _sys_fdelete,
	[ SYS_fioctl   ]               = _sys_fioctl,
	[ SYS_fseek    ]               = _sys_fseek,

	[ SYS_fchdir    ]			   = _sys_fchdir,
	[ SYS_fgetcwd   ]			   = _sys_fgetcwd,
};

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

	// sanity check!
	assert( _current != NULL );

	// much less likely to occur, but still problematic
	assert1( _current->context != NULL );

	// retrieve the syscall code
	uint32_t syscode = REG( _current, eax );

#if TRACING_SYSCALLS
	__cio_printf( "** --> SYS pid %u code %u\n", _current->pid, syscode );
#endif

	// validate it
	if( syscode >= N_SYSCALLS ) {

		// this is a problem!
		__sprint( _b256, "pid %d bad syscall 0x%x", _current->pid, syscode );
		WARNING( _b256 );

		// force a call to exit()
		syscode = SYS_exit;

		// falsify the "exit status"
		ARG(_current,1) = EXIT_ABORTED;
	}

	// call the handler
	_syscalls[syscode]();

#if TRACING_SYSCALLS
	__cio_printf( "** <-- SYS pid %u ret %u\n", _current->pid, RET(_current) );
#endif

	// Tell the PIC we're done.
	__outb( PIC_PRI_CMD_PORT, PIC_EOI );
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

	// install the second-stage ISR
	__install_isr( INT_VEC_SYSCALL, _sys_isr );
}
