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

VX_API_ENTRY vx_node VX_API_CALL tivxHarrisCornersNode(vx_graph graph,
                            vx_image  input,
                            vx_uint32 scaling_factor,
                            vx_int32  nms_threshold,
                            vx_uint8  q_shift,
                            vx_uint8  win_size,
                            vx_uint8  score_method,
                            vx_uint8  suppression_method,
                            vx_array  corners,
                            vx_scalar num_corners)
{
    vx_scalar sc_fact = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT32, &scaling_factor);
    vx_scalar sc_thr = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_INT32, &nms_threshold);
    vx_scalar sc_q_shift = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT8, &q_shift);
    vx_scalar sc_win = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT8, &win_size);
    vx_scalar sc_score = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT8, &score_method);
    vx_scalar sc_suppr = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT8, &suppression_method);

    vx_reference params[] = {
            (vx_reference)input,
            (vx_reference)sc_fact,
            (vx_reference)sc_thr,
            (vx_reference)sc_q_shift,
            (vx_reference)sc_win,
            (vx_reference)sc_score,
            (vx_reference)sc_suppr,
            (vx_reference)corners,
            (vx_reference)num_corners,
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_IVISION_HARRIS_CORNERS,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&sc_fact);
    vxReleaseScalar(&sc_thr);
    vxReleaseScalar(&sc_q_shift);
    vxReleaseScalar(&sc_win);
    vxReleaseScalar(&sc_score);
    vxReleaseScalar(&sc_suppr);
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxRgbIrNode(vx_graph graph,
                            vx_image  input,
                            vx_uint8   sensorPhase,
                            vx_uint16  threshold,
                            vx_float32  alphaR,
                            vx_float32  alphaG,
                            vx_float32  alphaB,
                            vx_uint8  borderMode,
                            vx_image  outputBayer,
                            vx_image  outputIR)
{
    vx_scalar sc_sensorPhase = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT8, &sensorPhase);
    vx_scalar sc_thr = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT16, &threshold);
    vx_scalar sc_alphaR = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_FLOAT32, &alphaR);
    vx_scalar sc_alphaG = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_FLOAT32, &alphaG);
    vx_scalar sc_alphaB = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_FLOAT32, &alphaB);
    vx_scalar sc_borderMode = vxCreateScalar(vxGetContext((vx_reference)graph),
        (vx_enum)VX_TYPE_UINT8, &borderMode);

    vx_reference params[] = {
            (vx_reference)input,
            (vx_reference)sc_sensorPhase,
            (vx_reference)sc_thr,
            (vx_reference)sc_alphaR,
            (vx_reference)sc_alphaG,
            (vx_reference)sc_alphaB,
            (vx_reference)sc_borderMode,
            (vx_reference)outputBayer,
            (vx_reference)outputIR,
    };
    vx_node node = tivxCreateNodeByKernelEnum(graph,
                                           TIVX_KERNEL_IVISION_RGB_IR,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&sc_sensorPhase);
    vxReleaseScalar(&sc_thr);
    vxReleaseScalar(&sc_alphaR);
    vxReleaseScalar(&sc_alphaG);
    vxReleaseScalar(&sc_alphaB);
    vxReleaseScalar(&sc_borderMode);
    return node;
}
