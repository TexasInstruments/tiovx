/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

void tivxRegisterOpenVXCoreKernels(void);
void tivxRegisterIVisionCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);
void tivxUnRegisterIVisionCoreKernels(void);

void tivxHostInit(void)
{
    tivxObjectInit();
    tivxObjDescInit();
    tivxRegisterOpenVXCoreKernels();
    tivxRegisterIVisionCoreKernels();
}

void tivxHostDeInit(void)
{
    tivxObjectDeInit();
    tivxUnRegisterOpenVXCoreKernels();
    tivxUnRegisterIVisionCoreKernels();
}
