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
#include <TI/tivx_test_kernels.h>

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

    /* negative test for the graph parameters */
    /*link twice the same graph parameter */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(color_convert_node = vxColorConvertNode(graph, input, output), VX_TYPE_NODE);
    vx_parameter p = vxGetParameterByIndex(color_convert_node, 0);
    vx_parameter p1 = vxGetParameterByIndex(color_convert_node, 0);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxAddParameterToGraph(graph, p));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxAddParameterToGraph(graph, p1));
    vxReleaseParameter(&p);
    vxReleaseParameter(&p1);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));
    VX_CALL(vxReleaseNode(&color_convert_node));
    VX_CALL(vxReleaseGraph(&graph));

    /* Maxing out the number of references */
    vx_uint16 numberOfnodes = TIVX_GRAPH_MAX_PARAM_REFS+2;
    vx_node node[numberOfnodes];
    vx_image imageOut[numberOfnodes];
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
    for (uint16_t i = 0; i < numberOfnodes; i++)
    {
        ASSERT_VX_OBJECT(imageOut[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(node[i]= vxColorConvertNode(graph, input, imageOut[i]), VX_TYPE_NODE);
    }
    p = vxGetParameterByIndex(node[0], 0);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxAddParameterToGraph(graph, p));
    vxReleaseParameter(&p);
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxVerifyGraph(graph));
    for (uint16_t i = 0; i < numberOfnodes; i++)
    {
        VX_CALL(vxReleaseImage(&imageOut[i]));
        VX_CALL(vxReleaseNode(&node[i]));
    }
    VX_CALL(vxReleaseImage(&input));
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

/* Test to hit negative portion of ownGraphInitVirtualNode */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphInitVirtualNode2)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 640, height = 480;
    vx_image input, virt_image;
    vx_node n1;

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating input image */
    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    /* Creating a virtual image */
    virt_image = vxCreateVirtualImage (graph, width, height, VX_DF_IMAGE_YUYV);

    /* Forcefully setting the input image width to 0 */
    ((tivx_obj_desc_image_t *)input->base.obj_desc)-> width = 0;

    /* Creating a color convert nodes (the actual node is irrelevant) */
    ASSERT_VX_OBJECT(n1 = vxColorConvertNode(graph, input, virt_image), VX_TYPE_NODE);

    /* Asserting for INVALID SCOPE */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxVerifyGraph(graph));

    /* Cleanup */
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&virt_image));

    VX_CALL(vxReleaseNode(&n1));

    VX_CALL(vxReleaseGraph(&graph));
}

/* Test to hit negative portion of ownGraphInitVirtualNode */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphInitVirtualNode3)
{
     vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 640, height = 480;
    vx_image input, virt_image;
    vx_node n1;

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating input image */
    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    /* Creating a virtual image */
    virt_image = vxCreateVirtualImage (graph, width, height, VX_DF_IMAGE_YUYV);

    /* Forcefully setting the input image height to 0 */
    ((tivx_obj_desc_image_t *)input->base.obj_desc)-> height = 0;

    /* Creating a color convert nodes (the actual node is irrelevant) */
    ASSERT_VX_OBJECT(n1 = vxColorConvertNode(graph, input, virt_image), VX_TYPE_NODE);

    /* Asserting for INVALID SCOPE */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxVerifyGraph(graph));

    /* Cleanup */
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&virt_image));

    VX_CALL(vxReleaseNode(&n1));

    VX_CALL(vxReleaseGraph(&graph));
}

/* Test to hit negative portion of ownGraphInitVirtualNode */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphInitVirtualNode4)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 16, height = 16;
    vx_node n;
    vx_pyramid pyr_virt;

    tivxTestKernelsLoadKernels(context);

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating a virtual pyramid */
    ASSERT_VX_OBJECT(pyr_virt = vxCreateVirtualPyramid(graph, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    /* Creating a tivxPyramidSourceNode(the actual node is irrelevant)
    with the virtual pyramid object */
    ASSERT_VX_OBJECT(n = tivxPyramidSourceNode(graph, pyr_virt), VX_TYPE_NODE);

    /* Calling vxVerifyGraph to call the ownGraphInitVirtualNode */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxVerifyGraph(graph));

    /* Cleanup */
    VX_CALL(vxReleasePyramid(&pyr_virt));
    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

TESTCASE_TESTS(tivxInternalGraphVerify,
               negativeBoundaryTestVerifyGraph,
               negativeBoundaryTestOwnGraphCreateNodeCallbackCommands,
               negativeTestOwnGraphInitVirtualNode1,
               negativeTestOwnGraphInitVirtualNode2,
               negativeTestOwnGraphInitVirtualNode3,
               negativeTestOwnGraphInitVirtualNode4
)