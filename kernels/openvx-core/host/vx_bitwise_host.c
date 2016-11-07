/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_bitwise.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_bitwise_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelBitwiseValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img;
    vx_df_image fmt;
    uint32_t val;

    /* Setting up metadata */
    img = (vx_image)parameters[TIVX_KERNEL_BITWISE_IN_IMG_IDX];
    vxQueryImage(img, VX_IMAGE_FORMAT, &fmt, sizeof(fmt));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_FORMAT, &fmt, sizeof(fmt));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_FORMAT, &fmt, sizeof(fmt));

    vxQueryImage(img, VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_WIDTH, &val, sizeof(val));

    vxQueryImage(img, VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_HEIGHT, &val, sizeof(val));

    return status;
}

vx_status tivxAddKernelBitwise(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.not",
                            VX_KERNEL_NOT,
                            NULL,
                            2,
                            tivxAddKernelBitwiseValidate,
                            NULL,
                            NULL);

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
                VX_OUTPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
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

    vx_bitwise_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelBitwise(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_bitwise_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_bitwise_kernel = NULL;

    return status;
}





