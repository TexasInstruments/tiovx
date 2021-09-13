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
#include <inttypes.h>
#include <TI/tivx.h>
#include <tivx_utils_graph_perf.h>

#define TIVX_UTILS_EXPORT_WRITELN(fp, message, ...) do { \
    snprintf(line, TIVX_UTILS_POINT_MAX_FILENAME, message"\n", ##__VA_ARGS__); \
    fwrite(line, 1, strlen(line), fp); \
    } while (1 == 0)

static vx_status tivx_utils_node_perf_export(FILE *fp, vx_node node);

vx_status tivx_utils_node_perf_print(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_perf_t node_perf;
    vx_char *node_name;
    char target_name[TIVX_TARGET_MAX_NAME];

    if(node!=NULL)
    {
        status = vxQueryReference((vx_reference)node, (vx_enum)VX_REFERENCE_NAME, &node_name, sizeof(vx_char*));
        if(status==(vx_status)VX_SUCCESS)
        {
            status = vxQueryNode(node, (vx_enum)TIVX_NODE_TARGET_STRING, target_name, TIVX_TARGET_MAX_NAME);
            if(status==(vx_status)VX_SUCCESS)
            {
                status = vxQueryNode(node, (vx_enum)VX_NODE_PERFORMANCE, &node_perf, sizeof(vx_perf_t));
                if(status==(vx_status)VX_SUCCESS)
                {
                    printf(" NODE: %14s: %24s: avg = %6"PRIu64" usecs, min/max = %6"PRIu64" / %6"PRIu64" usecs, #executions = %10"PRIu64"\n",
                        target_name,
                        node_name,
                        (node_perf.avg/1000u),
                        (node_perf.min/1000u),
                        (node_perf.max/1000u),
                        (node_perf.num)
                        );
                }

            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node object\n");
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

static vx_status tivx_utils_node_perf_export(FILE *fp, vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_perf_t node_perf;
    vx_char *node_name;
    char target_name[TIVX_TARGET_MAX_NAME];
    char line[TIVX_UTILS_MAX_LINE_SIZE];

    if(node!=NULL)
    {
        status = vxQueryReference((vx_reference)node, (vx_enum)VX_REFERENCE_NAME, &node_name, sizeof(vx_char*));
        if(status==(vx_status)VX_SUCCESS)
        {
            status = vxQueryNode(node, (vx_enum)TIVX_NODE_TARGET_STRING, target_name, TIVX_TARGET_MAX_NAME);
            if(status==(vx_status)VX_SUCCESS)
            {
                status = vxQueryNode(node, (vx_enum)VX_NODE_PERFORMANCE, &node_perf, sizeof(vx_perf_t));
                if(status==(vx_status)VX_SUCCESS)
                {
                    TIVX_UTILS_EXPORT_WRITELN(fp, " %24s (%10s)    | %6"PRIu64"    | %6"PRIu64" / %6"PRIu64"   | %10"PRIu64"",
                        node_name,
                        target_name,
                        (node_perf.avg/1000u),
                        (node_perf.min/1000u),
                        (node_perf.max/1000u),
                        (node_perf.num)
                        );
                }

            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node object\n");
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

vx_status tivx_utils_graph_perf_print(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t num_nodes, i;
    vx_perf_t graph_perf;
    vx_char *graph_name;

    status = vxQueryReference((vx_reference)graph, (vx_enum)VX_REFERENCE_NAME, &graph_name, sizeof(vx_char*));
    if(status==(vx_status)VX_SUCCESS)
    {
        status = vxQueryGraph(graph, (vx_enum)VX_GRAPH_PERFORMANCE, &graph_perf, sizeof(vx_perf_t));
        if(status==(vx_status)VX_SUCCESS)
        {
            status = vxQueryGraph(graph, (vx_enum)VX_GRAPH_NUMNODES, &num_nodes, sizeof(vx_uint32));
            if(status==(vx_status)VX_SUCCESS)
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

vx_status tivx_utils_graph_perf_export(FILE *fp, vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t num_nodes, i;
    vx_perf_t graph_perf;
    vx_char *graph_name;
    char line[TIVX_UTILS_MAX_LINE_SIZE];

    if (NULL != fp)
    {
        status = vxQueryReference((vx_reference)graph, (vx_enum)VX_REFERENCE_NAME, &graph_name, sizeof(vx_char*));

        if(status==(vx_status)VX_SUCCESS)
        {
            status = vxQueryGraph(graph, (vx_enum)VX_GRAPH_PERFORMANCE, &graph_perf, sizeof(vx_perf_t));
            if(status==(vx_status)VX_SUCCESS)
            {
                status = vxQueryGraph(graph, (vx_enum)VX_GRAPH_NUMNODES, &num_nodes, sizeof(vx_uint32));
                if(status==(vx_status)VX_SUCCESS)
                {
                    TIVX_UTILS_EXPORT_WRITELN(fp, "\n# GRAPH: Detailed Statistics\n");
                    TIVX_UTILS_EXPORT_WRITELN(fp, "\n##Node Execution Table\n");
                    TIVX_UTILS_EXPORT_WRITELN(fp, "Total Nodes      | Total executions");
                    TIVX_UTILS_EXPORT_WRITELN(fp, "----------|--------------");
                    TIVX_UTILS_EXPORT_WRITELN(fp, "%3d       | %6d\n", num_nodes, (uint32_t)graph_perf.num);
                    TIVX_UTILS_EXPORT_WRITELN(fp, "\n##Per Node Breakdown\n");
                    TIVX_UTILS_EXPORT_WRITELN(fp, "NODE      | avg (usecs)      | min/max (usecs)      | Total Executions");
                    TIVX_UTILS_EXPORT_WRITELN(fp, "----------|------------------|----------------------|------------");

                    for(i=0; i<num_nodes; i++)
                    {
                        vx_node node = tivxGraphGetNode(graph, i);

                        tivx_utils_node_perf_export(fp, node);
                    }
                }
            }
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "file pointer is NULL\n");
    }

    return status;
}
