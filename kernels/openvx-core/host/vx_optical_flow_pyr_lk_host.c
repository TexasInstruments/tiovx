/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_optical_flow_pyr_lk.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_optical_flow_pyr_lk_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_pyramid pyr[2U];
    vx_array arr[3U];
    vx_size i;
    vx_enum item_type[3U];
    vx_size capacity[3U];
    vx_size levels[2U];
    vx_uint32 w[2U], h[2U];
    vx_float32 scale[2U];
    vx_df_image df_image[2U];
    vx_border_t border;

    for (i = 0U; i < TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS; i ++)
    {
        /* Check for NULL */
        if (NULL == parameters[i])
        {
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }

    pyr[0U] = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_PYRAMID_IDX];
    pyr[1U] = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_PYRAMID_IDX];

    arr[0U] = (vx_array)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_PREVPTS_IDX];
    arr[1U] = (vx_array)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_ESTIMATEDPTS_IDX];
    arr[2U] = (vx_array)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEXTPTS_IDX];

    if (VX_SUCCESS == status)
    {
        for(i=0; i<2; i++)
        {
            status |= vxQueryPyramid(pyr[i], VX_PYRAMID_LEVELS, &levels[i], sizeof(levels[i]));
            status |= vxQueryPyramid(pyr[i], VX_PYRAMID_SCALE, &scale[i], sizeof(scale[i]));
            status |= vxQueryPyramid(pyr[i], VX_PYRAMID_WIDTH, &w[i], sizeof(w[i]));
            status |= vxQueryPyramid(pyr[i], VX_PYRAMID_HEIGHT, &h[i], sizeof(h[i]));
            status |= vxQueryPyramid(pyr[i], VX_PYRAMID_FORMAT, &df_image[i], sizeof(df_image[i]));
        }

        if((levels[0U] != levels[1U])
            || (scale[0U] != scale[1U])
            || (w[0U] != w[1U])
            || (h[0U] != h[1U])
            || (df_image[0U] != df_image[1U])
            || (df_image[0U] != VX_DF_IMAGE_U8))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        for(i=0; i<3U; i++)
        {
            status |= vxQueryArray(arr[i], VX_ARRAY_ITEMTYPE, &item_type[i], sizeof(item_type[i]));
            status |= vxQueryArray(arr[i], VX_ARRAY_CAPACITY, &capacity[i], sizeof(capacity[i]));
        }

        if(    item_type[0U] != item_type[1U]
            || capacity[0U] != capacity[1U]
            || item_type[0U] != VX_TYPE_KEYPOINT
            )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    if (VX_SUCCESS == status)
    {
        if(vx_false_e == tivxIsReferenceVirtual((vx_reference)arr[2U]))
        {
            if(    item_type[0U] != item_type[2U]
                || capacity[0U] != capacity[2U]
                )
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
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
                VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for optical flow\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        i = TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEXTPTS_IDX;

        vxSetMetaFormatAttribute(metas[i], VX_ARRAY_ITEMTYPE, &item_type[0U],
            sizeof(item_type[0U]));
        vxSetMetaFormatAttribute(metas[i], VX_ARRAY_CAPACITY, &capacity[0U],
            sizeof(capacity[0U]));
    }

    return status;
}

vx_status tivxAddKernelOpticalFlowPyrLk(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.optical_flow_pyr_lk",
                VX_KERNEL_OPTICAL_FLOW_PYR_LK,
                NULL,
                TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS,
                tivxAddKernelOpticalFlowPyrLkValidate,
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
                        VX_TYPE_PYRAMID,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_PYRAMID,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_ARRAY,
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
    vx_optical_flow_pyr_lk_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelOpticalFlowPyrLk(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_optical_flow_pyr_lk_kernel;

    status = vxRemoveKernel(kernel);
    vx_optical_flow_pyr_lk_kernel = NULL;

    return status;
}


