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

#include "test_engine/test.h"
#include "test_tiovx/test_tiovx.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>
#include <test_utils_mem_operations.h>

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
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_YUV4), VX_TYPE_IMAGE);

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
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_YUV4), VX_TYPE_IMAGE);

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
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_YUV4), VX_TYPE_IMAGE);

    /* Creating virtual image with the graph2 scope */
    intermediate = vxCreateVirtualImage (graph2, width, height, VX_DF_IMAGE_YUV4);

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
    virt_image = vxCreateVirtualImage (graph, width, height, VX_DF_IMAGE_YUV4);

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
    virt_image = vxCreateVirtualImage (graph, width, height, VX_DF_IMAGE_YUV4);

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

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* Calling vxVerifyGraph to call the ownGraphInitVirtualNode */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_VALUE, vxVerifyGraph(graph));

    /* Cleanup */
    VX_CALL(vxReleasePyramid(&pyr_virt));
    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

/* Test to hit negative portion of ownGraphInitVirtualNode */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphInitVirtualNode5)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_node n;
    vx_scalar scalar;

    tivxTestKernelsLoadKernels(context);

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating a scalar object */
    vx_uint8  scalar_val = 0;
    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(n = tivxScalarSourceNode(graph, scalar), VX_TYPE_NODE);

    /* Tampering the reference param is_virtual */
    vx_reference ref = &(scalar->base);
    vx_bool is_virtual = ref->is_virtual;
    ref->is_virtual = true;

    /* Calling vxVerifyGraph to call the ownGraphInitVirtualNode */
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_TYPE, vxVerifyGraph(graph));

    /* Resetting is_virtual param to original value */
    ref->is_virtual = is_virtual;

    /* Cleanup */
    VX_CALL(vxReleaseScalar(&scalar));
    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

/* Test to hit negative portion of ownGraphInitVirtualNode */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphInitVirtualNode6)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 10, height = 10;
    vx_node n;
    vx_pyramid pyr_virt, pyr_in;

    tivxTestKernelsLoadKernels(context);

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating a virtual pyramid */
    ASSERT_VX_OBJECT(pyr_in = vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    /* Creating a tivxPyramidSourceNode(the actual node is irrelevant)
    with the virtual pyramid object */
    ASSERT_VX_OBJECT(n = tivxPyramidSourceNode(graph, pyr_in), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    /* Tampering is_virtual parameter of pyramid object */
    vx_reference ref = &(pyr_in->base);
    vx_bool is_virtual = ref->is_virtual;
    ref->is_virtual = true;

    tivx_obj_desc_pyramid_t *obj_desc = NULL;
    obj_desc = (tivx_obj_desc_pyramid_t *)pyr_in->base.obj_desc;

    vx_uint32 num_levels = obj_desc->num_levels;
    obj_desc->num_levels = 0;

    /* Calling vxVerifyGraph to call the ownGraphInitVirtualNode */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    obj_desc->num_levels = num_levels;
    ref->is_virtual = is_virtual;

    /* This verifyGraph should pass, as tampered variables are reset with the original value */
    VX_CALL(vxVerifyGraph(graph));


    /* Cleanup */
    VX_CALL(vxReleasePyramid(&pyr_in));
    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

/* Test to hit negative portion of ownGraphNodeKernelInit */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphNodeKernelInit)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 10, height = 10, i = 0, j = 0;
    vx_node n;
    vx_pyramid pyr_in;

    tivxTestKernelsLoadKernels(context);

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    /* Creating a pyramid */
    ASSERT_VX_OBJECT(pyr_in = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);


    /* Creating a tivxPyramidSourceNode(the actual node is irrelevant) */
    ASSERT_VX_OBJECT(n = tivxPyramidSourceNode(graph, pyr_in), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    tivx_obj_desc_kernel_name_t* kernel_name_obj_desc[TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST] = {NULL};


    for (i=0; i < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST; i++)
    {
        kernel_name_obj_desc[i] = (tivx_obj_desc_kernel_name_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_KERNEL_NAME, NULL);
        if (kernel_name_obj_desc[i] == NULL)
        {
            break;
        }
    }

    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

    vx_status status = VX_FAILURE;

    /* Releasing Descriptors one by one and trying to Verify the Graph */
    for (j = 0; j < i ; j++)
    {
        VX_CALL(ownObjDescFree((tivx_obj_desc_t **) &kernel_name_obj_desc[j]));
        if (status != VX_SUCCESS)
        {
            status = vxVerifyGraph(graph);
        }

    }

    /* This verifyGraph should pass, as the allocated memory has been freed */
    VX_CALL(vxVerifyGraph(graph));

    /* Cleanup */
    VX_CALL(vxReleasePyramid(&pyr_in));
    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);
}

/* Test to hit negative portion of ownGraphAddSingleDataReference */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphAddSingleDataReference)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 640, height = 480, i = 0, j = 0;
    vx_node n_in[TIVX_NODE_MAX_IN_NODES+1];
    vx_node color_convert_node;

    vx_image input, output;

    tivx_shared_mem_info_t *shared_mem_info_array;
    uint32_t num_chunks;
    vx_enum mheap_region = TIVX_MEM_EXTERNAL;
    vx_status status = VX_SUCCESS;

    tivxTestKernelsLoadKernels(context);

    /* Creating Graph */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(input = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output = vxCreateImage(context, width, height, VX_DF_IMAGE_YUV4), VX_TYPE_IMAGE);



    /* Allocating all the memory under heap region TIVX_MEM_EXTERNAL */
    VX_CALL(test_utils_max_out_heap_mem(&shared_mem_info_array, &num_chunks, mheap_region));

    ASSERT_VX_OBJECT(color_convert_node = vxColorConvertNode(graph, input, output), VX_TYPE_NODE);

    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxVerifyGraph(graph));

    /* Freeing all the previously allocated memory */
    VX_CALL(test_utils_release_maxed_out_heap_mem(shared_mem_info_array, num_chunks));


    /* Cleanup */
    VX_CALL(vxReleaseImage(&input));
    VX_CALL(vxReleaseImage(&output));
    VX_CALL(vxReleaseNode(&color_convert_node));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);

}

/* Test to hit negative portions of ownGraphAllocateDataObject */
TEST(tivxInternalGraphVerify, negativeTestOwnGraphAllocateDataObject)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 640, height = 480, i = 0, j = 0;
    vx_node n;
    vx_delay delay;
    vx_pyramid pyr;

    tivx_shared_mem_info_t *shared_mem_info_array;
    uint32_t num_chunks;
    vx_enum mheap_region = TIVX_MEM_EXTERNAL;
    vx_status status = VX_SUCCESS;


    tivxTestKernelsLoadKernels(context);

    /* Creating Graph along with  pyramid and delay objects */
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(delay = vxCreateDelay(context, (vx_reference)(pyr), 2), VX_TYPE_DELAY);

    /* Allocating all the memory under heap region TIVX_MEM_EXTERNAL */
    VX_CALL(test_utils_max_out_heap_mem(&shared_mem_info_array, &num_chunks, mheap_region));

    ASSERT_VX_OBJECT(n = tivxPyramidIntermediateNode(graph, pyr,
                          (vx_pyramid)vxGetReferenceFromDelay(delay, -1)), VX_TYPE_NODE);

    #if defined(SOC_AM62A)
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU1_0));
    #else
    VX_CALL(vxSetNodeTarget(n, VX_TARGET_STRING, TIVX_TARGET_MCU2_0));
    #endif

    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_MEMORY, vxVerifyGraph(graph));

    status = VX_FAILURE;

    /* Freeing all the previously allocated memory */
    while (test_utils_single_release_heap_mem(&shared_mem_info_array, &num_chunks) == VX_SUCCESS && shared_mem_info_array != NULL)
    {
        if (status != VX_SUCCESS)
        {
            status = vxVerifyGraph(graph);
        }
    }

    /* Cleanup */
    VX_CALL(vxReleaseDelay(&delay));
    VX_CALL(vxReleasePyramid(&pyr));
    VX_CALL(vxReleaseNode(&n));
    VX_CALL(vxReleaseGraph(&graph));

    tivxTestKernelsUnLoadKernels(context);

}

/* Test to hit one branch of ownGraphCheckIsRefMatch */
TEST(tivxInternalGraphVerify, testOwnGraphCheckIsRefMatch)
{
    vx_context context = context_->vx_context_;
    vx_graph graph;
    uint32_t width = 640, height = 480, i = 0, j = 0;
    vx_image image1, image2, parent2;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    image1 = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);
    image2 = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);

    parent2 = image2->parent;

    /* Tampering the image2 parent and setting it to image1 */
    image2->parent = (vx_image)image1;

    /* Expecting true from ownGraphCheckIsRefMatch() */
    ASSERT_EQ_VX_STATUS(((ownGraphCheckIsRefMatch(graph, vxCastRefFromImage(image1), vxCastRefFromImage(image2)) == (vx_bool)vx_true_e)? VX_SUCCESS : VX_FAILURE), VX_SUCCESS);

    /* Resetting the image2 parent */
    image2->parent = parent2;

    /* Expecting false from ownGraphCheckIsRefMatch() */
    ASSERT_EQ_VX_STATUS(((ownGraphCheckIsRefMatch(graph, vxCastRefFromImage(image1), vxCastRefFromImage(image2)) == (vx_bool)vx_true_e)? VX_SUCCESS : VX_FAILURE), VX_FAILURE);


    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));

    VX_CALL(vxReleaseGraph(&graph));
}

/* Create a user kernel which returns failure on the deinitialize calls (ownGraphNodeKernelDeinit) */
typedef enum _own_params_e
{
    OWN_PARAM_INPUT = 0,
    OWN_PARAM_OUTPUT,
} own_params_e;

static vx_size local_size = 0;
static vx_bool is_kernel_alloc = vx_false_e;
static vx_size local_size_auto_alloc = 0;
static vx_size local_size_kernel_alloc = 0;

static enum vx_type_e type = VX_TYPE_INVALID;
static enum vx_type_e objarray_itemtype = VX_TYPE_INVALID;

static vx_status set_local_size_status_init = VX_SUCCESS;
static vx_status set_local_ptr_status_init = VX_SUCCESS;

static vx_status query_local_size_status_deinit = VX_SUCCESS;
static vx_status query_local_ptr_status_deinit = VX_SUCCESS;
static vx_status set_local_size_status_deinit = VX_SUCCESS;
static vx_status set_local_ptr_status_deinit = VX_SUCCESS;

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"

static vx_status VX_CALLBACK own_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

static vx_bool is_initialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

static vx_bool is_deinitialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    /* Returning FAILURE instead of SUCCESS */
    return VX_FAILURE;
}

static void own_register_kernel(vx_context context)
{
    vx_kernel kernel = 0;
    vx_size size = local_size_auto_alloc;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
        own_Kernel,
        2,
        NULL,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxSetKernelAttribute(kernel, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(size)));
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}

TEST(tivxInternalGraphVerify, negativeTestUserKernelDeinitializeFailure)
{
    vx_context context = context_->vx_context_;
    vx_reference exemplar = 0, src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    type = VX_TYPE_OBJECT_ARRAY;


    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;

    vx_uint32 width = 128, height = 128;
    vx_enum format = VX_DF_IMAGE_U8;
    vx_size capacity = 20;

    ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateImage(context, width, height, format), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src = (vx_reference)vxCreateObjectArray(context, exemplar, capacity), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateObjectArray(context, exemplar, capacity), VX_TYPE_OBJECT_ARRAY);

    ASSERT_NO_FAILURE(own_register_kernel(context));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    VX_CALL(vxVerifyGraph(graph));

    /* Calling vxVerifyGraph again to make first_time_verify false */
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    ASSERT_EQ_VX_STATUS(VX_FAILURE, vxReleaseGraph(&graph));

    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));
    VX_CALL(vxReleaseReference(&exemplar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst == 0);
    ASSERT(src == 0);
    ASSERT(exemplar == 0);
}

TESTCASE_TESTS(tivxInternalGraphVerify,
               negativeBoundaryTestVerifyGraph,
               negativeBoundaryTestOwnGraphCreateNodeCallbackCommands,
               negativeTestOwnGraphInitVirtualNode1,
               negativeTestOwnGraphInitVirtualNode2,
               negativeTestOwnGraphInitVirtualNode3,
               negativeTestOwnGraphInitVirtualNode4,
               negativeTestOwnGraphInitVirtualNode5,
               negativeTestOwnGraphInitVirtualNode6,
               negativeTestOwnGraphNodeKernelInit,
               negativeTestOwnGraphAddSingleDataReference,
               negativeTestOwnGraphAllocateDataObject,
               testOwnGraphCheckIsRefMatch,
               negativeTestUserKernelDeinitializeFailure
)