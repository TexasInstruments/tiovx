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


static tivx_obj_desc_graph_t *ownGraphDequeueFreeObjDesc(vx_graph graph);
static void ownGraphEnqueueFreeObjDesc(vx_graph graph,
                        tivx_obj_desc_graph_t *obj_desc);

VX_API_ENTRY vx_status vxSetGraphScheduleConfig(
    vx_graph graph,
    vx_enum graph_schedule_mode,
    uint32_t graph_parameters_list_size,
    const vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[]
    )
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if (graph->verified == vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"vxSetGraphScheduleConfig: Not supported on verified graph\n");
            status = VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            if(graph_schedule_mode==VX_GRAPH_SCHEDULE_MODE_NORMAL)
            {
                graph->schedule_mode = graph_schedule_mode;
            }
            else
            if( (   (graph_schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
                 || (graph_schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)
                )
              && (graph_parameters_list_size <= graph->num_params)
              )
            {
                uint32_t i;

                graph->schedule_mode = graph_schedule_mode;
                for(i=0; i<graph_parameters_list_size; i++)
                {
                    if((graph_parameters_queue_params_list[i].graph_parameter_index
                        >= graph->num_params)
                        ||
                        (graph_parameters_queue_params_list[i].refs_list_size >= TIVX_OBJ_DESC_QUEUE_MAX_DEPTH)
                        )
                    {
                        VX_PRINT(VX_ZONE_ERROR,"vxSetGraphScheduleConfig: Invalid parameters\n");
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                    else
                    {
                        graph->parameters[i].queue_enable = vx_true_e;
                        graph->parameters[i].num_buf = graph_parameters_queue_params_list[i].refs_list_size;
                        if(graph_parameters_queue_params_list[i].refs_list!=NULL)
                        {
                            uint32_t buf_id;

                            for(buf_id=0; buf_id<graph->parameters[i].num_buf; buf_id++)
                            {
                                graph->parameters[i].refs_list[buf_id] = graph_parameters_queue_params_list[i].refs_list[buf_id];
                            }
                        }
                    }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "user parameter list (%d) > number of graph parameters (%d)\n",
                    graph_parameters_list_size,
                    graph->num_params
                    );
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGraphParameterEnqueueReadyRef(vx_graph graph,
                vx_uint32 graph_parameter_index,
                vx_reference *refs,
                vx_uint32 num_refs)
{
    tivx_data_ref_queue data_ref_q = NULL;
    vx_status status = VX_SUCCESS;

    /* get data ref queue associated with a graph parameter
     * if this graph parameter is not enabled in queuing mode,
     * then a NULL data_ref_q is returned
     */
    data_ref_q = ownGraphGetParameterDataRefQueue(graph, graph_parameter_index);
    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Reference enqueue not supported at graph parameter index %d\n", graph_parameter_index);
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        vx_uint32 ref_id;
        vx_uint32 num_enqueue = 0;

        for(ref_id=0; ref_id<num_refs; ref_id++)
        {
            status = ownGraphParameterCheckValidEnqueueRef(graph, graph_parameter_index, refs[ref_id]);
            if(status!=VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "Unable to enqueue ref due to invalid ref\n");
            }
            if(status==VX_SUCCESS)
            {
                status = tivxDataRefQueueEnqueueReadyRef(data_ref_q, refs[ref_id]);
                if(status!=VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "Unable to enqueue ref\n");
                }
                else
                {
                    num_enqueue++;
                }
            }
            if(status!=VX_SUCCESS)
            {
                break;
            }
        }
        if(num_enqueue>0)
        {
            /* if graph mode is 'VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO' and
             * enqueue of a reference at this parameter should trigger
             * a graph schedule then schedule the graph */
            if(ownGraphDoScheduleGraphAfterEnqueue(graph, graph_parameter_index)==vx_true_e)
            {
                ownGraphScheduleGraph(graph, num_enqueue);
            }
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGraphParameterDequeueDoneRef(vx_graph graph,
            vx_uint32 graph_parameter_index,
            vx_reference *refs,
            vx_uint32 max_refs,
            vx_uint32 *num_refs)
{
    tivx_data_ref_queue data_ref_q = NULL;
    vx_status status = VX_SUCCESS;

    /* get data ref queue associated with a graph parameter
     * if this graph parameter is not enabled in queuing mode,
     * then a NULL data_ref_q is returned
     */
    data_ref_q = ownGraphGetParameterDataRefQueue(graph, graph_parameter_index);
    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Reference dequeue not supported at graph parameter index %d\n", graph_parameter_index);
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        vx_uint32 ref_id;


        for(ref_id=0; ref_id<max_refs; ref_id++)
        {
            vx_reference ref;
            vx_bool exit_loop = vx_false_e;

            /* wait until a reference is dequeued */
            do
            {
                ref = NULL;
                status = tivxDataRefQueueDequeueDoneRef(data_ref_q, &ref);
                if(status == VX_SUCCESS && ref != NULL)
                {
                    /* reference is dequeued break from do - while loop with success */
                    exit_loop = vx_true_e;
                }
                else
                {
                    /* wait for "ref available for dequeue" event */
                    status = tivxDataRefQueueWaitDoneRef(data_ref_q,
                            TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
                    if(status!=VX_SUCCESS)
                    {
                        /* some error in waiting for event, break loop with error */
                        exit_loop = vx_true_e;
                    }
                }
            } while(exit_loop == vx_false_e);
            if(status==VX_SUCCESS)
            {
                refs[ref_id] = ref;
            }
            else
            {
                /* some error in dequeue, dont try to dequeue further,
                 * break from loop with error */
                break;
            }
        }
        *num_refs = ref_id;
    }
    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxGraphParameterCheckDoneRef(vx_graph graph,
            vx_uint32 graph_parameter_index,
            vx_uint32 *num_refs)
{
    tivx_data_ref_queue data_ref_q = NULL;
    vx_status status = VX_SUCCESS;

    /* get data ref queue associated with a graph parameter
     * if this graph parameter is not enabled in queuing mode,
     * then a NULL data_ref_q is returned
     */
    data_ref_q = ownGraphGetParameterDataRefQueue(graph, graph_parameter_index);
    if(data_ref_q == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Reference dequeue not supported at graph parameter index %d\n", graph_parameter_index);
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        *num_refs = 0;
        status = tivxDataRefQueueGetDoneQueueCount(data_ref_q, num_refs);
    }
    return status;
}

vx_status ownGraphParameterCheckValidEnqueueRef(vx_graph graph, uint32_t graph_parameter_index, vx_reference ref)
{
    vx_status status = VX_FAILURE;

    if(graph != NULL && graph_parameter_index < graph->num_params && ref != NULL)
    {
        uint32_t buf_id;

        for(buf_id=0; buf_id<graph->parameters[graph_parameter_index].num_buf; buf_id++)
        {
            if(ref==graph->parameters[graph_parameter_index].refs_list[buf_id])
            {
                status = VX_SUCCESS;
                break;
            }
        }
    }
    return status;
}

static tivx_obj_desc_graph_t *ownGraphDequeueFreeObjDesc(vx_graph graph)
{
    vx_status status;
    tivx_obj_desc_graph_t *obj_desc = NULL;
    uint32_t pipeline_id;

    status = tivxQueueGet(&graph->free_q, &pipeline_id, 0);
    if(status==VX_SUCCESS && pipeline_id < graph->pipeline_depth)
    {
        obj_desc = graph->obj_desc[pipeline_id];
    }
    return obj_desc;
}

static void ownGraphEnqueueFreeObjDesc(vx_graph graph, tivx_obj_desc_graph_t *obj_desc)
{
    if(obj_desc != NULL && obj_desc->pipeline_id < graph->pipeline_depth)
    {
        tivxQueuePut(&graph->free_q, obj_desc->pipeline_id, 0);
    }
}

static tivx_obj_desc_graph_t *ownGraphGetObjDesc(vx_graph graph, uint32_t pipeline_id)
{
    tivx_obj_desc_graph_t *obj_desc = NULL;

    if(pipeline_id < graph->pipeline_depth)
    {
        obj_desc = graph->obj_desc[pipeline_id];
    }
    return obj_desc;
}

vx_status ownGraphCreateQueues(vx_graph graph)
{
    vx_status status;

    status = tivxQueueCreate(&graph->free_q, TIVX_GRAPH_MAX_PIPELINE_DEPTH, graph->free_q_mem, 0);

    return status;
}


void ownGraphDeleteQueues(vx_graph graph)
{
    tivxQueueDelete(&graph->free_q);
}

/* called during graph verify after pipeline_depth is calculated and set */
vx_status ownGraphAllocAndEnqueueObjDescForPipeline(vx_graph graph)
{
    uint32_t i;
    vx_status status = VX_SUCCESS;

    for(i=0; i<TIVX_GRAPH_MAX_PIPELINE_DEPTH; i++)
    {
        graph->obj_desc[i] = NULL;
    }
    if(graph->pipeline_depth >= TIVX_GRAPH_MAX_PIPELINE_DEPTH)
    {
        VX_PRINT(VX_ZONE_ERROR,"ownGraphAllocAndEnqueueObjDescForPipeline: Invalid graph pipeline depth\n");
        status = VX_FAILURE;
    }
    else
    {
        for(i=0; i<graph->pipeline_depth; i++)
        {
            graph->obj_desc[i] = (tivx_obj_desc_graph_t*)tivxObjDescAlloc(TIVX_OBJ_DESC_GRAPH, (vx_reference)graph);
            if(graph->obj_desc[i]==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR,"ownGraphAllocAndEnqueueObjDescForPipeline: Unable to alloc obj desc\n");
                status = VX_ERROR_NO_RESOURCES;
                break;
            }
        }
        if(status==VX_SUCCESS)
        {
            for(i=0; i<graph->pipeline_depth; i++)
            {
                graph->obj_desc[i]->pipeline_id = i;
                /* initial state of graph descriptor is verified since, this function is called during verify graph */
                graph->obj_desc[i]->state = VX_GRAPH_STATE_VERIFIED;
                graph->obj_desc[i]->complete_leaf_nodes = 0;
                graph->obj_desc[i]->exe_time_beg_h = 0;
                graph->obj_desc[i]->exe_time_beg_l = 0;
                graph->obj_desc[i]->exe_time_end_h = 0;
                graph->obj_desc[i]->exe_time_end_l = 0;

                ownGraphEnqueueFreeObjDesc(graph, graph->obj_desc[i]);
            }
        }
    }
    return status;
}

/* called during graph release */
void ownGraphFreeObjDesc(vx_graph graph)
{
    uint32_t i;

    for(i=0; i<graph->pipeline_depth; i++)
    {
        if(graph->obj_desc[i]!=NULL)
        {
            tivxObjDescFree((tivx_obj_desc_t**)&graph->obj_desc[i]);
            graph->obj_desc[i] = NULL;
        }
    }
}

vx_bool ownCheckGraphCompleted(vx_graph graph, uint32_t pipeline_id)
{
    vx_bool is_send_graph_complete_event = vx_false_e;
    tivx_obj_desc_graph_t *graph_obj_desc;
    uint32_t schedule_count = 0;

    ownReferenceLock(&graph->base);

    graph_obj_desc = ownGraphGetObjDesc(graph, pipeline_id);
    if (graph_obj_desc != NULL)
    {
        vx_bool is_completed = vx_false_e;

        /* a leaf node completed so increment 'complete_leaf_nodes' */
        graph_obj_desc->complete_leaf_nodes++;

        /* if all leaf nodes completed, then graph is completed */
        if(graph_obj_desc->complete_leaf_nodes==graph->num_leaf_nodes)
        {
            /* reset value to 0 for next graph run */
            graph_obj_desc->complete_leaf_nodes = 0;

            is_completed = vx_true_e;
        }

        /* all leaf nodes completed, threfore graph is completed */
        if(is_completed==vx_true_e)
        {
            uint64_t end_time;
            uint32_t i;

            is_send_graph_complete_event = vx_true_e;

            /* a submitted graph is completed so decrement this field */

            graph->submitted_count--;

            /* update node performance */
            for(i=0; i<graph->num_nodes; i++)
            {
                ownUpdateNodePerf(graph->nodes[i], graph_obj_desc->pipeline_id);
            }

            if(graph->schedule_mode == VX_GRAPH_SCHEDULE_MODE_NORMAL)
            {
                uint32_t i;

                /* delays need aging only if pipelining is not used */
                for(i=0; i<TIVX_GRAPH_MAX_DELAYS; i++)
                {
                    if(graph->delays[i])
                    {
                        vxAgeDelay(graph->delays[i]);
                    }
                    else
                    {
                        /* no more delays registered */
                        break;
                    }
                }
            }
            if (graph_obj_desc->state == VX_GRAPH_STATE_RUNNING)
            {
                graph_obj_desc->state = VX_GRAPH_STATE_COMPLETED;
            }

            end_time = tivxPlatformGetTimeInUsecs();

            tivx_uint64_to_uint32(
               end_time,
               &graph_obj_desc->exe_time_end_h,
               &graph_obj_desc->exe_time_end_l
              );

            tivxLogRtTraceGraphExeEnd(end_time, graph_obj_desc);

            ownUpdateGraphPerf(graph, graph_obj_desc->pipeline_id);

            /* if submitted queue is empty then state of graph object
             * is state on current completed graph pipeline instance
             * else dont change the state of graph object
             */
            if ( graph->submitted_count == 0 )
            {
                graph->state = graph_obj_desc->state;
            }

            ownGraphEnqueueFreeObjDesc(graph, graph_obj_desc);

            VX_PRINT(VX_ZONE_INFO,"Graph Completed (graph=%d, pipe=%d)\n",
                graph_obj_desc->base.obj_desc_id,
                graph_obj_desc->pipeline_id
                );

            /* if there are any pending graph desc to be scehdule
             * attempt to schedule them.
             * graph->schedule_pending_count is copied to a local variable
             * (schedule_count) and graph->schedule_pending_count is reset to 0.
             *
             * ownScheduleGraph, attempts to schedule the graph
             * schedule_count times, internally it reinits graph->schedule_pending_count
             * based on how many graph desc were scheduled
             * and how many were left pending.
             */
            if(graph->schedule_pending_count > 0)
            {
                schedule_count = graph->schedule_pending_count;

                graph->schedule_pending_count = 0;
            }

            /* if all submitted graphs completed and no more pending to be scheduled
             * then post all graph completed event
             */
            if ( ( graph->submitted_count == 0 )
                    && (schedule_count == 0))
            {
                VX_PRINT(VX_ZONE_INFO,"All Graphs Completed\n");

                /* there are no more pending graphs to set event to indicate no more pending graphs */
                tivxEventPost(graph->all_graph_completed_event);
            }
        }
    }

    ownReferenceUnlock(&graph->base);

    if(schedule_count > 0)
    {
        ownGraphScheduleGraph(graph, schedule_count);
    }

    return is_send_graph_complete_event;
}

vx_status ownGraphScheduleGraph(vx_graph graph, uint32_t num_schedule)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_graph_t *graph_obj_desc;
    uint32_t total_num_schedule, schedule_id, node_id;

    ownReferenceLock(&graph->base);

    /* total number of times to schedule is user requested num_schedule
     * + any other pending graph schedule
     */
    total_num_schedule = num_schedule + graph->schedule_pending_count;

    for(schedule_id=0; schedule_id<total_num_schedule; schedule_id++)
    {
        graph_obj_desc = ownGraphDequeueFreeObjDesc(graph);
        if(graph_obj_desc!=NULL)
        {
            uint64_t beg_time;

            beg_time = tivxPlatformGetTimeInUsecs();

            tivx_uint64_to_uint32(
                beg_time,
                &graph_obj_desc->exe_time_beg_h,
                &graph_obj_desc->exe_time_beg_l
            );

            tivxLogRtTraceGraphExeStart(beg_time, graph_obj_desc);

            ownGraphClearState(graph, graph_obj_desc->pipeline_id);

            graph_obj_desc->state = VX_GRAPH_STATE_RUNNING;
            graph->state = VX_GRAPH_STATE_RUNNING;

            /* a graph is about to be submitted, clear all_graph_completed_event if not already cleared */
            tivxEventClear(graph->all_graph_completed_event);

            /* a graph is submitted for execution so increment below field */
            graph->submitted_count++;

            VX_PRINT(VX_ZONE_INFO,"Scheduling Graph (graph=%d, pipe=%d)\n",
                graph_obj_desc->base.obj_desc_id,
                graph_obj_desc->pipeline_id
                );

            /* trigger graph execution by scheduling the head nodes
             * Head nodes will trigger further nodes execution after
             * their completion
             * This will continue until leaf nodes executes
             * After a leaf node executes, it will send a completion
             * event.
             * After all completion events are received, a graph is
             * considered to have
             * executed
             */
            for(node_id=0; node_id<graph->num_head_nodes; node_id++)
            {
                ownNodeKernelSchedule(graph->head_nodes[node_id], graph_obj_desc->pipeline_id);
            }
        }
        else
        {
            /* For VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO/MANUAL,
             * lack of graph descriptor, implies multiple graphs executions
             * on-going in pipeline hence lack of graph obj desc means
             * this graph schedule MUST be kept pending and tried after a previous graph execution
             * completes.
             */
            break;
        }
    }

    if(graph->schedule_mode!=VX_GRAPH_SCHEDULE_MODE_NORMAL)
    {
        /* Below logic updates the pending graph schedule
         */
        graph->schedule_pending_count = total_num_schedule - schedule_id;
    }
    else
    {
        if(schedule_id!=total_num_schedule)
        {
            /* for normal modes if all reqired graph schedules did not suceed
             * then this is a error condition as user has tried
             * doing schedule more times than is supported
             */
            VX_PRINT(VX_ZONE_ERROR,"Free graph descriptor not available, cannot schedule graph\n");
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    ownReferenceUnlock(&graph->base);

    return status;
}

vx_bool ownGraphDoScheduleGraphAfterEnqueue(vx_graph graph, uint32_t graph_parameter_index)
{
    vx_bool do_scehedule_graph_after_enqueue = vx_false_e;

    if(graph != NULL)
    {
        if(graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO)
        {
            if(graph_parameter_index == 0)
            {
                do_scehedule_graph_after_enqueue = vx_true_e;
            }
        }
    }
    return do_scehedule_graph_after_enqueue;
}

tivx_data_ref_queue ownGraphGetParameterDataRefQueue(vx_graph graph, vx_uint32 graph_parameter_index)
{
    tivx_data_ref_queue ref = NULL;

    if(graph != NULL && graph_parameter_index < graph->num_params)
    {
        ref = graph->parameters[graph_parameter_index].data_ref_queue;
    }
    return ref;
}

vx_status tivxSetGraphPipelineDepth(vx_graph graph, vx_uint32 pipeline_depth)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        if (graph->verified == vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxSetGraphPipelineDepth: Not supported on verified graph\n");
            status = VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            if (pipeline_depth < TIVX_GRAPH_MAX_PIPELINE_DEPTH)
            {
                graph->pipeline_depth = pipeline_depth;
                tivxLogSetResourceUsedValue("TIVX_GRAPH_MAX_PIPELINE_DEPTH", graph->pipeline_depth+1);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxSetGraphPipelineDepth: pipeline depth greater than max allowed pipeline depth\n");
                VX_PRINT(VX_ZONE_ERROR, "tivxSetGraphPipelineDepth: May need to increase the value of TIVX_GRAPH_MAX_PIPELINE_DEPTH in tiovx/include/tivx_config.h\n");
                status = VX_ERROR_INVALID_VALUE;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxSetGraphPipelineDepth: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

uint32_t ownGraphGetNumSchedule(vx_graph graph)
{
    uint32_t num_schedule = 0;

    if(graph != NULL)
    {
        if(graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)
        {
            uint32_t min_count = (uint32_t)-1;
            uint32_t count, i;

            for(i=0; i<graph->num_params; i++)
            {
                if(graph->parameters[i].queue_enable)
                {
                    count = 0;

                    tivxDataRefQueueGetReadyQueueCount(
                            graph->parameters[i].data_ref_queue, &count);

                    if(count<min_count)
                        min_count = count;
                }
            }
            if(min_count == (uint32_t)-1)
            {
                min_count = 0;
            }
            num_schedule = min_count;
        }
    }
    return num_schedule;
}
