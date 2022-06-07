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

#ifdef BUILD_DMPAC_DOF

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dof_visualize.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_dof_visualize_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelDofVisualizeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelDofVisualizeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelDofVisualizeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image flow_vector = NULL;
    vx_df_image flow_vector_fmt;
    vx_uint32 flow_vector_w, flow_vector_h;

    vx_scalar confidence_threshold = NULL;
    vx_enum confidence_threshold_scalar_type;

    vx_image flow_vector_image = NULL;
    vx_df_image flow_vector_image_fmt;
    vx_uint32 flow_vector_image_w, flow_vector_image_h;

    vx_image confidence_image = NULL;
    vx_df_image confidence_image_fmt;
    vx_uint32 confidence_image_w, confidence_image_h;

    if ( (num != TIVX_KERNEL_DOF_VISUALIZE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IDX])
        || (NULL == parameters[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_DOF_VISUALIZE_CONFIDENCE_IMAGE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        flow_vector = (vx_image)parameters[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IDX];
        confidence_threshold = (vx_scalar)parameters[TIVX_KERNEL_DOF_VISUALIZE_CONFIDENCE_THRESHOLD_IDX];
        flow_vector_image = (vx_image)parameters[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IMAGE_IDX];
        confidence_image = (vx_image)parameters[TIVX_KERNEL_DOF_VISUALIZE_CONFIDENCE_IMAGE_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(flow_vector, (vx_enum)VX_IMAGE_FORMAT, &flow_vector_fmt, sizeof(flow_vector_fmt)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector, (vx_enum)VX_IMAGE_WIDTH, &flow_vector_w, sizeof(flow_vector_w)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector, (vx_enum)VX_IMAGE_HEIGHT, &flow_vector_h, sizeof(flow_vector_h)));

        if (NULL != confidence_threshold)
        {
            tivxCheckStatus(&status, vxQueryScalar(confidence_threshold, (vx_enum)VX_SCALAR_TYPE, &confidence_threshold_scalar_type, sizeof(confidence_threshold_scalar_type)));
        }

        tivxCheckStatus(&status, vxQueryImage(flow_vector_image, (vx_enum)VX_IMAGE_FORMAT, &flow_vector_image_fmt, sizeof(flow_vector_image_fmt)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector_image, (vx_enum)VX_IMAGE_WIDTH, &flow_vector_image_w, sizeof(flow_vector_image_w)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector_image, (vx_enum)VX_IMAGE_HEIGHT, &flow_vector_image_h, sizeof(flow_vector_image_h)));

        tivxCheckStatus(&status, vxQueryImage(confidence_image, (vx_enum)VX_IMAGE_FORMAT, &confidence_image_fmt, sizeof(confidence_image_fmt)));
        tivxCheckStatus(&status, vxQueryImage(confidence_image, (vx_enum)VX_IMAGE_WIDTH, &confidence_image_w, sizeof(confidence_image_w)));
        tivxCheckStatus(&status, vxQueryImage(confidence_image, (vx_enum)VX_IMAGE_HEIGHT, &confidence_image_h, sizeof(confidence_image_h)));
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U32 != flow_vector_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'flow_vector' should be an image of type:\n VX_DF_IMAGE_U32 \n");
        }

        if (NULL != confidence_threshold)
        {
            if ((vx_enum)VX_TYPE_UINT32 != confidence_threshold_scalar_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'confidence_threshold' should be a scalar of type:\n VX_TYPE_UINT32 \n");
            }
        }

        if ((vx_df_image)VX_DF_IMAGE_RGB != flow_vector_image_fmt)
        {
          if((vx_df_image)VX_DF_IMAGE_NV12 != flow_vector_image_fmt)
          {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'flow_vector_image' should be an image of type:\n VX_DF_IMAGE_RGB or VX_DF_IMAGE_NV12 \n");
          }
        }

        if ((vx_df_image)VX_DF_IMAGE_U8 != confidence_image_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'confidence_image' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }
    }

    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if( (flow_vector_w != flow_vector_image_w) ||
            (flow_vector_w != confidence_image_w))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector' and 'flow_vector_image' and 'confidence_image' should have the same value for VX_IMAGE_WIDTH\n");
        }
        if( (flow_vector_h != flow_vector_image_h) ||
            (flow_vector_h != confidence_image_h))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector' and 'flow_vector_image' and 'confidence_image' should have the same value for VX_IMAGE_HEIGHT\n");
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDofVisualizeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_DOF_VISUALIZE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IDX])
        || (NULL == parameters[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_DOF_VISUALIZE_CONFIDENCE_IMAGE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    return status;
}

vx_status tivxAddKernelDofVisualize(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_DOF_VISUALIZE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_DOF_VISUALIZE_MAX_PARAMS,
                    tivxAddKernelDofVisualizeValidate,
                    tivxAddKernelDofVisualizeInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_0);
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != (vx_status)VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_dof_visualize_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelDofVisualize(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_dof_visualize_kernel;

    status = vxRemoveKernel(kernel);
    vx_dof_visualize_kernel = NULL;

    return status;
}

#endif