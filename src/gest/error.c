/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#include <gest/error.h>
#include <threads.h>

thread_local int gest_errno_value__ = GEST_ENONE;

int *gestErrno__(void) {
    return &gest_errno_value__;
}

const char *gestErrName(int error) {
    switch (error) {
        GESTX_ERRORS(GESTX_ENUM_NAME_CASE)
    default: return "(unknown)";
    }
}

const char *gestErrConstMessage(int error) {

    switch (error) {
    case GEST_ENONE: return "no errors";
    case GEST_EUNSUP: return "unsupported";
    case GEST_EINVAL: return "invalid value";
    case GEST_EINVALIGN: return "invalid alignment";
    case GEST_ENOSPACE: return "no space left";
    case GEST_EBIG: return "too big";
    case GEST_EARITH: return "arithmetic error";
    default: return "(unknown)";
    }
}
