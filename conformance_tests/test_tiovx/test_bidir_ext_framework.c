/*
 * Copyright (c) 2013-2023 The Khronos Group Inc.
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
#include "test_tiovx/test_tiovx.h"

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <VX/vx_khr_pipelining.h>

enum user_library_e
{
    USER_LIBRARY_EXAMPLE = 1
};

enum user_kernel_e
{
  MY_USER_KERNEL = VX_KERNEL_BASE( VX_ID_USER, USER_LIBRARY_EXAMPLE ) + 0x001
};

TESTCASE(bpExtFramework, CT_VXContext, ct_setup_vx_context, 0)

/* Write some data on an image */
void writeImage(vx_image image)
{
    vx_map_id id;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 1, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa;
    void *ptr;
    VX_CALL(vxMapImagePatch(image, &rect, 0, &id, &ipa, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa) = 1;
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 1, &ipa) = 2;
    VX_CALL(vxUnmapImagePatch(image, id));
}

/* Check data in an image */
vx_status checkImage(vx_image image, vx_uint8 a, vx_uint8 b)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_map_id id;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 1, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa;
    void *ptr;
    status = vxMapImagePatch(image, &rect, 0, &id, &ipa, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    vx_uint8 aa = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa);
    vx_uint8 bb = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 1, &ipa);
    status = vxUnmapImagePatch(image, id);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    if (aa == a && bb == b)
    {
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

vx_status myKernelFunction(vx_node node, const vx_reference parameters[], vx_uint32 nparams)
{
    /* Implement a very simple operation on two of the pixels so that we can verify
    that things have happened as they should, in the order they should. We will perform
    these operations:
    bidir(0,0) = bidir(0, 0) + input(0,0) + 1
    bidir(0,1) = bidir(0, 1) + input(0,1) + 2
    */
    vx_status status = (vx_status)VX_SUCCESS;
    vx_image input = (vx_image)parameters[0];
    vx_image bidir = (vx_image)parameters[1];
    vx_map_id id_in, id_bid;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 1, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa_in, ipa_bid;
    void *ptr_in, *ptr_bid;
    tivxTaskWaitMsecs(100); /* Slow this kernel down so anything executing in parallel may complete */
    status = vxMapImagePatch(input, &rect, 0, &id_in, &ipa_in, &ptr_in, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    status = vxMapImagePatch(bidir, &rect, 0, &id_bid, &ipa_bid, &ptr_bid, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    vx_uint8 inpix1 = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr_in, 0, 0, &ipa_in);
    vx_uint8 inpix2 = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr_in, 0, 1, &ipa_in);
    vx_uint8 bidpix1 = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr_bid, 0, 0, &ipa_bid);
    vx_uint8 bidpix2 = *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr_bid, 0, 1, &ipa_bid);
    vx_uint8 respix1 = inpix1 + bidpix1 + 1;
    vx_uint8 respix2 = inpix2 + bidpix2 + 2;

    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr_bid, 0, 0, &ipa_bid) = respix1;
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr_bid, 0, 1, &ipa_bid) = respix2;

    status = vxUnmapImagePatch(input, id_in);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    status = vxUnmapImagePatch(bidir, id_bid);
    return status;
}

vx_status myKernelValidator(vx_node node, const vx_reference * parameters, vx_uint32 num, vx_meta_format * metas)
{
    return VX_SUCCESS;
}

/* Create a user kernel */
vx_status registerUserKernel(vx_context context)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_kernel kernel = vxAddUserKernel(context, "myUserKernel", MY_USER_KERNEL, myKernelFunction, 2, myKernelValidator, NULL, NULL);
    EXPECT_VX_REFERENCE(kernel);
    status = vxAddParameterToKernel(kernel, 0, VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    /* Check [REQ-BP01] at compile time */
    (void)VX_BIDIRECTIONAL;
    //PASS("BP01");
    /* Check [REQ-BP02] */
    status = vxAddParameterToKernel(kernel, 1, VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    status = vxFinalizeKernel(kernel);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    status = vxReleaseKernel(&kernel);
    return status;
}

vx_status unRegisterUserKernel(vx_context context)
{
    return (vxRemoveKernel(vxGetKernelByEnum(context, MY_USER_KERNEL)));
}

/* Create a simple node with a bidirectional parameter */
vx_node createUserNode(vx_graph graph, vx_image input, vx_image bidir)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_kernel kernel = vxGetKernelByEnum(context, MY_USER_KERNEL);
    EXPECT_VX_REFERENCE(kernel);
    vx_node node = vxCreateGenericNode(graph, kernel);
    EXPECT_VX_REFERENCE(node);
    vxSetParameterByIndex(node, 0, (vx_reference)input);
    vxSetParameterByIndex(node, 1, (vx_reference)bidir);
    vxReleaseKernel(&kernel);
    return node;
}

/* Function to get a parameter, add it to a graph and then release it */
static vx_status addParameterToGraph(vx_graph graph, vx_node node, vx_uint32 num)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_parameter p = vxGetParameterByIndex(node, num);
    status = vxAddParameterToGraph(graph, p);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    status = vxReleaseParameter(&p);
    return status;
}

/* Graphs that should fail verification */
/* Create a graph with a non-virtual attached to one output, two bidirectional and two inputs */
void makeGraph1O2B2I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    writeImage(image1);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    vx_node noded = vxAndNode(graph, image2, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to two bidirectional and two inputs */
void makeGraph2B2I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node noded = vxAndNode(graph, image2, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to one output, two bidirectional */
void makeGraph1O2B(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to two bidirectional */
void makeGraph2B(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a virtual attached to one output, two bidirectional and one input */
void makeGraphV1O2B1I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateVirtualImage(graph, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    vx_node noded = vxNotNode(graph, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a virtual attached to one bidirectional and one input */
void makeGraphV1B1I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateVirtualImage(graph, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node noded = vxNotNode(graph, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a virtual attached to one output, one bidirectional */
void makeGraphV1O1B(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context),  VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateVirtualImage(graph, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a virtual attached to one bidirectional */
void makeGraphV1B(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateVirtualImage(graph, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to a bidirectional and an input of the same node */
void makeGraphCycle0(vx_context context)
{
    vx_graph graph;
    vx_image image1;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image1);
    EXPECT_VX_REFERENCE(nodea);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a cycle:
      object 1 is connected as bidirectional to node A and input to node B,
      object 2 is connected as bidirectional to node B and input to node A */
void makeGraphCycle1(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image2, image1);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    writeImage(image1);
    writeImage(image2);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a cycle:
       object 1 is connected as an input to node A, as bidirectional to node B
       and the output of node A is connected as another input to node B */
void makeGraphCycle2(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image2, image1);
    EXPECT_VX_REFERENCE(nodeb);
    writeImage(image2);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a uniform image attached as a bidirectional */
void makeGraphUniform(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_pixel_value_t pixel = {.U32 = 0};
    ASSERT_VX_OBJECT(image2 = vxCreateUniformImage(context, 100, 100, VX_DF_IMAGE_U8, &pixel), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    writeImage(image1);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Graphs that should pass verification */
/* Create a graph with a non-virtual attached to one output, one bidirectional and two inputs */
void makeGraph1O1B2I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    vx_node noded = vxAndNode(graph, image2, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image3, 0, 1));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to one bidirectional and two inputs */
void makeGraph1B2I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node noded = vxAndNode(graph, image2, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    writeImage(image2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image3, 3, 6));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to one output, one bidirectional */
void makeGraph1O1B(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    writeImage(image1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image2, 0, 1));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a non-virtual attached to one bidirectional */
void makeGraph1B(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodea);
    writeImage(image1);
    writeImage(image2);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image2, 3, 6));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a virtual attached to one output, one bidirectional and one input */
void makeGraphV1O1B1I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateVirtualImage(graph, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    vx_node noded = vxNotNode(graph, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image3, 255, 254));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a graph with a virtual attached to one output, one bidirectional and three inputs */
void makeGraphV1O1B3I(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3, image4;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateVirtualImage(graph, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image4 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodea = vxOrNode(graph, image2, image2, image4);
    EXPECT_VX_REFERENCE(nodea);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    vx_node noded = vxNotNode(graph, image2, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image3, 255, 254) || checkImage(image4, 0, 1));
    VX_CALL(vxReleaseNode(&nodea));
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseImage(&image4));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a test graph and check operation, especially execution order */
void checkGraphExecution(vx_context context)
{
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    vx_node nodec = vxNotNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodec);
    vx_node noded = vxAddNode(graph, image2, image1, VX_CONVERT_POLICY_WRAP, image3);
    EXPECT_VX_REFERENCE(noded);
    writeImage(image1);
    writeImage(image2);
    writeImage(image3);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image1, 1, 2));         /* Image1 should be unchanged */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image2, 0, 1));       /* Image2 should have been written by output before it was updated */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image3, 1, 3));       /* Image2 should have been written and updated before being used as an input */
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseNode(&nodec));
    VX_CALL(vxReleaseNode(&noded));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a test graph and check operation with graph parameters */
void checkGraphParameters(vx_context context)
{
    /*
    [REQ-BP10] A bidirectional parameter may be added as a graph parameter.
    In this case the effective direction of the parameter depends upon the graph configuration and rules determining the execution order of the nodes:
        The edge is connected between an output parameter of node A, a bidirectional parameter of node B, and any number of inputs.
           In this case the graph parameter is effectively an output, becoming "written" after node B has executed.
            This is the case whether the graph parameter was created as attached to node A or to node B.
            If this is confusing, please use a Copy node for clarity, and understand the order of execution of the nodes.
        The edge is not connected to any output parameter, but to one bidirectional parameter of node A and any number of inputs to other nodes.
            In this case the graph parameter is truly bidirectional, being at first an input to the node A, and becoming "written" after node A has executed.
    */
    vx_graph graph;
    vx_image image1, image2, image3;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image1 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image2 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(image3 = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    vx_node nodeb = createUserNode(graph, image1, image2);
    EXPECT_VX_REFERENCE(nodeb);
    writeImage(image1);
    writeImage(image2);
    writeImage(image3);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, addParameterToGraph(graph, nodeb, 1));   /* Parameter 0 of the graph is the bidirectional parameter */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));                   /* Should verify */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image1, 1, 2));                 /* Image1 should be unchanged */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image2, 3, 6));                 /* Image2 should have these new values */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphParameterByIndex(graph, 0, (vx_reference)image3)); /* Replace image 2 by image 3, should be allowed */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));                    /* Should be no need to verify again */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image2, 3, 6));               /* Image2 should not have been modified this time */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(image3, 3, 6));               /* But image3 should have been modified */
    VX_CALL(vxReleaseNode(&nodeb));
    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));
    VX_CALL(vxReleaseImage(&image3));
    VX_CALL(vxReleaseGraph(&graph));
}

void checkGraphPipelining(vx_context context)
{
    /*
    [REQ-BP11] A bidirectional parameter, whether effectively an output or truly bidirectional (see above),
     will not become ready for dequeueing until all the nodes to which it is connected have executed.

     Use a graph with one graph parameter connected to a bidirectional and an input, one connected to a
     bidirectional and an output, one connected to a bidirectional, an output and an input,
     and one connected to an input. Run through a 3-deep pipeline and check the values.
     Does nothing unless the pipelining extension is present.
    */
  #ifdef OPENVX_KHR_PIPELINING
    vx_graph graph;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    typedef vx_image image3[3]; /* for clarity */
    image3 images[5];
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 3; ++j)
        {
            images[i][j] = vxCreateImage(context, 2, 2, VX_DF_IMAGE_U8);
            if (i < 3 ) writeImage(images[i][j]);
        }
    vx_node nodes[6] = {
        createUserNode(graph, images[0][0], images[1][0]),
        createUserNode(graph, images[0][0], images[2][0]),
        createUserNode(graph, images[0][0], images[3][0]),
        vxNotNode(graph, images[1][0], images[2][0]),
        vxNotNode(graph, images[0][0], images[3][0]),
        vxNotNode(graph, images[3][0], images[4][0]),
    };
    for (int i = 3; i < 6; ++i)
    {
        /* We want to test that things are executed in sequence even if parallelised */
        VX_CALL(vxSetNodeTarget(nodes[i], VX_TARGET_STRING, TIVX_TARGET_DSP1));
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, addParameterToGraph(graph, nodes[0], 0U));    /* Images[0][], Input only */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, addParameterToGraph(graph, nodes[0], 1U));    /* Images[1][], Bidirectional */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, addParameterToGraph(graph, nodes[1], 1U));    /* Images[2][], Effectively an output */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, addParameterToGraph(graph, nodes[5], 1U));    /* Images[4][], Output only */
    /* set up pipelining */
    vx_graph_parameter_queue_params_t graph_params[5] =
    {
        {.graph_parameter_index = 0U, .refs_list_size = 3U, .refs_list = (vx_reference *)&images[0][0]},
        {.graph_parameter_index = 1U, .refs_list_size = 3U, .refs_list = (vx_reference *)&images[1][0]},
        {.graph_parameter_index = 2U, .refs_list_size = 3U, .refs_list = (vx_reference *)&images[2][0]},
        {.graph_parameter_index = 3U, .refs_list_size = 3U, .refs_list = (vx_reference *)&images[4][0]}
    };
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph, VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO, 4, graph_params));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    /* At this point we want to output the graph information */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, tivxExportGraphToDot(graph, "./", "BP11"));

    /* Initial pixels values for all images are (1,2) here we calculate what they should be after execution */
    vx_uint8 pixels[4][2] =
    {
        {1, 2},
        {3, 6},
        {0xFE, 0xFD},
        {0xFF, 0xFE}
    };

    for (int j = 0; j < 4; ++j) {
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, j, graph_params[j].refs_list, 3));
    }

    for(int i = 0; i < 3; ++i)
    {
        vx_image params[5];
        char label[8];
        for (int j = 0; j < 4; ++j)
        {
            vx_uint32 num_refs = 0;
            vx_uint32 timer = 200;
            while (0 == num_refs && --timer > 0)
            {
                if (0 == num_refs)
                    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterCheckDoneRef(graph, j, &num_refs));
                if (num_refs)
                {
                    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, j, (vx_reference *)&params[j], 1, &num_refs));
                    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(params[j], pixels[j][0], pixels[j][1]));
                    break;
                }
                else
                    tivxTaskWaitMsecs(100);
            }
            if (0 == timer)
            {
                FAIL(label);
            }
        }
    }

    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 3; ++j)
            VX_CALL(vxReleaseImage(&images[i][j]));
    for (int i = 0; i < 6; ++i)
        VX_CALL(vxReleaseNode(&nodes[i]));
    VX_CALL(vxReleaseGraph(&graph));
  #else
    printf("Pipelining is not implemented\n");
  #endif
}

/* Check replication works with object array, and that it fails unless bidirectional parameters are replicated */
void checkGraphReplicationArray(vx_context context)
{
    vx_graph graph;
    vx_image image, input, bidir0, bidir1;
    vx_object_array inp_array, bid_array;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(image = vxCreateImage(context, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(inp_array = vxCreateObjectArray(context, (vx_reference)image, 3), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(bid_array = vxCreateObjectArray(context, (vx_reference)image, 3), VX_TYPE_OBJECT_ARRAY);
    VX_CALL(vxReleaseImage(&image));
    input = (vx_image)vxGetObjectArrayItem(inp_array, 0);
    bidir0 = (vx_image)vxGetObjectArrayItem(bid_array, 0);
    bidir1 = (vx_image)vxGetObjectArrayItem(bid_array, 1);
    vx_node node = createUserNode(graph, input, bidir0);
    EXPECT_VX_REFERENCE(node);
    vx_bool replicate[2] = {vx_false_e, vx_false_e};
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, node, replicate, 2));
    replicate[1] = vx_true_e;
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, node, replicate, 2));
    writeImage(input);
    writeImage(bidir0);
    writeImage(bidir1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(bidir0, 3, 6));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(bidir1, 3, 6));
    VX_CALL(vxRemoveNode(&node));
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&bidir0));
    VX_CALL(vxReleaseImage(&bidir1));
    VX_CALL(vxReleaseObjectArray(&inp_array));
    VX_CALL(vxReleaseObjectArray(&bid_array));
    VX_CALL(vxReleaseGraph(&graph));
}

/* Check replication works with pyramids */
void checkGraphReplicationPyramid(vx_context context)
{
    vx_graph graph;
    vx_pyramid inp_pyramid, bid_pyramid;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(inp_pyramid = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(bid_pyramid = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 100, 100, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    vx_image input0 = vxGetPyramidLevel(inp_pyramid, 0);
    vx_image input1 = vxGetPyramidLevel(inp_pyramid, 1);
    vx_image bidir0 = vxGetPyramidLevel(bid_pyramid, 0);
    vx_image bidir1 = vxGetPyramidLevel(bid_pyramid, 1);
    vx_node node = createUserNode(graph, input0, bidir0);
    EXPECT_VX_REFERENCE(node);
    vx_bool replicate[2] = {vx_true_e, vx_true_e};
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReplicateNode(graph, node, replicate, 2));
    writeImage(input0);
    writeImage(input1);
    writeImage(bidir0);
    writeImage(bidir1);
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(bidir0, 3, 6));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImage(bidir1, 3, 6));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleasePyramid(&inp_pyramid));
    VX_CALL(vxReleasePyramid(&bid_pyramid));
    VX_CALL(vxReleaseImage(&input0));
    VX_CALL(vxReleaseImage(&bidir0));
    VX_CALL(vxReleaseImage(&input1));
    VX_CALL(vxReleaseImage(&bidir1));
    VX_CALL(vxReleaseGraph(&graph));
}


TEST(bpExtFramework, testGraphbidirFail)
{
    /* Graphs that should fail */
    vx_context context = context_->vx_context_;
    registerUserKernel(context);
    makeGraph1O2B2I(context);           /* BP03 */
    makeGraph2B2I(context);
    makeGraph1O2B(context);
    makeGraph2B(context);
    makeGraphV1O2B1I(context);          /* BP04 */
    makeGraphV1B1I(context);
    makeGraphV1O1B(context);
    makeGraphV1B(context);
    makeGraphCycle0(context);           /* BP05 */
    makeGraphUniform(context);          /* BP06 */
    makeGraphCycle1(context);           /* BP09 */
    makeGraphCycle2(context);           /* BP09 */
    unRegisterUserKernel(context);
}

TEST(bpExtFramework, testGraphbidirSuccess)
{
    vx_context context = context_->vx_context_;
    registerUserKernel(context);
    makeGraph1O1B2I(context);           /* BP03 */
    makeGraph1B2I(context);
    makeGraph1O1B(context);
    makeGraph1B(context);
    makeGraphV1O1B1I(context);          /* BP04 */
    makeGraphV1O1B3I(context);
    unRegisterUserKernel(context);
}

TEST(bpExtFramework, testGraphbidirExOrder)
{
    vx_context context = context_->vx_context_;
    registerUserKernel(context);
    checkGraphExecution(context);       /* BP07, BO08 */
    unRegisterUserKernel(context);
}

TEST(bpExtFramework, testGraphbidirGraphParam)
{
    vx_context context = context_->vx_context_;
    registerUserKernel(context);
    checkGraphParameters(context);      /* BP10 */
    unRegisterUserKernel(context);
}

TEST(bpExtFramework, testGraphbidirReplicate)
{
    vx_context context = context_->vx_context_;
    registerUserKernel(context);
    checkGraphReplicationArray(context);    /* BP12 a*/
    checkGraphReplicationPyramid(context);  /* BP12 p*/
    unRegisterUserKernel(context);
}

TEST(bpExtFramework, testGraphbidirPipelining)
{
    vx_context context = context_->vx_context_;
    registerUserKernel(context);
    checkGraphPipelining(context);      /* BP11 */
    unRegisterUserKernel(context);
}

TESTCASE_TESTS(bpExtFramework,
        testGraphbidirFail,
        testGraphbidirSuccess,
        testGraphbidirExOrder,
        testGraphbidirGraphParam,
        testGraphbidirReplicate,
        testGraphbidirPipelining)