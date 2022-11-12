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



#ifndef VX_KERNEL_H_
#define VX_KERNEL_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Kernel object
 */


/*! \brief The internal representation of the attributes associated with a run-time parameter.
 * \ingroup group_vx_kernel
 */
typedef struct _tivx_signature_t {
    /*! \brief The array of directions */
    vx_enum        directions[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief The array of types */
    vx_enum        types[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief The array of states */
    vx_enum        states[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief The number of items in both \ref tivx_signature_t::directions and \ref tivx_signature_t::types. */
    vx_uint32      num_parameters;
} tivx_signature_t;


/*!
 * \brief Kernel object internal state
 *
 * \ingroup group_vx_kernel
 */
typedef struct _vx_kernel
{
    /*! \brief reference object */
    tivx_reference_t base;

    /*! \brief name of kernel */
    vx_char        name[VX_MAX_KERNEL_NAME];
    /*! \brief enum associated with this kernel */
    vx_enum        enumeration;
    /*! \brief parameter signature of this kernel */
    tivx_signature_t signature;
    /*! \brief The parameters validator */
    vx_kernel_validate_f    validate;
    /*! \brief The initialization function */
    vx_kernel_initialize_f initialize;
    /*! \brief The deinitialization function */
    vx_kernel_deinitialize_f deinitialize;
    /*! \brief The pointer to the function to execute the kernel */
    vx_kernel_f             function;
    /*! \brief number of supported targets */
    vx_uint32               num_targets;
    /*! \brief target names, index 0 is the default or preferred target for this kernel */
    char                    target_name[TIVX_MAX_TARGETS_PER_KERNEL][TIVX_TARGET_MAX_NAME];
    /*! \brief Local data size for user kernels */
    vx_size                 local_data_size;
    /*! \brief Flag to check if this is a user kernel or target kernel */
    vx_bool                 is_target_kernel;

    /*! \brief when this flag is true, kernel cannot be removed via
     *         remove kernel API
     */
    vx_bool                 lock_kernel_remove;
    /*! \brief number of pipeup buffers */
    vx_uint32               num_pipeup_bufs;
    /*! \brief pipeup buf index, used for querying during enqueue */
    vx_uint32               pipeup_buf_idx;
    /*! \brief number of buffers needed for sink node */
    vx_uint32               num_sink_bufs;
    /*! \brief number of buffers needed for allocation at source node when sink node directly connected */
    vx_uint32               connected_sink_bufs;
    /*! \brief capture state (VX_NODE_STATE_PIPEUP or VX_NODE_STATE_STEADY)*/
    vx_enum                 state;
    /*! \brief Control API processing Timeout value in milli-sec. */
    vx_uint32               timeout_val;

} tivx_kernel_t;


/*!
 * \brief Get default target ID associated with this kernel
 *
 * \return TIVX_TARGET_ID_INVALID if no target or invalid target associated with this kernel
 *
 * \ingroup group_vx_kernel
 */
vx_enum ownKernelGetDefaultTarget(vx_kernel kernel);

/*!
 * \brief Match user provided target with supported targets
 *        and return target ID on which to run this kernel
 *
 * \return TIVX_TARGET_ID_INVALID if valid target match not found
 *
 * \ingroup group_vx_kernel
 */
vx_enum ownKernelGetTarget(vx_kernel kernel, const char *target_string);


#ifdef __cplusplus
}
#endif

#endif
