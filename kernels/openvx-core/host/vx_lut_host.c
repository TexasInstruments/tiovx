/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx.h>

static vx_kernel vx_lut_kernel = NULL;


static vx_status VX_CALLBACK tivxAddKernelLutValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelLutInitialize(vx_node node,
    const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelLutDeInitialize(vx_node node,
    const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    return status;
}


vx_status tivxAddKernelLut(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.table_lookup",
                            VX_KERNEL_TABLE_LOOKUP,
                            NULL,
                            3,
                            tivxAddKernelLutValidate,
                            tivxAddKernelLutInitialize,
                            tivxAddKernelLutDeInitialize);

    status = vxGetStatus((vx_reference)kernel);

    if ( status == VX_SUCCESS)
    {
        index = 0;

        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_LUT,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_IPU1_0);
        }

        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if( status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }

    vx_lut_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelLut(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_lut_kernel;

    status = vxRemoveKernel(kernel);
    if(status==VX_SUCCESS)
    {
        status = vxReleaseKernel(&kernel);
    }
    vx_lut_kernel = NULL;

    return status;
}





