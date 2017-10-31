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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/tda4x.h>
#include "test_engine/test.h"
#include <string.h>

TESTCASE(tivxHwaDmpacDof, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxHwaDmpacDof, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_enum params_type = VX_TYPE_INVALID;
    vx_array param_array = NULL;
    vx_graph graph = 0;
    vx_node node = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF))
    {
        uint32_t width = 256, height = 256;
        uint32_t levels = 2;
        vx_enum format = VX_DF_IMAGE_U8;

        hwaLoadKernels(context);

        params_type = vxRegisterUserStruct(context, sizeof(tivx_dmpac_dof_params_t));
        ASSERT(params_type >= VX_TYPE_USER_STRUCT_START && params_type <= VX_TYPE_USER_STRUCT_END);
        memset(&params, 0, sizeof(tivx_dmpac_dof_params_t));
        ASSERT_VX_OBJECT(param_array = vxCreateArray(context, params_type, 1), VX_TYPE_ARRAY);

        params.vertical_search_range[0] = 48;
        params.vertical_search_range[1] = 48;
        params.horizontal_search_range = 191;
        params.median_filter_enable = 1;
        params.motion_smoothness_factor = 24;
        params.motion_direction = 1; /* 1: forward direction */

        VX_CALL(vxAddArrayItems(param_array, 1, &params, sizeof(tivx_dmpac_dof_params_t)));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(flow_vector_in = vxCreateImage(context, width, height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, width, height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacDofNode(graph,
                        param_array,
                        input_current,
                        input_reference,
                        flow_vector_in,
                        NULL,
                        flow_vector_out,
                        confidence_histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_in));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseDistribution(&confidence_histogram));
        VX_CALL(vxReleaseArray(&param_array));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_array == 0);


        hwaUnLoadKernels(context);
    }
}

TEST(tivxHwaDmpacDof, testGraphProcessing)
{
    vx_context context = context_->vx_context_;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF))
    {
        hwaLoadKernels(context);


        hwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaDmpacDof, testNodeCreation, testGraphProcessing)

