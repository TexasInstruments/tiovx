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

#include "vx_tutorial_graph_user_kernel_pytiovx_uc.h"

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_create(vx_tutorial_graph_user_kernel_pytiovx_uc usecase, vx_bool add_as_target_kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    memset(usecase, 0, sizeof(vx_tutorial_graph_user_kernel_pytiovx_uc_t));
    
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->context = vxCreateContext();
        if (usecase->context == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_data_create(usecase);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_create(usecase, add_as_target_kernel);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_verify(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_verify(usecase);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_run(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_run(usecase);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_delete(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_delete(usecase);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vx_tutorial_graph_user_kernel_pytiovx_uc_data_delete(usecase);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseContext(&usecase->context);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_data_create(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    vx_context context = usecase->context;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->input = vxCreateImage(context, 640, 480, (vx_df_image)VX_DF_IMAGE_U8);
        if (usecase->input == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->input, "input");
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->grad_x = vxCreateImage(context, 640, 480, (vx_df_image)VX_DF_IMAGE_S16);
        if (usecase->grad_x == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->grad_x, "grad_x");
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->grad_y = vxCreateImage(context, 640, 480, (vx_df_image)VX_DF_IMAGE_S16);
        if (usecase->grad_y == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->grad_y, "grad_y");
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->phase = vxCreateImage(context, 640, 480, (vx_df_image)VX_DF_IMAGE_U8);
        if (usecase->phase == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->phase, "phase");
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->phase_rgb = vxCreateImage(context, 640, 480, (vx_df_image)VX_DF_IMAGE_RGB);
        if (usecase->phase_rgb == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->phase_rgb, "phase_rgb");
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_data_delete(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->input);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->grad_x);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->grad_y);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseReference((vx_reference*)&usecase->phase);
    }
    if (status == (vx_status)VX_SUCCESS)
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
    node = tivxCreateNodeByKernelEnum(graph, (vx_enum)VX_KERNEL_SOBEL_3x3, params, 3);
    
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
    node = tivxCreateNodeByKernelEnum(graph, (vx_enum)VX_KERNEL_PHASE, params, 3);
    
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
        
        if (vxGetStatus((vx_reference)kernel)==(vx_status)VX_SUCCESS)
        {
            node = tivxCreateNodeByKernelRef(graph, kernel, params, 2);
        }
        vxReleaseKernel(&kernel);
    }
    
    return node;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_create(vx_tutorial_graph_user_kernel_pytiovx_uc usecase, vx_bool add_as_target_kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    vx_context context = usecase->context;
    vx_graph graph = NULL;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        graph = vxCreateGraph(context);
        if (graph == NULL)
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->node_1 = usecase_node_create_node_1 (
            graph ,
            usecase->input ,
            usecase->grad_x ,
            usecase->grad_y 
          );
        vxSetReferenceName( (vx_reference)usecase->node_1, "node_1");
        vxSetNodeTarget(usecase->node_1, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->node_2 = usecase_node_create_node_2 (
            graph ,
            usecase->grad_x ,
            usecase->grad_y ,
            usecase->phase 
          );
        vxSetReferenceName( (vx_reference)usecase->node_2, "node_2");
        vxSetNodeTarget(usecase->node_2, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        usecase->node_3 = usecase_node_create_node_3 (
            graph ,
            usecase->phase ,
            usecase->phase_rgb 
          );
        vxSetReferenceName( (vx_reference)usecase->node_3, "node_3");
        if(add_as_target_kernel==(vx_bool)vx_true_e)
        {
            vxSetNodeTarget(usecase->node_3, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_DSP1);
        }
        else
        {
            vxSetNodeTarget(usecase->node_3, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_HOST);
        }
    }
    
    usecase->graph_0 = graph;
    vxSetReferenceName( (vx_reference)usecase->graph_0, "graph_0");
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_delete(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    vx_graph graph = usecase->graph_0;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_1 );
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_2 );
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseNode( &usecase->node_3 );
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxReleaseGraph(&graph);
    }
    
    usecase->graph_0 = graph;
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_verify(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    vx_graph graph = usecase->graph_0;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }
    
    return status;
}

vx_status vx_tutorial_graph_user_kernel_pytiovx_uc_graph_0_run(vx_tutorial_graph_user_kernel_pytiovx_uc usecase)
{
    vx_status status = (vx_status)VX_SUCCESS;
    
    vx_graph graph = usecase->graph_0;
    
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    
    return status;
}


