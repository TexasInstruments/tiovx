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

#include "uc_superset2.h"

vx_status uc_superset2_create(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    memset(usecase, 0, sizeof(uc_superset2_t));

    if (status == VX_SUCCESS)
    {
        usecase->context = vxCreateContext();
        if (usecase->context == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->graph_0 = vxCreateGraph(usecase->context);
        if (usecase->graph_0 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        status = uc_superset2_data_create(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_superset2_graph_0_create(usecase);
    }

    return status;
}

vx_status uc_superset2_verify(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_superset2_graph_0_verify(usecase);
    }

    return status;
}

vx_status uc_superset2_run(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_superset2_graph_0_run(usecase);
    }

    return status;
}

vx_status uc_superset2_delete(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_superset2_graph_0_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_superset2_data_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }

    return status;
}

vx_status uc_superset2_data_create(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_context context = usecase->context;

    if (status == VX_SUCCESS)
    {
        usecase->image_15 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_15 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_15, "image_15");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 0;

        usecase->scalar_16 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_16 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 0;

        usecase->scalar_18 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_18 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 0;

        usecase->scalar_19 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_19 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int32 value = 3;

        usecase->scalar_36 = vxCreateScalar(context, VX_TYPE_INT32, &value);
        if (usecase->scalar_36 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int32 value = 5;

        usecase->scalar_37 = vxCreateScalar(context, VX_TYPE_INT32, &value);
        if (usecase->scalar_37 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_20 = vxCreateArray(context, VX_TYPE_KEYPOINT, 100);
        if (usecase->array_20 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_size value = 0;

        usecase->scalar_21 = vxCreateScalar(context, VX_TYPE_SIZE, &value);
        if (usecase->scalar_21 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_22 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_22 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_22, "image_22");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->pyramid_23 = vxCreatePyramid(context, 5, VX_SCALE_PYRAMID_HALF, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->pyramid_23 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_24 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_24 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_24, "image_24");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 0;

        usecase->scalar_17 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_17 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_bool value = vx_true_e;

        usecase->scalar_27 = vxCreateScalar(context, VX_TYPE_BOOL, &value);
        if (usecase->scalar_27 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_25 = vxCreateArray(context, VX_TYPE_KEYPOINT, 100);
        if (usecase->array_25 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_size value = 0;

        usecase->scalar_26 = vxCreateScalar(context, VX_TYPE_SIZE, &value);
        if (usecase->scalar_26 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->pyramid_28 = vxCreatePyramid(context, 5, VX_SCALE_PYRAMID_HALF, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->pyramid_28 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_29 = vxCreateArray(context, VX_TYPE_KEYPOINT, 100);
        if (usecase->array_29 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_TERM_CRITERIA_EPSILON;

        usecase->scalar_40 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_40 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 1;

        usecase->scalar_41 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_41 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 value = 10;

        usecase->scalar_42 = vxCreateScalar(context, VX_TYPE_UINT32, &value);
        if (usecase->scalar_42 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_bool value = vx_true_e;

        usecase->scalar_43 = vxCreateScalar(context, VX_TYPE_BOOL, &value);
        if (usecase->scalar_43 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_size value = 9;

        usecase->scalar_44 = vxCreateScalar(context, VX_TYPE_SIZE, &value);
        if (usecase->scalar_44 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_13 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_RGB);
        if (usecase->image_13 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_13, "image_13");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_14 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_RGBX);
        if (usecase->image_14 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_14, "image_14");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_11 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_11 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_11, "image_11");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->distribution_12 = vxCreateDistribution(context, 10, 1, 100);
        if (usecase->distribution_12 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_1 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_1, "image_1");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 0;

        usecase->scalar_2 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_2 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_float32 value = 0;

        usecase->scalar_3 = vxCreateScalar(context, VX_TYPE_FLOAT32, &value);
        if (usecase->scalar_3 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_4 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_4 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_4, "image_4");
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint8 value = 0;

        usecase->scalar_5 = vxCreateScalar(context, VX_TYPE_UINT8, &value);
        if (usecase->scalar_5 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint8 value = 255;

        usecase->scalar_6 = vxCreateScalar(context, VX_TYPE_UINT8, &value);
        if (usecase->scalar_6 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_7 = vxCreateArray(context, VX_TYPE_COORDINATES2D, 100);
        if (usecase->array_7 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_8 = vxCreateArray(context, VX_TYPE_COORDINATES2D, 100);
        if (usecase->array_8 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 value = 10;

        usecase->scalar_9 = vxCreateScalar(context, VX_TYPE_UINT32, &value);
        if (usecase->scalar_9 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 value = 100;

        usecase->scalar_10 = vxCreateScalar(context, VX_TYPE_UINT32, &value);
        if (usecase->scalar_10 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_31 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_31 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_31, "image_31");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->pyramid_32 = vxCreatePyramid(context, 5, VX_SCALE_PYRAMID_HALF, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->pyramid_32 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_33 = vxCreateImage(context, 20, 15, VX_DF_IMAGE_U8);
        if (usecase->image_33 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_33, "image_33");
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_34 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_34 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        if (status == VX_SUCCESS)
        {
            status = vxSetReferenceName( (vx_reference)usecase->image_34, "image_34");
        }
    }

    return status;
}

vx_status uc_superset2_data_delete(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_15);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_16);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_18);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_19);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_36);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_37);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_20);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_21);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_22);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->pyramid_23);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_24);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_17);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_27);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_25);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_26);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->pyramid_28);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_29);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_40);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_41);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_42);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_43);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_44);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_13);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_14);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_11);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->distribution_12);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_1);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_2);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_3);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_4);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_5);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_6);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_7);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_8);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_9);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_10);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_31);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->pyramid_32);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_33);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_34);
    }

    return status;
}

static vx_node usecase_node_create_node_38 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_scalar scalar_2 ,
  vx_scalar scalar_3 ,
  vx_scalar scalar_4 ,
  vx_scalar scalar_5 ,
  vx_array array_6 ,
  vx_scalar scalar_7 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 ,
          (vx_reference)scalar_4 ,
          (vx_reference)scalar_5 ,
          (vx_reference)array_6 ,
          (vx_reference)scalar_7 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_HARRIS_CORNERS, params, 8);

    return node;
}

static vx_node usecase_node_create_node_35 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_pyramid pyramid_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)pyramid_1 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_GAUSSIAN_PYRAMID, params, 2);

    return node;
}

static vx_node usecase_node_create_node_39 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_scalar scalar_2 ,
  vx_array array_3 ,
  vx_scalar scalar_4 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)array_3 ,
          (vx_reference)scalar_4 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_FAST_CORNERS, params, 5);

    return node;
}

static vx_node usecase_node_create_node_45 (
  vx_graph graph ,
  vx_pyramid pyramid_0 ,
  vx_pyramid pyramid_1 ,
  vx_array array_2 ,
  vx_array array_3 ,
  vx_array array_4 ,
  vx_scalar scalar_5 ,
  vx_scalar scalar_6 ,
  vx_scalar scalar_7 ,
  vx_scalar scalar_8 ,
  vx_scalar scalar_9 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)pyramid_0 ,
          (vx_reference)pyramid_1 ,
          (vx_reference)array_2 ,
          (vx_reference)array_3 ,
          (vx_reference)array_4 ,
          (vx_reference)scalar_5 ,
          (vx_reference)scalar_6 ,
          (vx_reference)scalar_7 ,
          (vx_reference)scalar_8 ,
          (vx_reference)scalar_9 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_OPTICAL_FLOW_PYR_LK, params, 10);

    return node;
}

static vx_node usecase_node_create_node_46 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_COLOR_CONVERT, params, 2);

    return node;
}

static vx_node usecase_node_create_node_47 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_distribution distribution_1 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)distribution_1 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_HISTOGRAM, params, 2);

    return node;
}

static vx_node usecase_node_create_node_48 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_scalar scalar_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)scalar_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_MEAN_STDDEV, params, 3);

    return node;
}

static vx_node usecase_node_create_node_49 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_scalar scalar_2 ,
  vx_array array_3 ,
  vx_array array_4 ,
  vx_scalar scalar_5 ,
  vx_scalar scalar_6 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)array_3 ,
          (vx_reference)array_4 ,
          (vx_reference)scalar_5 ,
          (vx_reference)scalar_6 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_MINMAXLOC, params, 7);

    return node;
}

static vx_node usecase_node_create_node_50 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_pyramid pyramid_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)pyramid_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_LAPLACIAN_PYRAMID, params, 3);

    return node;
}

static vx_node usecase_node_create_node_51 (
  vx_graph graph ,
  vx_pyramid pyramid_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)pyramid_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_LAPLACIAN_RECONSTRUCT, params, 3);

    return node;
}

vx_status uc_superset2_graph_0_create(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        usecase->node_38 = usecase_node_create_node_38 (
            graph ,
            usecase->image_15 ,
            usecase->scalar_16 ,
            usecase->scalar_18 ,
            usecase->scalar_19 ,
            usecase->scalar_36 ,
            usecase->scalar_37 ,
            usecase->array_20 ,
            usecase->scalar_21 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_38, "node_38");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_38, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_35 = usecase_node_create_node_35 (
            graph ,
            usecase->image_22 ,
            usecase->pyramid_23 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_35, "node_35");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_35, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_39 = usecase_node_create_node_39 (
            graph ,
            usecase->image_24 ,
            usecase->scalar_17 ,
            usecase->scalar_27 ,
            usecase->array_25 ,
            usecase->scalar_26 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_39, "node_39");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_39, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_45 = usecase_node_create_node_45 (
            graph ,
            usecase->pyramid_23 ,
            usecase->pyramid_28 ,
            usecase->array_20 ,
            usecase->array_25 ,
            usecase->array_29 ,
            usecase->scalar_40 ,
            usecase->scalar_41 ,
            usecase->scalar_42 ,
            usecase->scalar_43 ,
            usecase->scalar_44 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_45, "node_45");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_45, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_46 = usecase_node_create_node_46 (
            graph ,
            usecase->image_13 ,
            usecase->image_14 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_46, "node_46");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_46, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_47 = usecase_node_create_node_47 (
            graph ,
            usecase->image_11 ,
            usecase->distribution_12 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_47, "node_47");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_47, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_48 = usecase_node_create_node_48 (
            graph ,
            usecase->image_1 ,
            usecase->scalar_2 ,
            usecase->scalar_3 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_48, "node_48");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_48, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_49 = usecase_node_create_node_49 (
            graph ,
            usecase->image_4 ,
            usecase->scalar_5 ,
            usecase->scalar_6 ,
            usecase->array_7 ,
            usecase->array_8 ,
            usecase->scalar_9 ,
            usecase->scalar_10 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_49, "node_49");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_49, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_50 = usecase_node_create_node_50 (
            graph ,
            usecase->image_31 ,
            usecase->pyramid_32 ,
            usecase->image_33 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_50, "node_50");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_50, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_51 = usecase_node_create_node_51 (
            graph ,
            usecase->pyramid_32 ,
            usecase->image_33 ,
            usecase->image_34 
          );
        status = vxSetReferenceName( (vx_reference)usecase->node_51, "node_51");
        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(usecase->node_51, VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
    }

    if (status == VX_SUCCESS)
    {
        status = vxSetReferenceName( (vx_reference)usecase->graph_0, "graph_0");
    }

    return status;
}

vx_status uc_superset2_graph_0_delete(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_38 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_35 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_39 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_45 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_46 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_47 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_48 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_49 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_50 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_51 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }

    usecase->graph_0 = graph;

    return status;
}

vx_status uc_superset2_graph_0_verify(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }

    return status;
}

vx_status uc_superset2_graph_0_run(uc_superset2 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    return status;
}


