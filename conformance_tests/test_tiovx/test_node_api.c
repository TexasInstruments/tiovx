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

#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_config.h>
#include <TI/tivx.h>

#include "test_engine/test.h"

TESTCASE(tivxNodeApi, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxNodeApi, negativeTestWarpAffineNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_matrix matrix = NULL;
    vx_enum type = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxWarpAffineNode(graph, input, matrix, type, output));
}

TEST(tivxNodeApi, negativeTestWarpPerspectiveNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_matrix matrix = NULL;
    vx_enum type = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxWarpPerspectiveNode(graph, input, matrix, type, output));
}

TEST(tivxNodeApi, negativeTestRemapNode)
{
    vx_context context = context_->vx_context_;

    vx_graph graph = NULL;
    vx_image input = NULL;
    vx_remap table = NULL;
    vx_enum policy = 0;
    vx_image output = NULL;

    ASSERT(NULL == vxRemapNode(graph, input, table, policy, output));
}

TESTCASE_TESTS(
    tivxNodeApi,
    negativeTestWarpAffineNode,
    negativeTestWarpPerspectiveNode,
    negativeTestRemapNode
)

