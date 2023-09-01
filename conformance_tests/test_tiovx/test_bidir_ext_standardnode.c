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
#include <VX/vx_khr_bidirectional_parameters.h>

TESTCASE(bpExtStandardNodes, CT_VXContext, ct_setup_vx_context, 0)

/* Write some data on an 8-bit image */
void bd_std_node_writeImage(vx_image image, vx_uint8 a, vx_uint8 b)
{
    vx_map_id id;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 1, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa;
    void *ptr;
    VX_CALL(vxMapImagePatch(image, &rect, 0, &id, &ipa, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa) = a;
    *(vx_uint8 *)vxFormatImagePatchAddress2d(ptr, 0, 1, &ipa) = b;
    VX_CALL(vxUnmapImagePatch(image, id));
}

/* Check data in an 8-bit image */
vx_status bd_std_node_checkImage(vx_image image, vx_uint8 a, vx_uint8 b)
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
    /* On failure return a non-zero value encoding the pixels, easily readable in decimal */
    return (aa == a) && (bb  == b) ? status : aa * 1000 + bb + 1000000;
}

/* Write some data on a 16-bit image */
void bd_std_node_writeImageS16(vx_image image, vx_int16 a, vx_int16 b)
{
    vx_map_id id;
    vx_rectangle_t rect = {.start_x = 0, .end_x = 1, .start_y = 0, .end_y = 2};
    vx_imagepatch_addressing_t ipa;
    void *ptr;
    VX_CALL(vxMapImagePatch(image, &rect, 0, &id, &ipa, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    *(vx_int16 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa) = a;
    *(vx_int16 *)vxFormatImagePatchAddress2d(ptr, 0, 1, &ipa) = b;
    VX_CALL(vxUnmapImagePatch(image, id));
}

/* Check data in an 8-bit image */
vx_status checkImageS16(vx_image image, vx_int16 a, vx_int16 b)
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
    vx_int16 aa = *(vx_int16 *)vxFormatImagePatchAddress2d(ptr, 0, 0, &ipa);
    vx_int16 bb = *(vx_int16 *)vxFormatImagePatchAddress2d(ptr, 0, 1, &ipa);
    status = vxUnmapImagePatch(image, id);
    if (status != (vx_status)VX_SUCCESS)
    {
        return status;
    }
    /* On failure output the data and return VX_FAILURE */
    if (aa == a && bb == b)
        return status;
    return VX_FAILURE;
}

vx_status releaseNode(vx_node node)
{
    return vxReleaseNode(&node);
}

void testAccumulate(vx_context context)
{
    vx_graph graph;
    vx_image in_param, accum_param;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    bd_std_node_writeImage(in_param, 0x01, 0xFF);
    bd_std_node_writeImageS16(accum_param, 0x0100, 0x7F03);
    VX_CALL(releaseNode(vxAccumulateImageNode(graph, in_param, accum_param)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImageS16(accum_param, 0x0101, 0x7FFF));
    VX_CALL(vxReleaseImage(&in_param));
    VX_CALL(vxReleaseImage(&accum_param));
    VX_CALL(vxReleaseGraph(&graph));
}

void testAccumulateBad(vx_context context)
{
    vx_graph graph;
    vx_image in_param, accum_param;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_param  = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    bd_std_node_writeImage(in_param, 0x01, 0xFF);
    bd_std_node_writeImage(accum_param, 0xFF, 0x03);
    vx_node node = vxAccumulateImageNode(graph, NULL, accum_param);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    VX_CALL(releaseNode(vxAccumulateImageNode(graph, in_param, accum_param)));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&in_param));
    VX_CALL(vxReleaseImage(&accum_param));
    VX_CALL(vxReleaseGraph(&graph));
}

void testAccumulateSquare(vx_context context)
{
    vx_graph graph;
    vx_image in_param, accum_param;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    bd_std_node_writeImage(in_param, 04, 0xFF);
    bd_std_node_writeImageS16(accum_param, 100, 0x7F03);
    VX_CALL(releaseNode(vxAccumulateSquareImageNodeX(graph, in_param, 0, accum_param)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, checkImageS16(accum_param, 116, 0x7FFF));
    VX_CALL(vxReleaseImage(&in_param));
    VX_CALL(vxReleaseImage(&accum_param));
    VX_CALL(vxReleaseGraph(&graph));
}

void testAccumulateSquareBad(vx_context context)
{
    vx_graph graph;
    vx_image in_param, accum_param;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    bd_std_node_writeImage(in_param, 0x01, 0xFF);
    bd_std_node_writeImageS16(accum_param, 0xFF, 0x03);
    vx_node node = vxAccumulateSquareImageNodeX(graph, in_param, 0, NULL);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    VX_CALL(releaseNode(vxAccumulateSquareImageNodeX(graph, in_param, 17, accum_param)));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&in_param));
    VX_CALL(vxReleaseImage(&accum_param));
    VX_CALL(vxReleaseGraph(&graph));
}

void testAccumulateWeighted(vx_context context)
{
    vx_graph graph;
    vx_image in_param, accum_param;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    bd_std_node_writeImage(in_param, 04, 100);
    bd_std_node_writeImage(accum_param, 100, 200);
    VX_CALL(releaseNode(vxAccumulateWeightedImageNodeX(graph, in_param, 0.25f, accum_param)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, bd_std_node_checkImage(accum_param, 76, 175));
    VX_CALL(vxReleaseImage(&in_param));
    VX_CALL(vxReleaseImage(&accum_param));
    VX_CALL(vxReleaseGraph(&graph));
}

void testAccumulateWeightedBad(vx_context context)
{
    vx_graph graph;
    vx_image in_param, accum_param;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(in_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(accum_param = vxCreateImage(context, 8, 8, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    bd_std_node_writeImage(in_param, 0x01, 0xFF);
    bd_std_node_writeImageS16(accum_param, 0xFF, 0x03);
    vx_node node = vxAccumulateWeightedImageNodeX(graph, in_param, 0.2f, NULL);
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)node));
    VX_CALL(releaseNode(vxAccumulateWeightedImageNodeX(graph, in_param, 1.5f, accum_param)));
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    VX_CALL(vxReleaseImage(&in_param));
    VX_CALL(vxReleaseImage(&accum_param));
    VX_CALL(vxReleaseGraph(&graph));
}

TEST(bpExtStandardNodes, testbidirAccumulatedNodeGood)
{
    vx_context context = context_->vx_context_;
    testAccumulate(context);
    testAccumulateSquare(context);
    testAccumulateWeighted(context);
}

TEST(bpExtStandardNodes, testbidirAccumulatedNodeFail)
{
    vx_context context = context_->vx_context_;
    testAccumulateBad(context);
    testAccumulateSquareBad(context);
    testAccumulateWeightedBad(context);
}

TESTCASE_TESTS(bpExtStandardNodes,
                testbidirAccumulatedNodeFail,
                testbidirAccumulatedNodeGood)