/// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/// SPDX-FileCopyrightText: Copyright 2026 Kerem Göksu

#ifndef GEST_ERROR_H
#define GEST_ERROR_H

#include <gest/common.h>

GEST_API int *gestErrno__(void);
#define gestErrno (*gestErrno__())

enum {
#define GESTX_ERRORS(X) \
    X(GEST_ENONE /* no errors */) \
    X(GEST_EUNSUP /* unsupported */) \
    X(GEST_EINVAL /* invalid value */) \
    X(GEST_EINVALIGN /* invalid alignment */) \
    X(GEST_ENOSPACE /* no space left */) \
    X(GEST_EBIG /* too big */) \
    X(GEST_ESMALL /* too small */) \
    X(GEST_EARITH /* arithmetic error */) \
    X(GEST_EILSEQ /* illegal sequence */)
    GESTX_ERRORS(GESTX_ENUM)
};

GEST_API const char *gestErrName(int error);
GEST_API const char *gestErrConstMessage(int error);

#endif // GEST_ERROR_H
