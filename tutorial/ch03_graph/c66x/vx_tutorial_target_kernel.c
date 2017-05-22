/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>

void vxTutorialAddTargetKernelPhaseRgb(void);
void vxTutorialRemoveTargetKernelPhaseRgb(void);

void tivxRegisterTutorialTargetKernels()
{
    vxTutorialAddTargetKernelPhaseRgb();
}

void tivxUnRegisterTutorialTargetKernels()
{
    vxTutorialRemoveTargetKernelPhaseRgb();
}

