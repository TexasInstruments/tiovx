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
    printf("[ %c%c ] Execution time for %9d pixels (avg = %4d.%-6d ms, min = %4d.%-6d ms, max = %4d.%-6d ms)\n",
        testName[0], testName[1],
        numPixels,
        (uint32_t)(performance.avg/1000000),
        (uint32_t)(performance.avg%1000000),
        (uint32_t)(performance.min/1000000),
        (uint32_t)(performance.min%1000000),
        (uint32_t)(performance.max/1000000),
        (uint32_t)(performance.max%1000000)
        );
}
