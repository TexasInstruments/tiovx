/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <uc_sample_05.h>

vx_status uc_sample_05_create(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

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
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_2 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_2 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
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
        usecase->image_4 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->image_4 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
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
        usecase->pyramid_17 = vxCreatePyramid(context, 5, 1, 640, 480, VX_DF_IMAGE_U8);
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
    }
    if (status == VX_SUCCESS)
    {
        usecase->threshold_28 = vxCreateThreshold(context, 0, VX_TYPE_UINT8);
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
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_37 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_37 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
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
    }
    if (status == VX_SUCCESS)
    {
        usecase->image_44 = vxCreateImage(context, 1280, 720, VX_DF_IMAGE_U8);
        if (usecase->image_44 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
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
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ACCUMULATE, params, 3);
}

static vx_node usecase_node_create_node_6 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_scalar scalar_1 ,
  vx_image image_2 ,
  vx_image image_3
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)scalar_1 ,
          (vx_reference)image_2 ,
          (vx_reference)image_3
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ACCUMULATE_WEIGHTED, params, 4);
}

static vx_node usecase_node_create_node_9 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_convolution convolution_1 ,
  vx_image image_2
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)convolution_1 ,
          (vx_reference)image_2
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CUSTOM_CONVOLUTION, params, 3);
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
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_MINMAXLOC, params, 7);
}

static vx_node usecase_node_create_node_18 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_pyramid pyramid_1
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)pyramid_1
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_GAUSSIAN_PYRAMID, params, 2);
}

static vx_node usecase_node_create_node_20 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_PHASE, params, 3);
}

static vx_node usecase_node_create_node_27 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_remap remap_1 ,
  vx_scalar scalar_2 ,
  vx_image image_3
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)remap_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)image_3
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_REMAP, params, 4);
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
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)threshold_1 ,
          (vx_reference)scalar_2 ,
          (vx_reference)scalar_3 ,
          (vx_reference)image_4
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CANNY_EDGE_DETECTOR, params, 5);
}

static vx_node usecase_node_create_node_36 (
  vx_graph graph ,
  vx_scalar scalar_0 ,
  vx_image image_1 ,
  vx_matrix matrix_2 ,
  vx_image image_3
  )
{
    vx_reference params[] =
    {
          (vx_reference)scalar_0 ,
          (vx_reference)image_1 ,
          (vx_reference)matrix_2 ,
          (vx_reference)image_3
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_NON_LINEAR_FILTER, params, 4);
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
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 ,
          (vx_reference)image_3 ,
          (vx_reference)image_4
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CHANNEL_COMBINE, params, 5);
}

static vx_node usecase_node_create_node_40 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_distribution distribution_1
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)distribution_1
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_HISTOGRAM, params, 2);
}

static vx_node usecase_node_create_node_43 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_lut lut_1 ,
  vx_image image_2
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)lut_1 ,
          (vx_reference)image_2
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_TABLE_LOOKUP, params, 3);
}

static vx_node usecase_node_create_node_45 (
  vx_graph graph ,
  vx_image image_0 ,
  vx_image image_1 ,
  vx_image image_2
  )
{
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2
    };
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_AND, params, 3);
}

vx_status uc_sample_05_graph_0_create(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_context context = usecase->context;
    vx_graph graph = NULL;

    if (status == VX_SUCCESS)
    {
        graph = vxCreateGraph(context);
        if (graph == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_3 = usecase_node_create_node_3 (
            graph ,
            usecase->image_1 ,
            usecase->image_2 ,
            usecase->image_2
          );
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
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_9 = usecase_node_create_node_9 (
            graph ,
            usecase->image_4 ,
            usecase->convolution_8 ,
            usecase->image_7
          );
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
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_18 = usecase_node_create_node_18 (
            graph ,
            usecase->image_4 ,
            usecase->pyramid_17
          );
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_20 = usecase_node_create_node_20 (
            graph ,
            usecase->image_2 ,
            usecase->image_7 ,
            usecase->image_19
          );
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
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_40 = usecase_node_create_node_40 (
            graph ,
            usecase->image_37 ,
            usecase->distribution_39
          );
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_43 = usecase_node_create_node_43 (
            graph ,
            usecase->image_22 ,
            usecase->lut_42 ,
            usecase->image_41
          );
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_45 = usecase_node_create_node_45 (
            graph ,
            usecase->image_41 ,
            usecase->image_33 ,
            usecase->image_44
          );
    }

    usecase->graph_0 = graph;

    return status;
}

vx_status uc_sample_05_graph_0_delete(uc_sample_05 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }
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


