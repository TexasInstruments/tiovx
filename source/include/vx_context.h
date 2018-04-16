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
    /*! \brief The next available dynamic user kernel ID */
    vx_uint32           next_dynamic_user_kernel_id;
    /*! \brief The next available dynamic user library ID */
    vx_uint32           next_dynamic_user_library_id;
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
    tivx_obj_desc_cmd_t *obj_desc_cmd;
    /*! Event used to received a command ACK */
    tivx_event cmd_ack_event;

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
 *
 * \ingroup group_vx_context
 */
vx_status ownContextSendCmd(vx_context context, uint32_t target_id, uint32_t cmd, uint32_t num_obj_desc, const uint16_t *obj_desc_id);

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


#ifdef __cplusplus
}
#endif

#endif
