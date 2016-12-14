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
#include <tivx_kernel_remap.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_remap_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelRemapValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_uint32 w[2U], h[2U], i;
    vx_uint32 rmp_w[2U], rmp_h[2U];
    vx_df_image fmt[2U];
    vx_scalar spolicy;
    vx_remap remap;
    vx_enum stype;
    vx_border_t border;

    for (i = 0U; i < TIVX_KERNEL_REMAP_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_REMAP_IN_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_REMAP_OUT_IMG_IDX];
        spolicy = (vx_scalar)parameters[TIVX_KERNEL_REMAP_IN_POLICY_IDX];
        remap   = (vx_remap)parameters[TIVX_KERNEL_REMAP_IN_TBL_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
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
        status = vxQueryScalar(spolicy, VX_SCALAR_TYPE, &stype, sizeof(stype));
        if ((VX_SUCCESS == status) && (stype == VX_TYPE_ENUM))
        {
            vx_enum interp = 0;

            status = vxCopyScalar(spolicy, &interp, VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST);

            if (VX_SUCCESS == status)
            {
                if ((interp == VX_INTERPOLATION_NEAREST_NEIGHBOR) ||
                    (interp == VX_INTERPOLATION_BILINEAR))
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
        status = vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));
        if (VX_SUCCESS == status)
        {
            if ((border.mode != VX_BORDER_UNDEFINED) &&
                (border.mode != VX_BORDER_CONSTANT))
            {
                status = VX_ERROR_NOT_SUPPORTED;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryRemap(remap, VX_REMAP_SOURCE_WIDTH, &rmp_w[0U],
            sizeof(rmp_w[0U]));
        status |= vxQueryRemap(remap, VX_REMAP_SOURCE_HEIGHT, &rmp_h[0U],
            sizeof(rmp_h[0U]));
        status |= vxQueryRemap(remap, VX_REMAP_DESTINATION_WIDTH, &rmp_w[1U],
            sizeof(rmp_w[1U]));
        status |= vxQueryRemap(remap, VX_REMAP_DESTINATION_HEIGHT, &rmp_h[1U],
            sizeof(rmp_h[1U]));

        if (VX_SUCCESS == status)
        {
            /* Check for frame sizes */
            if ((rmp_w[0U] != w[0U]) || (rmp_h[0U] != h[0U]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for frame sizes */
        if ((rmp_w[1U] != w[1U]) || (rmp_h[1U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (fmt[0U] != fmt[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < TIVX_KERNEL_REMAP_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &fmt[0U],
                    sizeof(fmt[0U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &rmp_w[1U],
                    sizeof(rmp_w[1U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &rmp_h[1U],
                    sizeof(rmp_h[1U]));
            }
        }
    }
    return status;
}

vx_status tivxAddKernelRemap(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.remap",
                            VX_KERNEL_REMAP,
                            NULL,
                            4,
                            tivxAddKernelRemapValidate,
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
                VX_TYPE_REMAP,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_ENUM,
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

    vx_remap_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelRemap(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_remap_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_remap_kernel = NULL;

    return status;
}





