/*
 * Copyright (C) 2001 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 *
 * Definitions for the sb1250_prof pseudo device.
 *
 */
//#include <sys/param.h>
//#include <sys/device.h>

#ifndef SB1250_PROF_H
#define SB1250_PROF_H 1

#define MAXCPUS 1
#define NEWPIDSIZE 160
/* The two definitions below must match those in pgather.c */
#define MAX_COUNTERS 4
#define MAX_SLOTS 8

#define STATS

typedef struct {
  u_int8_t  event;
  u_int8_t  hwcode;     /* pcarch-specific bits identifying desired event */
  u_int64_t period;     /* 40-bit sampling period during slot */
} sbprof_mux_counter_t;

typedef struct {
  sbprof_mux_counter_t
                counter[MAX_COUNTERS];	/* data for setting up counter */
  u_int8_t	n;			/* number of counters to use in slot */
} sbprof_mux_slot_t;

typedef struct {
  sbprof_mux_slot_t slots[MAX_SLOTS];
} sbprof_mux_slots;

typedef struct {
  u_int64_t total;	/* total number of interrupts */
  u_int64_t dropped;	/* total interrupts when buffers were full */
  u_int64_t value;	/* initial counter value when slot next entered */
  u_int32_t start_period_low;	/* counter val for starting a period */
  u_int8_t  start_period_high;
  u_int8_t  event;      /* pcarch-specific event_t */
  u_int8_t  hwcode;	/* pcarch-specific code for event */
  u_int8_t  inuse;	/* Is counter X slot actually used? */
} event_counter_state_t;

typedef struct {
  u_int32_t total;	 /* total for current run of slot */
  u_int32_t dropped;	 /* dropped for current run of slot */
  u_int64_t start_period;/* counter value to start a full period */
} active_counter_state_t;

struct _cpudata1 {
  /******* page-aligned boundary *********************************************/
  u_int32_t last_pid;	/* Pid for last sample in buffer (-1 initially) */
  u_int8_t  curbuf;	/* 2 -> both buffers full at end of last interrupt
			   1 -> use buffer[1]
			   0 -> use buffer[0] */
  u_int8_t  nextbuf;	/* the index of the next buffer to be filled */
  u_int8_t  nnewpid[2]; /* number of entries set in new_pid[i] */
  u_int32_t next;       /* index of next free entry in curbuf */
  int32_t   last_event; /* index of byte just past last event DP_D_EVENTS
			   message in curbuf that needs the number of events
			   filled in.  -1 means there is no such message and
			   that such a message must be entered before
			   adding event samples.
			 */
  /* 16-byte boundary */
  volatile u_int32_t full[2];
                        /* full[i] > 0 means buffer[i] needs emptying.
			   0 to nonzero done by sbprofintr().
			   nonzero to zero by sbprofioctl().
			   When nonzero, full[i] is the number of bytes
			   set in buffer[i].
			*/
  u_int32_t threshold;  /* when does the current multiplexing slot expire? */
  u_int8_t slotid;      /* index into slots[] of current multiplexing slot */
  u_int8_t nslots;	/* number of slots */
  u_int8_t needs_scan;
  u_int8_t pad[1];
  /******* 32-byte boundary *********************************************/
  active_counter_state_t cur_slot[MAX_COUNTERS];
  /******* 32-byte boundary *********************************************/
  event_counter_state_t event_counter_state[MAX_SLOTS][MAX_COUNTERS];
  /******* 32-byte boundary *********************************************/
  u_int32_t overshot[MAX_SLOTS];

  u_int32_t newpid[2][NEWPIDSIZE];  /* new_pid[i][] contains indices of
				       buffer[i][] where a new_pid message
				       is encoded and needs to have the
				       image_id, base addr, and num_inst
				       fields set by the user-level daemon */
#ifdef STATS
  u_int64_t newpidlimit;
  u_int64_t buflimit;
#endif
};

#define SBPROF_BUFSIZE (32*1024 - (sizeof(struct _cpudata1)+1)/2)

typedef struct _cpudata {
  struct _cpudata1 x;
  u_int8_t buffer[2][SBPROF_BUFSIZE];
} cpudata_t;

#define SBPROF_START    _IOW('S', 0x1, sbprof_mux_slots)
#define SBPROF_STOP     _IO('S', 0x2)
#define SBPROF_BUFFULL  _IOWR('S', 0x3, int)
#define SBPROF_BUFEMPTY _IOW('S', 0x4, int)

#if NEWPIDSIZE > 254
#error "newpidsize too big"
#endif

#endif /* SB1250_PROF_H */
