/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#ifndef J7_TIDL_H_
#define J7_TIDL_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include "itidl_ti.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIVX_TIDL_J7_CHECKSUM_SIZE (64)

/* These correspond to parameter indices of the TIDL node
 * They can be useful to aid in setting replicate parameters if
 * vxReplicateNode is used. */
#define TIVX_KERNEL_TIDL_IN_CONFIG_IDX                  (0U)
#define TIVX_KERNEL_TIDL_IN_NETWORK_IDX                 (1U)
#define TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX           (2U)
#define TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX                 (3U)
#define TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX                (4U)
#define TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX              (5U)
#define TIVX_KERNEL_TIDL_IN_FIRST_TENSOR                (6U)
#define TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS            (6U)
#define TIVX_KERNEL_TIDL_NUM_MIN_PARAMETERS             (TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + 2)

/*!
 * \brief TIDL params structure
 * \ingroup group_vision_function_tidl
 */
typedef struct{
  /** Checksum placeholder for TIDL config params */
  vx_uint8 config_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /** Checksum placeholder for TIDL network params */
  vx_uint8 network_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /** Flag to indicate if config params checksum is to be computed or not */
  vx_uint32 compute_config_checksum;

  /** Flag to indicate if network params checksum is to be computed or not */
  vx_uint32 compute_network_checksum;

  /**TIDL input/output buffer descriptor*/
  sTIDL_IOBufDesc_t ioBufDesc;

  /** Flag to enable optimization ivision alg activate, default it is disabled */
  vx_uint32 optimize_ivision_activation;

}tivxTIDLJ7Params;

/*!
 * \brief TIDL params initialization
 * \ingroup group_vision_function_tidl
 */
static inline void tivx_tidl_j7_params_init(tivxTIDLJ7Params *tidlParams)
{
  memset(&tidlParams->config_checksum[0], 0, TIVX_TIDL_J7_CHECKSUM_SIZE);
  memset(&tidlParams->network_checksum[0], 0, TIVX_TIDL_J7_CHECKSUM_SIZE);

  tidlParams->compute_config_checksum = 0;
  tidlParams->compute_network_checksum = 0;
  tidlParams->optimize_ivision_activation = 0;

  memset(&tidlParams->ioBufDesc, 0, sizeof(sTIDL_IOBufDesc_t));
}

#ifdef __cplusplus
}
#endif

#endif /* J7_TIDL_H_ */
