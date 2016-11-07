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
#include <tivx_kernel_addsub.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_add_kernel = NULL, vx_subtract_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelAddValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image src0, src1;
    vx_df_image fmt0, fmt1;
    vx_uint32 val;

    src0 = (vx_image)parameters[0];
    src1 = (vx_image)parameters[1];

    vxQueryImage(src0, VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxQueryImage(src1, VX_IMAGE_FORMAT, &fmt1, sizeof(fmt1));

    if ((VX_DF_IMAGE_S16 == fmt0) || (VX_DF_IMAGE_S16 == fmt1))
    {
        fmt0 = VX_DF_IMAGE_S16;
    }
    else
    {
        fmt0 = VX_DF_IMAGE_U8;
    }
    fmt0 = VX_DF_IMAGE_S16;

    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxSetMetaFormatAttribute(metas[2], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxSetMetaFormatAttribute(metas[3], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));

    vxQueryImage(src0, VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[2], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[3], VX_IMAGE_WIDTH, &val, sizeof(val));

    vxQueryImage(src0, VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[2], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[3], VX_IMAGE_HEIGHT, &val, sizeof(val));


    return status;
}

static vx_status VX_CALLBACK tivxAddKernelSubValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image src0, src1;
    vx_df_image fmt0, fmt1;
    vx_uint32 val;

    src0 = (vx_image)parameters[0];
    src1 = (vx_image)parameters[1];

    vxQueryImage(src0, VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxQueryImage(src1, VX_IMAGE_FORMAT, &fmt1, sizeof(fmt1));

    if ((VX_DF_IMAGE_S16 == fmt0) || (VX_DF_IMAGE_S16 == fmt1))
    {
        fmt0 = VX_DF_IMAGE_S16;
    }
    else
    {
        fmt0 = VX_DF_IMAGE_U8;
    }
    fmt0 = VX_DF_IMAGE_S16;

    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxSetMetaFormatAttribute(metas[2], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));
    vxSetMetaFormatAttribute(metas[3], VX_IMAGE_FORMAT, &fmt0, sizeof(fmt0));

    vxQueryImage(src0, VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[2], VX_IMAGE_WIDTH, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[3], VX_IMAGE_WIDTH, &val, sizeof(val));

    vxQueryImage(src0, VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[0], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[1], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[2], VX_IMAGE_HEIGHT, &val, sizeof(val));
    vxSetMetaFormatAttribute(metas[3], VX_IMAGE_HEIGHT, &val, sizeof(val));

    return status;
}


vx_status tivxAddKernelAdd(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.add",
                            VX_KERNEL_ADD,
                            NULL,
                            4,
                            tivxAddKernelAddValidate,
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
                VX_TYPE_SCALAR,
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

    vx_add_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelAdd(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_add_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_add_kernel = NULL;

    return status;
}


vx_status tivxAddKernelSub(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.subtract",
                            VX_KERNEL_SUBTRACT,
                            NULL,
                            4,
                            tivxAddKernelSubValidate,
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
                VX_TYPE_SCALAR,
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

    vx_subtract_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelSub(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_subtract_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_add_kernel = NULL;

    return status;
}



