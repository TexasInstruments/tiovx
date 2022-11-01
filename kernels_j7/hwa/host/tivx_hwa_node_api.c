/*
 *
 * Copyright (c) 2017-2021 Texas Instruments Incorporated
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

#include <TI/tivx.h>

#ifdef BUILD_VPAC_NF
VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfGenericNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input,
                                      vx_convolution       conv,
                                      vx_image             output)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input,
            (vx_reference)conv,
            (vx_reference)output
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_VPAC_NF_GENERIC_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfBilateralNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input,
                                      vx_user_data_object  sigmas,
                                      vx_image             output)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input,
            (vx_reference)sigmas,
            (vx_reference)output
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_VPAC_NF_BILATERAL_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_DMPAC_SDE
VX_API_ENTRY vx_node VX_API_CALL tivxDmpacSdeNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             left,
                                      vx_image             right,
                                      vx_image             output,
                                      vx_distribution      confidence_histogram)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)left,
            (vx_reference)right,
            (vx_reference)output,
            (vx_reference)confidence_histogram
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DMPAC_SDE_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_VPAC_LDC
VX_API_ENTRY vx_node VX_API_CALL tivxVpacLdcNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_matrix            warp_matrix,
                                      vx_user_data_object  region_prms,
                                      vx_user_data_object  mesh_prms,
                                      vx_image             mesh_img,
                                      vx_user_data_object  dcc_db,
                                      vx_image             in_img,
                                      vx_image             out0_img,
                                      vx_image             out1_img)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)warp_matrix,
            (vx_reference)region_prms,
            (vx_reference)mesh_prms,
            (vx_reference)mesh_img,
            (vx_reference)dcc_db,
            (vx_reference)in_img,
            (vx_reference)out0_img,
            (vx_reference)out1_img
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_VPAC_LDC_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif


#ifdef BUILD_VPAC_MSC
VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscScaleNode(vx_graph graph,
                                      vx_image             in_img,
                                      vx_image             out0_img,
                                      vx_image             out1_img,
                                      vx_image             out2_img,
                                      vx_image             out3_img,
                                      vx_image             out4_img)
{
    vx_reference prms[] = {
            (vx_reference)in_img,
            (vx_reference)out0_img,
            (vx_reference)out1_img,
            (vx_reference)out2_img,
            (vx_reference)out3_img,
            (vx_reference)out4_img
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscPyramidNode(vx_graph graph,
                                      vx_image             in_img,
                                      vx_pyramid           out_pyramid)
{
    vx_reference prms[] = {
            (vx_reference)in_img,
            (vx_reference)out_pyramid
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_DMPAC_DOF
VX_API_ENTRY vx_node VX_API_CALL tivxDmpacDofNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_current_base,
                                      vx_image             input_reference_base,
                                      vx_pyramid           input_current,
                                      vx_pyramid           input_reference,
                                      vx_image             flow_vector_in,
                                      vx_user_data_object  sparse_of_config,
                                      vx_image             sparse_of_map,
                                      vx_image             flow_vector_out,
                                      vx_distribution      confidence_histogram)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_current_base,
            (vx_reference)input_reference_base,
            (vx_reference)input_current,
            (vx_reference)input_reference,
            (vx_reference)flow_vector_in,
            (vx_reference)sparse_of_config,
            (vx_reference)sparse_of_map,
            (vx_reference)flow_vector_out,
            (vx_reference)confidence_histogram
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DMPAC_DOF_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDofVisualizeNode(vx_graph graph,
                                      vx_image             flow_vector,
                                      vx_scalar            confidence_threshold,
                                      vx_image             flow_vector_rgb,
                                      vx_image             confidence_image)
{
    vx_reference prms[] = {
            (vx_reference)flow_vector,
            (vx_reference)confidence_threshold,
            (vx_reference)flow_vector_rgb,
            (vx_reference)confidence_image
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DOF_VISUALIZE_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_VPAC_VISS
VX_API_ENTRY vx_node VX_API_CALL tivxVpacVissNode(vx_graph  graph,
                                      vx_user_data_object   configuration,
                                      vx_user_data_object   ae_awb_result,
                                      vx_user_data_object   dcc_buf,
                                      tivx_raw_image        raw,
                                      vx_image              output0,
                                      vx_image              output1,
                                      vx_image              output2,
                                      vx_image              output3,
                                      vx_image              output4,
                                      vx_user_data_object   h3a_output,
                                      vx_distribution       histogram0,
                                      vx_distribution       histogram1,
                                      vx_distribution       raw_histogram)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)ae_awb_result,
            (vx_reference)dcc_buf,
            (vx_reference)raw,
            (vx_reference)output0,
            (vx_reference)output1,
            (vx_reference)output2,
            (vx_reference)output3,
            (vx_reference)output4,
            (vx_reference)h3a_output,
            (vx_reference)histogram0,
            (vx_reference)histogram1,
            (vx_reference)raw_histogram
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_VPAC_VISS_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_DISPLAY
VX_API_ENTRY vx_node VX_API_CALL tivxDisplayNode(
                                            vx_graph graph,
                                            vx_user_data_object configuration,
                                            vx_image image)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)image
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DISPLAY_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDisplayM2MNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input,
                                      vx_image             output)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input,
            (vx_reference)output
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DISPLAY_M2M_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_CAPTURE
VX_API_ENTRY vx_node VX_API_CALL tivxCaptureNode(vx_graph graph,
                                      vx_user_data_object  input,
                                      vx_object_array      output)
{
    vx_reference prms[] = {
            (vx_reference)input,
            (vx_reference)output
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_CAPTURE_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

#ifdef BUILD_CSITX
VX_API_ENTRY vx_node VX_API_CALL tivxCsitxNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_object_array             input)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_CSITX_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
#endif

VX_API_ENTRY vx_node VX_API_CALL tivxObjArraySplitNode(vx_graph graph,
                                      vx_object_array      in,
                                      vx_object_array      out0,
                                      vx_object_array      out1,
                                      vx_object_array      out2,
                                      vx_object_array      out3)
{
    vx_reference prms[] = {
            (vx_reference)in,
            (vx_reference)out0,
            (vx_reference)out1,
            (vx_reference)out2,
            (vx_reference)out3
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_OBJ_ARRAY_SPLIT_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
