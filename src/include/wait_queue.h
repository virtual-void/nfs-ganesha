/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) 2012, The Linux Box Corporation
 * Contributor : Matt Benjamin <matt@linuxbox.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * -------------
 */

/**
 *
 * \file wait_queue.h
 * \author Matt Benjamin
 * \brief Pthreads-based wait queue package
 *
 * \section DESCRIPTION
 *
 * This module provides simple wait queues using pthreads primitives.
 */

#ifndef WAIT_QUEUE_H
#define WAIT_QUEUE_H

#include <errno.h>
#include <pthread.h>
#include "nlm_list.h" /* XXX really time to rename this */

typedef struct wait_entry
{
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} wait_entry_t;

#define Wqe_LFlag_None        0x0000
#define Wqe_LFlag_WaitSync    0x0001
#define Wqe_LFlag_SyncDone    0x0002

/* thread wait queue */
typedef struct wait_q_entry
{
    uint32_t flags;
    uint32_t waiters;
    wait_entry_t lwe; /* left */
    wait_entry_t rwe; /* right */
    struct glist_head waitq;
} wait_q_entry_t;

static inline void
init_wait_entry(wait_entry_t *we)
{
    pthread_mutex_init(&we->mtx, NULL);
    pthread_cond_init(&we->cv, NULL);
}

static inline void
init_wait_q_entry(wait_q_entry_t *wqe)
{
    init_glist(&wqe->waitq);
    init_wait_entry(&wqe->lwe);
    init_wait_entry(&wqe->rwe);
}

static inline bool_t
thread_delay_ms(unsigned long ms)
{

    static const uint32_t S_NSECS = 1000000000UL; /* nsecs in 1s */
    static const uint32_t MS_NSECS = 1000000UL; /* nsecs in 1ms */

    wait_entry_t we = {
        .mtx = PTHREAD_MUTEX_INITIALIZER,
        .cv = PTHREAD_COND_INITIALIZER
    };
    
    time_t now = time(NULL);
    uint64_t nsecs = (S_NSECS * now) + (MS_NSECS * ms);
    struct timespec then = {
        .tv_sec = nsecs / S_NSECS,
        .tv_nsec = nsecs % S_NSECS
    };
    bool_t woke = FALSE;

    pthread_mutex_lock(&we.mtx);
    woke = (pthread_cond_timedwait(&we.cv, &we.mtx, &then) != ETIMEDOUT);
    pthread_mutex_unlock(&we.mtx);

    return (woke);
}

#endif /* WAIT_QUEUE_H */