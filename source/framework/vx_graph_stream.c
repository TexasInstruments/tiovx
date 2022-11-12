/*
 * Copyright (c) 2012-2018 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#define STREAMING_EVENT (0x00000001U)

static void VX_CALLBACK ownStreamingNoPipeliningTask(void *app_var);
static void VX_CALLBACK ownStreamingPipeliningTask(void *app_var);

static void VX_CALLBACK ownStreamingNoPipeliningTask(void *app_var)
{
    vx_graph graph = (vx_graph)app_var;
    vx_event_t event;
    vx_bool done = (vx_bool)vx_false_e;
    vx_uint32 state = IDLE;
    vx_status status;

    while((vx_bool)vx_false_e == done)
    {
        tivxWaitGraphEvent(graph, &event, (vx_bool)vx_false_e);

        switch (state)
        {
            case IDLE:
                if(START == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: START\n");
                    state = RUNNING;
                    tivxSendUserGraphEvent(graph, RUN, NULL);
                }

                if(STOP == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
                    tivxEventPost(graph->stop_done);
                }

                if(DELETE == event.app_value)
                {
                    /* Break from loop and exit task */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: DELETE\n");
                    done = (vx_bool)vx_true_e;
                }

                if(RUN == event.app_value)
                {
                    /* Do nothing, graph is stopped */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: RUN\n");
                }

                break;
            case RUNNING:
                if(START == event.app_value)
                {
                    /* Already running, ignore */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: START\n");
                }

                if(STOP == event.app_value)
                {
                    /* Change state to IDLE; if any pending RUN events then they get ignored in IDLE state */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: STOP\n");
                    state = IDLE;
                    tivxEventPost(graph->stop_done);
                }

                if(DELETE == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: DELETE\n");

                    status = vxWaitGraph(graph);

                    if (status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph() failed.\n");
                    }

                    state = IDLE;
                    done = (vx_bool)vx_true_e;
                }

                if(RUN == event.app_value)
                {
                    /* Execute graph then trigger another graph execution */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: RUN\n");
                    ownGraphScheduleGraph(graph, 1);

                    status = vxWaitGraph(graph);

                    if (status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph() failed.\n");
                    }

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

static void VX_CALLBACK ownStreamingPipeliningTask(void *app_var)
{
    vx_graph graph = (vx_graph)app_var;
    vx_event_t event;
    vx_bool done = (vx_bool)vx_false_e;
    vx_uint32 state = IDLE;
    vx_status status;

    while((vx_bool)vx_false_e == done)
    {
        tivxWaitGraphEvent(graph, &event, (vx_bool)vx_false_e);

        switch (state)
        {
            case IDLE:
                if(START == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: START\n");
                    state = RUNNING;
                    /* Execute graph then trigger another graph execution */
                    if ((vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL == graph->schedule_mode)
                    {
                        graph->streaming_executions++;
                        ownGraphScheduleGraphWrapper(graph);
                    }
                }

                if(STOP == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
                    tivxEventPost(graph->stop_done);
                }

                if(DELETE == event.app_value)
                {
                    /* Break from loop and exit task */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: DELETE\n");
                    done = (vx_bool)vx_true_e;
                }

                if(RUN == event.app_value)
                {
                    /* Do nothing, graph is stopped */
                    VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: RUN\n");
                }

                break;
            case RUNNING:
                if(START == event.app_value)
                {
                    /* Already running, ignore */
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: START\n");
                }

                if(STOP == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: STOP\n");
                    /* Change state to IDLE; if any pending RUN events then they get ignored in IDLE state */
                    state = IDLE;

                    status = vxWaitGraph(graph);

                    if (status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph() failed.\n");
                    }

                    tivxEventPost(graph->stop_done);
                }

                if(DELETE == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: DELETE\n");
                    state = IDLE;
                    done = (vx_bool)vx_true_e;
                }

                if(RUN == event.app_value)
                {
                    VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: RUN\n");
                }

                if(STREAMING_EVENT==event.app_value)
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
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ((vx_bool)vx_true_e == graph->is_streaming_enabled)
        {
            if ((vx_bool)vx_false_e == graph->is_streaming)
            {
                graph->is_streaming  = (vx_bool)vx_true_e;

                status = tivxSendUserGraphEvent(graph, START, NULL);

                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxSendUserGraphEvent() failed.\n");
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "this graph is currently streaming\n");
                status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "streaming has not been enabled. Please enable streaming prior to verifying graph\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status vxStopGraphStreaming(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ((vx_bool)vx_true_e == graph->is_streaming)
        {
            tivxEventClear(graph->stop_done);
            tivxSendUserGraphEvent(graph, STOP, NULL);
            status = tivxEventWait(graph->stop_done, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
            }

            graph->is_streaming  = (vx_bool)vx_false_e;
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            VX_PRINT(VX_ZONE_ERROR, "Streaming has not been started\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status VX_API_CALL tivxSendUserGraphEvent(vx_graph graph, vx_uint32 app_value, const void *parameter)
{
    vx_status status = (vx_status)VX_SUCCESS;

    uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

    status = ownEventQueueAddEvent(
                &graph->event_queue,
                (vx_enum)VX_EVENT_USER,
                timestamp, app_value,
                (uintptr_t)app_value, (uintptr_t)parameter, (uintptr_t)0);

    return status;
}

vx_status VX_API_CALL tivxWaitGraphEvent(
                    vx_graph graph, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        /* Call general wait function */
        status = vxWaitEventQueue(&graph->event_queue, event, do_not_block);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status VX_API_CALL vxEnableGraphStreaming(vx_graph graph, vx_node trigger_node)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        graph->is_streaming_enabled = (vx_bool)vx_true_e;
        graph->is_pipelining_enabled = (vx_bool)vx_true_e;

        status = (vx_status)VX_SUCCESS;

        if(ownIsValidSpecificReference((vx_reference)trigger_node, (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_false_e)
        {
            int32_t i;

            for (i = 0; i < (int32_t)graph->num_nodes; i++)
            {
                if (graph->nodes[i] == trigger_node)
                {
                    graph->trigger_node_index = (uint32_t)i;
                    graph->trigger_node_set = (vx_bool)vx_true_e;
                    break;
                }
            }

            if ((vx_bool)vx_false_e == graph->trigger_node_set)
            {
                VX_PRINT(VX_ZONE_ERROR, "trigger_node does not belong to graph\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphAllocForStreaming(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_task_create_params_t streamingTaskParams;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ((vx_bool)vx_true_e == graph->is_streaming_enabled)
        {
            tivxTaskSetDefaultCreateParams(&streamingTaskParams);

            streamingTaskParams.app_var = graph;
            streamingTaskParams.stack_ptr = NULL;
            streamingTaskParams.stack_size = TIVX_STREAMING_STACK_SIZE;
            streamingTaskParams.core_affinity = TIVX_TASK_AFFINITY_ANY;
            streamingTaskParams.priority = TIVX_STREAMING_TASK_PRIORITY;
            strncpy(streamingTaskParams.task_name, "TIVX_STRM", TIVX_MAX_TASK_NAME);
            streamingTaskParams.task_name[TIVX_MAX_TASK_NAME-1U] = (char)0;

            status = ownEventQueueCreate(&graph->event_queue);

            if ((vx_status)VX_SUCCESS == status)
            {
                ownEventQueueEnableEvents(&graph->event_queue, (vx_bool)vx_true_e);

                if (graph->pipeline_depth > 1U)
                {
                    /* Note: if pipelining with AUTO or MANUAL mode is enabled, re-trigger is done by pipelining and this is not needed */
                    if ( ((vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL == graph->schedule_mode) &&
                         ((vx_bool)vx_true_e == graph->trigger_node_set) )
                    {
                        status = ownRegisterEvent((vx_reference)graph->nodes[graph->trigger_node_index], TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_COMPLETED, 0, STREAMING_EVENT);
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                        VX_PRINT(VX_ZONE_ERROR, "trigger node is not set with pipelining\n");
                    }

                    if ((vx_status)VX_SUCCESS == status)
                    {
                        streamingTaskParams.task_main = &ownStreamingPipeliningTask;
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                        VX_PRINT(VX_ZONE_ERROR, "event could not be registered\n");
                    }
                }
                else
                {
                    streamingTaskParams.task_main = &ownStreamingNoPipeliningTask;
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    status = tivxEventCreate(&graph->delete_done);

                    if ((vx_status)VX_SUCCESS == status)
                    {
                        status = tivxEventCreate(&graph->stop_done);

                        if ((vx_status)VX_SUCCESS == status)
                        {
                            status = tivxTaskCreate(&graph->streaming_task_handle, &streamingTaskParams);

                            if ((vx_status)VX_SUCCESS != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "streaming task could not be created\n");
                            }
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "event could not be created\n");
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "event could not be created\n");
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "graph event queue could not be registered\n");
                }
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownGraphVerifyStreamingMode(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference((vx_reference)graph, (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ( ((vx_bool)vx_true_e == graph->is_streaming_enabled) &&
             ((graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO) ||
             (graph->schedule_mode==(vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL)) )
        {
            VX_PRINT(VX_ZONE_ERROR, "streaming is not supported with manual or auto pipelining\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid graph reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

void ownGraphFreeStreaming(vx_graph graph)
{
    if (graph->is_streaming_enabled != 0)
    {
        /* Clear event and send user event */
        tivxEventClear(graph->delete_done);
        tivxSendUserGraphEvent(graph, DELETE, NULL);
        tivxEventWait(graph->delete_done, graph->timeout_val);

        tivxTaskDelete(&graph->streaming_task_handle);

        ownEventQueueDelete(&graph->event_queue);

        tivxEventDelete(&graph->stop_done);

        tivxEventDelete(&graph->delete_done);
    }
}

