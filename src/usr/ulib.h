/**
** @file	ulib.h
**
** @author	Numerous CSCI-452 classes
**
** @brief	Declarations for user-level library functions
**
** This module implements a simple collection of support functions
** similar to the standard C library.
*/

#ifndef ULIB_H_
#define ULIB_H_

#include "common.h"

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

/*
** Prototypes
*/

/*
**********************************************
** SYSTEM CALLS
**********************************************
*/

/**
** exit - terminate the calling process
**
** usage:   exit(status);
**
** @param status   Termination status of this process
**
** @return Does not return
*/
void exit( int32_t status );

/**
** sleep - put the current process to sleep for some length of time
**
** usage:   sleep(n);
**
** @param ms Desired sleep time (in ms), or 0 to yield the CPU
**
** @return E_SUCCESS, or an error code if the sleep couldn't be performed
*/
int32_t sleep( uint32_t ms );

/**
** read - read into a buffer from a stream
**
** usage:   n = read(channel,buf,length)
**
** @param chan   I/O stream to read from
** @param buf    Buffer to read into
** @param length Maximum capacity of the buffer
**
** @returns  The count of bytes transferred, or an error code
*/
int32_t read( int32_t chan, void *buffer, uint32_t length );

/**
** write - write from a buffer to a stream
**
** usage:   n = write(channel,buf,length)
**
** @param chan   I/O stream to write to
** @param buf    Buffer to write from
** @param length Maximum capacity of the buffer
**
** @returns  The count of bytes transferred, or an error code
*/
int32_t write( int32_t chan, const void *buffer, uint32_t length );

/**
** waitpid - wait for a child process to terminate
**
** usage:   pid = waitpid(pid,&status);
**
** @param pid    PID of the desired child, or 0 for any child
** @param status Pointer to int32_t into which the child's status is placed,
**               or NULL
**
** @returns The PID of the terminated child, or an error code
**
** If there are no children in the system, returns an error code (*status
** is unchanged).
**
** If there are one or more children in the system and at least one has
** terminated but hasn't yet been cleaned up, cleans up that process and
** returns its information; otherwise, blocks until a child terminates.
*/
int32_t waitpid( pid_t pid, int32_t *status );

/**
** getdata - get some information from the OS
**
** usage:   n = getdata( which )
**
** @param which  which OS datum is to be retrieved
**
** @returns  the requested information, or -1 on error
*/
int32_t getdata( datum_t which );

/**
** setdata - set an OS value of some type
**
** usage:   n = setdata( which, value )
**
** @param which  which OS datum is to be modified
** @param value  what to set that item to
**
** @returns  the previous value, or -1 on error
*/
int32_t setdata( datum_t which, int32_t value );

/**
** kill - terminate a process with extreme prejudice
**
** usage:   n = kill( pid )
**
** @param pid  the intended victim
**
** @returns  0 on success, else an error code
*/
int32_t kill( pid_t pid );

/**
** fork - create a duplicate of the calling process
**
** usage:   pid = fork();
**
** @returns  parent - the pid of the new child, or an error code
**           child  - 0
*/
int32_t fork( void );

/**
** exec - replace the memory image of the calling process
**
** usage:   exec( entry, args )
**
** @param entry   the entry point of the new user code
** @param args    the command-line argument vector
**
** @returns  does not return if it succeeds; else, an error code
*/
int32_t exec( userfcn_t entry, char *args[] );

/**
** vgatextclear - clear the VGA console window
**
** usage:   vgatextclear()
**
** @returns void
*/
void vgatextclear( void );

/**
** vgatextgetactivecolor - get the current active VGA Text Mode Color
**
** usage:   vgatextgetactivecolor()
**
** @returns the current active VGA Text Mode Color
*/
unsigned int vgatextgetactivecolor( void );

/**
** vgatextsetactivecolor - set the current active VGA Text Mode Color
**
** usage:   vgatextsetactivecolor( color )
**
** @param color   the color to set as active
**
** @returns void
*/
void vgatextsetactivecolor( unsigned int color );

void acpicommand( int );

/**
** vgagetmode - get the current active VGA Mode
**
** usage:   vgagetmode()
** 
** @returns the active VGA Mode, as defined in vga.h
*/
unsigned int vgagetmode( void );

/**
** vgasetmode - set the current active VGA Mode
**
** usage:   vgasetmode( mode )
**
** @param mode   the mode to set as active
** 
** @returns void
*/
void vgasetmode( unsigned int mode );

/**
** vgaclearscreen - clear the VGA Graphics Screen
**
** usage:   vgaclearscreen()
** 
** @returns void
*/
void vgaclearscreen( void );

/**
** vgatest - draw a VGA Graphics Test Pattern
**
** usage:   vgatest()
** 
** @returns void
*/
void vgatest( void );

/**
** vgadrawimage - draw an Image in VGA Graphics Modes
**
** usage:   vgadrawimage( width, height, x_offset, y_offset, image )
** 
** @returns void
*/
void vgadrawimage( uint16_t width, uint8_t height, uint16_t x_offset, uint8_t y_offset, uint8_t* image );

/**
** vgawritepixel - write an individual Pixel in VGA Graphics Modes
**
** usage:   vgawritepixel( x, y, color )
**
** @returns void
*/
void vgawritepixel( uint16_t x, uint16_t y, uint8_t color );

void ciogetcursorpos(unsigned int *x, unsigned int *y);

void ciosetcursorpos(unsigned int x, unsigned int y);

unsigned int ciogetspecialdown(void);


/**
 * @brief Open a file for i/o operations
 *
 * @param path the path of the file to be opened
 * @param mode the operating mode of the file (O_READ, O_WRITE, O_RDWR)
 * @param flags any additional driver-specific options for the operation
 *
 * @return fd_t a file descriptor representing the newly opened file or an error code
 */
fd_t fopen(char *path, uint32_t mode, uint32_t flags);

/**
 * @brief Close an open file and free its associated resources
 *
 * @param fd the file descriptor representing the file to close
 *
 * @return int32_t An error code reporting any error that occurred
 */
int32_t fclose(fd_t fd);

/**
 * @brief Read data from an open file
 *
 * The file must have been opened for reading (O_READ, O_RDWR).
 *
 * @param fd the file descriptor representing the file to read from
 * @param buf the buffer to read into (must be >= num_bytes in size)
 * @param num_bytes the number of bytes to read from the file
 * @param flags any additional driver-specific options for the operation
 * @param status an optional pointer to the location to store the error code generated (if any)
 *
 * @return uint32_t the number of bytes read from the file / written into buffer
 */
uint32_t fread(fd_t fd, void *buf, uint32_t num_bytes, uint32_t flags, int32_t *status);

/**
 * @brief Write data to an open file
 *
 * The file must have been opened for writing (O_WRITE, O_RDWR).
 *
 * @param fd the file descriptor representing the file to write to
 * @param buf the buffer to write from (must be >= num_bytes in size)
 * @param num_bytes the number of bytes to write to the file
 * @param flags any additional driver-specific options for the operation
 * @param status an optional pointer to the location to store the error code generated (if any)
 *
 * @return uint32_t the number of bytes written to the file / read from buffer
 */
uint32_t fwrite(fd_t fd, void *buf, uint32_t num_bytes, uint32_t flags, int32_t *status);

/**
 * @brief Read the contents of a directory (child names and types)
 *
 * If buffer is NULL, the number of adinfs_dent_t's that would be written is returned.
 * No other side effects occur in this case.
 *
 * @param fd the file descriptor representing the directory to query
 * @param buffer a buffer to write the resulting adinfs_dent_t's to
 * @param count the number of adinfs_dent_t's buffer is capable of storing
 * @param flags any additional driver-specific options for the operation
 * @param status an optional pointer to the location to store the error code generated (if any)
 *
 * @return uint32_t the number of adinfs_dent_t's read from the directory / written to buffer
 */
uint32_t flistdir(fd_t fd, adinfs_dent_t *buffer, uint32_t count, int32_t *status, uint32_t flags);

/**
 * @brief Create a file at the specified path (without opening it)
 *
 * Not implemented due to time.
 *
 * @param path the path of the file to be created
 * @param type the type of file to create
 * @param flags any additional driver-specific options for the operation
 *
 * @return int32_t an error code representing any error that occurred during the operation
 */
int32_t fcreate(char *path, uint32_t type, uint32_t flags);

/**
 * @brief Delete a file at the specified path
 *
 * This operation will fail if any process has the file open (for reading or
 * writing) or is currently within it (the process's cwd is set to the file).
 *
 * Not implemented due to time.
 *
 * @param path the path of the file to be deleted
 * @param flags any additional driver-specific options for the operation
 *
 * @return int32_t an error code representing the first error that occurred during the operation
 */
int32_t fdelete(char *path, uint32_t flags);

/**
 * @brief Perform a backing driver specific operation on an open file or the inode it represents
 *
 * @param fd the file descriptor representing the file to modify
 * @param action the driver-specific action to perform
 * @param data any data the action may consume or return
 *
 * @return int32_t an error code representing the first error to occur during the action
 */
int32_t fioctl(fd_t fd, uint32_t action, void *data);

/**
 * @brief Update an open file's r/w head and report the new location
 *
 * @param fd the file descriptor representing the file to update
 * @param offset the offset from the location specified by whence to move
 * @param whence the base location to add offset to when calculating the final position
                 (SEEK_SET, SEEK_CURR, SEEK_END)
 * @param status an optional pointer to store the location of the error code associated with the action
 *
 * @return uint32_t the new r/w head position of the file (or 0 on error)
 */
uint32_t fseek(fd_t fd, int32_t offset, uint32_t whence, int32_t *status);

/**
 * @brief Change the current working directory of the file
 *
 * @param path the path of the new directory to set the process's cwd to
 *
 * @return int32_t an error code representing the first error that occured during the operation
 */
int32_t fchdir(char *path);

/**
 * @brief Get the path of the calling process's current working directory as a string
 *
 * @param buffer the buffer to write the path to
 * @param buffer_len the size (in bytes) of buffer
 *
 * @return uint32_t the number of bytes written to buffer
 */
uint32_t fgetcwd(char *buffer, uint32_t buffer_len);

/**
** bogus - a nonexistent system call, to test our syscall ISR
**
** usage:   bogus()
**
** Does not return.
*/
void bogus( void );

/*
**********************************************
** CONVENIENT "SHORTHAND" VERSIONS OF SYSCALLS
**********************************************
**
** These are library functions that perform specific common
** variants of system calls. This helps reduce the total number
** of system calls, keeping our baseline OS as lean and mean
** as we can make it. :-)
*/

/**
** wait - wait for any child to exit
**
** usage:   pid = wait(&status)
**
** Calls waitpid(0,status)
**
** @param status Pointer to int32_t into which the child's status is placed,
**               or NULL
**
** @returns The PID of the terminated child, or an error code
*/
int32_t wait( int32_t *status );

/**
** cwritech(ch) - write a single character to the console
**
** @param ch The character to write
**
** @returns The return value from calling write()
*/
int32_t cwritech( char ch );

/**
** cwrites(str) - write a NUL-terminated string to the console
**
** @param str The string to write
**
*/
int32_t cwrites( const char *str );

/**
** cwrite(buf,leng) - write a sized buffer to the console
**
** @param buf  The buffer to write
** @param leng The number of bytes to write
**
** @returns The return value from calling write()
*/
int32_t cwrite( const char *buf, uint32_t leng );

/**
** swritech(ch) - write a single character to the SIO
**
** @param ch The character to write
**
** @returns The return value from calling write()
*/
int32_t swritech( char ch );

/**
** swrites(str) - write a NUL-terminated string to the SIO
**
** @param str The string to write
**
** @returns The return value from calling write()
*/
int32_t swrites( const char *str );

/**
** swrite(buf,leng) - write a sized buffer to the SIO
**
** @param buf  The buffer to write
** @param leng The number of bytes to write
**
** @returns The return value from calling write()
*/
int32_t swrite( const char *buf, uint32_t leng );

/**
** spawn(entry,prio,args) - create a new process running a different program
**
** usage:       n = spawn(entry,prio,args)
**
** Creates a new process, changes its priority, and then execs 'entry'
**
** @param entry The entry point of the new code
** @param prio  The desired priority for the process, or -1
** @param args  The command-line argument vector for the process
**
** @returns     The PID of the child, or an error code
*/
int32_t spawn( userfcn_t entry, int32_t prio, char *args[] );

/*
**********************************************
** STRING MANIPULATION FUNCTIONS
**********************************************
*/

/**
** str2int(str,base) - convert a string to a number in the specified base
**
** @param str   The string to examine
** @param base  The radix to use in the conversion
**
** @return The converted integer
*/
int str2int( register const char *str, register int base );

/**
** strlen(str) - return length of a NUL-terminated string
**
** @param str The string to examine
**
** @return The length of the string, or 0
*/
uint32_t strlen( const char *str );

/**
** strcpy(dst,src) - copy a NUL-terminated string
**
** @param dst The destination buffer
** @param src The source buffer
**
** @return The dst parameter
**
** NOTE:  assumes dst is large enough to hold the copied string
*/
char *strcpy( register char *dst, register const char *src );

/**
** strcat(dst,src) - append one string to another
**
** @param dst The destination buffer
** @param src The source buffer
**
** @return The dst parameter
**
** NOTE:  assumes dst is large enough to hold the resulting string
*/
char *strcat( register char *dst, register const char *src );

/**
** strcmp(s1,s2) - compare two NUL-terminated strings
**
** @param s1 The first source string
** @param s2 The second source string
**
** @return negative if s1 < s2, zero if equal, and positive if s1 > s2
*/
int strcmp( register const char *s1, register const char *s2 );

/**
** pad(dst,extra,padchar) - generate a padding string
**
** @param dst     Pointer to where the padding should begin
** @param extra   How many padding bytes to add
** @param padchar What character to pad with
**
** @return Pointer to the first byte after the padding
**
** NOTE: does NOT NUL-terminate the buffer
*/
char *pad( char *dst, int extra, int padchar );

/**
** padstr(dst,str,len,width,leftadjust,padchar - add padding characters
**                                               to a string
**
** @param dst        The destination buffer
** @param str        The string to be padded
** @param len        The string length, or -1
** @param width      The desired final length of the string
** @param leftadjust Should the string be left-justified?
** @param padchar    What character to pad with
**
** @return Pointer to the first byte after the padded string
**
** NOTE: does NOT NUL-terminate the buffer
*/
char *padstr( char *dst, char *str, int len, int width,
                   int leftadjust, int padchar );

/**
** sprint(dst,fmt,...) - formatted output into a string buffer
**
** @param dst The string buffer
** @param fmt Format string
**
** The format string parameter is followed by zero or more additional
** parameters which are interpreted according to the format string.
**
** NOTE:  assumes the buffer is large enough to hold the result string
**
** NOTE:  relies heavily on the x86 parameter passing convention
** (parameters are pushed onto the stack in reverse order as
** 32-bit values).
*/
void sprint( char *dst, char *fmt, ... );

/*
**********************************************
** MISCELLANEOUS USEFUL SUPPORT FUNCTIONS
**********************************************
*/

/**
** fake_exit()
**
** dummy "startup" function
**
** calls exit(%eax) - serves as the "return to" code for
** main() functions, in case they don't call exit() themselves
*/
void fake_exit( void );

/**
** cvt_dec(buf,value)
**
** convert a 32-bit signed value into a NUL-terminated character string
**
** @param buf    Destination buffer
** @param value  Value to convert
**
** @return The number of characters placed into the buffer
**          (not including the NUL)
**
** NOTE:  assumes buf is large enough to hold the resulting string
*/
int cvt_dec( char *buf, int32_t value );

/**
** cvt_hex(buf,value)
**
** convert a 32-bit unsigned value into an (up to) 8-character
** NUL-terminated character string
**
** @param buf    Destination buffer
** @param value  Value to convert
**
** @return The number of characters placed into the buffer
**          (not including the NUL)
**
** NOTE:  assumes buf is large enough to hold the resulting string
*/
int cvt_hex( char *buf, uint32_t value );

/**
** cvt_oct(buf,value)
**
** convert a 32-bit unsigned value into an (up to) 11-character
** NUL-terminated character string
**
** @param buf   Destination buffer
** @param value Value to convert
**
** @return The number of characters placed into the buffer
**          (not including the NUL)
**
** NOTE:  assumes buf is large enough to hold the resulting string
*/
int cvt_oct( char *buf, uint32_t value );

#endif
/* SP_ASM_SRC */

#endif
