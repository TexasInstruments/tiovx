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

#include "uc_sample_05.h"

vx_status uc_sample_05_create(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    memset(usecase, 0, sizeof(uc_sample_05_t));

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
        status = uc_sample_05_data_create(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_sample_05_graph_0_create(usecase);
    }

    return status;
}

vx_status uc_sample_05_verify(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_sample_05_graph_0_verify(usecase);
    }

    return status;
}

vx_status uc_sample_05_run(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_sample_05_graph_0_run(usecase);
    }

    return status;
}

vx_status uc_sample_05_delete(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_sample_05_graph_0_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_sample_05_data_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }

    return status;
}

vx_status uc_sample_05_data_create(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_context context = usecase->context;

    if (status == VX_SUCCESS)
    {
        usecase->image_1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_1 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_1, "image_1");
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_2 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_2 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_2, "image_2");
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
        usecase->image_4 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_4 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_4, "image_4");
    }
    if (status == VX_SUCCESS)
    {
        usecase->convolution_8 = vxCreateConvolution(context, 3, 3);
        if (usecase->convolution_8 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_7 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_7 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_7, "image_7");
    }
    if (status == VX_SUCCESS)
    {
        vx_uint8 value = 0;

        usecase->scalar_10 = vxCreateScalar(context, VX_TYPE_UINT8, &value);
        if (usecase->scalar_10 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint8 value = 255;

        usecase->scalar_11 = vxCreateScalar(context, VX_TYPE_UINT8, &value);
        if (usecase->scalar_11 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_12 = vxCreateArray(context, VX_TYPE_ENUM, 100);
        if (usecase->array_12 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->array_13 = vxCreateArray(context, VX_TYPE_ENUM, 100);
        if (usecase->array_13 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 value = 10;

        usecase->scalar_14 = vxCreateScalar(context, VX_TYPE_UINT32, &value);
        if (usecase->scalar_14 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_uint32 value = 100;

        usecase->scalar_15 = vxCreateScalar(context, VX_TYPE_UINT32, &value);
        if (usecase->scalar_15 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->pyramid_17 = vxCreatePyramid(context, 5, VX_SCALE_PYRAMID_HALF, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->pyramid_17 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_19 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_19 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_19, "image_19");
    }
    if (status == VX_SUCCESS)
    {
        usecase->remap_21 = vxCreateRemap(context, 640, 480, 1280, 720);
        if (usecase->remap_21 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_INTERPOLATION_BILINEAR;

        usecase->scalar_26 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_26 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_22 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_22 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_22, "image_22");
    }
    if (status == VX_SUCCESS)
    {
        usecase->threshold_28 = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8);
        if (usecase->threshold_28 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_int32 value = 100;

        usecase->scalar_30 = vxCreateScalar(context, VX_TYPE_INT32, &value);
        if (usecase->scalar_30 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_NORM_L1;

        usecase->scalar_31 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_31 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_29 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_29 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_29, "image_29");
    }
    if (status == VX_SUCCESS)
    {
        vx_enum value = VX_NONLINEAR_FILTER_MIN;

        usecase->scalar_35 = vxCreateScalar(context, VX_TYPE_ENUM, &value);
        if (usecase->scalar_35 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->matrix_34 = vxCreateMatrix(context, VX_TYPE_UINT8, 3, 3);
        if (usecase->matrix_34 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_33 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_33 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_33, "image_33");
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_37 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_37 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_37, "image_37");
    }
    if (status == VX_SUCCESS)
    {
        usecase->distribution_39 = vxCreateDistribution(context, 10, 1, 100);
        if (usecase->distribution_39 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->lut_42 = vxCreateLUT(context, VX_TYPE_UINT8, 255);
        if (usecase->lut_42 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_41 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_41 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_41, "image_41");
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_44 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_44 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_44, "image_44");
    }

    return status;
}

vx_status uc_sample_05_data_delete(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_1);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_2);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_5);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_4);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->convolution_8);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_7);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_10);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_11);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_12);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->array_13);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_14);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_15);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->pyramid_17);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_19);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->remap_21);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_26);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_22);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->threshold_28);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_30);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_31);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_29);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->scalar_35);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->matrix_34);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_33);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_37);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->distribution_39);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->lut_42);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_41);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->image_44);
    }

    return status;
}

static vx_node usecase_node_create_node_3 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ACCUMULATE, params, 3);

    return node;
}

static vx_node usecase_node_create_node_6 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_image image_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)image_2 ,
          (vx_reference)image_3 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ACCUMULATE_WEIGHTED, params, 4);

    return node;
}

static vx_node usecase_node_create_node_9 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_convolution convolution_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)convolution_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CUSTOM_CONVOLUTION, params, 3);

    return node;
}

static vx_node usecase_node_create_node_16 (
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

static vx_node usecase_node_create_node_18 (
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

static vx_node usecase_node_create_node_20 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_PHASE, params, 3);

    return node;
}

static vx_node usecase_node_create_node_27 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_remap remap_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)remap_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_REMAP, params, 4);

    return node;
}

static vx_node usecase_node_create_node_32 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_threshold threshold_1 ,
  vx_scalar scalar_2 ,
  vx_scalar scalar_3 ,
  vx_image image_4 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)threshold_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 ,
          (vx_reference)image_4 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CANNY_EDGE_DETECTOR, params, 5);

    return node;
}

static vx_node usecase_node_create_node_36 (
  vx_graph graph ,
  vx_scalar scalar_0 ,
  vx_image image_1 ,
  vx_matrix matrix_2 ,
  vx_image image_3 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)scalar_0 ,
          (vx_reference)image_1 ,
          (vx_reference)matrix_2 ,
          (vx_reference)image_3 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_NON_LINEAR_FILTER, params, 4);

    return node;
}

static vx_node usecase_node_create_node_38 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 ,
  vx_image image_3 ,
  vx_image image_4 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 ,
          (vx_reference)image_3 ,
          (vx_reference)image_4 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CHANNEL_COMBINE, params, 5);

    return node;
}

static vx_node usecase_node_create_node_40 (
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

static vx_node usecase_node_create_node_43 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_lut lut_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)lut_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_TABLE_LOOKUP, params, 3);

    return node;
}

static vx_node usecase_node_create_node_45 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2 
  )
{
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_AND, params, 3);

    return node;
}

vx_status uc_sample_05_graph_0_create(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        usecase->node_3 = usecase_node_create_node_3 (
            graph ,
            usecase->image_1 ,
            usecase->image_2 ,
            usecase->image_2 
          );
        vxSetReferenceName( (vx_reference)usecase->node_3, "node_3");
        vxSetNodeTarget(usecase->node_3, VX_TARGET_STRING, TIVX_TARGET_EVE1);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_6 = usecase_node_create_node_6 (
            graph ,
            usecase->image_1 ,
            usecase->scalar_5 ,
            usecase->image_4 ,
            usecase->image_4 
          );
        vxSetReferenceName( (vx_reference)usecase->node_6, "node_6");
        vxSetNodeTarget(usecase->node_6, VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_9 = usecase_node_create_node_9 (
            graph ,
            usecase->image_4 ,
            usecase->convolution_8 ,
            usecase->image_7 
          );
        vxSetReferenceName( (vx_reference)usecase->node_9, "node_9");
        vxSetNodeTarget(usecase->node_9, VX_TARGET_STRING, TIVX_TARGET_EVE2);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_16 = usecase_node_create_node_16 (
            graph ,
            usecase->image_7 ,
            usecase->scalar_10 ,
            usecase->scalar_11 ,
            usecase->array_12 ,
            usecase->array_13 ,
            usecase->scalar_14 ,
            usecase->scalar_15 
          );
        vxSetReferenceName( (vx_reference)usecase->node_16, "node_16");
        vxSetNodeTarget(usecase->node_16, VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_18 = usecase_node_create_node_18 (
            graph ,
            usecase->image_4 ,
            usecase->pyramid_17 
          );
        vxSetReferenceName( (vx_reference)usecase->node_18, "node_18");
        vxSetNodeTarget(usecase->node_18, VX_TARGET_STRING, TIVX_TARGET_EVE3);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_20 = usecase_node_create_node_20 (
            graph ,
            usecase->image_2 ,
            usecase->image_7 ,
            usecase->image_19 
          );
        vxSetReferenceName( (vx_reference)usecase->node_20, "node_20");
        vxSetNodeTarget(usecase->node_20, VX_TARGET_STRING, TIVX_TARGET_A15_0);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_27 = usecase_node_create_node_27 (
            graph ,
            usecase->image_19 ,
            usecase->remap_21 ,
            usecase->scalar_26 ,
            usecase->image_22 
          );
        vxSetReferenceName( (vx_reference)usecase->node_27, "node_27");
        vxSetNodeTarget(usecase->node_27, VX_TARGET_STRING, TIVX_TARGET_DSP2);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_32 = usecase_node_create_node_32 (
            graph ,
            usecase->image_22 ,
            usecase->threshold_28 ,
            usecase->scalar_30 ,
            usecase->scalar_31 ,
            usecase->image_29 
          );
        vxSetReferenceName( (vx_reference)usecase->node_32, "node_32");
        vxSetNodeTarget(usecase->node_32, VX_TARGET_STRING, TIVX_TARGET_EVE3);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_36 = usecase_node_create_node_36 (
            graph ,
            usecase->scalar_35 ,
            usecase->image_29 ,
            usecase->matrix_34 ,
            usecase->image_33 
          );
        vxSetReferenceName( (vx_reference)usecase->node_36, "node_36");
        vxSetNodeTarget(usecase->node_36, VX_TARGET_STRING, TIVX_TARGET_EVE4);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_38 = usecase_node_create_node_38 (
            graph ,
            usecase->image_19 ,
            usecase->image_22 ,
            usecase->image_29 ,
            usecase->image_33 ,
            usecase->image_37 
          );
        vxSetReferenceName( (vx_reference)usecase->node_38, "node_38");
        vxSetNodeTarget(usecase->node_38, VX_TARGET_STRING, TIVX_TARGET_A15_0);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_40 = usecase_node_create_node_40 (
            graph ,
            usecase->image_37 ,
            usecase->distribution_39 
          );
        vxSetReferenceName( (vx_reference)usecase->node_40, "node_40");
        vxSetNodeTarget(usecase->node_40, VX_TARGET_STRING, TIVX_TARGET_IPU1_0);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_43 = usecase_node_create_node_43 (
            graph ,
            usecase->image_22 ,
            usecase->lut_42 ,
            usecase->image_41 
          );
        vxSetReferenceName( (vx_reference)usecase->node_43, "node_43");
        vxSetNodeTarget(usecase->node_43, VX_TARGET_STRING, TIVX_TARGET_IPU1_1);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_45 = usecase_node_create_node_45 (
            graph ,
            usecase->image_41 ,
            usecase->image_33 ,
            usecase->image_44 
          );
        vxSetReferenceName( (vx_reference)usecase->node_45, "node_45");
        vxSetNodeTarget(usecase->node_45, VX_TARGET_STRING, TIVX_TARGET_IPU2);
    }

    vxSetReferenceName( (vx_reference)usecase->graph_0, "graph_0");

    return status;
}

vx_status uc_sample_05_graph_0_delete(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_3 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_6 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_9 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_16 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_18 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_20 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_27 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_32 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_36 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_38 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_40 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_43 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_45 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }

    usecase->graph_0 = graph;

    return status;
}

vx_status uc_sample_05_graph_0_verify(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }

    return status;
}

vx_status uc_sample_05_graph_0_run(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    return status;
}


