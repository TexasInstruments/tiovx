/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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


#ifndef _OPENVX_COPY_SWAP_H_
#define _OPENVX_COPY_SWAP_H_

#ifdef  __cplusplus
extern "C" {
#endif

/* Header for the Copy, Swap and Move node utility functions */

/*! \brief Graph Verification: Eliminate Copy & Pass nodes, preserving execution order
 * \param [in] graph - the graph to process. In and out nodes must already have been calculated
 * \returns VX_SUCCESS if all OK
 */
vx_status ownGraphProcessCopyMoveNodes(vx_graph graph);

/*! \brief kernel callback for user data objects. (Placed here for licence reasons)
 * \param [in] kernel_enum - The kernel that is requested (VX_KERNEL_COPY etc)
 * \param [in] validate_only - vx_true_e if the operation is to validate the parameters
 * \param [in] optimization - non-zero if some form of optimisation is possible or requested
 * \param [in] params - the list of parameters provided for the kernel
 * \param [in] num_params - the number of parameters provided in the list
*/
vx_status VX_CALLBACK userDataKernelCallback(vx_enum kernel_enum, vx_bool validate_only, vx_enum optimization, const vx_reference params[], vx_uint32 num_params);

#ifdef  __cplusplus
}
#endif

#endif
