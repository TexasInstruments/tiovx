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

static vx_status VX_CALLBACK tivxAddKernelAddSubValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[3U];
    vx_scalar scalar;
    vx_df_image fmt[3U], out_fmt;
    vx_enum type;
    vx_uint32 i, w[3U], h[3U];

    for (i = 0U; i < TIVX_KERNEL_ADDSUB_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_ADDSUB_IN0_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_ADDSUB_IN1_IMG_IDX];
        img[2U] = (vx_image)parameters[TIVX_KERNEL_ADDSUB_OUT_IMG_IDX];
        scalar = (vx_scalar)parameters[TIVX_KERNEL_ADDSUB_IN_SCALAR_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &type, sizeof(type));

        if (VX_SUCCESS == status)
        {
            if (type != VX_TYPE_ENUM)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));

        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for frame sizes */
        if ((w[0U] != w[1U]) || (h[0U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (((fmt[0U] != VX_DF_IMAGE_U8) && (VX_DF_IMAGE_S16 != fmt[0U])) ||
            ((fmt[1U] != VX_DF_IMAGE_U8) && (VX_DF_IMAGE_S16 != fmt[1U])))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    out_fmt = VX_DF_IMAGE_S16;
    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[2U])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[2U], VX_IMAGE_FORMAT, &fmt[2U],
            sizeof(fmt[2U]));
        status |= vxQueryImage(img[2U], VX_IMAGE_WIDTH, &w[2U], sizeof(w[2U]));
        status |= vxQueryImage(img[2U], VX_IMAGE_HEIGHT, &h[2U], sizeof(h[2U]));

        /* Check for frame sizes */
        if ((w[0U] != w[2U]) || (h[0U] != h[2U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Output format is U8 only if both source images are
           VX_DF_IMAGE_U8 and the output image is explicitly set
           to VX_DF_IMAGE_U8 */
        if ((fmt[2U] == VX_DF_IMAGE_U8) &&
            (fmt[1U] != VX_DF_IMAGE_U8) && (fmt[0U] != VX_DF_IMAGE_U8))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            out_fmt = VX_DF_IMAGE_U8;
        }
    }

    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < TIVX_KERNEL_ADDSUB_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
                    sizeof(out_fmt));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &w[0U],
                    sizeof(w[0U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &h[0U],
                    sizeof(h[0U]));
            }
        }
    }

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
                            tivxAddKernelAddSubValidate,
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
                            tivxAddKernelAddSubValidate,
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



