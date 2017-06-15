/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 * \file vx_tutorial_target_kernel.c APIs to add/remove target kernels defined in tutorial to the target side.
 *
 */
#include <TI/tivx.h>

void vxTutorialAddTargetKernelPhaseRgb(void);
void vxTutorialRemoveTargetKernelPhaseRgb(void);

/**
 * \brief Add target kernels for tutorial to C66x DSP target
 *
 *   This API is called by during target init by the target executable
 */
void tivxRegisterTutorialTargetKernels()
{
    vxTutorialAddTargetKernelPhaseRgb();
}

/**
 * \brief Remove target kernels for tutorial from C66x DSP target
 *
 *   This API is called by during target de-init by the target executable
 */
void tivxUnRegisterTutorialTargetKernels()
{
    vxTutorialRemoveTargetKernelPhaseRgb();
}

