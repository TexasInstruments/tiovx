/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_halfscale_gaussian.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_halfscale_gaussian_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelHalfscaleGaussianValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_uint32 w[2U], h[2U], i;
    vx_df_image fmt[2U];
    vx_scalar scalar;
    vx_int32 gsize;
    vx_enum stype;
    vx_border_t border;

    for (i = 0U; i < TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];
        scalar  = (vx_scalar)parameters[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype, sizeof(stype));
        if ((VX_SUCCESS == status) && (stype == VX_TYPE_INT32))
        {
            status = vxCopyScalar(scalar, &gsize, VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST);

            if (VX_SUCCESS == status)
            {
                if ((gsize == 1) ||
                    (gsize == 3) ||
                    (gsize == 5))
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    status = VX_ERROR_INVALID_VALUE;
                }
            }
        }
        else
        {
            status = VX_ERROR_INVALID_TYPE;
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt[0U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for format */
        if (fmt[0U] != fmt[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));
        if (VX_SUCCESS == status)
        {
            if (border.mode != VX_BORDER_UNDEFINED)
            {
                status = VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for halfscale gaussian\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < 2U; i ++)
        {
            if (NULL != metas[i])
            {
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &fmt[i],
                    sizeof(fmt[i]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &w[i],
                    sizeof(w[i]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &h[i],
                    sizeof(h[i]));
            }
        }
    }
    
    return status;
}

vx_status tivxAddKernelHalfscaleGaussian(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    
    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.halfscale_gaussian",
                VX_KERNEL_HALFSCALE_GAUSSIAN,
                NULL,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS,
                tivxAddKernelHalfscaleGaussianValidate,
                NULL,
                NULL);
    
    status = vxGetStatus((vx_reference)kernel);
    if (status == VX_SUCCESS)
    {
        index = 0;
        
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_SCALAR,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
        }
        if (status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_halfscale_gaussian_kernel = kernel;
    
    return status;
}

vx_status tivxRemoveKernelHalfscaleGaussian(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_halfscale_gaussian_kernel;
    
    status = vxRemoveKernel(kernel);
    vx_halfscale_gaussian_kernel = NULL;
    
    return status;
}


