/*
*
* Copyright (c) 2017-2023 Texas Instruments Incorporated
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



#ifndef TIVX_NODES_H_
#define TIVX_NODES_H_

#include <VX/vx.h>
#include <TI/tivx_tensor.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief [Graph] Creates a TIDL Node.
 * \param [in] graph Reference to vx_graph.
 * \param [in] kernel Reference to vx_kernel.
 * \param [in,out] appParams is an array of 5 parameters:
 *             - config vx_user_data_object type corresponding to the configuration (named string: sTIDL_IOBufDesc_t)
 *             - network vx_user_data_object type corresponding to the network (named string: TIDL_network)
 *             - createParams: vx_user_data_object type corresponding to the structure TIDL_CreateParams.
 *                             This structure contains create-time parameters and are used only one time to initialize
 *                             the node.
 *                             Currently members quantHistoryParam1, quantHistoryParam2, quantMargin need to be initialized by
 *                             the application.
 *             - inArgs: vx_user_data_object type corresponding to the structure TIDL_InArgs. Used to pass input parameters
 *                       that goes in effect during the node's process call when graph is executed. These parameters
 *                       can be updated before each frame execution of a graph. Currently the list of parameters is empty.
 *                       So inArgs is just a placeholder for future extension.
 *             - outArgs: vx_user_data_object type corresponding to the structure TIDL_outArgs. Used to collect output parameters
 *                       that are returned by the node's process call when graph is executed. These parameters
 *                       are refreshed after each frame execution of a graph. Currently the list of parameters is empty.
 *                       So outArgs is just a placeholder for future extension.
 * \param [in] input_tensors Array of input tensors
 *             This parameter is ignored when the first layer of the network is a data layer, which is most of the time.
 *             Only networks that are dependent on the output of a previous networks have first layer that are not data layer.
 * \param [out] output_tensors Array of output tensors
 *
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 * \ingroup group_vision_function_tidl
 */
VX_API_ENTRY vx_node VX_API_CALL tivxTIDLNode(vx_graph  graph,
                                              vx_kernel kernel,
                                              vx_reference appParams[],
                                              vx_tensor input_tensors[],
                                              vx_tensor output_tensors[]);

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

/*! \brief [Graph] Creates a Object Array Split Node.  The input object array elements are split amongst all provided
 *                 outputs.  The first 2 outputs are required, but up to 4 outputs can be provided.  The number of
 *                 input elements must equal the sum of all elements across the provided outputs.  Once split, the
 *                 out0 will have the first N elements of the input object array, with N being the number of elements
 *                 within out0.  The remaining elements of input will be distributed across the remaining provided
 *                 outputs in the same manner.
 * \param [in] graph The reference to the graph.
 * \param [in]  in    Input object array.  The channels of this object array will be split amongst the provided outputs
 *                    according to the sizes of the output object arrays
 * \param [out] out0  First output object array.  After processing, this will contain the first N elements of the input,
 *                    with N being the number of elements of out0.
 * \param [out] out1  Second output object array.  After processing, this will contain the next N elements of the input
 *                    after the number of elements of out0, with N being the number of elements of out1.
 * \param [out] out2 (optional) Third output object array.  After processing, this will contain the next N elements of
 *                    the input after the sum of the elements of out0 and out1, with N being the number of elements of
 *                    out2.
 * \param [out] out3 (optional) Fourth output object array.  After processing, this will contain the next N elements of
 *                    the input after the sum of the elements of out0, out1 and out2, with N being the number of
 *                    elements of out3.
 * \see <tt>TIVX_KERNEL_OBJ_ARRAY_SPLIT_NAME</tt>
 * \ingroup group_vision_function_obj_array_split
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxObjArraySplitNode(vx_graph graph,
                                      vx_object_array      in,
                                      vx_object_array      out0,
                                      vx_object_array      out1,
                                      vx_object_array      out2,
                                      vx_object_array      out3);


#ifdef __cplusplus
}
#endif

#endif
