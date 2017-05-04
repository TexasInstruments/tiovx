/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include "vx_tutorial_graph_user_kernel_pytiovx_uc.h"

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_create(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    memset(usecase, 0, sizeof(vx_tutorial_graph_user_kernel_pytiovx_uc_t));
    
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
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_data_create(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_create(usecase);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_verify(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_verify(usecase);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_run(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_run(usecase);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_delete(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_data_delete(usecase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_data_create(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    vx_context context = usecase->context;
    
    if (status == VX_SUCCESS)
    {
        usecase->input = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->input == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->input, "input");
    }
    if (status == VX_SUCCESS)
    {
        usecase->grad_x = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->grad_x == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->grad_x, "grad_x");
    }
    if (status == VX_SUCCESS)
    {
        usecase->grad_y = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->grad_y == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->grad_y, "grad_y");
    }
    if (status == VX_SUCCESS)
    {
        usecase->phase = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
        if (usecase->phase == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->phase, "phase");
    }
    if (status == VX_SUCCESS)
    {
        usecase->phase_rgb = vxCreateImage(context, 640, 480, VX_DF_IMAGE_RGB);
        if (usecase->phase_rgb == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->phase_rgb, "phase_rgb");
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_data_delete(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->input);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->grad_x);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->grad_y);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->phase);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->phase_rgb);
    }
    
    return status;
}

static vx_node usecase_node_create_node_1 (
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
    node = tivxCreateNodeByStructure(graph, VX_KERNEL_SOBEL_3x3, params, 3);
    
    return node;
}

static vx_node usecase_node_create_node_2 (
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
    node = tivxCreateNodeByStructure(graph, VX_KERNEL_PHASE, params, 3);
    
    return node;
}

static vx_node usecase_node_create_node_3 (
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
    {
        vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), "vx_tutorial_graph.phase_rgb");
        
        if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)
        {
            node = tivxCreateNodeByKernel(graph, kernel, params, 2);
        }
    }
    
    return node;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_create(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
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
        usecase->node_1 = usecase_node_create_node_1 (
            graph ,
            usecase->input ,
            usecase->grad_x ,
            usecase->grad_y 
          );
        vxSetReferenceName( (vx_reference)usecase->node_1, "node_1");
        vxSetNodeTarget(usecase->node_1, VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_2 = usecase_node_create_node_2 (
            graph ,
            usecase->grad_x ,
            usecase->grad_y ,
            usecase->phase 
          );
        vxSetReferenceName( (vx_reference)usecase->node_2, "node_2");
        vxSetNodeTarget(usecase->node_2, VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    if (status == VX_SUCCESS)
    {
        usecase->node_3 = usecase_node_create_node_3 (
            graph ,
            usecase->phase ,
            usecase->phase_rgb 
          );
        vxSetReferenceName( (vx_reference)usecase->node_3, "node_3");
        vxSetNodeTarget(usecase->node_3, VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    
    usecase->graph_0 = graph;
    vxSetReferenceName( (vx_reference)usecase->graph_0, "graph_0");
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_delete(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    vx_graph graph = usecase->graph_0;
    
    if (status == VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_1 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_2 );
    }
    if (status == VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_3 );
    }
    
    usecase->graph_0 = graph;
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_verify(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    vx_graph graph = usecase->graph_0;
    
    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_run(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = VX_SUCCESS;
    
    vx_graph graph = usecase->graph_0;
    
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    
    return status;
}


