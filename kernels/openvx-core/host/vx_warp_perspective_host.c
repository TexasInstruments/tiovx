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
#include <tivx_kernel_warp_perspective.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_warp_perspective_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelWarpPerspectiveValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_matrix matrix;
    vx_uint32 w[2U], h[2U], i;
    vx_size mat_h, mat_w;
    vx_df_image fmt[2U];
    vx_enum matrix_type = 0;
    vx_enum interpolation_type = 0;

    for (i = 0U; i < TIVX_KERNEL_WARP_PERSPECTIVE_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_WARP_PERSPECTIVE_IN0_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_WARP_PERSPECTIVE_OUT_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));

        if (VX_SUCCESS == status)
        {
            /* Check for validity of data format */
            if (VX_DF_IMAGE_U8 != fmt[0U])
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        matrix = (vx_matrix)parameters[TIVX_KERNEL_WARP_PERSPECTIVE_IN0_MAT_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryMatrix(matrix, VX_MATRIX_TYPE, &matrix_type, sizeof(matrix_type));
        status |= vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &mat_w, sizeof(mat_w));
        status |= vxQueryMatrix(matrix, VX_MATRIX_ROWS, &mat_h, sizeof(mat_h));

        if (VX_SUCCESS == status)
        {
            /* Check for validity of data format */
            if (VX_TYPE_FLOAT32 != matrix_type)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }

            /* Check for frame sizes */
            if ((mat_w != 3U) || (mat_h != 3U))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        vx_enum stype = 0;
        vx_scalar scalar = (vx_scalar)parameters[TIVX_KERNEL_WARP_PERSPECTIVE_IN0_SC_IDX];

        status = vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype,
            sizeof(stype));

        if ((VX_SUCCESS == status) && (stype == VX_TYPE_ENUM))
        {
            status = vxCopyScalar(scalar, &interpolation_type, VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST);

            if (VX_SUCCESS == status)
            {
                if((interpolation_type == VX_INTERPOLATION_NEAREST_NEIGHBOR) ||
                   (interpolation_type == VX_INTERPOLATION_BILINEAR))
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

    /* Warp output can not be virtual */
    if ((VX_SUCCESS == status) &&
        (vx_true_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        status = VX_ERROR_INVALID_TYPE;
    }

    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for frame sizes */
        if ((w[1U] <= 0) || (h[1U] <= 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (VX_DF_IMAGE_U8 != fmt[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
#if 0
    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < TIVX_KERNEL_WARP_PERSPECTIVE_MAX_PARAMS; i ++)
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
#endif
    return status;
}

vx_status tivxAddKernelWarpPerspective(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.warp_perspective",
                            VX_KERNEL_WARP_PERSPECTIVE,
                            NULL,
                            TIVX_KERNEL_WARP_PERSPECTIVE_MAX_PARAMS,
                            tivxAddKernelWarpPerspectiveValidate,
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
                VX_TYPE_MATRIX,
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

    vx_warp_perspective_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelWarpPerspective(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_warp_perspective_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_warp_perspective_kernel = NULL;

    return status;
}





