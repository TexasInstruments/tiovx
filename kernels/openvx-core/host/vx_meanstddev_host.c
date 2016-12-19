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
#include <tivx_kernel_meanstddev.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_meanstddev_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelMeanStdDevValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img;
    vx_uint32 w, h, i;
    vx_df_image fmt;
    vx_scalar scalar;
    vx_enum stype;

    for (i = 0U; i < TIVX_KERNEL_MSD_MAX_PARAMS; i ++)
    {
        /* Check for NULL */
        if (NULL == parameters[i])
        {
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        img = (vx_image)parameters[TIVX_KERNEL_MSD_IN_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, VX_IMAGE_FORMAT, &fmt, sizeof(fmt));

        status |= vxQueryImage(img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_MSD_OUT_MEAN_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if (stype != VX_TYPE_FLOAT32)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        scalar = (vx_scalar)parameters[TIVX_KERNEL_MSD_OUT_STDDEV_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));
        if (VX_SUCCESS == status)
        {
            if (stype != VX_TYPE_FLOAT32)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        stype = VX_TYPE_FLOAT32;
        if (NULL != metas[TIVX_KERNEL_MSD_OUT_MEAN_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MSD_OUT_MEAN_IDX],
                VX_SCALAR_TYPE, &stype, sizeof(stype));
        }
        if (NULL != metas[TIVX_KERNEL_MSD_OUT_STDDEV_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MSD_OUT_STDDEV_IDX],
                VX_SCALAR_TYPE, &stype, sizeof(stype));
        }
    }

    return status;
}

vx_status tivxAddKernelMeanStdDev(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.mean_stddev",
                            VX_KERNEL_MEAN_STDDEV,
                            NULL,
                            3,
                            tivxAddKernelMeanStdDevValidate,
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
                VX_TYPE_SCALAR,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
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

    vx_meanstddev_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelMeanStdDev(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_meanstddev_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_meanstddev_kernel = NULL;

    return status;
}





