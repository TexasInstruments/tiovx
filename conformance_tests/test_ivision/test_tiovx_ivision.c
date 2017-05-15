/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include "test_tiovx_ivision.h"
#include <TI/tivx.h>

void tivxRegisterIVisionCoreKernels(void);
void tivxUnRegisterIVisionCoreKernels(void);

static uint32_t gIsIVisionKernelsLoad = 0u;

void IVisionPrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName)
{
    printf("[ %c%c ] Execution time for %9d pixels (avg = %4d.%-6d ms, min = %4d.%-6d ms, max = %4d.%-6d ms)\n",
        testName[0], testName[1],
        (int)numPixels,
        (int)(performance.avg/1000000),
        (int)(performance.avg%1000000),
        (int)(performance.min/1000000),
        (int)(performance.min%1000000),
        (int)(performance.max/1000000),
        (int)(performance.max%1000000)
        );
}

void IVisionLoadKernels(vx_context context)
{
    if ((0 == gIsIVisionKernelsLoad) &&
        (NULL != context))
    {
        tivxRegisterIVisionCoreKernels();
        vxLoadKernels(context, TIVX_MODULE_NAME1);
        gIsIVisionKernelsLoad = 1U;
    }
}

void IVisionUnLoadKernels(vx_context context)
{
    if ((1u == gIsIVisionKernelsLoad) &&
        (NULL != context))
    {
        vxUnloadKernels(context, TIVX_MODULE_NAME1);
        tivxUnRegisterIVisionCoreKernels();

        gIsIVisionKernelsLoad = 0U;
    }
}

