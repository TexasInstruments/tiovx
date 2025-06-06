/*

 * Copyright (c) 2024 The Khronos Group Inc.
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

#ifdef  __cplusplus
}
#endif

#endif
