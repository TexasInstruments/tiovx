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

#if defined(LINUX) || defined(QNX)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <tivx_log_rt_if.h>

/** \brief Event logger queue header, structure size MUST be 64b aligned */
typedef struct {

    uint32_t rd_index;     /**< queue read index within event log */
    uint32_t wr_index;     /**< queue write index within event log */
    uint32_t count;        /**< number of element in queue */
    uint32_t rsv[1];       /**< used to aligned to 64b */

} tivx_log_rt_queue_t;

/** \brief Event logger shared memory data structure header */
typedef struct {

    tivx_log_rt_queue_t queue; /**< event logger queue header */
    tivx_log_rt_index_t index[TIVX_LOG_RT_INDEX_MAX]; /**< event logger index entries */

} tivx_log_rt_shm_header_t;

/** \brief Event logger object */
typedef struct {

    vx_bool is_valid; /**< vx_true_e: event logging is valid and can be used, else event logging is not supported */

    void    *log_rt_shm_base; /**< base address of shared memory */
    uint32_t log_rt_shm_size; /**< size of shared memory */

    tivx_log_rt_queue_t *queue; /**< pointer to queue header within shared memory */
    tivx_log_rt_index_t *index; /**< pointer to identifier index within shared memory */

    tivx_log_rt_entry_t *event_log_base; /**< pointer to event log within shared memory */
    uint32_t event_log_max_entries; /**< max possible events in event log */

} tivx_log_rt_obj_t;

static tivx_log_rt_obj_t g_tivx_log_rt_obj;

static void ownLogRtTraceAddEventClass(uint64_t event_id, uint16_t event_class, char *event_name);
static vx_bool ownLogRtTraceFindEventId(uint64_t event_id, uint16_t event_class);
static vx_bool ownLogRtTraceFindEventName(char *event_name);
static vx_status ownLogRtTraceSetup(vx_graph graph, vx_bool is_enable);
static void ownLogRtTraceLogEvent(uint64_t timestamp, uint64_t event_id, uint32_t event_value, uint16_t event_class, uint16_t event_type);


void ownLogRtInit()
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;

    memset(obj, 0, sizeof(tivx_log_rt_obj_t));

    ownPlatformGetLogRtShmInfo(&obj->log_rt_shm_base, &obj->log_rt_shm_size);

    if(obj->log_rt_shm_base != NULL && obj->log_rt_shm_size > sizeof(tivx_log_rt_shm_header_t))
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
        index[i].event_class = TIVX_LOG_RT_EVENT_CLASS_INVALID;
        index[i].event_id = 0u;
        index[i].event_class = TIVX_LOG_RT_EVENT_CLASS_INVALID;
        tivx_obj_desc_strncpy(index[i].event_name, "INVALID", TIVX_LOG_RT_EVENT_NAME_MAX);
    }
}

static vx_bool ownLogRtTraceFindEventId(uint64_t event_id, uint16_t event_class)
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;
    vx_bool is_found = vx_false_e;

    if(obj->is_valid)
    {
        uint32_t i;

        for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
        {
            if(    obj->index[i].event_class == event_class
                && obj->index[i].event_id == event_id
                )
            {
                is_found = vx_true_e;
                break;
            }
        }
    }
    return is_found;
}

static vx_bool ownLogRtTraceFindEventName(char *event_name)
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;
    vx_bool is_found = vx_false_e;

    if(obj->is_valid)
    {
        uint32_t i;

        for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
        {
            if( tivx_obj_desc_strncmp(obj->index[i].event_name, event_name, TIVX_LOG_RT_EVENT_NAME_MAX)
                == 0
                )
            {
                is_found = vx_true_e;
                break;
            }
        }
    }
    return is_found;
}

static void ownLogRtTraceAddEventClass(uint64_t event_id, uint16_t event_class, char *event_name)
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;

    if(obj->is_valid)
    {
        uint32_t i;
        vx_bool do_add_event = vx_true_e;

        if(ownLogRtTraceFindEventId(event_id, event_class)==vx_true_e)
        {
            VX_PRINT(VX_ZONE_WARNING,"Log RT event %ld of event class %d already exists, not adding again\n", event_id, (uint32_t)event_class);
            do_add_event = vx_false_e;
        }
        if(do_add_event)
        {
            if(ownLogRtTraceFindEventName(event_name)==vx_true_e)
            {
                VX_PRINT(VX_ZONE_WARNING,"Log RT event name %s of event %ld of event class %d already exists, not adding again."
                    "Recommend to use a unique event name.\n",
                    event_name, event_id, (uint32_t)event_class);
                do_add_event = vx_false_e;
            }
        }
        if(do_add_event)
        {
            for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
            {
                if(obj->index[i].event_class == TIVX_LOG_RT_EVENT_CLASS_INVALID)
                {
                    /* free index entry found, fill it */
                    obj->index[i].event_id = event_id;
                    obj->index[i].event_class = event_class;
                    tivx_obj_desc_strncpy(obj->index[i].event_name, event_name, TIVX_LOG_RT_EVENT_NAME_MAX);
                    break;
                }
            }
        }
    }
}

void tivxLogRtTraceRemoveEventClass(uint64_t event_id, uint16_t event_class, char *event_name)
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;

    if(obj->is_valid)
    {
        uint32_t i;

        for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
        {
            if(    obj->index[i].event_class == event_class
                && obj->index[i].event_id == event_id)
            {
                /* index entry found, mark it as empty */
                obj->index[i].event_id = 0u;
                obj->index[i].event_class = TIVX_LOG_RT_EVENT_CLASS_INVALID;
                tivx_obj_desc_strncpy(obj->index[i].event_name, "INVALID", TIVX_LOG_RT_EVENT_NAME_MAX);
                break;
            }
        }
    }
}

#if defined(LINUX) || defined(QNX)

static vx_status ownLogRtFileWrite(int fd, uint8_t *buf, uint32_t bytes_to_write, uint8_t *tmp_buf, uint32_t tmp_buf_size)
{
    uint32_t write_size;
    ssize_t ret_size;
    vx_status status = VX_SUCCESS;

    while(buf != NULL && bytes_to_write != 0)
    {
        if(bytes_to_write < tmp_buf_size)
            write_size = bytes_to_write;
        else
            write_size = tmp_buf_size;



        tivx_obj_desc_memcpy(tmp_buf, buf, write_size);
        ret_size = write(fd, tmp_buf, write_size);
        buf += write_size;
        bytes_to_write -= write_size;

        if(ret_size < write_size)
        {
            status = VX_FAILURE;
            break;
        }
    }
    return status;
}

vx_status tivxLogRtTraceExportToFile(char *filename)
{
    vx_status status = VX_SUCCESS;
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;
    tivx_log_rt_queue_t *queue = obj->queue;

    int fd = -1;
    vx_bool is_write_to_file = vx_false_e;

    if(obj->is_valid)
    {
        if(filename!=NULL)
        {
            fd = open(filename, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO );
            if(fd < 0)
            {
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "[%s] file could not be opened for writing run-time log\n", filename);
            }
            else
            {
                is_write_to_file = vx_true_e;
            }
        }

        if( status==VX_SUCCESS )
        {
            uint32_t tmp_buf_size = 128*1024;
            void *tmp_buf = malloc(tmp_buf_size);
            uint32_t num_log_entries;

            if(tmp_buf == NULL)
            {
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "Unable to allocate tmp buffer for writing run-time log\n");
            }
            else
            {
                uint8_t *buf_1, *buf_2;
                uint32_t bytes_to_write_1, bytes_to_write_2;

                bytes_to_write_1 = 0;
                bytes_to_write_2 = 0;
                buf_1 = NULL;
                buf_2 = NULL;

                ownPlatformSystemLock(TIVX_PLATFORM_LOCK_LOG_RT_INDEX);

                if(is_write_to_file)
                {
                    buf_1 = (uint8_t*)obj->index;
                    bytes_to_write_1 = TIVX_LOG_RT_INDEX_MAX*sizeof(tivx_log_rt_index_t);

                    status = ownLogRtFileWrite(fd,
                        buf_1, bytes_to_write_1,
                        tmp_buf, tmp_buf_size);

                    if(status != VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Unable to write run-time log index\n");
                    }
                }

                ownPlatformSystemUnlock(TIVX_PLATFORM_LOCK_LOG_RT_INDEX);

                bytes_to_write_1 = 0;
                bytes_to_write_2 = 0;
                buf_1 = NULL;
                buf_2 = NULL;

                ownPlatformSystemLock(TIVX_PLATFORM_LOCK_LOG_RT);

                num_log_entries = queue->count;
                if(num_log_entries > 0)
                {
                    if(queue->rd_index < queue->wr_index)
                    {
                        bytes_to_write_1 = (queue->wr_index - queue->rd_index)*sizeof(tivx_log_rt_entry_t);
                        buf_1 = (uint8_t*)&obj->event_log_base[queue->rd_index];
                    }
                    else
                    {
                        bytes_to_write_1 = (obj->event_log_max_entries - queue->rd_index)*sizeof(tivx_log_rt_entry_t);
                        buf_1 = (uint8_t*)&obj->event_log_base[queue->rd_index];

                        bytes_to_write_2 = queue->wr_index*sizeof(tivx_log_rt_entry_t);
                        buf_2 = (uint8_t*)&obj->event_log_base[0];
                    }
                }

                ownPlatformSystemUnlock(TIVX_PLATFORM_LOCK_LOG_RT);

                if(is_write_to_file)
                {
                    if(status == VX_SUCCESS)
                    {
                        status = ownLogRtFileWrite(fd,
                            buf_1, bytes_to_write_1,
                            tmp_buf, tmp_buf_size);

                        if(status == VX_SUCCESS)
                        {
                            ownLogRtFileWrite(fd,
                                buf_2, bytes_to_write_2,
                                tmp_buf, tmp_buf_size);
                        }

                        if(status != VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Unable to write run-time log\n");
                        }
                    }
                }

                if(num_log_entries > 0)
                {
                    volatile uint32_t tmp;

                    ownPlatformSystemLock(TIVX_PLATFORM_LOCK_LOG_RT);
                    queue->count -= num_log_entries;
                    queue->rd_index = (queue->rd_index + num_log_entries) % obj->event_log_max_entries;
                    tmp = queue->count;
                    tmp; /* readback to make sure update has reached the memory */
                    ownPlatformSystemUnlock(TIVX_PLATFORM_LOCK_LOG_RT);
                }

                free(tmp_buf);
            }

            if(is_write_to_file)
            {
                close(fd);
            }
        }
    }

    return status;
}
#endif

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

void tivxLogRtTraceKernelInstanceAddEvent(vx_node node, uint16_t event_index, char *event_name)
{
    char name[TIVX_LOG_RT_EVENT_NAME_MAX];
    snprintf(name, TIVX_LOG_RT_EVENT_NAME_MAX, "%s_%s", node->base.name, event_name);

    if(vx_false_e == ownLogRtTraceFindEventName(name))
    {
        ownLogRtTraceAddEventClass((uintptr_t)node+event_index, TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE, name);
    }
}

void tivxLogRtTraceKernelInstanceRemoveEvent(vx_node node, uint16_t event_index)
{
    tivxLogRtTraceRemoveEventClass((uintptr_t)node+event_index, TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE, NULL);
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

static vx_status ownLogRtTraceSetup(vx_graph graph, vx_bool is_enable)
{
    uint32_t node_id, pipe_id, target_id;
    #define TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH  (64u)
    vx_enum targets[TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH];
    vx_status status = (vx_status)VX_SUCCESS;
    char target_name[TIVX_TARGET_MAX_NAME];

    if (   (ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
        && (graph->verified == (vx_bool)vx_true_e))
    {
        ownPlatformSystemLock(TIVX_PLATFORM_LOCK_LOG_RT_INDEX);

        for(target_id=0; target_id<TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH; target_id++)
        {
            targets[target_id] = (vx_enum)TIVX_TARGET_ID_INVALID;
        }
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            vx_node node = graph->nodes[node_id];

            if(node != NULL)
            {
                if(is_enable)
                {
                    ownLogRtTraceAddEventClass((uintptr_t)node, TIVX_LOG_RT_EVENT_CLASS_NODE, node->base.name);
                }
                else
                {
                    tivxLogRtTraceRemoveEventClass((uintptr_t)node, TIVX_LOG_RT_EVENT_CLASS_NODE, node->base.name);
                }

                for(pipe_id=0; pipe_id<node->pipeline_depth; pipe_id++)
                {
                    if(node->obj_desc[pipe_id] != NULL)
                    {
                        if(is_enable)
                        {
                            tivxFlagBitSet(
                                &node->obj_desc[pipe_id]->base.flags,
                                TIVX_REF_FLAG_LOG_RT_TRACE);
                        }
                        else
                        {
                            tivxFlagBitClear(
                                &node->obj_desc[pipe_id]->base.flags,
                                TIVX_REF_FLAG_LOG_RT_TRACE);
                        }
                    }
                }
                if(node->obj_desc[0] != NULL)
                {
                    vx_bool done = (vx_bool)vx_false_e;
                    for(target_id=0;target_id<TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH;target_id++)
                    {
                        done = (vx_bool)vx_false_e;
                        if((vx_enum)node->obj_desc[0]->target_id==targets[target_id])
                        {
                            done = (vx_bool)vx_true_e;
                        }
                        if(targets[target_id]==(vx_enum)TIVX_TARGET_ID_INVALID)
                        {
                            targets[target_id] = (vx_enum)node->obj_desc[0]->target_id;
                            done = (vx_bool)vx_true_e;
                        }
                        if(done != 0)
                        {
                            break;
                        }
                    }
                }
            }
        }

        if(is_enable)
        {
            ownLogRtTraceAddEventClass(graph->obj_desc[0]->base.obj_desc_id, TIVX_LOG_RT_EVENT_CLASS_GRAPH, graph->base.name);
        }
        else
        {
            tivxLogRtTraceRemoveEventClass(graph->obj_desc[0]->base.obj_desc_id, TIVX_LOG_RT_EVENT_CLASS_GRAPH, graph->base.name);
        }

        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            if(graph->obj_desc[pipe_id] != NULL)
            {
                if(is_enable)
                {
                    tivxFlagBitSet(
                        &graph->obj_desc[pipe_id]->base.flags,
                        TIVX_REF_FLAG_LOG_RT_TRACE);
                }
                else
                {
                    tivxFlagBitClear(
                        &graph->obj_desc[pipe_id]->base.flags,
                        TIVX_REF_FLAG_LOG_RT_TRACE);
                }
            }
        }
        for(target_id=0;target_id<TIVX_LOG_RT_TRACE_MAX_TARGETS_IN_GRAPH;target_id++)
        {
            if(targets[target_id]==(vx_enum)TIVX_TARGET_ID_INVALID)
            {
                break;
            }

            ownPlatformGetTargetName(targets[target_id], target_name);

            if(is_enable)
            {
                ownLogRtTraceAddEventClass(targets[target_id], TIVX_LOG_RT_EVENT_CLASS_TARGET, target_name);
            }
            else
            {
                tivxLogRtTraceRemoveEventClass(targets[target_id], TIVX_LOG_RT_EVENT_CLASS_TARGET, target_name);
            }
        }
        ownPlatformSystemUnlock(TIVX_PLATFORM_LOCK_LOG_RT_INDEX);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters or graph node not verified");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

vx_status tivxLogRtTraceEnable(vx_graph graph)
{
    return ownLogRtTraceSetup(graph, vx_true_e);
}

vx_status tivxLogRtTraceDisable(vx_graph graph)
{
    return ownLogRtTraceSetup(graph, vx_false_e);
}
