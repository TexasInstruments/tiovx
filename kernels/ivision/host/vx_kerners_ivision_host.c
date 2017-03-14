/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx.h>

typedef vx_status (*tivxHostKernel_Fxn) (vx_context context);

typedef struct {
    tivxHostKernel_Fxn    add_kernel;
    tivxHostKernel_Fxn    remove_kernel;
} Tivx_Host_Kernel_List;

vx_status tivxAddIVisionKernelHarrisCorners(vx_context context);

vx_status tivxRemoveIVisionKernelHarrisCorners(vx_context context);

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {tivxAddIVisionKernelHarrisCorners, tivxRemoveIVisionKernelHarrisCorners}
};

static vx_status VX_CALLBACK tivxPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i <
        (sizeof(gTivx_host_kernel_list)/sizeof(Tivx_Host_Kernel_List)); i ++)
    {
        if (gTivx_host_kernel_list[i].add_kernel)
        {
            status = gTivx_host_kernel_list[i].add_kernel(context);
        }

        if (VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxUnPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i <
        (sizeof(gTivx_host_kernel_list)/sizeof(Tivx_Host_Kernel_List)); i ++)
    {
        if (gTivx_host_kernel_list[i].remove_kernel)
        {
            status = gTivx_host_kernel_list[i].remove_kernel(context);
        }

        if (VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

void tivxRegisterIVisionCoreKernels(void)
{
    tivxRegisterModule(TIVX_MODULE_NAME1, tivxPublishKernels, tivxUnPublishKernels);
}

void tivxUnRegisterIVisionCoreKernels(void)
{
    tivxUnRegisterModule(TIVX_MODULE_NAME1);
}
