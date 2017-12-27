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

#include <TI/tivx.h>
#include <TI/tda4x.h>

VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfGenericNode(vx_graph graph,
                                      vx_array             configuration,
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
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_VPAC_NF_GENERIC,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfBilateralNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_image             input,
                                      vx_array             sigmas,
                                      vx_image             output)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input,
            (vx_reference)sigmas,
            (vx_reference)output
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_VPAC_NF_BILATERAL,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDmpacSdeNode(vx_graph graph,
                                      vx_array             configuration,
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
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_DMPAC_SDE,
                                           prms,
                                           dimof(prms));
    return node;
}

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
                                      vx_scalar            error_status)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)region_params,
            (vx_reference)mesh_table,
            (vx_reference)warp_matrix,
            (vx_reference)out_2_luma_lut,
            (vx_reference)out_3_chroma_lut,
            (vx_reference)bandwidth_params,
            (vx_reference)in_luma_or_422,
            (vx_reference)in_chroma,
            (vx_reference)out_0_luma_or_422,
            (vx_reference)out_1_chroma,
            (vx_reference)out_2_luma_or_422,
            (vx_reference)out_3_chroma,
            (vx_reference)error_status
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_VPAC_LDC,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDmpacDofNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_pyramid           input_current,
                                      vx_pyramid           input_reference,
                                      vx_image             flow_vector_in,
                                      vx_image             sparse_of_map,
                                      vx_image             flow_vector_out,
                                      vx_distribution      confidence_histogram)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_current,
            (vx_reference)input_reference,
            (vx_reference)flow_vector_in,
            (vx_reference)sparse_of_map,
            (vx_reference)flow_vector_out,
            (vx_reference)confidence_histogram
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_DMPAC_DOF,
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
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_DOF_VISUALIZE,
                                           prms,
                                           dimof(prms));
    return node;
}

