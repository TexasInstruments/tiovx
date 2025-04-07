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

static void VX_CALLBACK ownStreamingPipeliningTask(void *app_var);
static vx_status ownGraphStreamDeleteTask(vx_graph graph);
static vx_status ownGraphStreamDeleteQueues(vx_graph graph);

static void VX_CALLBACK ownStreamingPipeliningTask(void *app_var)
{
    vx_graph graph = (vx_graph)app_var;
    vx_event_t event;
    vx_bool done = (vx_bool)vx_false_e;
    vx_uint32 state = IDLE;
    vx_status status;

    while((vx_bool)vx_false_e == done)
    {
        status = ownWaitGraphEvent(graph, &event, (vx_bool)vx_false_e);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_STREAM_UBR003
<justification end> */
        if((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
        {
            switch (state)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_STREAM_UBR004
<justification end> */
            {
/* LDRA_JUSTIFY_END */
                case IDLE:
                    switch (event.app_value)
                    {
                        case START:
                            VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: START\n");
                            state = RUNNING;
                            /* Execute graph then trigger another graph execution */
                            graph->streaming_executions++;
                            status = ownGraphScheduleGraphWrapper(graph);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM009
<justification end> */
                            if (status != (vx_status)VX_SUCCESS)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "ownGraphScheduleGraphWrapper() failed.\n");
                            }
/* LDRA_JUSTIFY_END */
                            break;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UTJT003
<justification end> */
                        case STOP:
                            VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: STOP\n");
                            (void)tivxEventPost(graph->stop_done);
                            break;
/* LDRA_JUSTIFY_END */

                        case DELETE:
                            /* Break from loop and exit task */
                            VX_PRINT(VX_ZONE_INFO, "state: IDLE; event: DELETE\n");
                            done = (vx_bool)vx_true_e;
                            break;

                        case STREAMING_EVENT:
                            VX_PRINT(VX_ZONE_INFO, "streaming has stopped, but not yet deleted; ignoring STREAMING_EVENT\n");
                            break;

                        default:
                            VX_PRINT(VX_ZONE_ERROR, "Streaming: idle; Unexpected app_value %llx\n", event.app_value);
                            break;
                    }
                    break;
                case RUNNING:

                    switch (event.app_value)
                    {
                        case STOP:
                            VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: STOP\n");
                            /* Change state to IDLE; if any pending RUN events then they get ignored in IDLE state */
                            state = IDLE;

                            status = vxWaitGraph(graph);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM011
<justification end> */
                            if (status != (vx_status)VX_SUCCESS)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "graph error received, exiting streaming mode!!\n");
                                graph->is_streaming_error = (vx_bool)vx_true_e;

                                /* Sending error event to trigger node queue if registered */
                                ownNodeCheckAndSendErrorEvent(graph->nodes[graph->trigger_node_index]->obj_desc[0], tivxPlatformGetTimeInUsecs()*1000U, (vx_status)VX_ERROR_TIMEOUT);
                                done = (vx_bool)vx_true_e;
                            }
/* LDRA_JUSTIFY_END */

                            (void)tivxEventPost(graph->stop_done);
                            break;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UTJT006
<justification end> */
                        case DELETE:
                            VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: DELETE\n");
                            state = IDLE;
                            done = (vx_bool)vx_true_e;
                            break;
/* LDRA_JUSTIFY_END */

                        case STREAMING_EVENT:
                            VX_PRINT(VX_ZONE_INFO, "state: RUNNING; event: NODE COMPLETE\n");
                            graph->streaming_executions++;
                            status = ownGraphScheduleGraph(graph, 1);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM013
<justification end> */
                            if (status != (vx_status)VX_SUCCESS)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "ownGraphScheduleGraph() failed.\n");
                            }
/* LDRA_JUSTIFY_END */
                            break;

                        default :
                            VX_PRINT(VX_ZONE_ERROR, "Streaming: running; Unexpected app_value %llx\n", event.app_value);
                            break;
                    }
                    break;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM014
<justification end> */
                default :
                    break;
/* LDRA_JUSTIFY_END */
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "streaming event queue timed out.\n");
            graph->is_streaming_error = (vx_bool)vx_true_e;
            (void)vxWaitGraph(graph);
            (void)tivxEventPost(graph->stop_done);

            /* Sending error event to trigger node queue if registered */
            ownNodeCheckAndSendErrorEvent(graph->nodes[graph->trigger_node_index]->obj_desc[0], tivxPlatformGetTimeInUsecs()*1000U, (vx_status)VX_ERROR_TIMEOUT);
            done = (vx_bool)vx_true_e;
        }
    }
    (void)tivxEventPost(graph->delete_done);
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

                status = ownSendUserGraphEvent(graph, START, NULL);

                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "ownSendUserGraphEvent() failed.\n");
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
            (void)tivxEventClear(graph->stop_done);

            if ((vx_status)vx_false_e == graph->is_streaming_error)
            {
                status = ownSendUserGraphEvent(graph, STOP, NULL);
            }
            if (status == (vx_status)VX_SUCCESS)
            {
                status = tivxEventWait(graph->stop_done, graph->timeout_val);

                graph->is_streaming  = (vx_bool)vx_false_e;

                if ((vx_bool)vx_true_e == graph->is_streaming_error)
                {
                    status = (vx_status)VX_ERROR_TIMEOUT;
                }
            }
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

vx_status ownSendUserGraphEvent(vx_graph graph, vx_uint32 app_value, const void *parameter)
{
    vx_status status = (vx_status)VX_SUCCESS;

    uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

    status = ownEventQueueAddEvent(
                &graph->streaming_event_queue,
                (vx_enum)VX_EVENT_USER,
                timestamp, app_value,
                (uintptr_t)app_value, (uintptr_t)parameter, (uintptr_t)0);

    return status;
}

vx_status ownWaitGraphEvent(
                    vx_graph graph, vx_event_t *event,
                    vx_bool do_not_block)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 timeout;

    if(ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_false_e)
    {
        if((vx_bool)vx_true_e == do_not_block)
        {
            timeout = 0U;
        }
        else
        {
            timeout = graph->timeout_val;
        }          
        /* Call general wait function */
        status = ownWaitEventQueue(&graph->streaming_event_queue, event, timeout);
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

            status = ownEventQueueCreate(&graph->streaming_event_queue);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_STREAM_UBR006
<justification end> */
            if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
            {
                ownEventQueueEnableEvents(&graph->streaming_event_queue, (vx_bool)vx_true_e);

                if (graph->pipeline_depth > 1U)
                {
                    /* Note: if pipelining with AUTO or MANUAL mode is enabled, re-trigger is done by pipelining and this is not needed */
                    if ( ((vx_enum)VX_GRAPH_SCHEDULE_MODE_NORMAL == graph->schedule_mode) &&
                         ((vx_bool)vx_true_e == graph->trigger_node_set) )
                    {
                        status = ownRegisterEvent(vxCastRefFromNode(graph->nodes[graph->trigger_node_index]), TIVX_EVENT_GRAPH_STREAMING_QUEUE, VX_EVENT_NODE_COMPLETED, 0, STREAMING_EVENT, (vx_bool)vx_false_e);
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                        VX_PRINT(VX_ZONE_ERROR, "trigger node is not set with pipelining\n");
                        (void)ownEventQueueDelete(&graph->streaming_event_queue);
                    }
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    streamingTaskParams.task_main = &ownStreamingPipeliningTask;
                }
                else
                {
                    status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                    VX_PRINT(VX_ZONE_ERROR, "event could not be registered\n");
                    (void)ownEventQueueDelete(&graph->streaming_event_queue);
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    status = tivxEventCreate(&graph->delete_done);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_STREAM_UBR007
<justification end>*/
                    if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                    {
                        status = tivxEventCreate(&graph->stop_done);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_GRAPH_STREAM_UBR008
<justification end>*/
                        if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                        {
                            status = tivxTaskCreate(&graph->streaming_task_handle, &streamingTaskParams);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM016
<justification end>*/
                            if ((vx_status)VX_SUCCESS != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "streaming task could not be created\n");
                                (void)tivxEventDelete(&graph->delete_done);
                                (void)tivxEventDelete(&graph->stop_done);
                            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM016
<justification end>*/
                            else
/* LDRA_JUSTIFY_END */
                            {
                                /* Everything necessary for streaming has been created */
                                graph->is_streaming_alloc = (vx_bool)vx_true_e;
                            }
                        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM017
<justification end>*/
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "event could not be created\n");
                            (void)tivxEventDelete(&graph->delete_done);
                        }
/* LDRA_JUSTIFY_END */
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM018
<justification end>*/
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "event could not be created\n");
                    }
/* LDRA_JUSTIFY_END */
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

static vx_status ownGraphStreamDeleteTask(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status tmp_status = (vx_status)VX_SUCCESS;

    if ((vx_bool)vx_false_e == graph->is_streaming_error)
    {
        if (NULL != graph->delete_done)
        {
            /* Clear event and send user event */
            tmp_status = tivxEventClear(graph->delete_done);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM019
<justification end> */
            if (tmp_status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxEventClear() failed.\n");
                status = tmp_status;
            }
/* LDRA_JUSTIFY_END */

            tmp_status = ownSendUserGraphEvent(graph, DELETE, NULL);
            if (tmp_status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "ownSendUserGraphEvent() failed.\n");
                status = tmp_status;
            }

            tmp_status = tivxEventWait(graph->delete_done, graph->timeout_val);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM021
<justification end> */
            if (tmp_status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
                status = tmp_status;
            }
/* LDRA_JUSTIFY_END */
        }
    }

    (void)tivxTaskDelete(&graph->streaming_task_handle);

    return status;
}

static vx_status ownGraphStreamDeleteQueues(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status tmp_status = (vx_status)VX_SUCCESS;

    tmp_status = ownEventQueueDelete(&graph->streaming_event_queue);
    if (tmp_status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to delete event queue.\n");
        status = tmp_status;
    }

    if (NULL != graph->stop_done)
    {
        tmp_status = tivxEventDelete(&graph->stop_done);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM022
<justification end> */
        if (tmp_status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventDelete() failed.\n");
            status = tmp_status;
        }
/* LDRA_JUSTIFY_END */
    }

    if (NULL != graph->delete_done)
    {
        tmp_status = tivxEventDelete(&graph->delete_done);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_GRAPH_STREAM_UM023
<justification end> */
        if (tmp_status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventDelete() failed.\n");
            status = tmp_status;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

vx_status ownGraphFreeStreaming(vx_graph graph)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status tmp_status = (vx_status)VX_SUCCESS;

    tmp_status = ownGraphStreamDeleteTask(graph);
    if (tmp_status != (vx_status)VX_SUCCESS)
    {
        status = tmp_status;
    }

    tmp_status = ownGraphStreamDeleteQueues(graph);
    if (tmp_status != (vx_status)VX_SUCCESS)
    {
        status = tmp_status;
    }

    return status;
}

