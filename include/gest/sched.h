/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_SCHED_H
#define GEST_SCHED_H

#include <threads.h>

typedef void (*GestSchedFunc)(void *);
typedef struct GestSchedulerJob GestSchedulerJob;
typedef struct GestSchedCircularQueue GestSchedCircularQueue;

typedef struct GestLinearScheduler GestLinearScheduler;

struct GestSchedulerJob {
    GestSchedFunc func;
    void *arg;
};

#define GEST_DEFAULT_JOB_CAPACITY 4096

struct GestSchedCircularQueue {
    size_t head;
    size_t tail;
    size_t count;
    size_t capacity;
    GestSchedulerJob *queue;
};

struct GestLinearScheduler {
    int threadCount;
    thrd_t *threads;

    GestSchedCircularQueue queue;
};

#endif // GEST_SCHED_H
