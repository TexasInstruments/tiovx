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

#include "uc_sample_03.h"

vx_status uc_sample_03_create(uc_sample_03 usecase)
{
    vx_status status = VX_SUCCESS;
    
    memset(usecase, 0, sizeof(uc_sample_03_t));
    
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
        usecase->image_3 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
        if (usecase->image_3 == NULL)
        {
            status = VX_ERROR_NO_RESOURCES;
        }
        vxSetReferenceName( (vx_reference)usecase->image_3, "image_3");
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
    vx_node node = NULL;
    vx_reference params[] =
    {
          (vx_reference)image_0 ,
          (vx_reference)image_1 ,
          (vx_reference)image_2 
    };
    node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_ABSDIFF, params, 3);
    
    return node;
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
        vxSetReferenceName( (vx_reference)usecase->node_4, "node_4");
        vxSetNodeTarget(usecase->node_4, VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    
    usecase->graph_0 = graph;
    vxSetReferenceName( (vx_reference)usecase->graph_0, "graph_0");
    
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


