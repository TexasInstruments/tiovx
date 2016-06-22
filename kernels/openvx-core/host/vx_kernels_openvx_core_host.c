/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/vx_ext_ti.h>

#define TIVX_MODULE_NAME    "openvx-core"

vx_status tivxAddKernelAbsDiff(vx_context context);
vx_status tivxRemoveKernelAbsDiff(vx_context context);


static vx_status tivxPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;

    if(status == VX_SUCCESS)
    {
        status  = tivxAddKernelAbsDiff(context);
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

    return status;
}

void tivxRegisterOpenVXCoreKernels()
{
    vxRegisterModule(TIVX_MODULE_NAME, tivxPublishKernels, tivxUnPublishKernels);
}

void tivxUnRegisterOpenVXCoreKernels()
{
    vxUnRegisterModule(TIVX_MODULE_NAME);
}
