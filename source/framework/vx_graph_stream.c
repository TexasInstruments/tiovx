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

#define TIVX_STREAMING_STACK_SIZE      (128U * 1024U)
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
        if((vx_status)VX_SUCCESS == tivxWaitGraphEvent(graph, &event, (vx_bool)vx_false_e))
        {
            switch (state)
            {
                case IDLE:
                    if(START == event.app_value)
                    {
                        VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: START\n");
                        state = RUNNING;
                        status = tivxSendUserGraphEvent(graph, RUN, NULL);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM001 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxSendUserGraphEvent() failed.\n");
                        }
#endif
                    }

                    if(STOP == event.app_value)
                    {
                        VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM002 */
                        status = tivxEventPost(graph->stop_done);
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxEventPost() failed.\n");
                        }
#endif
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
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM003 */
                    if(START == event.app_value)
                    {
                        /* Already running, ignore */
                        VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: START\n");
                    }
#endif

                    if(STOP == event.app_value)
                    {
                        /* Change state to IDLE; if any pending RUN events then they get ignored in IDLE state */
                        VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: STOP\n");
                        state = IDLE;
                        status = tivxEventPost(graph->stop_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM004 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxEventPost() failed.\n");
                        }
#endif
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
                        status = ownGraphScheduleGraph(graph, 1);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM005 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "ownGraphScheduleGraph() failed.\n");
                        }
#endif

                        status = vxWaitGraph(graph);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM006 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph() failed.\n");
                        }
#endif

                        graph->streaming_executions++;
                        status = tivxSendUserGraphEvent(graph, RUN, NULL);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM007 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxSendUserGraphEvent() failed.\n");
                        }
#endif
                    }

                    break;
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM008 */
                default :
                    break;
#endif
            }
        }
    }
    status = tivxEventPost(graph->delete_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM009 */
    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxEventPost() failed.\n");
    }
#endif
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
        if((vx_status)VX_SUCCESS == tivxWaitGraphEvent(graph, &event, (vx_bool)vx_false_e))
        {
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
                            status = ownGraphScheduleGraphWrapper(graph);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM010 */
                            if (status != (vx_status)VX_SUCCESS)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "ownGraphScheduleGraphWrapper() failed.\n");
                            }
#endif
                        }
                    }

                    if(STOP == event.app_value)
                    {
                        VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
                        status = tivxEventPost(graph->stop_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM011 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxEventPost() failed.\n");
                        }
#endif
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
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM012 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph() failed.\n");
                        }
#endif

                        status = tivxEventPost(graph->stop_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM013 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxEventPost() failed.\n");
                        }
#endif
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
                        status = ownGraphScheduleGraph(graph, 1);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM014 */
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "ownGraphScheduleGraph() failed.\n");
                        }
#endif
                    }

                    break;
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM015 */
                default :
                    break;
#endif
            }
        }
    }
    status = tivxEventPost(graph->delete_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM016 */
    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxEventPost() failed.\n");
    }
#endif
}

VX_API_ENTRY vx_status vxStartGraphStreaming(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ( ((vx_bool)vx_true_e == graph->is_streaming_enabled) &&
             ((vx_bool)vx_true_e == graph->is_streaming_alloc) )
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
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ((vx_bool)vx_true_e == graph->is_streaming)
        {
            status = tivxEventClear(graph->stop_done);
            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxEventClear() failed.\n");
            }

            status = tivxSendUserGraphEvent(graph, STOP, NULL);
            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxSendUserGraphEvent() failed.\n");
            }
            status = tivxEventWait(graph->stop_done, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
            }

            graph->is_streaming  = (vx_bool)vx_false_e;
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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

vx_status tivxSendUserGraphEvent(vx_graph graph, vx_uint32 app_value, const void *parameter)
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

vx_status tivxWaitGraphEvent(
                    vx_graph graph, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
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

VX_API_ENTRY vx_status VX_API_CALL vxEnableGraphStreaming(vx_graph graph, vx_node trigger_node)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        graph->is_streaming_enabled = (vx_bool)vx_true_e;
        graph->is_pipelining_enabled = (vx_bool)vx_true_e;

        status = (vx_status)VX_SUCCESS;

        if(ownIsValidSpecificReference(vxCastRefFromNode(trigger_node), (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_false_e)
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

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if ((vx_bool)vx_true_e == graph->is_streaming_enabled)
        {
            tivxTaskSetDefaultCreateParams(&streamingTaskParams);

            streamingTaskParams.app_var = graph;
            streamingTaskParams.stack_ptr = NULL;
            streamingTaskParams.stack_size = TIVX_STREAMING_STACK_SIZE;
            streamingTaskParams.core_affinity = TIVX_TASK_AFFINITY_ANY;
            streamingTaskParams.priority = TIVX_STREAMING_TASK_PRIORITY;
            (void)strncpy(streamingTaskParams.task_name, "TIVX_STRM", TIVX_MAX_TASK_NAME);
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
                        status = ownRegisterEvent(vxCastRefFromNode(graph->nodes[graph->trigger_node_index]), TIVX_EVENT_GRAPH_QUEUE, VX_EVENT_NODE_COMPLETED, 0, STREAMING_EVENT);
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

#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM017 */
                            if ((vx_status)VX_SUCCESS != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "streaming task could not be created\n");
                            }
                            else
#endif
                            {
                                /* Everything necessary for streaming has been created */
                                graph->is_streaming_alloc = (vx_bool)vx_true_e;
                            }
                        }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM018 */
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "event could not be created\n");
                        }
#endif
                    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM019 */
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "event could not be created\n");
                    }
#endif
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

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
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

vx_status ownGraphFreeStreaming(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status tmp_status = (vx_status)VX_SUCCESS;

    if (NULL != graph->delete_done)
    {
        /* Clear event and send user event */
        tmp_status = tivxEventClear(graph->delete_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM020 */
        if (tmp_status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventClear() failed.\n");
            status = tmp_status;
        }
#endif
    }

    tmp_status = tivxSendUserGraphEvent(graph, DELETE, NULL);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM021 */
    if (tmp_status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxSendUserGraphEvent() failed.\n");
        status = tmp_status;
    }
#endif

    if (NULL != graph->delete_done)
    {
        tmp_status = tivxEventWait(graph->delete_done, graph->timeout_val);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM022 */
        if (tmp_status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
            status = tmp_status;
        }
#endif
    }

    (void)tivxTaskDelete(&graph->streaming_task_handle);

    tmp_status = ownEventQueueDelete(&graph->event_queue);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM023 */
    if (tmp_status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to delete event queue.\n");
        status = tmp_status;
    }
#endif

    if (NULL != graph->stop_done)
    {
        tmp_status = tivxEventDelete(&graph->stop_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM024 */
        if (tmp_status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventDelete() failed.\n");
            status = tmp_status;
        }
#endif
    }


    if (NULL != graph->delete_done)
    {
        tmp_status = tivxEventDelete(&graph->delete_done);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1714- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM025 */
        if (tmp_status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventDelete() failed.\n");
            status = tmp_status;
        }
#endif
    }

    return status;
}

