/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx_types.h>
#include <tivx_kernel_channel_combine.h>

static vx_kernel vx_channel_combine_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelChannelCombineValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_image img[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_df_image fmt[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_uint32 w[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_uint32 h[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_df_image out_fmt;
    vx_uint32 out_w, out_h;
    vx_uint32 i;
    vx_uint16 in_channel, out_channel;

    in_channel = 0;
    for (i = 0U; i < TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS; i ++)
    {
        img[i] = NULL;
        w[i] = 0;
        h[i] = 0;
        fmt[i] = VX_DF_IMAGE_VIRT;

        if (NULL == parameters[i])
        {
            /* Check for NULL for required parameters */
            if((i == TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX)
               || (i == TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX)
               || (i == TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX)
                )
            {
                status = VX_ERROR_NO_MEMORY;
                break;
            }
        }
        else
        {
            img[i] = (vx_image)parameters[i];

            /* Get the image width/heigh and format */
            status |= vxQueryImage(img[i], VX_IMAGE_FORMAT, &fmt[i],
                sizeof(fmt[i]));

            status |= vxQueryImage(img[i], VX_IMAGE_WIDTH, &w[i], sizeof(w[i]));
            status |= vxQueryImage(img[i], VX_IMAGE_HEIGHT, &h[i], sizeof(h[i]));

            if(i == TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX)
            {

            }
            else
            {
                in_channel++;

                if(fmt[i] != VX_DF_IMAGE_U8)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }
        }
    }

    out_w = w[0U];
    out_h = h[0U];
    out_fmt = fmt[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX];

    /* check if output format matches number of planes given as input */
    if( status == VX_SUCCESS)
    {
        out_channel = 0;
        switch(out_fmt)
        {
            case VX_DF_IMAGE_RGBX:
                out_channel = 4;
                break;
            case VX_DF_IMAGE_RGB:
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
            case VX_DF_IMAGE_YUYV:
            case VX_DF_IMAGE_UYVY:
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_YUV4:
                out_channel = 3;
                break;
            default:
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
        }
    }

    if ( status == VX_SUCCESS )
    {
        if(in_channel!=out_channel)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    if ( status == VX_SUCCESS )
    {
        switch(out_fmt)
        {
            case VX_DF_IMAGE_YUV4:
            case VX_DF_IMAGE_RGBX:
            case VX_DF_IMAGE_RGB:
                for(i=1U; i<in_channel; i++)
                {
                    if( w[i] != w[0]
                        ||
                        h[i] != h[0]
                    )
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                break;
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
                for(i=1U; i<in_channel; i++)
                {
                    if( w[i] != w[0]/2
                        ||
                        h[i] != h[0]/2
                    )
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                break;
            case VX_DF_IMAGE_YUYV:
            case VX_DF_IMAGE_UYVY:
                for(i=1U; i<in_channel; i++)
                {
                    if( w[i] != w[0]/2
                        ||
                        h[i] != h[0]
                    )
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                break;
            default:
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
        }

    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX])))
    {
        /* Check for frame sizes */
        if ((out_w != w[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX]) || (out_h != h[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        i = TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX;

        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
            sizeof(out_fmt));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &out_w,
            sizeof(w[0U]));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &out_h,
            sizeof(h[0U]));
    }

    return status;
}

vx_status tivxAddKernelChannelCombine(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.channel_combine",
                VX_KERNEL_CHANNEL_COMBINE,
                NULL,
                TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS,
                tivxAddKernelChannelCombineValidate,
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
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
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
    vx_channel_combine_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelChannelCombine(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_channel_combine_kernel;

    status = vxRemoveKernel(kernel);
    vx_channel_combine_kernel = NULL;

    return status;
}


