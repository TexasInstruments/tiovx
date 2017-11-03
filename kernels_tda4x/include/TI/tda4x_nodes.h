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

#ifndef TDA4X_NODES_H_
#define TDA4X_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief [Graph] Creates a VPAC_NF_GENERIC Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input array of a single params structure of type <tt>\ref tivx_vpac_nf_common_params_t</tt>.
 * \param [in] input The input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] conv The input convolution matrix. Max columns or rows supported is 5.  Scale value is ignored.  Coefficients are 9-bit signed.
 * \param [out] output The output image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \see <tt>TIVX_KERNEL_VPAC_NF_GENERIC</tt>
 * \ingroup group_vision_function_vpac_nf_generic
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfGenericNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_image             input,
                                      vx_convolution       conv,
                                      vx_image             output);

/*! \brief [Graph] Creates a VPAC_NF_BILATERAL Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input array of a single params structure of type <tt>\ref tivx_vpac_nf_bilateral_params_t</tt>.
 * \param [in] input The input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] sigmas The input array of a single params structure of type <tt>\ref tivx_vpac_nf_bilateral_sigmas_t</tt>.
 * \param [out] output The output image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \see <tt>TIVX_KERNEL_VPAC_NF_BILATERAL</tt>
 * \ingroup group_vision_function_vpac_nf_bilateral
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfBilateralNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_image             input,
                                      vx_array             sigmas,
                                      vx_image             output);

/*! \brief [Graph] Creates a DMPAC_SDE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input array of a single params structure of type <tt>\ref tivx_dmpac_sde_params_t</tt>.
 * \param [in] left The left input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] right The right input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] output The output image in <tt>\ref VX_DF_IMAGE_S16</tt> format. Bit packing format: Sign[15], Integer[14:7], Fractional[6:3], Confidence[2:0]
 * \param [out] confidence_histogram (optional) Histogram of the confidence scores.  Must be configured to 128 bins.
 * \see <tt>TIVX_KERNEL_DMPAC_SDE</tt>
 * \ingroup group_vision_function_dmpac_sde
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDmpacSdeNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_image             left,
                                      vx_image             right,
                                      vx_image             output,
                                      vx_distribution      confidence_histogram);

/*! \brief [Graph] Creates a VPAC_LDC Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input array of a single params structure of type <tt>\ref tivx_vpac_ldc_params_t</tt>.
 * \param [in] region_params The input array of a single params structure of type <tt>\ref tivx_vpac_ldc_region_params_t</tt> or <tt>\ref tivx_vpac_ldc_subregion_params_t</tt>.
 * \param [in] mesh_table (optional) The mesh table image. This can be a full remap table, but is typically sub-sampled by a power
 *              of 2 to save memory footprint and bandwidth. The coordinates are of type S16Q3, and are stored as X,Y pairs of type
 *              <tt>\ref VX_DF_IMAGE_U32</tt>.
 * \param [in] warp_matrix (optional) The affine or perspective warp matrix. Must be 2x3 (affine) or 3x3 (perspective),
 *              and of type <tt>\ref VX_TYPE_INT16</tt>.
 * \param [in] out_2_luma_lut (optional) The bit depth conversion LUT for out_2_luma channel. Use 12-bit data in a container of
 *              data_type <tt>\ref VX_TYPE_UINT16</tt>, and count of <tt>513</tt>
 * \param [in] out_3_chroma_lut (optional) The bit depth conversion LUT for out_3_chroma channel. Use 12-bit data in a container of
 *              data_type <tt>\ref VX_TYPE_UINT16</tt>, and count of <tt>513</tt>
 * \param [in] bandwidth_params (optional) The input array of a single params structure of type <tt>\ref tivx_vpac_ldc_bandwidth_params_t</tt>.
 * \param [in] in_luma_or_422 (optional) The input image in <tt>\ref VX_DF_IMAGE_UYVY</tt>, <tt>\ref VX_DF_IMAGE_U8</tt>,
 *              <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] in_chroma (optional) The input chroma interleaved plane in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>,
 *              or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] out_0_luma_or_422 (optional) The output image in <tt>\ref VX_DF_IMAGE_UYVY</tt>, <tt>\ref VX_DF_IMAGE_YUYV</tt>,
 *               <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] out_1_chroma (optional) The output chroma interleaved plane in <tt>\ref VX_DF_IMAGE_U8</tt>,
 *               <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] out_2_luma_or_422 (optional) Additional output image (typically with different bit depth) in
 *               <tt>\ref VX_DF_IMAGE_UYVY</tt>, <tt>\ref VX_DF_IMAGE_YUYV</tt>, <tt>\ref VX_DF_IMAGE_U8</tt>,
 *               <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] out_3_chroma (optional) Additional output chroma interleaved plane (typically with different bit depth)
 *               in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] error_status (optional) Bit mask indicating error flags.  Use a <tt>\ref VX_TYPE_UINT32</tt> scalar.
 * \see <tt>TIVX_KERNEL_VPAC_LDC</tt>
 * \ingroup group_vision_function_vpac_ldc
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacLdcNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_array             region_params,
                                      vx_image             mesh_table,
                                      vx_matrix            warp_matrix,
                                      vx_lut               out_2_luma_lut,
                                      vx_lut               out_3_chroma_lut,
                                      vx_array             bandwidth_params,
                                      vx_image             in_luma_or_422,
                                      vx_image             in_chroma,
                                      vx_image             out_0_luma_or_422,
                                      vx_image             out_1_chroma,
                                      vx_image             out_2_luma_or_422,
                                      vx_image             out_3_chroma,
                                      vx_scalar            error_status);

/*! \brief [Graph] Creates a DMPAC_DOF Node.
 *
 * - The dataformat of image within pyramid MUST be <tt>\ref VX_DF_IMAGE_U8</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * The pyramid MUST use scale of VX_SCALE_PYRAMID_HALF
 * The max number of pyramid levels can be 5
 * The width and height of base level MUST be interger multiple of 2^pyramidlevels
 * The meta properties of input_current, input_ref MUST be identical
 *
 * \param [in] graph                              The reference to the graph.
 * \param [in] configuration                      The input array of a single params structure of
 *                                                type <tt>\ref tivx_dmpac_dof_params_t</tt>.
 * \param [in] input_current                      Current input pyramid.
 * \param [in] input_reference                    Reference input pyramid.
 * \param [in] flow_vector_in                     Flow vector from previous execution of DOF.
 *                                                Size of image is base_width x base_height x 32bpp.
 *                                                Use <tt>\ref VX_DF_IMAGE_U32 </tt> dataformat.
 * \param [in] sparse_of_map           (optional) Sparse OF bit-mask of size base_width/8 x base_height x 1bpp.
 *                                                Use <tt>\ref VX_DF_IMAGE_U8 </tt> dataformat.
 * \param [out] flow_vector_out                   Flow vector output.
 *                                                Size of image is base_width x base_height x 32bpp.
 *                                                Use <tt>\ref VX_DF_IMAGE_U32 </tt> dataformat.
 * \param [out] confidence_histogram   (optional) Confidence histogram.
 *                                                Distribution meta properties, num_bins = 16, offset = 0, range = 16.
 * \see <tt>TIVX_KERNEL_DMPAC_DOF</tt>
 * \ingroup group_vision_function_dmpac_dof
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDmpacDofNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_pyramid           input_current,
                                      vx_pyramid           input_reference,
                                      vx_image             flow_vector_in,
                                      vx_image             sparse_of_map,
                                      vx_image             flow_vector_out,
                                      vx_distribution      confidence_histogram);

#ifdef __cplusplus
}
#endif

#endif /* TDA4X_NODES_H_ */


