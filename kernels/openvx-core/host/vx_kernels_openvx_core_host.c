/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx.h>



vx_status tivxAddKernelAbsDiff(vx_context context);
vx_status tivxRemoveKernelAbsDiff(vx_context context);
vx_status tivxAddKernelLut(vx_context context);
vx_status tivxRemoveKernelLut(vx_context context);
vx_status tivxAddKernelBitwise(vx_context context);
vx_status tivxRemoveKernelBitwise(vx_context context);
vx_status tivxAddKernelAdd(vx_context context);
vx_status tivxRemoveKernelAdd(vx_context context);
vx_status tivxAddKernelSub(vx_context context);
vx_status tivxRemoveKernelSub(vx_context context);
vx_status tivxAddKernelThreshold(vx_context context);
vx_status tivxRemoveKernelThreshold(vx_context context);



static vx_status tivxPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;

    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelAbsDiff(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelLut(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelBitwise(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelAdd(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelSub(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelThreshold(context);
    }

    return status;
}

static vx_status tivxUnPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;

    if(status == VX_SUCCESS)
    {
        status  = tivxRemoveKernelAbsDiff(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxRemoveKernelLut(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxRemoveKernelBitwise(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxRemoveKernelAdd(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxRemoveKernelSub(context);
    }
    if(status == VX_SUCCESS)
    {
        status  = tivxRemoveKernelThreshold(context);
    }

    return status;
}

void tivxRegisterOpenVXCoreKernels()
{
    tivxRegisterModule(TIVX_MODULE_NAME, tivxPublishKernels, tivxUnPublishKernels);
}

void tivxUnRegisterOpenVXCoreKernels()
{
    tivxUnRegisterModule(TIVX_MODULE_NAME);
}
