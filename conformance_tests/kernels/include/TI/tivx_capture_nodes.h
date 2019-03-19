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

#ifndef TIVX_CAPTURE_NODES_H_
#define TIVX_CAPTURE_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Control Command to increment scalar value by 1
 *         This is test for the control command,
 *         it increments u8 scalar value by 1
 * \param [in] vx_scalar object.
 * \ingroup group_vision_function_scalar_source
 * \retval vx_status, VX_SUCCESS on successfully incrementing variable.
 *                    VX_FAILURE in case of error
 */
#define TIVX_SCALAR_SRC_NODE_INC_SCALAR (0x01000000u)

/*! \brief Control Command to decrement scalar value by 1
 *         This is test for the control command,
 *         it decrement u8 scalar value by 1
 * \param [in] vx_scalar object.
 * \ingroup group_vision_function_scalar_source
 * \retval vx_status, VX_SUCCESS on successfully incrementing variable.
 *                    VX_FAILURE in case of error
 */
#define TIVX_SCALAR_SRC_NODE_DEC_SCALAR (0x01000001u)


/*! \brief [Graph] Creates a FILE_READ_CAPTURE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [out] output_image
 * \see <tt>TIVX_KERNEL_FILE_READ_CAPTURE_NAME</tt>
 * \ingroup group_vision_function_file_read_capture
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxFileReadCaptureNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_object_array      output_image);

/*! \brief [Graph] Creates a SCALAR_SINK Node.
 * \param [in] graph The reference to the graph.
 * \param [in] in
 * \see <tt>TIVX_KERNEL_SCALAR_SINK_NAME</tt>
 * \ingroup group_vision_function_scalar_sink
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSinkNode(vx_graph graph,
                                      vx_scalar            in);

/*! \brief [Graph] Creates a SCALAR_SOURCE Node.
 * \param [in] graph The reference to the graph.
 * \param [out] out
 * \see <tt>TIVX_KERNEL_SCALAR_SOURCE_NAME</tt>
 * \ingroup group_vision_function_scalar_source
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSourceNode(vx_graph graph,
                                      vx_scalar            out);

/*! \brief [Graph] Creates a SCALAR_SINK Node.
 * \param [in] graph The reference to the graph.
 * \param [in] in
 * \see <tt>TIVX_KERNEL_SCALAR_SINK_NAME</tt>
 * \ingroup group_vision_function_scalar_sink
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSink2Node(vx_graph graph,
                                      vx_scalar            in);

/*! \brief [Graph] Creates a SCALAR_SOURCE Node.
 * \param [in] graph The reference to the graph.
 * \param [out] out
 * \see <tt>TIVX_KERNEL_SCALAR_SOURCE_NAME</tt>
 * \ingroup group_vision_function_scalar_source
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarSource2Node(vx_graph graph,
                                      vx_scalar            out);

/*! \brief [Graph] Creates a SCALAR_INTERMEDIATE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] in
 * \param [out] out
 * \see <tt>TIVX_KERNEL_SCALAR_INTERMEDIATE_NAME</tt>
 * \ingroup group_vision_function_scalar_intermediate
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxScalarIntermediateNode(vx_graph graph,
                                      vx_scalar            in,
                                      vx_scalar            out);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_CAPTURE_NODES_H_ */


