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



#ifndef VX_NODE_H_
#define VX_NODE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Node object
 */


/*! \brief The internal representation of a node.
 * \ingroup group_vx_node
 */
typedef struct _vx_node {
    /*! \brief The internal reference object. */
    tivx_reference_t    base;
    /*! \brief The pointer to parent graph */
    vx_graph            graph;
    /*! \brief The pointer to the kernel structure */
    vx_kernel           kernel;
    /*! \brief The list of references which are the values to pass to the kernels */
    vx_reference        parameters[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief pipeline depth of graph this node belongs to */
    uint32_t pipeline_depth;
    /*! \brief Node Object descriptor */
    tivx_obj_desc_node_t  *obj_desc[TIVX_GRAPH_MAX_PIPELINE_DEPTH];
    /*! \brief Command Object descriptor */
    tivx_obj_desc_cmd_t *obj_desc_cmd[TIVX_GRAPH_MAX_PIPELINE_DEPTH];
    /*! \brief Node performance */
    vx_perf_t perf;
    /*! \brief parameter replicated flags */
    vx_bool replicated_flags[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief reset valid rectangle */
    vx_bool valid_rect_reset;
    /*! \brief Node completeion callback */
    vx_nodecomplete_f user_callback;
    /*! \brief to check if kernel is created */
    vx_bool is_kernel_created;
    /*! \brief local data pointer */
    void *local_data_ptr;
    /*! \brief local data size */
    vx_size local_data_size;
    /*! \brief flag to indicate if local_data_ptr is allocated by system
     *  TRUE: allocated by system (TIOVX implementation)
     *  FALSE: allocated by user, or not allocated
     */
    vx_bool local_data_ptr_is_alloc;

    /*! \brief Flag to indicate if local data size and ptr set is allowed */
    vx_bool local_data_set_allow;

    /*! \brief used by graph topological sort for internal state keeping */
    uint16_t incounter;

    /*! \brief when true a event is sent when a node execution is completed */
    vx_bool is_enable_send_complete_event;

    /*! \brief number of buffer to allocate at a node parameter
     *    Valid only for output parameters and when graph containing the
     *    node is pipelined.
     *    Default is 1
     */
    uint32_t parameter_index_num_buf[TIVX_KERNEL_MAX_PARAMS];
} tivx_node_t;

/**
 * \brief Set the target for node based on immediate mode target
 *        set in context
 *
 * \param [in] node The reference to the node
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_node
 */
vx_status ownSetNodeImmTarget(vx_node node);

/**
 * \brief Set the attribute VX_NODE_VALID_RECT_RESET in node
 *
 * \param [in] node The reference to the node
 * \param [in] is_reset The value to set for the attribute
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_node
 */
vx_status ownSetNodeAttributeValidRectReset(vx_node node, vx_bool is_reset);


/*! \brief Validate user kernel or target kernel associated with this node
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelValidate(vx_node node, vx_meta_format meta[]);

/*! \brief Init user kernel or target kernel associated with this node
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelInit(vx_node node);


/*! \brief Schedule user kernel or target kernel associated with this node
 *         for execution
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelSchedule(vx_node node, uint32_t pipeline_id);

/*! \brief DeInit user kernel or target kernel associated with this node
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelDeinit(vx_node node);

/*! \brief Reset node performance statistics
 * \ingroup group_vx_node
 */
vx_status ownResetNodePerf(vx_node node);

/*! \brief Called by graph each time after node execution
 * \ingroup group_vx_node
 */
vx_status ownUpdateNodePerf(vx_node node, uint32_t pipeline_id);

/*! \brief Return number of node parameters
 * \ingroup group_vx_node
 */
uint32_t ownNodeGetNumParameters(vx_node node);

/*! \brief Get direction of a parameter
 * \ingroup group_vx_node
 */
vx_enum ownNodeGetParameterDir(vx_node node, uint32_t prm_index);

/*! \brief Get reference associated with a parameters of a parameter
 * \ingroup group_vx_node
 */
vx_reference ownNodeGetParameterRef(vx_node node, uint32_t prm_index);


/*! \brief Associate a output node with a given node
 * \ingroup group_vx_node
 */
vx_status ownNodeAddOutNode(vx_node node, vx_node out_node);

/*! \brief Associate a input node with a given node
 * \ingroup group_vx_node
 */
vx_status ownNodeAddInNode(vx_node node, vx_node out_node);

/*! \brief Get number of input nodes associated with this node
 * \ingroup group_vx_node
 */
uint32_t ownNodeGetNumInNodes(vx_node node);

/*! \brief Get number of output nodes associated with this node
 * \ingroup group_vx_node
 */
uint32_t ownNodeGetNumOutNodes(vx_node node);

/*! \brief Create resources required to call a user callback
 *         If user callback is assigned then nothing is done
 *
 * \ingroup group_vx_node
 */
vx_status ownNodeCreateUserCallbackCommand(vx_node node, uint32_t pipeline_id);

/*! \brief Call user specified callback
 *
 * \ingroup group_vx_node
 */
vx_action ownNodeExecuteUserCallback(vx_node node);

/*! \brief clears execute status of the node
 *
 * \ingroup group_vx_node
 */
void ownNodeClearExecuteState(vx_node node, uint32_t pipeline_id);

/*! \brief Execute user kernel
 *
 * \ingroup group_vx_node
 */
vx_status ownNodeUserKernelExecute(vx_node node, vx_reference prm_ref[]);


/*! \brief Is prm_idx of node replicated
 *
 * \ingroup group_vx_node
 */
vx_bool ownNodeIsPrmReplicated(vx_node node, uint32_t prm_idx);

/*! \brief Set parameter at node
 *
 * \ingroup group_vx_node
 */
void ownNodeSetParameter(vx_node node, vx_uint32 index, vx_reference value);

/*! \brief Get next node at given output index
 *
 * \ingroup group_vx_node
 */
vx_node ownNodeGetNextNode(vx_node node, vx_uint32 index);


/*! \brief Check if node is completed
 *
 *         Does not block for node completion.
 *
 * \ingroup group_vx_node
 */
vx_bool ownCheckNodeCompleted(vx_node node, uint32_t pipeline_id);


/*! \brief Register node completion event
 *
 * \ingroup group_vx_node
 */
vx_status ownNodeRegisterEvent(vx_node node, vx_enum event_type);


/*! \brief Send node completion event if enabled
 *
 * \ingroup group_vx_node
 */
void ownNodeCheckAndSendCompletionEvent(tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp);


/*! \brief Alloc additional node obj desc's for pipelining
 *
 * \ingroup group_vx_node
 */
vx_status ownNodeAllocObjDescForPipeline(vx_node node, uint32_t pipeline_depth);

/*! \brief Set param direction for 0th node obj desc
 *
 * \ingroup group_vx_node
 */
void ownNodeSetObjDescParamDirection(vx_node node);

/*! \brief Update in node id and out node id for node pipeline obj desc's
 *
 * \ingroup group_vx_node
 */
void ownNodeLinkObjDescForPipeline(vx_node node);


/*! \brief Link data ref q to node at parameter index 'prm_id'
 *
 * \ingroup group_vx_node
 */
void ownNodeLinkDataRefQueue(vx_node node, uint32_t prm_id, tivx_data_ref_queue data_ref_q);


/*! \brief Return number of buffers to alocate at a node,index
 *
 *  Valid only for output parameters
 *
 *  Returns 0 is user has not made any specific choice or invalid parameter
 *
 * \ingroup group_vx_node
 */
uint32_t ownNodeGetParameterNumBuf(vx_node node, vx_uint32 index);

#ifdef __cplusplus
}
#endif

#endif
