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
#include <tivx_kernel_laplacian_reconstruct.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_laplacian_reconstruct_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image src_img, out_img;
    vx_pyramid pmd;
    vx_uint32 w, h, i;
    vx_uint32 p_w, p_h;
    vx_df_image fmt, o_fmt, p_fmt;
    vx_float32 scale;
    vx_border_t border;

    for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
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
        src_img = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_IMG_IDX];
        pmd = (vx_pyramid)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];
        out_img = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(src_img, VX_IMAGE_FORMAT, &fmt, sizeof(fmt));

        status |= vxQueryImage(src_img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(src_img, VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if ((VX_DF_IMAGE_S16 != fmt) && (VX_DF_IMAGE_U8 != fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_SCALE, &scale,
            sizeof(scale));
        if (VX_SUCCESS == status)
        {
            if (scale!= VX_SCALE_PYRAMID_HALF)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_WIDTH, &p_w, sizeof(p_w));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt));

        /* Check for validity of data format */
        if (VX_DF_IMAGE_S16 != p_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)pmd)))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(out_img, VX_IMAGE_FORMAT, &o_fmt, sizeof(o_fmt));
        status |= vxQueryImage(out_img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(out_img, VX_IMAGE_HEIGHT, &h, sizeof(h));

        /* Check for frame sizes */
        if ((w != p_w) || (h != p_h))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (o_fmt != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));
        if (VX_SUCCESS == status)
        {
            if ((border.mode != VX_BORDER_UNDEFINED) &&
                (border.mode != VX_BORDER_REPLICATE))
            {
                status = VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only undefined and replicate border mode is supported for laplacian reconstruct\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if (NULL != metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX],
                VX_IMAGE_WIDTH, &p_w, sizeof(p_w));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX],
                VX_IMAGE_HEIGHT, &p_h, sizeof(p_h));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX],
                VX_IMAGE_FORMAT, &p_fmt, sizeof(p_fmt));
        }
    }

    return status;
}

vx_status tivxAddKernelLaplacianReconstruct(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.laplacian_reconstruct",
                            VX_KERNEL_LAPLACIAN_RECONSTRUCT,
                            NULL,
                            TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS,
                            tivxAddKernelLaplacianReconstructValidate,
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
                VX_OUTPUT,
                VX_TYPE_PYRAMID,
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

    vx_laplacian_reconstruct_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelLaplacianReconstruct(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_laplacian_reconstruct_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_laplacian_reconstruct_kernel = NULL;

    return status;
}





