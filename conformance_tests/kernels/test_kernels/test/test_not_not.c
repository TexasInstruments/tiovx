/* 

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_engine/test.h"

#include <VX/vx.h>
#include <TI/tivx_test_kernels.h>

static void fillSquence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

TESTCASE(tivxTestKernelsNotNot,  CT_VXContext, ct_setup_vx_context, 0)

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

TEST_WITH_ARG(tivxTestKernelsNotNot, testSizes, size_arg, NOT_SIZE_ARGS)
{
    vx_image src, dst;
    CT_Image ref_src, vxdst;
    vx_graph graph;
    vx_context context = context_->vx_context_;

    ASSERT_NO_FAILURE({
        ref_src = ct_allocate_image(arg_->width, arg_->height, VX_DF_IMAGE_U8);
        fillSquence(ref_src, (uint32_t)CT()->seed_);
        src = ct_image_to_vx_image(ref_src, context);
    });

    tivxTestKernelsLoadKernels(context);

    // build one-node graph
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(dst   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(tivxNotNotNode(graph, src, dst), VX_TYPE_NODE);

    // run graph
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    ASSERT_NO_FAILURE({
        vxdst = ct_image_from_vx_image(dst);
    });

    ASSERT_EQ_CTIMAGE(ref_src, vxdst);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}


TESTCASE_TESTS(tivxTestKernelsNotNot, testSizes)
