/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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



#ifndef VX_CONTEXT_H_
#define VX_CONTEXT_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Context object
 */

/*! \brief The largest convolution matrix the specification requires support for is 9x9.
 * \ingroup group_vx_context_cfg
 */
#define TIVX_CONTEXT_MAX_CONVOLUTION_DIM (9)

/*! \brief The largest optical flow pyr LK window.
 * \ingroup group_vx_context_cfg
 */
#define TIVX_CONTEXT_MAX_OPTICALFLOWPYRLK_DIM (9)

/*! \brief The largest nonlinear filter matrix the specification requires support for is 9x9.
* \ingroup group_vx_context_cfg
*/
#define TIVX_CONTEXT_MAX_NONLINEAR_DIM (9)

/*! \brief Context used while sorting a graph
 * \ingroup group_vx_context
 */
typedef struct _tivx_graph_sort_context {

    /*! \brief Sorted node list */
    vx_node   sorted_nodes[TIVX_GRAPH_MAX_NODES];
    /*! stack used while sorting */
    vx_node   stack[TIVX_GRAPH_MAX_NODES];
    /*! stack top */
    uint16_t  stack_top;
    /*! stack max size */
    uint16_t  stack_max_elems;

} tivx_graph_sort_context;

/*! \brief The top level context data for the entire OpenVX instance
 * \ingroup group_vx_context
 */
typedef struct _vx_context {

    /*! \brief The base reference object */
    tivx_reference_t      base;

    /*! \brief References associated with a context */
    vx_reference        reftable[TIVX_CONTEXT_MAX_REFERENCES];
    /*! \brief The number of references in the table. */
    vx_uint32           num_references;
    /*! \brief The combined number of unique kernels in the system */
    vx_uint32           num_unique_kernels;
    /*! \brief Callback to call for logging messages from framework */
    vx_log_callback_f   log_callback;
    /*! \brief The log enable toggle. */
    vx_bool             log_enabled;
    /*! \brief Lock to use for locking log print's */
    tivx_mutex          log_lock;
    /*! \brief Lock to use for locking context */
    tivx_mutex          lock;
    /*! \brief The performance counter enable toggle. */
    vx_bool             perf_enabled;
    /*! \brief The immediate mode border */
    vx_border_t         imm_border;
    /*! \brief The unsupported border mode policy for immediate mode functions */
    vx_enum             imm_border_policy;
    /*! \brief Flag indicating whether corresponding dynamic_user_kernel_id is being used */
    vx_bool             is_dynamic_user_kernel_id_used[TIVX_MAX_KERNEL_ID];
    /*! \brief Number of dynamic_user_kernel_id's being used */
    vx_uint32           num_dynamic_user_kernel_id;
    /*! \brief Flag indicating whether corresponding dynamic_user_library_id is being used */
    vx_bool             is_dynamic_user_library_id_used[TIVX_MAX_LIBRARY_ID];
    /*! \brief The immediate mode enumeration */
    vx_enum             imm_target_enum;
    /*! \brief The immediate mode target string */
    vx_char             imm_target_string[TIVX_TARGET_MAX_NAME];
    /*! \brief The list of user defined structs. */
    struct {
        /*! \brief Type constant */
        vx_enum type;
        /*! \brief Size in bytes */
        vx_size size;
    } user_structs[TIVX_CONTEXT_MAX_USER_STRUCTS];
    /*! Information about all kernels suported in this context */
    vx_kernel kerneltable[TIVX_CONTEXT_MAX_KERNELS];

    /*! Command object that is used to send control messages to different target */
    tivx_obj_desc_cmd_t *obj_desc_cmd[TIVX_MAX_CTRL_CMD_OBJECTS];

    /*! Event used to received a command ACK */
    tivx_event cmd_ack_event[TIVX_MAX_CTRL_CMD_OBJECTS];

    /*! \brief handle to free queue holding tivx_event_queue_elem_t's
     * NOTE: queue holds index's to event_list[]
     * */
    tivx_queue free_queue;

    /*! \brief free queue memory */
    uintptr_t free_queue_memory[TIVX_MAX_CTRL_CMD_OBJECTS];

    /*! \brief handle to ready queue holding tivx_event_queue_elem_t's which are ready to be delivered to users
     * NOTE: queue holds index's to event_list[]
     * */
    tivx_queue pend_queue;

    /*! \brief free queue memory */
    uintptr_t pend_queue_memory[TIVX_MAX_CTRL_CMD_OBJECTS];


    /*! Flag to disallow kernel removal,
     *  used for built in kernels added during context create
     */
    vx_bool remove_kernel_lock;

    /*! Context used while sorting a graph */
    tivx_graph_sort_context graph_sort_context;

    /*! Event queue */
    tivx_event_queue_t event_queue;
} tivx_context_t;

/**
 * \brief Check if 'context' is valid
 *
 * \param [in] context The reference to context
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_context
 */
vx_bool ownIsValidContext(vx_context context);

/**
 * \brief Flushes the command pend queue, if not empty.
 *
 * \param [in] context The reference to context
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_context
 */
vx_status ownContextFlushCmdPendQueue(vx_context context);

/*! \brief Add reference to a context
 * \param [in] context The overall context.
 * \param [in] ref The reference to add.
 * \return vx_true_e on success
 * \return vx_false_e on failure
 * \ingroup group_vx_context
 */
vx_bool ownAddReferenceToContext(vx_context context, vx_reference ref);


/*! \brief Remove reference from a context
 * \param [in] context The overall context.
 * \param [in] ref The reference to remove.
 * \return vx_true_e on success
 * \return vx_false_e on failure
 * \ingroup group_vx_context
 */
vx_bool ownRemoveReferenceFromContext(vx_context context, vx_reference ref);

/*!
 * \brief Add's unique kernel to context
 *
 *        Increment internal reference count of the kernel
 * \ingroup group_vx_context
 */
vx_status ownAddKernelToContext(vx_context context, vx_kernel kernel);

/*!
 * \brief Remove unique kernel from context
 *
 *        Decrement internal reference count of the kernel
 * \ingroup group_vx_context
 */
vx_status ownRemoveKernelFromContext(vx_context context, vx_kernel kernel);

/*!
 * \brief Check if kernel exists inside the context
 *
 * \ingroup group_vx_context
 */
vx_status ownIsKernelInContext(vx_context context, vx_enum enumeration, const vx_char string[VX_MAX_KERNEL_NAME], vx_bool *is_found);


/*!
 * \brief Send a command to specified target with object descriptor ID's as parameters
 *
 *        This API waits until ACK for the command is received
 *
 * \param context      [in] context to use when sending the command
 * \param target_id    [in] ID of Target to whom the command is being sent
 * \param cmd          [in] command to send
 * \param num_obj_desc [in] number of object descriptors to send
 * \param obj_desc_id  [in] List of object descriptor to send
 * \param timeout      [in] Timeout in units of msecs, use TIVX_EVENT_TIMEOUT_WAIT_FOREVER to wait forever
 *
 * \ingroup group_vx_context
 */
vx_status ownContextSendCmd(vx_context context, uint32_t target_id, uint32_t cmd, uint32_t num_obj_desc, const uint16_t *obj_desc_id, uint32_t timeout);

/*!
 * \brief Send a control command to specified target with object descriptor ID's as parameters
 *
 *        This API waits for 'timeout' msecs, until ACK for the command is received
 *
 * \param context             [in] context to use when sending the command
 * \param node_obj_desc       [in] ID of node object descriptor to which command is being sent
 * \param target_id           [in] ID of Target to whom the command is being sent
 * \param replicated_node_idx [in] If the node is replicated, index of the replicant to send the command to
 * \param node_cmd_id         [in] ID of the command to send to the node
 * \param obj_desc_id[]       [in] Array of IDs of object descriptor
 * \param num_obj_desc        [in] Number of object descriptors in obj_desc_id[] array
 * \param timeout             [in] Timeout in units of msecs, use TIVX_EVENT_TIMEOUT_WAIT_FOREVER to wait forever
 *
 * \ingroup group_vx_context
 */
vx_status ownContextSendControlCmd(vx_context context, uint16_t node_obj_desc,
    uint32_t target_id, uint32_t replicated_node_idx, uint32_t node_cmd_id,
    const uint16_t obj_desc_id[], uint32_t num_obj_desc, uint32_t timeout);

/*!
 * \brief Get value of kernel remove lock flag
 *
 * \param context      [in] context
 *
 * \ingroup group_vx_context
 */
vx_bool ownContextGetKernelRemoveLock(vx_context context);

/*!
 * \brief Set value of kernel remove lock flag
 *
 * \param context      [in] context
 * \param do_lock      [in] kernel remove lock value to set
 *
 * \ingroup group_vx_context
 */
void ownContextSetKernelRemoveLock(vx_context context, vx_bool do_lock);

/*!
 * \brief Lock context
 *
 * \param context      [in] context
 *
 * \ingroup group_vx_context
 */
vx_status ownContextLock(vx_context context);

/*!
 * \brief Lock context
 *
 * \param context      [in] context
 *
 * \ingroup group_vx_context
 */
vx_status ownContextUnlock(vx_context context);

/*! \brief Returns the vx_context object currently being used.
 * \details This API returns the context being used.  If the context has not
 *          yet been created using \ref vxCreateContext, then it returns NULL.
 *          This API does not include reference counting like in the
 *          \ref vxCreateContext.
 *
 * \return The <tt>\ref vx_context</tt> object being used.
 *
 *  * \ingroup group_vx_context
 */
vx_context ownGetContext(void);

#ifdef __cplusplus
}
#endif

#endif
