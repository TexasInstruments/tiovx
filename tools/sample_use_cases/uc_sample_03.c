/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <uc_sample_03.h>

vx_status uc_sample_03_create(uc_sample_03 usecase)
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
        status = uc_sample_03_data_create(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_sample_03_graph_0_create(usecase);
    }

    return status;
}

vx_status uc_sample_03_verify(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_sample_03_graph_0_verify(usecase);
    }

    return status;
}

vx_status uc_sample_03_run(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_sample_03_graph_0_run(usecase);
    }

    return status;
}

vx_status uc_sample_03_delete(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    if (status == VX_SUCCESS)
    {
        status = uc_sample_03_graph_0_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = uc_sample_03_data_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }

    return status;
}

vx_status uc_sample_03_data_create(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_context context = usecase->context;

    if (status == VX_SUCCESS)
    {
        usecase->image_1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
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
        usecase->image_3 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_3 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }

    return status;
}

vx_status uc_sample_03_data_delete(uc_sample_03 usecase)
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
        status = vxReleaseReference((vx_reference*)&usecase->image_3);
    }

    return status;
}

static vx_node usecase_node_create_node_4 (
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
    return tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ABSDIFF, params, 3);
}

vx_status uc_sample_03_graph_0_create(uc_sample_03 usecase)
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
        usecase->node_4 = usecase_node_create_node_4 (
            graph ,
            usecase->image_1 ,
            usecase->image_2 ,
            usecase->image_3
          );
    }

    usecase->graph_0 = graph;

    return status;
}

vx_status uc_sample_03_graph_0_delete(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_4 );
    }

    usecase->graph_0 = graph;

    return status;
}

vx_status uc_sample_03_graph_0_verify(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }

    return status;
}

vx_status uc_sample_03_graph_0_run(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;

    vx_graph graph = usecase->graph_0;

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    return status;
}


