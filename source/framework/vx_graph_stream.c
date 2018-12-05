/*
 * Copyright (c) 2012-2018 The Khronos Group Inc.
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

#define TIVX_STREAMING_STACK_SIZE      (32U * 1024U)
#define TIVX_STREAMING_TASK_PRIORITY   (8u)

/* Streaming states */
#define IDLE (0U)
#define RUNNING (1U)

/* Streaming events */
#define START  (0x00010000U)
#define STOP   (0x00010001U)
#define DELETE (0x00010010U)
#define RUN    (0x00010011U)

static void VX_CALLBACK tivxStreamingNoPipeliningTask(void *app_var)
{
    vx_graph graph = (vx_graph)app_var;
    vx_event_t event;
    vx_bool done = vx_false_e;
    vx_uint32 state = IDLE;

    while(vx_false_e == done)
    {
        tivxWaitGraphEvent(graph, &event, vx_false_e);

        switch (state)
        {
            case IDLE:
                if( (VX_EVENT_USER==event.type) &&
                    (START == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: START\n");
                    state = RUNNING;
                    tivxSendUserGraphEvent(graph, RUN, NULL);
                }

                if( (VX_EVENT_USER==event.type) &&
                    (STOP == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
                    tivxEventPost(graph->stop_done);
                }

                if( (VX_EVENT_USER==event.type) &&
                    (DELETE == event.event_info.user_event.user_event_id) )
                {
                    /* Break from loop and exit task */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: DELETE\n");
                    done = vx_true_e;
                }

                if( (VX_EVENT_USER==event.type) &&
                    (RUN == event.event_info.user_event.user_event_id) )
                {
                    /* Do nothing, graph is stopped */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: RUN\n");
                }

                break;
            case RUNNING:
                if( (VX_EVENT_USER==event.type) &&
                    (START == event.event_info.user_event.user_event_id) )
                {
                    /* Already running, ignore */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: START\n");
                }

                if( (VX_EVENT_USER==event.type) &&
                    (STOP == event.event_info.user_event.user_event_id) )
                {
                    /* Change state to IDLE; if any pending RUN events then they get ignored in IDLE state */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: STOP\n");
                    state = IDLE;
                    tivxEventPost(graph->stop_done);
                }

                if( (VX_EVENT_USER==event.type) &&
                    (DELETE == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: DELETE\n");
                    vxWaitGraph(graph);
                    state = IDLE;
                    done = vx_true_e;
                }

                if( (VX_EVENT_USER==event.type) &&
                    (RUN == event.event_info.user_event.user_event_id) )
                {
                    /* Execute graph then trigger another graph execution */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: RUN\n");
                    ownGraphScheduleGraph(graph, 1);
                    vxWaitGraph(graph);
                    graph->streaming_executions++;
                    tivxSendUserGraphEvent(graph, RUN, NULL);
                }

                break;
            default :
                break;
        }
    }
    tivxEventPost(graph->delete_done);
}

static void VX_CALLBACK tivxStreamingPipeliningTask(void *app_var)
{
    vx_graph graph = (vx_graph)app_var;
    vx_event_t event;
    vx_bool done = vx_false_e;
    vx_uint32 state = IDLE;

    while(vx_false_e == done)
    {
        tivxWaitGraphEvent(graph, &event, vx_false_e);

        switch (state)
        {
            case IDLE:
                if( (VX_EVENT_USER==event.type) &&
                    (START == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: START\n");
                    state = RUNNING;
                    /* Execute graph then trigger another graph execution */
                    if (VX_GRAPH_SCHEDULE_MODE_NORMAL == graph->schedule_mode)
                    {
                        graph->streaming_executions++;
                        ownGraphScheduleGraphWrapper(graph);
                    }
                }

                if( (VX_EVENT_USER==event.type) &&
                    (STOP == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
                    vxWaitGraph(graph);
                    tivxEventPost(graph->stop_done);
                }

                if( (VX_EVENT_USER==event.type) &&
                    (DELETE == event.event_info.user_event.user_event_id) )
                {
                    /* Break from loop and exit task */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: DELETE\n");
                    done = vx_true_e;
                }

                if( (VX_EVENT_USER==event.type) &&
                    (RUN == event.event_info.user_event.user_event_id) )
                {
                    /* Do nothing, graph is stopped */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: RUN\n");
                }

                break;
            case RUNNING:
                if( (VX_EVENT_USER==event.type) &&
                    (START == event.event_info.user_event.user_event_id) )
                {
                    /* Already running, ignore */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: START\n");
                }

                if( (VX_EVENT_USER==event.type) &&
                    (STOP == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: STOP\n");
                    /* Change state to IDLE; if any pending RUN events then they get ignored in IDLE state */
                    state = IDLE;
                    vxWaitGraph(graph);
                    tivxEventPost(graph->stop_done);
                }

                if( (VX_EVENT_USER==event.type) &&
                    (DELETE == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: DELETE\n");
                    state = IDLE;
                    done = vx_true_e;
                }

                if( (VX_EVENT_USER==event.type) &&
                    (RUN == event.event_info.user_event.user_event_id) )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: RUN\n");
                }

                if( (VX_EVENT_NODE_COMPLETED==event.type)
                    && (event.event_info.node_completed.node == graph->nodes[graph->trigger_node_index])
                    )
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: NODE COMPLETE\n");
                    graph->streaming_executions++;
                    ownGraphScheduleGraph(graph, 1);
                }

                break;
            default :
                break;
        }
    }
    tivxEventPost(graph->delete_done);
}

VX_API_ENTRY vx_status vxStartGraphStreaming(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        if (vx_true_e == graph->is_streaming_enabled)
        {
            if (vx_false_e == graph->is_streaming)
            {
                graph->is_streaming  = vx_true_e;

                status = tivxSendUserGraphEvent(graph, START, NULL);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "vxStartGraphStreaming: this graph is currently streaming\n");
                status = VX_ERROR_INVALID_REFERENCE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vxStartGraphStreaming: streaming has not been enabled. Please enable streaming prior to verifying graph\n");
            status = VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStartGraphStreaming: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status vxStopGraphStreaming(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        if (vx_true_e == graph->is_streaming)
        {
            tivxEventClear(graph->stop_done);
            tivxSendUserGraphEvent(graph, STOP, NULL);
            tivxEventWait(graph->stop_done, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

            if (VX_SUCCESS == status)
            {
                graph->is_streaming  = vx_false_e;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "vxStopGraphStreaming: tivxEventWait for streaming task failed\n");
            }

        }
        else
        {
            status = VX_ERROR_INVALID_REFERENCE;
            VX_PRINT(VX_ZONE_ERROR, "vxStopGraphStreaming: Streaming has not been started\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStopGraphStreaming: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status VX_API_CALL tivxSendUserGraphEvent(vx_graph graph, vx_uint32 id, void *parameter)
{
    vx_status status = VX_SUCCESS;

    uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000;

    status = tivxEventQueueAddEvent(
                &graph->event_queue,
                VX_EVENT_USER,
                timestamp,
                (uintptr_t)id, (uintptr_t)parameter, (uintptr_t)0);

    return status;
}

vx_status VX_API_CALL tivxWaitGraphEvent(
                    vx_graph graph, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = VX_SUCCESS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        /* Call general wait function */
        status = vxWaitEventQueue(&graph->event_queue, event, do_not_block);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxWaitGraphEvent: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status VX_API_CALL tivxEnableGraphStreaming(vx_graph graph, vx_node trigger_node)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        graph->is_streaming_enabled = vx_true_e;

        status = VX_SUCCESS;

        if((NULL != trigger_node) &&
           (ownIsValidSpecificReference((vx_reference)trigger_node, VX_TYPE_NODE)))
        {
            int i;

            for (i = 0; i < graph->num_nodes; i++)
            {
                if (graph->nodes[i] == trigger_node)
                {
                    graph->trigger_node_index = i;
                    graph->trigger_node_set = vx_true_e;
                    break;
                }
            }

            if (vx_false_e == graph->trigger_node_set)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxGraphSetStreamingTriggerNode: trigger_node does not belong to graph\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxGraphSetStreamingTriggerNode: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphAllocForStreaming(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    tivx_task_create_params_t streamingTaskParams;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        if (vx_true_e == graph->is_streaming_enabled)
        {
            tivxTaskSetDefaultCreateParams(&streamingTaskParams);

            streamingTaskParams.app_var = graph;
            streamingTaskParams.stack_ptr = NULL;
            streamingTaskParams.stack_size = TIVX_STREAMING_STACK_SIZE;
            streamingTaskParams.core_affinity = TIVX_TASK_AFFINITY_ANY;
            streamingTaskParams.priority = TIVX_STREAMING_TASK_PRIORITY;

            status = tivxEventQueueCreate(&graph->event_queue);

            if (VX_SUCCESS == status)
            {
                tivxEventQueueEnableEvents(&graph->event_queue, vx_true_e);

                if (graph->pipeline_depth > 1)
                {
                    /* Note: if pipelining with AUTO or MANUAL mode is enabled, re-trigger is done by pipelining and this is not needed */
                    if ( (VX_GRAPH_SCHEDULE_MODE_NORMAL == graph->schedule_mode) &&
                         (vx_true_e == graph->trigger_node_set) )
                    {
                        status = tivxRegisterEvent((vx_reference)graph->nodes[graph->trigger_node_index], TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_COMPLETED, 0);
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_REFERENCE;
                        VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: trigger node is not set with pipelining\n");
                    }

                    if (VX_SUCCESS == status)
                    {
                        streamingTaskParams.task_main = &tivxStreamingPipeliningTask;
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_REFERENCE;
                        VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: event could not be registered\n");
                    }
                }
                else
                {
                    streamingTaskParams.task_main = &tivxStreamingNoPipeliningTask;
                }

                if (VX_SUCCESS == status)
                {
                    status = tivxEventCreate(&graph->delete_done);

                    if (VX_SUCCESS == status)
                    {
                        status = tivxEventCreate(&graph->stop_done);

                        if (VX_SUCCESS == status)
                        {
                            status = tivxTaskCreate(&graph->streaming_task_handle, &streamingTaskParams);

                            if (VX_SUCCESS != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: streaming task could not be created\n");
                            }
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: event could not be created\n");
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: event could not be created\n");
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: graph event queue could not be registered\n");
                }
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownGraphAllocForStreaming: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphVerifyStreamingMode(vx_graph graph)
{
    vx_status status = VX_SUCCESS;

    if((NULL != graph) &&
       (ownIsValidSpecificReference((vx_reference)graph, VX_TYPE_GRAPH)))
    {
        if ( (vx_true_e == graph->is_streaming_enabled) &&
             ((graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO) ||
             (graph->schedule_mode==VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)) )
        {
            VX_PRINT(VX_ZONE_ERROR,"ownGraphVerifyStreamingMode: streaming is not supported with manual or auto pipelining\n");
            status = VX_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownGraphVerifyStreamingMode: invalid graph reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

void ownGraphFreeStreaming(vx_graph graph)
{
    if (graph->is_streaming_enabled)
    {
        /* Clear event and send user event */
        tivxEventClear(graph->delete_done);
        tivxSendUserGraphEvent(graph, DELETE, NULL);
        tivxEventWait(graph->delete_done, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        tivxTaskDelete(&graph->streaming_task_handle);

        tivxEventQueueDelete(&graph->event_queue);

        tivxEventDelete(&graph->stop_done);

        tivxEventDelete(&graph->delete_done);
    }
}

