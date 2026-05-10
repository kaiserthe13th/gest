/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/gest.h>

uint32_t gestVersion() {
    return GEST_VERSION;
}

const char *gestVersionString() {
    return GEST_VERSION_STRING;
}
