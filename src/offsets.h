/**
** @file offsets.h
**
** GENERATED AUTOMATICALLY - DO NOT EDIT
**
** Creation date: Mon Mar  4 17:59:17 2024
**
** This header file contains C Preprocessor macros which expand
** into the byte offsets needed to reach fields within structs
** used in the baseline system.  Should those struct declarations
** change, the Offsets program should be modified (if needed),
** recompiled, and re-run to recreate this file.
*/

#ifndef OFFSETS_H_
#define OFFSETS_H_

// Sizes of basic types

#define	SZ_char                	1
#define	SZ_short               	2
#define	SZ_int                 	4
#define	SZ_long                	4
#define	SZ_long_long           	8
#define	SZ_pointer             	4


// Sizes of our types

#define	SZ_int8_t              	1
#define	SZ_uint8_t             	1
#define	SZ_int16_t             	2
#define	SZ_uint16_t            	2
#define	SZ_int32_t             	4
#define	SZ_uint32_t            	4
#define	SZ_int64_t             	8
#define	SZ_uint64_t            	8
#define	SZ_bool_t              	1
#define	SZ_status_t            	4
#define	SZ_state_t             	1
#define	SZ_pid_t               	2
#define	SZ_prio_t              	1
#define	SZ_context_t           	72
#define	SZ_pcb_t               	32
#define	SZ_time_t              	4
#define	SZ_qnode_t             	12
#define	SZ_queue_t             	16
#define	SZ_compare_t           	4


// context_t structure

#define	SZ_CTX                 	72

#define	CTX_ss                 	0
#define	CTX_gs                 	4
#define	CTX_fs                 	8
#define	CTX_es                 	12
#define	CTX_ds                 	16
#define	CTX_edi                	20
#define	CTX_esi                	24
#define	CTX_ebp                	28
#define	CTX_esp                	32
#define	CTX_ebx                	36
#define	CTX_edx                	40
#define	CTX_ecx                	44
#define	CTX_eax                	48
#define	CTX_vector             	52
#define	CTX_code               	56
#define	CTX_eip                	60
#define	CTX_cs                 	64
#define	CTX_eflags             	68


// pcb_t structure

#define	SZ_PCB                 	32

#define	PCB_context            	0
#define	PCB_stack              	4
#define	PCB_exit_status        	8
#define	PCB_wakeup             	12
#define	PCB_pid                	16
#define	PCB_ppid               	18
#define	PCB_state              	20
#define	PCB_ticks_left         	21
#define	PCB_priority           	22
#define	PCB_filler             	23


// qnode_t structure

#define	SZ_QND                 	12

#define	QND_prev               	0
#define	QND_next               	4
#define	QND_data               	8


// queue_t structure

#define	SZ_QUE                 	16

#define	QUE_head               	0
#define	QUE_tail               	4
#define	QUE_compare            	8
#define	QUE_length             	12

#endif
