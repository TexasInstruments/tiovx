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
#include <vx_internal.h>
#include <TI/tivx.h>
#include "enumstring.h"
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

static vx_status tivx_utils_ref_get_mem_size(vx_reference ref,  uint32_t *mem_size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    *mem_size = 0;
    switch (ref->type)
    {
        case (vx_enum)VX_TYPE_ARRAY: /* 0x80E */
        {
            tivx_obj_desc_array_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_CONVOLUTION: /* 0x80C */
        {
            tivx_obj_desc_convolution_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_DISTRIBUTION: /* 0x808 */
        {
            tivx_obj_desc_distribution_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_LUT: /* 0x807 */
        {
            tivx_obj_desc_lut_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_lut_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_MATRIX: /* 0x80B */
        {
            tivx_obj_desc_matrix_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            };
            break;
        }
        case (vx_enum)VX_TYPE_REMAP: /* 0x810 */
        {
            tivx_obj_desc_remap_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_remap_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_TENSOR: /* 0x815 */
        {
            tivx_obj_desc_tensor_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_USER_DATA_OBJECT: /* 0x816 */
        {
            tivx_obj_desc_user_data_object_t *obj_desc = NULL;
            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = obj_desc->mem_size;
            }
            break;
        }
        case (vx_enum)VX_TYPE_IMAGE: /* 0x80F */
        {
            vx_image image = (vx_image)ref;
            tivx_obj_desc_image_t *obj_desc = NULL;
            vx_uint32 p;

            obj_desc = (tivx_obj_desc_image_t *)image->base.obj_desc;
            if (NULL!=obj_desc)
            {
                for (p=0; p<obj_desc->planes; p++)
                {
                    *mem_size += obj_desc->mem_size[p];
                }
            }
            break;
        }
        case (vx_enum)TIVX_TYPE_RAW_IMAGE: /* 0x817 */
        {
            tivx_raw_image image = (tivx_raw_image)ref;
            tivx_obj_desc_raw_image_t *obj_desc = NULL;
            vx_uint32 p;

            obj_desc = (tivx_obj_desc_raw_image_t *)image->base.obj_desc;
            if (NULL!=obj_desc)
            {
                for (p=0; p<obj_desc->params.num_exposures; p++)
                {
                    *mem_size += obj_desc->mem_size[p];
                }
            }
            break;
        }
        case (vx_enum)VX_TYPE_SCALAR: /* 0x80D */
        {
            vx_scalar scalar = (vx_scalar)ref;
            tivx_obj_desc_scalar_t *obj_desc = NULL;

            obj_desc = (tivx_obj_desc_scalar_t *)scalar->base.obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = sizeof(uintptr_t);
            }
            break;
        }
        case (vx_enum)VX_TYPE_THRESHOLD: /* 0x80A */
        {
            vx_threshold thresh = (vx_threshold)ref;
            tivx_obj_desc_threshold_t *obj_desc = NULL;

            obj_desc = (tivx_obj_desc_threshold_t *)thresh->base.obj_desc;
            if (NULL!=obj_desc)
            {
                *mem_size = sizeof(vx_uint32);
            }
            break;
        }
        default:
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            break;
        }
    }
    return status;
}

vx_status tivx_utils_node_mem_print(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_node node;
    vx_char *node_name;
    uint32_t cur_node_idx = 0, num_prm = 0, prm_idx = 0, prm_dir = 0;
    vx_reference ref;
    vx_reference handled_input_ref[TIVX_UTILS_MAX_MATCHED_INPUT_REF];
    uint32_t handled_input_ref_index = 0;
    uint32_t mem_size = 0, num_buf = 0;

    for(cur_node_idx=0; cur_node_idx<graph->num_nodes; cur_node_idx++)
    {
        node = tivxGraphGetNode(graph, cur_node_idx);
        if(node==NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "invalid node object\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            status = vxQueryReference((vx_reference)node, (vx_enum)VX_REFERENCE_NAME, &node_name, sizeof(vx_char*));
        }

        if(status==(vx_status)VX_SUCCESS)
        {
            num_prm = node->kernel->signature.num_parameters;
            for(prm_idx=0; prm_idx<num_prm; prm_idx++)
            {
                prm_dir = node->kernel->signature.directions[prm_idx];
                ref = node->parameters[prm_idx];
                if(NULL!=ref)
                {
                    /* Handling of input param data object to avoid double counting */
                    if(prm_dir==(vx_enum)VX_INPUT)
                    {
                        uint32_t next_node_idx = 0, next_node_num_prm = 0, next_node_prm_idx = 0;
                        vx_node next_node;
                        vx_bool input_param_handled = vx_false_e;

                        for(next_node_idx=0u; next_node_idx<graph->num_nodes; next_node_idx++)
                        {
                            next_node = tivxGraphGetNode(graph, next_node_idx);
                            if(node!=next_node)
                            {
                                next_node_num_prm = next_node->kernel->signature.num_parameters;
                                for(next_node_prm_idx=0; next_node_prm_idx<next_node_num_prm; next_node_prm_idx++)
                                {
                                    if(ref==next_node->parameters[next_node_prm_idx])
                                    {
                                        if(next_node->kernel->signature.directions[next_node_prm_idx]==(vx_enum)VX_OUTPUT)
                                        {
                                            input_param_handled = vx_true_e;
                                            break;
                                        }
                                        else if(next_node->kernel->signature.directions[next_node_prm_idx]==(vx_enum)VX_INPUT)
                                        {
                                            if(next_node_idx<cur_node_idx)
                                            {
                                                input_param_handled = vx_true_e;
                                                break;
                                            }
                                            else 
                                            {
                                                uint32_t ref_idx;
                                                for(ref_idx=0;ref_idx<handled_input_ref_index;ref_idx++)
                                                {
                                                    if(handled_input_ref[ref_idx]==next_node->parameters[next_node_prm_idx])
                                                    {
                                                        input_param_handled = vx_true_e;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if(input_param_handled==vx_true_e)
                        {
                            continue;
                        }
                        else
                        {
                            if(handled_input_ref_index<TIVX_UTILS_MAX_MATCHED_INPUT_REF)
                            {
                                handled_input_ref[handled_input_ref_index++] = ref;
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Too many ref index to store, need to increase the value of TIVX_UTILS_MAX_MATCHED_INPUT_REF\n");
                                status = (vx_status)VX_FAILURE;
                                return status;
                            }
                        }
                    }
                    /* We have now removed any input object that is already handled*/

                    mem_size = 0;
                    num_buf = ownNodeGetParameterNumBuf(node, prm_idx);
                    if(num_buf==0)
                    {
                        /* Some data objects has num_buf as 0, which actually means it has a single buf */
                        num_buf = 1;
                    }

                    status = tivx_utils_ref_get_mem_size(ref, &mem_size);
                    if(status==(vx_status)VX_SUCCESS)
                    {
                        printf(" NODE_MEM: %24s: param = %d, dir = %10s, mem_size = (%10d * %2d (depth) = %10d) bytes, type = %24s\n",
                            node_name, prm_idx, enumToString(prm_dir), mem_size, num_buf, (mem_size * num_buf),
                            enumToString(ref->type));
                    }
                    else
                    {
                        if(ref->type==(vx_enum)VX_TYPE_PYRAMID) /* 0x809 */
                        {
                            vx_pyramid prmd = (vx_pyramid)ref;
                            tivx_obj_desc_pyramid_t *obj_desc = NULL;
                            vx_uint32 i;
                            vx_reference ref1;

                            obj_desc = (tivx_obj_desc_pyramid_t *)prmd->base.obj_desc;
                            if(NULL!=obj_desc)
                            {
                                for(i=0u; i<obj_desc->num_levels; i++)
                                {
                                    mem_size = 0;
                                    ref1 = (vx_reference)prmd->img[i];
                                    if(NULL!=ref1)
                                    {
                                        status = tivx_utils_ref_get_mem_size(ref1, &mem_size);
                                        if(status==(vx_status)VX_SUCCESS)
                                        {
                                            printf(" NODE_MEM: %24s: param = %d, dir = %10s, mem_size = (%10d * %2d (depth) = %10d) bytes, type = %24s, level = %d, type = %24s\n",
                                                node_name, prm_idx, enumToString(prm_dir), mem_size, num_buf, (mem_size * num_buf),
                                                enumToString(ref->type), i, enumToString(ref1->type));
                                        }
                                    }
                                }
                            }
                        }
                        else if(ref->type==(vx_enum)VX_TYPE_OBJECT_ARRAY) /*0x813 */
                        {
                            vx_object_array objarr = (vx_object_array)ref;
                            tivx_obj_desc_object_array_t *obj_desc = NULL;
                            vx_uint32 i;
                            vx_reference ref1;

                            obj_desc = (tivx_obj_desc_object_array_t *)objarr->base.obj_desc;
                            if(NULL!=obj_desc)
                            {
                                for(i=0u; i<obj_desc->num_items; i++)
                                {
                                    mem_size = 0;
                                    ref1 = objarr->ref[i];
                                    if(NULL!=ref1)
                                    {
                                        status = tivx_utils_ref_get_mem_size(ref1, &mem_size);
                                        if(status==(vx_status)VX_SUCCESS)
                                        {
                                            printf(" NODE_MEM: %24s: param = %d, dir = %10s, mem_size = (%10d * %2d (depth) = %10d) bytes, type = %24s, item = %d, type = %24s\n",
                                                node_name, prm_idx, enumToString(prm_dir), mem_size, num_buf, (mem_size * num_buf),
                                                enumToString(ref->type), i, enumToString(ref1->type));
                                        }
                                    }
                                }
                            }
                        }
                        else if(ref->type==(vx_enum)VX_TYPE_DELAY) /* 0x806 */
                        {
                            vx_delay delay = (vx_delay)ref;
                            vx_uint32 i;
                            vx_reference ref1;

                            for(i=0u; i<delay->count; i++)
                            {
                                mem_size = 0;
                                ref1 = delay->refs[i];
                                if(NULL!=ref1)
                                {
                                    status = tivx_utils_ref_get_mem_size(ref1, &mem_size);
                                    if(status==(vx_status)VX_SUCCESS)
                                    {
                                        printf(" NODE_MEM: %24s: param = %d, dir = %10s, mem_size = (%10d * %2d (depth) = %10d) bytes, type = %24s, index = %d, type = %24s\n",
                                            node_name, prm_idx, enumToString(prm_dir),  mem_size, num_buf, (mem_size * num_buf),
                                            enumToString(ref->type), i, enumToString(ref1->type));
                                    }
                                }
                            }
                        }
                        else
                        {
                            printf(" NODE_MEM: %24s: param = %d, dir = %10s, type = %24s - Unsupported reference type\n",
                                node_name, prm_idx, enumToString(prm_dir), enumToString(ref->type));
                        }
                    }
                }
            }
        }
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
                printf("Graph Node performance,\n");
                printf("======================\n");
                printf("\n");
                printf("GRAPH: %16s (#nodes = %3d, #executions = %6d)\n", graph_name, num_nodes, (uint32_t)graph_perf.num);

                for(i=0; i<num_nodes; i++)
                {
                    vx_node node = tivxGraphGetNode(graph, i);

                    tivx_utils_node_perf_print(node);
                }
                printf("\n");

                printf(" Node memory buffer usage,\n");
                printf("==========================\n");
                printf("\n");
                tivx_utils_node_mem_print(graph);
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
