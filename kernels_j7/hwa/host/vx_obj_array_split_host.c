/*
 *
 * Copyright (c) 2022 Texas Instruments Incorporated
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
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_obj_array_split.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_obj_array_split_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelObjArraySplitValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelObjArraySplitInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelObjArraySplit(vx_context context);
vx_status tivxRemoveKernelObjArraySplit(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelObjArraySplitValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_object_array in = NULL;
    vx_object_array out0 = NULL;
    vx_object_array out1 = NULL;
    vx_object_array out2 = NULL;
    vx_object_array out3 = NULL;
    vx_size in_num_items = 0;
    vx_size out0_num_items = 0;
    vx_size out1_num_items = 0;
    vx_size out2_num_items = 0;
    vx_size out3_num_items = 0;

    vx_enum in_type = VX_TYPE_INVALID;
    vx_enum out0_type = VX_TYPE_INVALID;
    vx_enum out1_type = VX_TYPE_INVALID;
    vx_enum out2_type = VX_TYPE_INVALID;
    vx_enum out3_type = VX_TYPE_INVALID;

    if ( (num != TIVX_KERNEL_OBJ_ARRAY_SPLIT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT0_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT1_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        in = (vx_object_array)parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_IN_IDX];
        out0 = (vx_object_array)parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT0_IDX];
        out1 = (vx_object_array)parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT1_IDX];
        out2 = (vx_object_array)parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT2_IDX];
        out3 = (vx_object_array)parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT3_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        /* IN parameter query */
        tivxCheckStatus(&status, vxQueryObjectArray(in, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &in_num_items, sizeof(in_num_items)));
        tivxCheckStatus(&status, vxQueryObjectArray(in, (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE, &in_type, sizeof(in_type)));

        /* OUT0 parameter query */
        tivxCheckStatus(&status, vxQueryObjectArray(out0, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &out0_num_items, sizeof(out0_num_items)));
        tivxCheckStatus(&status, vxQueryObjectArray(out0, (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE, &out0_type, sizeof(out0_type)));

        /* OUT1 parameter query */
        tivxCheckStatus(&status, vxQueryObjectArray(out1, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &out1_num_items, sizeof(out1_num_items)));
        tivxCheckStatus(&status, vxQueryObjectArray(out1, (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE, &out1_type, sizeof(out1_type)));

        if (NULL != out2)
        {
            /* OUT2 parameter query */
            tivxCheckStatus(&status, vxQueryObjectArray(out2, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &out2_num_items, sizeof(out2_num_items)));
            tivxCheckStatus(&status, vxQueryObjectArray(out2, (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE, &out2_type, sizeof(out2_type)));
        }

        if (NULL != out3)
        {
            /* OUT3 parameter query */
            tivxCheckStatus(&status, vxQueryObjectArray(out3, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &out3_num_items, sizeof(out3_num_items)));
            tivxCheckStatus(&status, vxQueryObjectArray(out3, (vx_enum)VX_OBJECT_ARRAY_ITEMTYPE, &out3_type, sizeof(out3_type)));
        }
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        vx_size total_output_size = out0_num_items + out1_num_items + out2_num_items + out3_num_items;

        if (in_num_items != total_output_size)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "in object array size is different than the sum of out object array sizes \n");
        }

        if ( in_type != out0_type )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "in object array type is different than the out0 object array type \n");
        }

        if ( in_type != out1_type )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "in object array type is different than the out1 object array type \n");
        }

        if (NULL != out2)
        {
            if ( in_type != out2_type )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "in object array type is different than the out2 object array type \n");
            }
        }

        if (NULL != out3)
        {
            if ( in_type != out3_type )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "in object array type is different than the out3 object array type \n");
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelObjArraySplitInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_OBJ_ARRAY_SPLIT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT0_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT1_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelObjArraySplit(vx_context context)
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
                    TIVX_KERNEL_OBJ_ARRAY_SPLIT_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_OBJ_ARRAY_SPLIT_MAX_PARAMS,
                    tivxAddKernelObjArraySplitValidate,
                    tivxAddKernelObjArraySplitInitialize,
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
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_OBJECT_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_A72_0);
            tivxAddKernelTarget(kernel, TIVX_TARGET_A72_1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_A72_2);
            tivxAddKernelTarget(kernel, TIVX_TARGET_A72_3);
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
    vx_obj_array_split_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelObjArraySplit(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_obj_array_split_kernel;

    status = vxRemoveKernel(kernel);
    vx_obj_array_split_kernel = NULL;

    return status;
}


