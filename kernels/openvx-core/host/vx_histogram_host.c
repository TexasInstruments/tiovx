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
#include <tivx_kernel_histogram.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_histogram_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelHistogramValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img;
    vx_distribution dist;
    vx_uint32 w[2U], h[2U], i;
    vx_df_image fmt;
    vx_int32 offset = 0;
    vx_uint32 range = 0;
    vx_size numBins = 0;

    for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
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
        img = (vx_image)parameters[TIVX_KERNEL_HISTOGRAM_IN_IMG_IDX];
        dist = (vx_distribution)parameters[TIVX_KERNEL_HISTOGRAM_OUT_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, VX_IMAGE_FORMAT, &fmt,
            sizeof(fmt));

        status |= vxQueryImage(img, VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img, VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status |= vxQueryDistribution(dist, VX_DISTRIBUTION_BINS, &numBins, sizeof(numBins));
        status |= vxQueryDistribution(dist, VX_DISTRIBUTION_RANGE, &range, sizeof(range));
        status |= vxQueryDistribution(dist, VX_DISTRIBUTION_OFFSET, &offset, sizeof(offset));
    }

    if (VX_SUCCESS == status)
    {
        if( ((uint32_t)offset + (uint32_t)range) > 256U )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        if( (uint32_t)numBins > 256U )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        fmt = VX_DF_IMAGE_U8;
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &fmt,
                    sizeof(fmt));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &w[0U],
                    sizeof(w[0U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &h[0U],
                    sizeof(h[0U]));
            }
        }
    }
    return status;
}

vx_status tivxAddKernelHistogram(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.histogram",
                            VX_KERNEL_HISTOGRAM,
                            NULL,
                            2,
                            tivxAddKernelHistogramValidate,
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
                VX_TYPE_DISTRIBUTION,
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

    vx_histogram_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelHistogram(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_histogram_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_histogram_kernel = NULL;

    return status;
}





