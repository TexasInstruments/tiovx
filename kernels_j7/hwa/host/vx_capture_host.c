/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/j7.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_capture.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_capture_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelCaptureValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelCaptureInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelCapture(vx_context context);
vx_status tivxRemoveKernelCapture(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelCaptureValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object input = NULL;
    vx_object_array output = NULL;
    vx_char input_name[VX_MAX_REFERENCE_NAME];
    vx_size input_size, output_num_items;
    vx_reference obj_arr_element;
    vx_df_image img_fmt;
    vx_enum ref_type;

    if ( (num != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        input = (vx_user_data_object)parameters[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX];
        output = (vx_object_array)parameters[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(input, VX_USER_DATA_OBJECT_NAME, &input_name, sizeof(input_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(input, VX_USER_DATA_OBJECT_SIZE, &input_size, sizeof(input_size)));

        tivxCheckStatus(&status, vxQueryObjectArray(output, VX_OBJECT_ARRAY_NUMITEMS, &output_num_items, sizeof(output_num_items)));

    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ((input_size != sizeof(tivx_capture_params_t)) ||
            (strncmp(input_name, "tivx_capture_params_t", sizeof(input_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be a user_data_object of type:\n tivx_capture_params_t \n");
        }

    }

    if (VX_SUCCESS == status)
    {
        obj_arr_element = vxGetObjectArrayItem(output, 0);

        if (NULL != obj_arr_element)
        {
            tivxCheckStatus(&status, vxQueryReference(obj_arr_element, VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type)));

            if (VX_SUCCESS == status)
            {
                if ( (TIVX_TYPE_RAW_IMAGE != ref_type) &&
                     (VX_TYPE_IMAGE != ref_type) )
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "output object array must contain either TIVX_TYPE_RAW_IMAGE or VX_TYPE_IMAGE \n");
                }
                else if (VX_TYPE_IMAGE == ref_type)
                {
                    tivxCheckStatus(&status, vxQueryImage((vx_image)obj_arr_element, VX_IMAGE_FORMAT, &img_fmt, sizeof(img_fmt)));

                    /* Only support RGBX format for now */
                    if ((VX_DF_IMAGE_RGBX != img_fmt) &&
                        (VX_DF_IMAGE_U16 != img_fmt))
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "image format is invalid \n");
                    }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "query 'output' object array reference failed \n");
            }

            vxReleaseReference(&obj_arr_element);
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' object array elements are NULL \n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelCaptureInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == parameters[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelCapture(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_CAPTURE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_CAPTURE_MAX_PARAMS,
                    tivxAddKernelCaptureValidate,
                    tivxAddKernelCaptureInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 num_bufs = TIVX_CAPTURE_MIN_PIPEUP_BUFS;

        vxSetKernelAttribute(kernel, VX_KERNEL_PIPEUP_OUTPUT_DEPTH, &num_bufs, sizeof(num_bufs));

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_OBJECT_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_CAPTURE2);
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
    vx_capture_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelCapture(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_capture_kernel;

    status = vxRemoveKernel(kernel);
    vx_capture_kernel = NULL;

    return status;
}

void tivx_capture_params_init(tivx_capture_params_t *prms)
{
    if (NULL != prms)
    {
        prms->instId = 0u;
        prms->enableCsiv2p0Support = (uint32_t)vx_true_e;
        prms->numDataLanes = 4u;
        prms->dataLanesMap[0u] = 1u;
        prms->dataLanesMap[1u] = 2u;
        prms->dataLanesMap[2u] = 3u;
        prms->dataLanesMap[3u] = 4u;

        prms->vcNum[0u] = 0u;
        prms->vcNum[1u] = 1u;
        prms->vcNum[2u] = 2u;
        prms->vcNum[3u] = 3u;
    }
}
