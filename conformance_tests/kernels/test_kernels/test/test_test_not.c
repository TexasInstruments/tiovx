/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include "test_engine/test.h"
#include "test_tiovx.h"

#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>

static void fillSequence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

TESTCASE(tivxTestKernelNot, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    uint32_t width;
    uint32_t height;
} size_arg;

#define SIZE_ARG(w,h) ARG(#w "x" #h, w, h)

#define NOT_SIZE_ARGS       \
    SIZE_ARG(640, 480),     \
    ARG_EXTENDED_BEGIN(),   \
    SIZE_ARG(1, 1),         \
    SIZE_ARG(15, 17),       \
    SIZE_ARG(32, 32),       \
    SIZE_ARG(1231, 1234),   \
    SIZE_ARG(1280, 720),    \
    SIZE_ARG(1920, 1080),   \
    ARG_EXTENDED_END()

/*
 * Runs tivxTestNotNode and vxNotNode on the same input, then compares output.
 * Output should be identical.
 */
TEST_WITH_ARG(tivxTestKernelNot, testMatchesVxNot, size_arg, NOT_SIZE_ARGS)
{
    vx_image src, dst_test, dst_ref;
    CT_Image ref_src, ct_test, ct_ref;
    vx_graph graph_test, graph_ref;
    vx_node node_test, node_ref;
    vx_context context = context_->vx_context_;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSequence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    tivxTestKernelsLoadKernels(context);

    /* graph under test: uses tivxTestNotNode */
    ASSERT_VX_OBJECT(graph_test = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst_test   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node_test  = tivxTestNotNode(graph_test, src, dst_test), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph_test));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph_test));

    /* reference graph: uses standard OpenVX vxNotNode */
    ASSERT_VX_OBJECT(graph_ref = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst_ref   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node_ref  = vxNotNode(graph_ref, src, dst_ref), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph_ref));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph_ref));

    ASSERT_NO_FAILURE({
        ct_test = ct_image_from_vx_image(dst_test);
        ct_ref  = ct_image_from_vx_image(dst_ref);
    });

    ASSERT_EQ_CTIMAGE(ct_ref, ct_test);

    VX_CALL(vxReleaseNode(&node_test));
    VX_CALL(vxReleaseNode(&node_ref));
    VX_CALL(vxReleaseImage(&dst_test));
    VX_CALL(vxReleaseImage(&dst_ref));
    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseGraph(&graph_test));
    VX_CALL(vxReleaseGraph(&graph_ref));

    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxTestKernelNot, testMatchesVxNot)
