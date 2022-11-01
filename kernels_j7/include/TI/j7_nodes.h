/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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

#ifndef J7_NODES_H_
#define J7_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif



/*! \brief [Graph] Creates a DOF visualization node Node.
 * \param [in] graph The reference to the graph.
 * \param [in] flow_vector Flow vector output from dmpac_dof node
 * \param [in] confidence_threshold (optional) Threshold to use when generating flow_vector_rgb. vx_scalar of type vx_uint32.
 *                                   Valid values are 0 (low threshold/confidence) .. 15 (high threshold/confidence).
 *                                   When NULL, default value of 8 is used.
 * \param [out] flow_vector_rgb flow vector representated as 24 RGB image
 * \param [out] confidence_image confidence values represented as U8 grayscale image, 255 is high confidence
 * \see <tt>TIVX_KERNEL_DOF_VISUALIZE_NAME</tt>
 * \ingroup group_vision_function_dmpac_dof
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDofVisualizeNode(vx_graph graph,
                                      vx_image             flow_vector,
                                      vx_scalar            confidence_threshold,
                                      vx_image             flow_vector_rgb,
                                      vx_image             confidence_image);


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

#endif /* J7_NODES_H_ */
