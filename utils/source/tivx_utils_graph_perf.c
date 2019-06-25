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

#include <stdio.h>
#include <TI/tivx.h>


vx_status tivx_utils_node_perf_print(vx_node node)
{
    vx_status status = VX_SUCCESS;
    vx_perf_t node_perf;
    vx_char *node_name;
    char target_name[TIVX_TARGET_MAX_NAME];

    if(node!=NULL)
    {
        status = vxQueryReference((vx_reference)node, VX_REFERENCE_NAME, &node_name, sizeof(vx_char*));
        if(status==VX_SUCCESS)
        {
            status = vxQueryNode(node, TIVX_NODE_TARGET_STRING, target_name, TIVX_TARGET_MAX_NAME);
            if(status==VX_SUCCESS)
            {
                status = vxQueryNode(node, VX_NODE_PERFORMANCE, &node_perf, sizeof(vx_perf_t));
                if(status==VX_SUCCESS)
                {
                    uint32_t fps;

                    if(node_perf.avg>0)
                    {
                        fps = (uint32_t)((1000*1000*1000*100ull)/node_perf.avg);
                    }
                    else
                    {
                        fps = 0;
                    }

                    printf(" NODE: %10s: %16s: %4d.%2d FPS (avg = %6d usecs, min/max = %6d / %6d usecs, #executions = %6d)\n",
                        target_name,
                        node_name,
                        fps/100,
                        fps%100,
                        (uint32_t)(node_perf.avg/1000),
                        (uint32_t)(node_perf.min/1000),
                        (uint32_t)(node_perf.max/1000),
                        (uint32_t)(node_perf.num)
                        );
                }

            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node object\n");
        status = VX_FAILURE;
    }
    return status;
}

vx_status tivx_utils_graph_perf_print(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    uint32_t num_nodes, i;
    vx_perf_t graph_perf;
    vx_char *graph_name;

    status = vxQueryReference((vx_reference)graph, VX_REFERENCE_NAME, &graph_name, sizeof(vx_char*));
    if(status==VX_SUCCESS)
    {
        status = vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &graph_perf, sizeof(vx_perf_t));
        if(status==VX_SUCCESS)
        {
            status = vxQueryGraph(graph, VX_GRAPH_NUMNODES, &num_nodes, sizeof(vx_uint32));
            if(status==VX_SUCCESS)
            {
                printf("GRAPH: %16s (#nodes = %3d, #executions = %6d)\n", graph_name, num_nodes, (uint32_t)graph_perf.num);

                for(i=0; i<num_nodes; i++)
                {
                    vx_node node = tivxGraphGetNode(graph, i);

                    tivx_utils_node_perf_print(node);
                }
                printf("\n");
            }
        }
    }

    return status;
}
