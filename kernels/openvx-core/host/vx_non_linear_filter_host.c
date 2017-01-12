/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx_types.h>
#include <tivx_kernel_non_linear_filter.h>

static vx_kernel vx_non_linear_filter_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelNonLinearFilterValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_matrix matrix;
    vx_scalar function;
    vx_enum func;
    vx_uint32 w[2U], h[2U], i;
    vx_size mat_h, mat_w;
    vx_df_image fmt[2U];
    vx_df_image out_fmt;
    vx_enum matrix_type = 0;

    for (i = 0U; i < TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));

        if (VX_SUCCESS == status)
        {
            /* Check for validity of data format */
            if ( (VX_DF_IMAGE_U8 != fmt[0U]) )
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        matrix = (vx_matrix)parameters[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryMatrix(matrix, VX_MATRIX_TYPE, &matrix_type, sizeof(matrix_type));
        status |= vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &mat_w, sizeof(mat_w));
        status |= vxQueryMatrix(matrix, VX_MATRIX_ROWS, &mat_h, sizeof(mat_h));

        if (VX_SUCCESS == status)
        {
            /* Check for validity of data format */
            if (VX_TYPE_UINT8 != matrix_type)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        function = (vx_scalar)parameters[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];
        status |= vxCopyScalar(function, &func, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        if ( (VX_NONLINEAR_FILTER_MEDIAN != func) &&
             (VX_NONLINEAR_FILTER_MIN != func) &&
             (VX_NONLINEAR_FILTER_MAX != func) )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U], sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for frame sizes */
        if ((w[0U] != w[1U]) || (h[1U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if ( fmt[1U] != fmt[0U] )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        out_fmt = VX_DF_IMAGE_U8;

        i = TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX;

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

    return status;
}

vx_status tivxAddKernelNonLinearFilter(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.non_linear_filter",
                VX_KERNEL_NON_LINEAR_FILTER,
                NULL,
                TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS,
                tivxAddKernelNonLinearFilterValidate,
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
                        VX_TYPE_SCALAR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
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
                        VX_INPUT,
                        VX_TYPE_MATRIX,
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
    vx_non_linear_filter_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelNonLinearFilter(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_non_linear_filter_kernel;

    status = vxRemoveKernel(kernel);
    vx_non_linear_filter_kernel = NULL;

    return status;
}


