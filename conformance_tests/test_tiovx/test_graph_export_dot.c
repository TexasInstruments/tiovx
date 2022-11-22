/*

 * Copyright (c) 2015-2017 The Khronos Group Inc.
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
/*
 * Copyright (c) 2022 Texas Instruments Incorporated
 */

#include <TI/tivx.h>

#include "test_engine/test.h"

TESTCASE(tivxGraphExportDot, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxGraphExportDot, negativeTestExportGraphToDot)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL, accum = NULL;
    vx_node node = NULL;
    char of_path[5];
    char of_prefix[5];

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxExportGraphToDot(graph, NULL, NULL));
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum = vxCreateImage(context, 128, 128, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(node = vxAccumulateImageNode(graph, input, accum), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxExportGraphToDot((vx_graph)(input), NULL, NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxExportGraphToDot((vx_graph)(input), of_path, NULL));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxExportGraphToDot((vx_graph)(input), of_path, of_prefix));
    VX_CALL(vxVerifyGraph(graph));
    /* Disabling for now until TIOVX-1285 is resolved */
    //ASSERT_EQ_VX_STATUS(VX_FAILURE, tivxExportGraphToDot(graph, of_path, of_prefix));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseImage(&accum));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(
    tivxGraphExportDot,
    negativeTestExportGraphToDot
)

