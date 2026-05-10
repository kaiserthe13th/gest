/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_VM_H
#define GEST_VM_H

#include <gest/common.h>

typedef struct GestVM GestVM;
typedef struct GestProcess GestProcess;

typedef uint64_t GestHandle;
typedef GestHandle GestProcessHandle;

#ifdef GEST_BUILD
struct GestVM {
    size_t processCount;
    GestProcess *process;
};

struct GestProcess {
    GestProcessHandle processId;
};
#endif

#endif // GEST_VM_H
