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
 * Copyright (c) 2024 Texas Instruments Incorporated
 */
#include <vx_internal.h>
#include <tivx_objects.h>

#include "test_tiovx.h"
#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>

TESTCASE(tivxInternalGraphVerify,  CT_VXContext, ct_setup_vx_context, 0)

/* To hit negative portion of vxVerifyGraph, ownNodeCreateUserCallbackCommand */
TEST(tivxInternalGraphVerify, negativeBoundaryTestOwnGraphCreateNodeCallbackCommands)
{
    vx_context context = context_->vx_context_;

    vx_graph graph;
    int i, j;
    uint32_t width = 640, height = 480;
    vx_image input, output;
    tivx_obj_desc_t *obj_desc[TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST] = {NULL};
    vx_node color_convert_node;

    /* Creating graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(color_convert_node = vxColorConvertNode(graph, input, output), VX_TYPE_NODE);

    /* Maxing out the OBJECT DESCRIPTORS */
    for (i = 0; i < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST; i++)
    {
        obj_desc[i] = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, NULL);
        if (NULL == obj_desc[i])
        {
            break;
        }
    }

    /* Calling Graph verify to invoke ownNodeCreateUserCallbackCommand, and thus to hit
    some negative portion of vxVerifyGraph */
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxVerifyGraph(graph));

    /* Cleanup */
    for (j = 0; j < i; j++)
    {
        if (NULL == obj_desc[j])
        {
            break;
        }
        VX_CALL(ownObjDescFree((tivx_obj_desc_t**)&obj_desc[j]));
    }

    /* Releasing images */
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));

    /* Releasing node */
    VX_CALL(vxReleaseNode(&color_convert_node));

    /* Releasing graph */
    VX_CALL(vxReleaseGraph(&graph));
}

/* To hit Negative portion of Graph Verify corresponding to ownCreateMetaFormat failure*/
TEST(tivxInternalGraphVerify, negativeBoundaryTestVerifyGraph)
{
    vx_context context = context_->vx_context_;

    vx_graph graph;

    vx_meta_format meta[TIVX_META_FORMAT_MAX_OBJECTS];

    int i, j;
    uint32_t width = 640, height = 480;
    vx_image input, output;
    vx_node color_convert_node;

    tivx_obj_desc_t *obj_desc[TIVX_META_FORMAT_MAX_OBJECTS] = {NULL};

    /* Creating graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating input and output images */
    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

    /* Creating a color convert node */
    ASSERT_VX_OBJECT(color_convert_node = vxColorConvertNode(graph, input, output), VX_TYPE_NODE);

    /* Maxing out the Meta Format Objects */
    for (i = 0; i < TIVX_META_FORMAT_MAX_OBJECTS; i++)
    {
        meta[i] = ownCreateMetaFormat(context);
        if (NULL == meta[i])
        {
            break;
        }
    }

    /* Calling Graph verify to invoke ownCreateMetaFormat failure, and thus to hit
    corresponding negative portion of vxVerifyGraph */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxVerifyGraph(graph));

    /* Cleanup */
    for (j = 0; j < i; j++)
    {
        if (NULL == meta[j])
        {
            break;
        }
        VX_CALL(ownReleaseMetaFormat(&meta[j]));
    }

    /* Releasing images */
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));

    VX_CALL(vxReleaseNode(&color_convert_node));

    /* Releasing graph */
    VX_CALL(vxReleaseGraph(&graph));
}

/* Test to hit invalid scope portion of ownGraphInitVirtualNode and corresponding negative sections
   of ownGraphNodeKernelValidate and vxVerifyGraph */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphInitVirtualNode1)
{
    vx_context context = context_->vx_context_;
    vx_graph graph1, graph2;
    uint32_t width = 640, height = 480;
    vx_image input, intermediate, output;
    vx_node color_convert_node1, color_convert_node2;

    /* Creating Graphs */
    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating input and output images */
    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);

    /* Creating virtual image with the graph2 scope */
    intermediate = vxCreateVirtualImage (graph2, width, height, VX_DF_IMAGE_YUYV);

    /* Creating 2 color convert nodes, with intermediate image of scope graph2, given to graph1 */
    ASSERT_VX_OBJECT(color_convert_node1 = vxColorConvertNode(graph1, input, intermediate), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(color_convert_node2 = vxColorConvertNode(graph1, intermediate, output), VX_TYPE_NODE);

    /* Asserting for INVALID SCOPE */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_SCOPE, vxVerifyGraph(graph1));

    /* Cleanup */
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));
    VX_CALL(vxReleaseImage(&intermediate));

    VX_CALL(vxReleaseNode(&color_convert_node1));
    VX_CALL(vxReleaseNode(&color_convert_node2));

    VX_CALL(vxReleaseGraph(&graph1));
    VX_CALL(vxReleaseGraph(&graph2));
}

TESTCASE_TESTS(tivxInternalGraphVerify,
               negativeBoundaryTestVerifyGraph,
               negativeBoundaryTestOwnGraphCreateNodeCallbackCommands,
               negativeTestOwnGraphInitVirtualNode1
)