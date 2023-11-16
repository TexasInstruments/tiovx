/*
*
* Copyright (c) 2018-2023 Texas Instruments Incorporated
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

/** \brief Event logger shared memory data structure header */
typedef struct {

    tivx_log_rt_queue_t queue; /**< event logger queue header */
    tivx_log_rt_index_t index[TIVX_LOG_RT_INDEX_MAX]; /**< event logger index entries */

} tivx_log_rt_shm_header_t;

tivx_log_rt_obj_t g_tivx_log_rt_obj;

static void ownLogRtTraceLogEvent(uint64_t timestamp, uint64_t event_id, uint32_t event_value, uint16_t event_class, uint16_t event_type);

void ownLogRtInit()
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;

    (void)memset(obj, 0, sizeof(tivx_log_rt_obj_t));

    ownPlatformGetLogRtShmInfo(&obj->log_rt_shm_base, &obj->log_rt_shm_size);

    if((obj->log_rt_shm_base != NULL) && (obj->log_rt_shm_size > sizeof(tivx_log_rt_shm_header_t)))
    {
        obj->is_valid = vx_true_e;
    }

    if(obj->is_valid)
    {
        obj->queue          = (tivx_log_rt_queue_t*)( (uintptr_t)obj->log_rt_shm_base );
        obj->index          = (tivx_log_rt_index_t*)( (uintptr_t)obj->log_rt_shm_base + sizeof(tivx_log_rt_queue_t)      );
        obj->event_log_base = (tivx_log_rt_entry_t*)( (uintptr_t)obj->log_rt_shm_base + sizeof(tivx_log_rt_shm_header_t) );
        obj->event_log_max_entries = (obj->log_rt_shm_size - sizeof(tivx_log_rt_shm_header_t) ) / ( sizeof(tivx_log_rt_entry_t) );
    }
}

void ownLogRtResetShm(void *shm_base, uint32_t shm_size)
{
    tivx_log_rt_queue_t *queue;
    tivx_log_rt_index_t *index;
    uint32_t i;

    queue = (tivx_log_rt_queue_t*)( (uintptr_t)shm_base );
    index = (tivx_log_rt_index_t*)( (uintptr_t)shm_base + sizeof(tivx_log_rt_queue_t) );

    queue->rd_index = 0;
    queue->wr_index = 0;
    queue->count = 0;

    for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
    {
        index[i].event_id = 0u;
        index[i].event_class = TIVX_LOG_RT_EVENT_CLASS_INVALID;
        tivx_obj_desc_strncpy(index[i].event_name, "INVALID", TIVX_LOG_RT_EVENT_NAME_MAX);
    }
}

static void ownLogRtTraceLogEvent(uint64_t timestamp, uint64_t event_id, uint32_t event_value, uint16_t event_class, uint16_t event_type)
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;
    tivx_log_rt_entry_t *entry;
    tivx_log_rt_queue_t *queue = obj->queue;
    volatile uint32_t tmp;

    if(obj->is_valid)
    {
        ownPlatformSystemLock(TIVX_PLATFORM_LOCK_LOG_RT);

        if( queue->count < obj->event_log_max_entries )
        {
            entry = &obj->event_log_base[queue->wr_index];

            entry->timestamp   = timestamp;
            entry->event_id    = event_id;
            entry->event_value = event_value;
            entry->event_class = event_class;
            entry->event_type  = event_type;

            queue->wr_index = (queue->wr_index+1) % obj->event_log_max_entries;
            queue->count++;

            tmp = queue->count; /* readback to make sure update has reached the memory */
            tmp;
        }

        ownPlatformSystemUnlock(TIVX_PLATFORM_LOCK_LOG_RT);
    }
}

void tivxLogRtTraceKernelInstanceExeStartTimestamp(tivx_target_kernel_instance kernel, uint16_t event_index, uint64_t timestamp)
{
    if(tivxFlagIsBitSet(kernel->node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(timestamp,
            kernel->node_obj_desc->base.host_ref+event_index, 0,
            TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE, TIVX_LOG_RT_EVENT_TYPE_START);
    }
}

void tivxLogRtTraceKernelInstanceExeEndTimestamp(tivx_target_kernel_instance kernel, uint16_t event_index, uint64_t timestamp)
{
    if(tivxFlagIsBitSet(kernel->node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(timestamp,
            kernel->node_obj_desc->base.host_ref+event_index, 0,
            TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE, TIVX_LOG_RT_EVENT_TYPE_END);
    }
}

void tivxLogRtTraceKernelInstanceExeStart(tivx_target_kernel_instance kernel, uint16_t event_index)
{
    if(tivxFlagIsBitSet(kernel->node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(tivxPlatformGetTimeInUsecs(),
            kernel->node_obj_desc->base.host_ref+event_index, 0,
            TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE, TIVX_LOG_RT_EVENT_TYPE_START);
    }
}

void tivxLogRtTraceKernelInstanceExeEnd(tivx_target_kernel_instance kernel, uint16_t event_index)
{
    if(tivxFlagIsBitSet(kernel->node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(tivxPlatformGetTimeInUsecs(),
            kernel->node_obj_desc->base.host_ref+event_index, 0,
            TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE, TIVX_LOG_RT_EVENT_TYPE_END);
    }
}

void ownLogRtTraceNodeExeStart(uint64_t timestamp, tivx_obj_desc_node_t *node_obj_desc)
{
    if(tivxFlagIsBitSet(node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(timestamp,
            node_obj_desc->base.host_ref, node_obj_desc->pipeline_id,
            TIVX_LOG_RT_EVENT_CLASS_NODE, TIVX_LOG_RT_EVENT_TYPE_START);
    }
}

void ownLogRtTraceNodeExeEnd(uint64_t timestamp, tivx_obj_desc_node_t *node_obj_desc)
{
    if(tivxFlagIsBitSet(node_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(timestamp,
            node_obj_desc->base.host_ref, node_obj_desc->pipeline_id,
            TIVX_LOG_RT_EVENT_CLASS_NODE, TIVX_LOG_RT_EVENT_TYPE_END);
    }
}

void ownLogRtTraceGraphExeStart(uint64_t timestamp, tivx_obj_desc_graph_t *graph_obj_desc)
{
    if(tivxFlagIsBitSet(graph_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(timestamp,
            graph_obj_desc->base.obj_desc_id, graph_obj_desc->pipeline_id,
            TIVX_LOG_RT_EVENT_CLASS_GRAPH, TIVX_LOG_RT_EVENT_TYPE_START);
    }
}

void ownLogRtTraceGraphExeEnd(uint64_t timestamp, tivx_obj_desc_graph_t *graph_obj_desc)
{
    if(tivxFlagIsBitSet(graph_obj_desc->base.flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(timestamp,
            graph_obj_desc->base.obj_desc_id,
            graph_obj_desc->pipeline_id,
            TIVX_LOG_RT_EVENT_CLASS_GRAPH, TIVX_LOG_RT_EVENT_TYPE_END);
    }
}

void ownLogRtTraceTargetExeStart(tivx_target target, const tivx_obj_desc_t *obj_desc)
{
    if(tivxFlagIsBitSet(obj_desc->flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(tivxPlatformGetTimeInUsecs(),
            target->target_id, 0,
            TIVX_LOG_RT_EVENT_CLASS_TARGET, TIVX_LOG_RT_EVENT_TYPE_START);
    }
}

void ownLogRtTraceTargetExeEnd(tivx_target target, const tivx_obj_desc_t *obj_desc)
{
    if(tivxFlagIsBitSet(obj_desc->flags, TIVX_REF_FLAG_LOG_RT_TRACE) != 0)
    {
        ownLogRtTraceLogEvent(tivxPlatformGetTimeInUsecs(),
            target->target_id, 0,
            TIVX_LOG_RT_EVENT_CLASS_TARGET, TIVX_LOG_RT_EVENT_TYPE_END);
    }
}
