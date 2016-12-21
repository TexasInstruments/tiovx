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
#include <tivx_kernel_color_convert.h>
#include <TI/tivx_target_kernel.h>
#include <stdio.h>

static vx_kernel vx_color_convert_kernel = NULL;

static vx_status tivxCheckFormatAndPlanes(vx_size plane, vx_df_image format)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    switch(plane)
    {
        case 1:
        {
            switch (format)
            {
                case VX_DF_IMAGE_RGB:
                case VX_DF_IMAGE_RGBX:
                case VX_DF_IMAGE_UYVY:
                case VX_DF_IMAGE_YUYV:
                    status = VX_SUCCESS;
                default:
                    break;
            }
            break;
        }
        case 2:
        {
            switch (format)
            {
                case VX_DF_IMAGE_NV12:
                case VX_DF_IMAGE_NV21:
                    status = VX_SUCCESS;
                default:
                    break;
            }
            break;
        }
        case 3:
        {
            switch (format)
            {
                case VX_DF_IMAGE_IYUV:
                case VX_DF_IMAGE_YUV4:
                    status = VX_SUCCESS;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelColorConvertValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_df_image out_fmt;
    vx_uint32 i, w[2U], h[2U];
    vx_df_image src_format, dst_format;
    vx_size src_planes, dst_planes;
    vx_enum src_space;
    vx_rectangle_t rect;

    for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX];

        status |= vxQueryImage(img[0U], VX_IMAGE_FORMAT, &src_format, sizeof(src_format));
        status |= vxQueryImage(img[0U], VX_IMAGE_SPACE, &src_space, sizeof(src_space));
        status |= vxQueryImage(img[0U], VX_IMAGE_PLANES, &src_planes, sizeof(src_planes));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    status = vxGetValidRegionImage(img[0U], &rect);

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        status |= vxQueryImage(img[1U], VX_IMAGE_FORMAT, &dst_format, sizeof(dst_format));
        status |= vxQueryImage(img[1U], VX_IMAGE_PLANES, &dst_planes, sizeof(dst_planes));

        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));
    }

    if (VX_SUCCESS == status)
    {
        /* Verifies luma channel size */
        if ((w[0U] != w[1U]) || (h[0U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if(src_format == dst_format)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = VX_ERROR_INVALID_PARAMETERS;

        status = tivxCheckFormatAndPlanes(src_planes, src_format);
        status |= tivxCheckFormatAndPlanes(dst_planes, dst_format);
    }

    out_fmt = dst_format;
    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
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


vx_status tivxAddKernelColorConvert(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.color_convert",
                            VX_KERNEL_COLOR_CONVERT,
                            NULL,
                            2,
                            tivxAddKernelColorConvertValidate,
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

    vx_color_convert_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelColorConvert(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_color_convert_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_color_convert_kernel = NULL;

    return status;
}


