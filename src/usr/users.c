/**
** @file	users.c
**
** @author	numerous CSCI-452 classes
**
** @brief	User-level code.
*/

#include "common.h"

#include "users.h"

/*
** USER PROCESSES
**
** Each is designed to test some facility of the OS; see the users.h
** header file for a summary of which system calls are tested by
** each user function.
**
** Output from user processes is usually alphabetic.  Uppercase
** characters are "expected" output; lowercase are (typically) "erroneous"
** output.
**
** More specific information about each user process can be found in
** the header comment for that function in its source file.
**
** To spawn a specific user process, uncomment its SPAWN_x
** definition in the users.h header file.
*/

/*
** Prototypes for all user main routines (even ones that may not exist,
** for completeness)
*/

USERMAIN(idle);  USERMAIN(shell); USERMAIN(wtsh_main);

USERMAIN(main1); USERMAIN(main2); USERMAIN(main3); USERMAIN(main4);
USERMAIN(main5); USERMAIN(main6); USERMAIN(main7); USERMAIN(main8);

USERMAIN(userA); USERMAIN(userB); USERMAIN(userC); USERMAIN(userD);
USERMAIN(userE); USERMAIN(userF); USERMAIN(userG); USERMAIN(userH);
USERMAIN(userI); USERMAIN(userJ); USERMAIN(userK); USERMAIN(userL);
USERMAIN(userM); USERMAIN(userN); USERMAIN(userO); USERMAIN(userP);
USERMAIN(userQ); USERMAIN(userR); USERMAIN(userS); USERMAIN(userT);
USERMAIN(userU); USERMAIN(userV); USERMAIN(userW); USERMAIN(userX);
USERMAIN(userY); USERMAIN(userZ);

USERMAIN(test_vga);
USERMAIN(test_vfs);

/*
** The user processes
**
** We #include the source code from the userland/ directory only if
** a specific process is being spawned.
**
** Remember to #include the code required by any process that will
** be spawned - e.g., userH spawns userZ.  The user code files all
** contain CPP include guards, so multiple inclusion of a source
** file shouldn't cause problems.
*/

#if defined(SPAWN_A) || defined(SPAWN_B) || defined(SPAWN_C)
#include "userland/main1.c"
#endif

#if defined(SPAWN_D) || defined(SPAWN_E)
#include "userland/main2.c"
#endif

#if defined(SPAWN_F) || defined(SPAWN_G)
#include "userland/main3.c"
#endif

#if defined(SPAWN_H)
#include "userland/userH.c"
#include "userland/userZ.c"
#endif

#if defined(SPAWN_I)
#include "userland/userI.c"
#include "userland/userW.c"
#endif

#if defined(SPAWN_J)
#include "userland/userJ.c"
#include "userland/userY.c"
#endif

#if defined(SPAWN_K) || defined(SPAWN_L)
#include "userland/main4.c"
#include "userland/userX.c"
#endif

#if defined(SPAWN_M) || defined(SPAWN_N)
#include "userland/main5.c"
#include "userland/userW.c"
#include "userland/userZ.c"
#endif

#if defined(SPAWN_P)
#include "userland/userP.c"
#endif

#if defined(SPAWN_Q)
#include "userland/userQ.c"
#endif

#if defined(SPAWN_R)
#include "userland/userR.c"
#endif

#if defined(SPAWN_S)
#include "userland/userS.c"
#endif

#if defined(SPAWN_T) || defined(SPAWN_U) || defined(SPAWN_V)
#include "userland/main6.c"
#include "userland/userW.c"
#endif

#if defined(SPAWN_TEST_VGA)
#include "userland/test_vga.c"
#endif

#if defined(SPAWN_TEST_VFS)
#include "userland/test_vfs.c"
#endif

#if defined(WTSH_SHELL)
#include "userland/wtsh.c"
#endif

/*
** System processes - these should always be included here
*/

// init.c contains 'init' and 'shell'
#include "userland/init.c"

#include "userland/idle.c"
