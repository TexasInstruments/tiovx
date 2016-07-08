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
    tivxRegisterModule(TIVX_MODULE_NAME, tivxPublishKernels, tivxUnPublishKernels);
}

void tivxUnRegisterOpenVXCoreKernels()
{
    tivxUnRegisterModule(TIVX_MODULE_NAME);
}
