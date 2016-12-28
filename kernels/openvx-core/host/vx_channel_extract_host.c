/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx_types.h>
#include <tivx_kernel_channel_extract.h>

static vx_kernel vx_channel_extract_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelChannelExtractValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_image img[2U] = {NULL};
    vx_scalar scalar;
    vx_df_image fmt[2U];
    vx_df_image out_fmt;
    vx_enum type, channel;
    vx_uint32 i, w[2U], h[2U], out_w, out_h;

    for (i = 0U; i < TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];
        scalar = (vx_scalar)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));

        status |= vxQueryScalar(scalar, VX_SCALAR_TYPE, &type, sizeof(type));

        status |= vxCopyScalar(scalar, &channel, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    }

    if (VX_SUCCESS == status)
    {
        if (type != VX_TYPE_ENUM)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        if (   (fmt[0U] != VX_DF_IMAGE_RGB)
            && (fmt[0U] != VX_DF_IMAGE_RGBX)
            && (fmt[0U] != VX_DF_IMAGE_NV12)
            && (fmt[0U] != VX_DF_IMAGE_NV21)
            && (fmt[0U] != VX_DF_IMAGE_UYVY)
            && (fmt[0U] != VX_DF_IMAGE_YUYV)
            && (fmt[0U] != VX_DF_IMAGE_IYUV)
            && (fmt[0U] != VX_DF_IMAGE_YUV4)
           )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    out_fmt = VX_DF_IMAGE_U8;
    out_w = w[0U];
    out_h = h[0U];
    /* output WxH MUST be equal to input WxH except in below conditions */
    switch(fmt[0U])
    {
        case VX_DF_IMAGE_IYUV:
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
            if(channel == VX_CHANNEL_U || channel == VX_CHANNEL_V)
            {
                out_w = out_w/2;
                out_h = out_h/2;
            }
            break;
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
            if(channel == VX_CHANNEL_U || channel == VX_CHANNEL_V)
            {
                out_w = out_w/2;
            }
            break;
    }


    if (VX_SUCCESS == status)
    {
        if(vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U]))
        {
            /* Get the image width/heigh and format */
            status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U], sizeof(fmt[1U]));
            status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
            status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

            /* Check for frame sizes */
            if ((out_w != w[1U]) || (out_h != h[1U]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }

            if ( fmt[1U] != out_fmt )
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        i = TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX;

        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
            sizeof(out_fmt));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &out_w,
            sizeof(w[0U]));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &out_h,
            sizeof(h[0U]));
    }

    return status;
}

vx_status tivxAddKernelChannelExtract(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.channel_extract",
                VX_KERNEL_CHANNEL_EXTRACT,
                NULL,
                TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS,
                tivxAddKernelChannelExtractValidate,
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
                        VX_INPUT,
                        VX_TYPE_ENUM,
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
    vx_channel_extract_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelChannelExtract(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_channel_extract_kernel;

    status = vxRemoveKernel(kernel);
    vx_channel_extract_kernel = NULL;

    return status;
}


