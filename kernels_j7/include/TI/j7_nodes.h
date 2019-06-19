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


/*! \brief Channels parameters for selecting channel to be displayed.
 *         Passed as an argument to #TIVX_NODE_VPAC_DISPLAY_SELECT_CHANNEL
 *         control command.
 *
 */
typedef struct
{
    uint32_t                   active_channel_id;
    /**< Id of the active channel to be displayed */
} tivx_display_select_channel_params_t;

/*! \brief Control Command to select the channel to be displayed.
 *
 *          The display node can only display one of the input channels.
 *          In case, input is obeject array, there could be frames from
 *          multiple channels. In this case, this control command is used
 *          to select the channels to be displayed.
 *          This cmd can be called at runtime, provided all channels are
 *          exactly same in format, size, storage format etc..
 *
 *         This control command uses pointer to structure
 *         tivx_display_select_channel_params_t as an input argument.
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_NODE_VPAC_DISPLAY_SELECT_CHANNEL       (0x30000000u)


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

/*! \brief [Graph] Creates a DSS Display Node.
 *
 * \param [in] graph         The reference to the graph.
 * \param [in] configuration The input user data object of a single display params structure of type <tt>\ref tivx_display_params_t</tt>.
 * \param [in] image         The input image in one of the below formats:
 *                           <tt>\ref VX_DF_IMAGE_RGB</tt>,
 *                           <tt>\ref VX_DF_IMAGE_RGBX</tt>,
 *                           <tt>\ref VX_DF_IMAGE_UYVY</tt> or
 *                           <tt>\ref VX_DF_IMAGE_NV12</tt>.
 *
 * \see <tt>TIVX_KERNEL_DISPLAY_NAME</tt>
 * \ingroup group_vision_function_display
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDisplayNode(
                                            vx_graph graph,
                                            vx_user_data_object configuration,
                                            vx_image image);

/*! \brief [Graph] Creates a capture Node. The capture node takes in a user data object of type <tt>\ref tivx_capture_params_t</tt> to
                   configure the sensors. The outputs are of type object array which contain vx_image.
 * \param [in] graph The reference to the graph.
 * \param [in] input The input user data object of a single capture params structure of type <tt>\ref tivx_capture_params_t</tt>.
 * \param [out] output Object array output which has been created from an exemplar of vx_image's. The input MUST be made from format
                <tt>\ref VX_DF_IMAGE_RGBX</tt>.
 * \see <tt>TIVX_KERNEL_CAPTURE_NAME</tt>
 * \ingroup group_vision_function_capture
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxCaptureNode(vx_graph graph,
                                      vx_user_data_object  input,
                                      vx_object_array      output);

#ifdef __cplusplus
}
#endif

#endif /* J7_NODES_H_ */
