/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */


#include <vx_internal.h>
#include <inttypes.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void tivxLogRtTraceNodeExeStart(uint64_t time, tivx_obj_desc_node_t *node_obj_desc)
{
    if(tivxFlagIsBitSet(node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "b"BYTE_TO_BINARY_PATTERN" n_%" PRIuPTR "\n",
            time,
            BYTE_TO_BINARY(node_obj_desc->pipeline_id),
            node_obj_desc->base.host_ref);
    }
}

void tivxLogRtTraceNodeExeEnd(uint64_t time, tivx_obj_desc_node_t *node_obj_desc)
{
    if(tivxFlagIsBitSet(node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "bZ n_%" PRIuPTR "\n",
            time,
            node_obj_desc->base.host_ref);
    }
}

void tivxLogRtTraceGraphExeStart(uint64_t time, tivx_obj_desc_graph_t *graph_obj_desc)
{
    if(tivxFlagIsBitSet(graph_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "b"BYTE_TO_BINARY_PATTERN" g_%d\n",
            time,
            BYTE_TO_BINARY(graph_obj_desc->pipeline_id),
            graph_obj_desc->base.obj_desc_id);
    }
}

void tivxLogRtTraceGraphExeEnd(uint64_t time, tivx_obj_desc_graph_t *graph_obj_desc)
{
    if(tivxFlagIsBitSet(graph_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "bZ g_%d\n",
            time,
            graph_obj_desc->base.obj_desc_id);
    }
}

void tivxLogRtTraceTargetExeStart(tivx_target target, tivx_obj_desc_t *obj_desc)
{
    if(tivxFlagIsBitSet(obj_desc->flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "b1 t_%d\n",
            tivxPlatformGetTimeInUsecs(),
            target->target_id);
    }
}

void tivxLogRtTraceTargetExeEnd(tivx_target target, tivx_obj_desc_t *obj_desc)
{
    if(tivxFlagIsBitSet(obj_desc->flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "b0 t_%d\n",
            tivxPlatformGetTimeInUsecs(),
            target->target_id);
    }
}

vx_status tivxLogRtTrace(vx_graph graph)
{
    uint32_t node_id, pipe_id, target_id;
    #define TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH  (64u)
    vx_enum targets[TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH];
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];

    if (   (NULL != graph)
        && (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
        && (graph->verified == vx_true_e))
    {
        for(target_id=0; target_id<TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH; target_id++)
        {
            targets[target_id] = TIVX_TARGET_ID_INVALID;
        }
        printf("$timescale\n");
        printf("1 us\n");
        printf("$end\n");
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            vx_node node = graph->nodes[node_id];

            if(node)
            {
                printf("$var reg 4 n_%" PRIuPTR " %s $end\n",
                    (uintptr_t)node,
                    node->base.name);

                for(pipe_id=0; pipe_id<node->pipeline_depth; pipe_id++)
                {
                    if(node->obj_desc[pipe_id])
                    {
                        tivxFlagBitSet(
                            &node->obj_desc[pipe_id]->base.flags,
                            TIVX_REF_FLAG_LOG_RT_TRACE);
                    }
                }
                if(node->obj_desc[0])
                {
                    vx_bool done = vx_false_e;
                    for(target_id=0;target_id<TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH;target_id++)
                    {
                        done = vx_false_e;
                        if(node->obj_desc[0]->target_id==targets[target_id])
                        {
                            done = vx_true_e;
                        }
                        if(targets[target_id]==TIVX_TARGET_ID_INVALID)
                        {
                            targets[target_id] = node->obj_desc[0]->target_id;
                            done = vx_true_e;
                        }
                        if(done)
                        {
                            break;
                        }
                    }
                }
            }
        }
        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            if(graph->obj_desc[pipe_id])
            {
                printf("$var reg 4 g_%d %s-%d $end\n",
                    graph->obj_desc[pipe_id]->base.obj_desc_id,
                    graph->base.name,
                    pipe_id);
                tivxFlagBitSet(
                    &graph->obj_desc[pipe_id]->base.flags,
                    TIVX_REF_FLAG_LOG_RT_TRACE);
            }
        }
        for(target_id=0;target_id<TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH;target_id++)
        {
            if(targets[target_id]==TIVX_TARGET_ID_INVALID)
            {
                break;
            }

            tivxPlatformGetTargetName(targets[target_id], target_name);

            printf("$var reg 1 t_%d %s $end\n",
                targets[target_id], target_name);

        }
        printf("$enddefinitions $end\n");
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters or graph node not verified");
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}
