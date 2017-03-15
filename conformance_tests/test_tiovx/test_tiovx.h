/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "test_engine/test.h"

#define ADD_SIZE_18x18(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=18x18", __VA_ARGS__, 18, 18))

#define ADD_SIZE_640x480(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ADD_SIZE_644x258(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=644x258", __VA_ARGS__, 644, 258))

#define ADD_SIZE_1600x1200(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=1600x1200", __VA_ARGS__, 1600, 1200))


#define __STDC_FORMAT_MACROS
#include <inttypes.h>

void printPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);

