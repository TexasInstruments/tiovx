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
#include <TI/tivx.h>

#include "test_engine/test.h"

TESTCASE(tivxDebug, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxDebug, negativeTestSetDebugZone)
{
    vx_context context = context_->vx_context_;

    vx_enum zone = -1;

    tivx_set_debug_zone(zone);
    tivx_set_debug_zone(VX_ZONE_MAX);
}

TEST(tivxDebug, negativeTestClrDebugZone)
{
    vx_context context = context_->vx_context_;

    vx_enum zone = -1;

    tivx_clr_debug_zone(zone);
    tivx_clr_debug_zone(VX_ZONE_MAX);
}

TEST(tivxDebug, negativeTestGetDebugZone)
{
    vx_context context = context_->vx_context_;

    vx_enum zone = -1;

    ASSERT(vx_false_e == tivx_is_zone_enabled(zone));
    ASSERT(vx_false_e == tivx_is_zone_enabled(VX_ZONE_MAX));
    ASSERT(vx_false_e == tivx_is_zone_enabled(0));
    ASSERT(vx_true_e == tivx_is_zone_enabled(1));
}

TEST(tivxDebug, testSetNodeDebugZone)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_kernel kernel = 0;
    vx_node node = 0;
    vx_uint32 set_zone = (vx_enum)VX_ZONE_INFO;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    set_zone = (vx_enum)VX_ZONE_INFO;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSetDebugZone(node, set_zone, vx_true_e));

    set_zone = (vx_enum)VX_ZONE_ERROR;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSetDebugZone(node, set_zone, vx_false_e));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxDebug, testSetGraphDebugZone)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_kernel kernel = 0;
    vx_uint32 set_gZone = (vx_enum)VX_ZONE_INFO, set_nZone = (vx_enum)VX_ZONE_WARNING, num_nodes = 10, i;
    vx_node node[num_nodes];

    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < num_nodes; i++)
    {
        ASSERT_VX_OBJECT(node[i] = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxNodeSetDebugZone(node[i], set_nZone, vx_true_e));
    }

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxGraphSetDebugZone(graph, set_gZone, vx_true_e));

    set_gZone = (vx_enum)VX_ZONE_WARNING;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, tivxGraphSetDebugZone(graph, set_gZone, vx_false_e));

    for (i = 0; i < num_nodes; i++)
    {
        VX_CALL(vxReleaseNode(&node[i]));
    }
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxDebug, negativetestSetNodeDebugZone)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_kernel kernel = 0;
    vx_node node = 0;
    vx_uint32 level;
    vx_size size = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_BOX_3x3), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, kernel), VX_TYPE_NODE);

    level = VX_ZONE_MAX+1;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxNodeSetDebugZone(node, level, vx_true_e));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseKernel(&kernel));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxDebug, negativeTestSetGraphDebugZone)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_uint32 level;
    vx_size size = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    level = VX_ZONE_MAX+1;
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, tivxGraphSetDebugZone(graph, level, vx_true_e));

    VX_CALL(vxReleaseGraph(&graph));
}

TESTCASE_TESTS(
    tivxDebug,
    negativeTestSetDebugZone,
    negativeTestClrDebugZone,
    negativeTestGetDebugZone,
    testSetNodeDebugZone,
    testSetGraphDebugZone,
    negativetestSetNodeDebugZone,
    negativeTestSetGraphDebugZone
)

