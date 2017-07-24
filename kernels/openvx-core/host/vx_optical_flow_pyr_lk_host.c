/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_optical_flow_pyr_lk.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_optical_flow_pyr_lk_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

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

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    tivxKernelValidRectParams prms;
    vx_pyramid pyr[2U];
    vx_size levels[2U];
    vx_image img[2U];

    if (num_params != TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        pyr[0U] = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_PYRAMID_IDX];
        pyr[1U] = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_PYRAMID_IDX];

        for(i=0; i<2; i++)
        {
            status |= vxQueryPyramid(pyr[i], VX_PYRAMID_LEVELS, &levels[i], sizeof(levels[i]));
        }

        if(levels[0U] != levels[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((VX_SUCCESS == status) && (vx_false_e == tivxIsReferenceVirtual((vx_reference)pyr[0U])) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)pyr[1U])) )
    {
        for (i = 0; i < levels[0U]; i++)
        {
            img[0] = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_PYRAMID_IDX], i);
            img[1] = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_PYRAMID_IDX], i);

            prms.in_img[0] = img[0];
            prms.in_img[1] = img[1];

            prms.num_input_images = 2;
            prms.num_output_images = 0;

            prms.top_pad = 0;
            prms.bot_pad = 0;
            prms.left_pad = 0;
            prms.right_pad = 0;
            prms.border_mode = VX_BORDER_UNDEFINED;

            status |= tivxKernelConfigValidRect(&prms);

            status |= vxReleaseImage(&img[0]);
            status |= vxReleaseImage(&img[1]);
        }
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
                tivxAddKernelOpticalFlowPyrLkInitialize,
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


