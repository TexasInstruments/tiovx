/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx_target_kernel.h>

typedef void (*tivxTargetKernel_Fxn) ();

typedef struct  {
    tivxTargetKernel_Fxn    add_kernel;
    tivxTargetKernel_Fxn    remove_kernel;
} Tivx_Target_Kernel_List;

void tivxAddTargetKernelHarrisCorners();
void tivxRemoveTargetKernelHarrisCorners();

Tivx_Target_Kernel_List gTivx_target_kernel_list[] = {
    {tivxAddTargetKernelHarrisCorners, tivxRemoveTargetKernelHarrisCorners}
};

void tivxRegisterIVisionTargetKernels()
{
    vx_uint32 i;

    for (i = 0; i <
        sizeof(gTivx_target_kernel_list)/sizeof(Tivx_Target_Kernel_List); i ++)
    {
        if (gTivx_target_kernel_list[i].add_kernel)
        {
            gTivx_target_kernel_list[i].add_kernel();
        }
    }
}

void tivxUnRegisterIVisionTargetKernels()
{
    vx_uint32 i;

    for (i = 0; i <
        (sizeof(gTivx_target_kernel_list)/sizeof(Tivx_Target_Kernel_List));
        i ++)
    {
        if (gTivx_target_kernel_list[i].remove_kernel)
        {
            gTivx_target_kernel_list[i].remove_kernel();
        }
    }
}
