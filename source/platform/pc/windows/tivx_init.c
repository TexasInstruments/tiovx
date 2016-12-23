/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

void tivxRegisterOpenVXCoreTargetKernels(void);
void tivxUnRegisterOpenVXCoreTargetKernels(void);

void tivxInit(void)
{
    /* Initialize platform */
    tivxPlatformInit();

    /* Initialize Target */
    tivxTargetInit();

    /* Initialize Host */
    tivxRegisterOpenVXCoreTargetKernels();
    tivxHostInit();

    tivxObjDescInit();

    tivxTargetConfig();
}

void tivxDeInit(void)
{
    /* DeInitialize Host */
    tivxUnRegisterOpenVXCoreTargetKernels();
    tivxHostDeInit();

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();
}
