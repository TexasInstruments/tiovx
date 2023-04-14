/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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

#ifndef J7_TVM_H_
#define J7_TVM_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include <TI/tivx_tensor.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIVX_TVM_J7_CHECKSUM_SIZE (64)

/* These correspond to parameter indices of the TVM node
 * They can be useful to aid in setting replicate parameters if
 * vxReplicateNode is used.
 * 0: config: meta data for the kernel call (checksum, num inputs/output, etc)
 * 1: deploy_mod:  tvm c7x deployable module built as shared lib
 * 2: optional trace output
 * 3 - ?: in_tensors, out_tensors
*/
#define TIVX_KERNEL_TVM_IN_CONFIG_IDX                   (0U)
#define TIVX_KERNEL_TVM_IN_DEPLOY_MOD_IDX               (1U)
#define TIVX_KERNEL_TVM_IN_TRACE_DATA_IDX               (2U)
#define TIVX_KERNEL_TVM_IN_FIRST_TENSOR                 (3U)
#define TIVX_KERNEL_TVM_NUM_BASE_PARAMETERS             (3U)
#define TIVX_KERNEL_TVM_NUM_MIN_PARAMETERS             (TIVX_KERNEL_TVM_NUM_BASE_PARAMETERS + 2)

#define TIVX_KERNEL_TVM_MAX_INPUT_TENSORS 16
#define TIVX_KERNEL_TVM_MAX_TOTAL_INPUT_TENSOR_NAMES_SIZE 512
#define TIVX_KERNEL_TVM_MAX_TOTAL_TENSORS 32
#define TIVX_KERNEL_TVM_MAX_TENSOR_DIM    8

/*! \brief Parameters describing a TVMRT Tensor */
typedef struct{
  uint32_t size_in_bytes;
} tivxTVMTensorParams;

/*! \brief Runtime info needed by TVM C runtime and TIDL */
typedef struct{
  /** Flag to indicate TVM RT debug level*/
  vx_int32 tvm_rt_debug_level;
  /** Flag to indicate TIDL debug trace log level*/
  vx_int32 tidl_trace_log_level;
  /** Flag to indicate TIDL debug trace write level*/
  vx_int32 tidl_trace_write_level;
  /** Maximum Tolerated delay for TIDL pre-emption in milliSecond */
  vx_float32 max_preempt_delay;
  /** tvm trace target pointer (set by OpenVX from TRACE user data object) */
  vx_uint64 tvm_rt_trace_ptr;
  /** tvm trace size (set by OpenVX from TRACE user data object) */
  vx_int32  tvm_rt_trace_size;
  /** Dump all output tensors for a specific tvm trace node */
  vx_int32  tvm_rt_trace_node;
} tvm_tidl_rt_info;

/*!
 * \brief TVM params structure
 * \ingroup group_vision_function_tvm
 */
typedef struct{
  /** Checksum placeholder for TVM config params */
  vx_uint8 config_checksum[TIVX_TVM_J7_CHECKSUM_SIZE];

  /** Checksum placeholder for TVM deploy_mod params */
  vx_uint8 deploy_mod_checksum[TIVX_TVM_J7_CHECKSUM_SIZE];

  /** Flag to indicate if config params checksum is to be computed or not */
  vx_uint32 compute_config_checksum;

  /** Flag to indicate if network params checksum is to be computed or not */
  vx_uint32 compute_network_checksum;

  /** TVM input/output tensors */
  vx_uint32 num_input_tensors;
  vx_uint32 num_output_tensors;

  /** TVM input tensors names are packed 0-terminated strings, offset array
   *     points to the start of each name */
  vx_uint32 input_names_offset[TIVX_KERNEL_TVM_MAX_INPUT_TENSORS];
  vx_uint8  input_names[TIVX_KERNEL_TVM_MAX_TOTAL_INPUT_TENSOR_NAMES_SIZE];

  /** TVM tensors info for recreating DLTensors to interface with TVM runtime */
  tivxTVMTensorParams tensors_params[TIVX_KERNEL_TVM_MAX_TOTAL_TENSORS];

  /** Flag to enable optimization ivision alg activate, default is disabled */
  vx_uint32 optimize_ivision_activation;

  /** Runtime info needed by TVM C runtime and TIDL */
  tvm_tidl_rt_info rt_info;

  /* Begin workaround, to be removed once update TIDL is in SDK build */
  /** Flag to indicate TVM RT debug level*/
  vx_int32 tvm_rt_debug_level;
  /** Flag to indicate TIDL debug trace log level*/
  vx_int32 tidl_trace_log_level;
  /** Flag to indicate TIDL debug trace write level*/
  vx_int32 tidl_trace_write_level;
  /* End workaround, to be removed once update TIDL is in SDK build */

} tivxTVMJ7Params;

/*! \brief [Graph] Creates a TVM Node.
 * \param [in] graph Reference to vx_graph.
 * \param [in] kernel Reference to vx_kernel.
 * \param [in,out] appParams is an array of 2 parameters:
 *             - config vx_user_data_object type corresponding to the configuration (named string: tivxTVMJ7Params)
 *             - deploy_mod vx_user_data_object type corresponding to the TVM deployable module (named string: tivxTVMJ7DeployMod)
 * \param [in] input_tensors Array of input tensors
 *             This parameter is ignored when the first layer of the network is a data layer, which is most of the time.
 *             Only networks that are dependent on the output of a previous networks have first layer that are not data layer.
 * \param [out] output_tensors Array of output tensors
 *
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 * \ingroup group_vision_function_tvm
 */
VX_API_ENTRY vx_node VX_API_CALL tivxTVMNode(vx_graph  graph,
                                             vx_kernel kernel,
                                             vx_reference appParams[],
                                             vx_tensor input_tensors[],
                                             vx_tensor output_tensors[]);

/*!
 * \brief Used for the Application to create the tvm kernel from the context.
 * \ingroup group_vision_function_tvm
 *
 * \param [in]  context             OpenVX context which must be created using \ref vxCreateContext
 * \param [in]  num_input_tensors   Number of input vx_tensor objects to be created
 * \param [in]  num_output_tensors  Number of output vx_tensor objects to be created
 *
 * \returns Handle to vx_kernel object if successful, NULL otherwise
 *
 * \note The caller of this function should check status of the return vx_kernel handle
 *       and if found to be NULL must handle or propagate error appropriately.
 *
 */

vx_kernel tivxAddKernelTVM(vx_context context,
                           uint32_t num_input_tensors,
                           uint32_t num_output_tensors);

#ifdef __cplusplus
}
#endif

#endif /* J7_TVM_H_ */
