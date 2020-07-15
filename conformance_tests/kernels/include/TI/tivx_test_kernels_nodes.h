/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#ifndef TIVX_TEST_KERNELS_NODES_H_
#define TIVX_TEST_KERNELS_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief [Graph] Creates a NOT_NOT Node.
 * \param [in] graph The reference to the graph.
 * \param [in] input
 * \param [out] output
 * \see <tt>TIVX_KERNEL_NOT_NOT_NAME</tt>
 * \ingroup group_vision_function_not_not
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxNotNotNode(vx_graph graph,
                                      vx_image             input,
                                      vx_image             output);

/*! \brief [Graph] Creates a SCALAR_SOURCE_ERROR Node.
 * \param [in] graph The reference to the graph.
 * \param [out] out
 * \see <tt>TIVX_KERNEL_SCALAR_SOURCE_ERROR_NAME</tt>
 * \ingroup group_vision_function_scalar_source_error
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSourceErrorNode(vx_graph graph,
                                      vx_scalar            out);

/*! \brief [Graph] Creates a SCALAR_SOURCE_OBJ_ARRAY Node.
 * \param [in] graph The reference to the graph.
 * \param [out] out_object_array
 * \see <tt>TIVX_KERNEL_SCALAR_SOURCE_OBJ_ARRAY_NAME</tt>
 * \ingroup group_vision_function_scalar_source_obj_array
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSourceObjArrayNode(vx_graph graph,
                                      vx_object_array      out_object_array);

/*! \brief [Graph] Creates a SCALAR_SINK_OBJ_ARRAY Node.
 * \param [in] graph The reference to the graph.
 * \param [in] in_object_array
 * \see <tt>TIVX_KERNEL_SCALAR_SINK_OBJ_ARRAY_NAME</tt>
 * \ingroup group_vision_function_scalar_sink_obj_array
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSinkObjArrayNode(vx_graph graph,
                                      vx_object_array      in_object_array);

/*! \brief [Graph] Creates a PYRAMID_INTERMEDIATE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] input
 * \param [out] output
 * \see <tt>TIVX_KERNEL_PYRAMID_INTERMEDIATE_NAME</tt>
 * \ingroup group_vision_function_pyramid_intermediate
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPyramidIntermediateNode(vx_graph graph,
                                      vx_pyramid           input,
                                      vx_pyramid           output);

/*! \brief [Graph] Creates a PYRAMID_SOURCE Node.
 * \param [in] graph The reference to the graph.
 * \param [out] output
 * \see <tt>TIVX_KERNEL_PYRAMID_SOURCE_NAME</tt>
 * \ingroup group_vision_function_pyramid_source
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPyramidSourceNode(vx_graph graph,
                                      vx_pyramid           output);

/*! \brief [Graph] Creates a PYRAMID_SINK Node.
 * \param [in] graph The reference to the graph.
 * \param [in] output
 * \see <tt>TIVX_KERNEL_PYRAMID_SINK_NAME</tt>
 * \ingroup group_vision_function_pyramid_sink
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPyramidSinkNode(vx_graph graph,
                                      vx_pyramid           output);

/*! \brief [Graph] Creates a CMD_TIMEOUT_TEST Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] in
 * \param [out] out
 * \see <tt>TIVX_KERNEL_CMD_TIMEOUT_TEST_NAME</tt>
 * \ingroup group_vision_function_cmd_timeout_test
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxCmdTimeoutTestNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_scalar            in,
                                      vx_scalar            out);
/*! \brief [Graph] Creates a SCALAR_INTERMEDIATE_2 Node.
 * \param [in] graph The reference to the graph.
 * \param [in] in
 * \param [out] out
 * \see <tt>TIVX_KERNEL_SCALAR_INTERMEDIATE_2_NAME</tt>
 * \ingroup group_vision_function_scalar_intermediate_2
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarIntermediate2Node(vx_graph graph,
                                      vx_scalar            in,
                                      vx_scalar            out);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_TEST_KERNELS_NODES_H_ */


