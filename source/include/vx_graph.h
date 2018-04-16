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



#ifndef VX_GRAPH_H_
#define VX_GRAPH_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Graph object
 */

/*! \brief The top level data for graph object
 * \ingroup group_vx_graph
 */
typedef struct _vx_graph {

    /*! \brief The base reference object */
    tivx_reference_t      base;

    /*! \brief Flag to maintain state of graph verification */
    vx_bool verified;

    /*! \brief Flag to maintain state of graph re-verification */
    vx_bool reverify;

    /*! \brief Nodes in a graph */
    vx_node nodes[TIVX_GRAPH_MAX_NODES];

    /* \brief Number of nodes in a graph */
    uint32_t num_nodes;

    /*! \brief Head Nodes in a graph */
    vx_node head_nodes[TIVX_GRAPH_MAX_HEAD_NODES];

    /* \brief Number of head nodes in a graph */
    uint32_t num_head_nodes;

    /*! \brief Leaf Nodes in a graph */
    vx_node leaf_nodes[TIVX_GRAPH_MAX_HEAD_NODES];

    /* \brief Number of leaf nodes in a graph */
    uint32_t num_leaf_nodes;

    /*! \brief The list of graph parameters. */
    struct {
        /*! \brief The reference to the node which has the parameter */
        vx_node node;
        /*! \brief The index to the parameter on the node. */
        uint32_t  index;
        /*! \brief vx_true_e, enqueue operation is supported on this parameter */
        vx_bool queue_enable;
        /*! \brief when queue_enable = vx_true_e, this hold the max buffers that can be enqueued */
        uint32_t num_buf;
        /*! \brief data ref queue handle when queueing is enabled at this graph parameter */
        tivx_data_ref_queue data_ref_queue;
        /*! \brief references that can be queued into data ref queue */
        vx_reference refs_list[TIVX_OBJ_DESC_QUEUE_MAX_DEPTH];
        /*! \brief flag to control event send enable/disable */
        vx_bool is_enable_send_ref_consumed_event;
    } parameters[TIVX_GRAPH_MAX_PARAMS];
    /*! \brief The number of graph parameters. */
    uint32_t      num_params;

    /*! \brief The list of data refs other than graph parameters. */
    struct {
        /*! \brief The reference to the node which has the data ref queue */
        vx_node node;
        /*! \brief The index to the parameter on the node. */
        uint32_t  index;
        /*! \brief this hold the max buffers that can be enqueued */
        uint32_t num_buf;
        /*! \brief data ref queue handle */
        tivx_data_ref_queue data_ref_queue;
        /*! \brief references that can be queued into data ref queue */
        vx_reference refs_list[TIVX_OBJ_DESC_QUEUE_MAX_DEPTH];
    } data_ref_q_list[TIVX_GRAPH_MAX_DATA_REF_QUEUE];
    /*! \brief The number of graph parameters. */
    uint32_t      num_data_ref_q;

    /*! \brief The list of data refs other than graph parameters. */
    struct {
        /*! \brief The reference to the node which has the data ref queue */
        vx_node node;
        /*! \brief The index to the parameter on the node. */
        uint32_t  index;
        /*! \brief data ref queue handle */
        tivx_data_ref_queue data_ref_queue;
        /*! \brief delay associated with this data ref queue */
        vx_delay delay_ref;
        /*! \brief delay slot index associated with this data ref queue */
        uint32_t delay_slot_index;
    } delay_data_ref_q_list[TIVX_GRAPH_MAX_DATA_REF_QUEUE];
    /*! \brief The number of graph parameters. */
    uint32_t      num_delay_data_ref_q;

    /*! \brief The state of the graph (vx_graph_state_e) */
    vx_enum        state;

    /*! \brief The performance logging variable. */
    vx_perf_t      perf;

    /*! \brief The array of all delays in this graph */
    vx_delay       delays[TIVX_GRAPH_MAX_DELAYS];

    /*! \brief valid rect information for input image's */
    vx_rectangle_t in_valid_rect[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief valid rect information for input image's */
    vx_rectangle_t *in_valid_rect_ptr[TIVX_KERNEL_MAX_PARAMS];


    /*! \brief valid rect information for output image's */
    vx_rectangle_t out_valid_rect[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    /*! \brief valid rect information for output image's */
    vx_rectangle_t *out_valid_rect_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    /*! \brief graph pipeline depth */
    uint32_t pipeline_depth;

    /*! \brief graph object descriptors */
    tivx_obj_desc_graph_t *obj_desc[TIVX_GRAPH_MAX_PIPELINE_DEPTH];

    /*! \brief free graph object descriptors that are ready for scheduling */
    tivx_queue free_q;

    /*! \brief memory used by free Q */
    uint32_t free_q_mem[TIVX_GRAPH_MAX_PIPELINE_DEPTH];

    /*! \brief graph schedule mode as defined by vx_graph_schedule_mode_type_e */
    vx_enum schedule_mode;

    /*! \brief number of graph schedule's that are requested but not submitted i.e pending */
    uint32_t schedule_pending_count;

    /*! \brief when true a event is sent when a graph execution is completed */
    vx_bool is_enable_send_complete_event;

    /*! \brief event to indicate all schedule graphs have finished execution
     *         and none are pending
     */
    tivx_event  all_graph_completed_event;

    /*! \brief counts the number of graphs schedule or submitted but not yet completed */
    uint32_t submitted_count;

    /*! \brief number of data references in the graph */
    uint32_t num_data_ref;

    /*! \brief list of data references included in this graph */
    vx_reference data_ref[TIVX_GRAPH_MAX_DATA_REF];

    /*! \brief number nodes that take this data reference as input */
    uint8_t data_ref_num_in_nodes[TIVX_GRAPH_MAX_DATA_REF];

    /*! \brief number nodes that take this data reference as output */
    uint8_t data_ref_num_out_nodes[TIVX_GRAPH_MAX_DATA_REF];



} tivx_graph_t;



/*! \brief Get next free node entry in graph
 *
 * \param graph [in] graph object
 *
 * \return 0 or more value, index of node in graph
 * \return -1, all nodes in graph are used and free node not found
 *
 * \ingroup group_vx_graph
 */
int32_t ownGraphGetFreeNodeIndex(vx_graph graph);

/*! \brief Add's a node to a graph
 *
 *         'index' should be the one that is returned via
 *         ownGraphGetFreeNodeIndex()
 *
 * \param graph [in] graph object
 * \param node  [in] the node to add
 * \param index [in] the index in graph at which to add the node
 *
 * \return VX_SUCCESS, on sucess
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphAddNode(vx_graph graph, vx_node node, int32_t index);

/*! \brief Remove a node from a graph
 *
 * \param graph [in] graph object
 * \param node  [in] the node to remove
 *
 * \return VX_SUCCESS, on sucess
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphRemoveNode(vx_graph graph, vx_node node);


/*! \brief Perform topological sort of graph nodes
 *
 * \param context   [in] context to use while sorting
 * \param nodes     [in/out] IN: Unosrted node, OUT: Sorted nodes
 * \param num_nodes [in] Number of nodes
 * \param has_cycle [out] vx_true_e: Graph has cycles and cannot be sorted
 *                         vx_false_e: Graph is acyclic and nodes[] contains the sorted nodes
 *
 * \ingroup group_vx_graph
 */
void ownGraphTopologicalSort(tivx_graph_sort_context *context, vx_node *nodes, uint32_t num_nodes, vx_bool *has_cycle);


/*! \brief Mark graph to be reverified
 * \ingroup group_vx_graph
 */
void ownGraphSetReverify(vx_graph graph);


/*! \brief If a data ref queue is associated with 'graph_parameter_index' return it
 *
 * If a data ref queue is NOT associated with 'graph_parameter_index' NULL is returned
 *
 * \ingroup group_vx_graph
 */
tivx_data_ref_queue ownGraphGetParameterDataRefQueue(vx_graph graph, vx_uint32 graph_parameter_index);


/*! \brief Alloc and enqueue graph obj desc based on graph pipeline depth
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphAllocAndEnqueueObjDescForPipeline(vx_graph graph);

/*! \brief Free graph obj desc based allocated during
 *         ownGraphAllocAndEnqueueObjDescForPipeline()
 *
 * \ingroup group_vx_graph
 */
void ownGraphFreeObjDesc(vx_graph graph);

/*! \brief Create queues to maintain submitted graph desc and free graph desc
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphCreateQueues(vx_graph graph);

/*! \brief Delete queues created during ownGraphCreateQueues
 *
 * \ingroup group_vx_graph
 */
void ownGraphDeleteQueues(vx_graph graph);


/*! \brief Scehdule a graph for execution 'num_schedule' times
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphScheduleGraph(vx_graph graph, uint32_t num_schedule);

/*! \brief Check if a previoulsy scehduled graph execution is complete
 *
 * Called every time a leaf node completes execution.
 * When all leaf nodes complete execution a graph is said to be completed
 *
 * \ingroup group_vx_graph
 */
vx_bool ownCheckGraphCompleted(vx_graph graph, uint32_t pipeline_id);


/*! \brief Check if a graph should be scheduled after a graph parameter
 *         has been enqueued
 *
 *         This returns true only when graph execution mode is
 *         VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO and graph_parameter_index
 *         is the graph parameter which acts as a trigger for auto
 *         scheduling.
 *
 * \ingroup group_vx_graph
 */
vx_bool ownGraphDoScheduleGraphAfterEnqueue(vx_graph graph, uint32_t graph_parameter_index);

/*! \brief Update graph performance
 *
 * \ingroup group_vx_graph
 */
vx_status ownUpdateGraphPerf(vx_graph graph, uint32_t pipeline_id);


/*! \brief Graph execution state for given pipeline ID
 *
 * \ingroup group_vx_graph
 */
void ownGraphClearState(vx_graph graph, uint32_t pipeline_id);

/*! \brief Set graph state for a given pipeline ID
 *
 * \ingroup group_vx_graph
 */
void ownSetGraphState(vx_graph graph, uint32_t pipeline_id, vx_enum state);

/*! \brief Register event on graph completion
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphRegisterCompletionEvent(vx_graph graph);

/*! \brief Register event on graph parameter consumed
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphRegisterParameterConsumedEvent(vx_graph graph, uint32_t graph_parameter_index);


/*! \brief Send graph completion event if enabled
 *
 * \ingroup group_vx_graph
 */
void ownSendGraphCompletedEvent(vx_graph graph);

/*!
 * \brief Checks if 'ref' is valid ref that can be enqueued
 *
 *  'ref' is compared against pre-registered ref's that can be enqueued
 *  to confirm that ref can be enqueued.
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphParameterCheckValidEnqueueRef(vx_graph graph, uint32_t graph_parameter_index, vx_reference ref);

/*!
 * \brief Counts number of enqueued 'refs' and returns number of times graph can be scheduled sucessfully
 *
 *  Value returned is minimum of number of refs enqueued at each graph parameter
 *  Valid only in VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL mode
 *
 * \ingroup group_vx_graph
 */
uint32_t ownGraphGetNumSchedule(vx_graph graph);

#ifdef __cplusplus
}
#endif

#endif
