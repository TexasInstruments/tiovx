/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "test_engine/test.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

void IVisionPrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);
void IVisionLoadKernels(vx_context context);
void IVisionUnLoadKernels(vx_context context);
