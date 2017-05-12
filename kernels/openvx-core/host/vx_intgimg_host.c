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
#include <tivx_kernel_intgimg.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_intgimg_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelIntgImgValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_uint32 w[2U], h[2U], i;
    vx_df_image fmt[2U];

    for (i = 0U; i < TIVX_KERNEL_INTGIMG_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_INTGIMG_IN_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

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

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for frame sizes */
        if ((w[0U] != w[1U]) || (h[0U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (VX_DF_IMAGE_U32 != fmt[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        fmt[1U] = VX_DF_IMAGE_U32;
        for (i = 0U; i < TIVX_KERNEL_INTGIMG_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &fmt[1U],
                    sizeof(fmt[1U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &w[0U],
                    sizeof(w[0U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &h[0U],
                    sizeof(h[0U]));
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxAddKernelIntgImgInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    tivxKernelValidRectParams prms;

    if (num_params != TIVX_KERNEL_INTGIMG_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; (i < TIVX_KERNEL_INTGIMG_MAX_PARAMS) &&
            (VX_SUCCESS == status); i ++)
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
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_INTGIMG_IN_IMG_IDX];
        prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

        prms.num_input_images = 1;
        prms.num_output_images = 1;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}


vx_status tivxAddKernelIntegralImage(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.integral_image",
                            VX_KERNEL_INTEGRAL_IMAGE,
                            NULL,
                            2,
                            tivxAddKernelIntgImgValidate,
                            tivxAddKernelIntgImgInitialize,
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

    vx_intgimg_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelIntegralImage(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_intgimg_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_intgimg_kernel = NULL;

    return status;
}





