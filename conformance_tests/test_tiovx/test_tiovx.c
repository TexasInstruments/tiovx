/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "test_tiovx.h"

void printPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName)
{
    printf("%c%c tmp = %" PRIu64 "\n", testName[0], testName[1], performance.tmp);
    printf("%c%c beg = %" PRIu64 "\n", testName[0], testName[1], performance.beg);
    printf("%c%c end = %" PRIu64 "\n", testName[0], testName[1], performance.end);
    printf("%c%c sum = %" PRIu64 "\n", testName[0], testName[1], performance.sum);
    printf("%c%c avg = %" PRIu64 "\n", testName[0], testName[1], performance.avg);
    printf("%c%c min = %" PRIu64 "\n", testName[0], testName[1], performance.min);
    printf("%c%c num = %" PRIu64 "\n", testName[0], testName[1], performance.num);
    printf("%c%c max = %" PRIu64 "\n", testName[0], testName[1], performance.max);
    printf("%c%c num pixels = %d\n", testName[0], testName[1], numPixels);
}
