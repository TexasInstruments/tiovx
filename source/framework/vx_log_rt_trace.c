/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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
            (uintptr_t)node_obj_desc->base.host_ref);
    }
}

void tivxLogRtTraceNodeExeEnd(uint64_t time, tivx_obj_desc_node_t *node_obj_desc)
{
    if(tivxFlagIsBitSet(node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE))
    {
        printf("#%" PRIu64 "\n" "bZ n_%" PRIuPTR "\n",
            time,
            (uintptr_t)node_obj_desc->base.host_ref);
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
