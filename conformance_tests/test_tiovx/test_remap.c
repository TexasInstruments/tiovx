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



#include "test_tiovx.h"

#include <VX/vx.h>

#include <math.h>
#include <float.h>

#ifndef M_PI
#define M_PIF   3.14159265358979323846f
#else
#define M_PIF   (vx_float32)M_PI
#endif


TESTCASE(tivxRemap, CT_VXContext, ct_setup_vx_context, 0)

#define SRC_WIDTH       128
#define SRC_HEIGHT      128

#define VX_MAP_IDENT         0
#define VX_MAP_SCALE         1
#define VX_MAP_SCALE_ROTATE  2
#define VX_MAP_RANDOM        3

#define VX_NN_AREA_SIZE         1.5
#define VX_BILINEAR_TOLERANCE   1

static CT_Image remap_read_image_8u(const char* fileName, int width, int height)
{
    CT_Image image = NULL;

    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);

    return image;
}

static CT_Image remap_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);

static vx_remap remap_generate_map(vx_context context, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_uint32 i;
    vx_uint32 j;
    vx_float32 x;
    vx_float32 y;
    vx_remap map = 0;
    vx_status status;

    map = vxCreateRemap(context, src_width, src_height, dst_width, dst_height);
    if (vxGetStatus((vx_reference)map) == VX_SUCCESS)
    {
        vx_float32 mat[3][2];
        vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
        if (VX_MAP_IDENT == type)
        {
            mat[0][0] = 1.f;
            mat[0][1] = 0.f;

            mat[1][0] = 0.f;
            mat[1][1] = 1.f;

            mat[2][0] = 0.f;
            mat[2][1] = 0.f;
        }
        else if (VX_MAP_SCALE == type)
        {
            scale_x = src_width  / (vx_float32)dst_width;
            scale_y = src_height / (vx_float32)dst_height;

            mat[0][0] = scale_x;
            mat[0][1] = 0.f;

            mat[1][0] = 0.f;
            mat[1][1] = scale_y;

            mat[2][0] = 0.f;
            mat[2][1] = 0.f;
        }
        else if (VX_MAP_SCALE_ROTATE == type)
        {
            angle = M_PIF / RND_FLT(3.f, 6.f);
            scale_x = src_width  / (vx_float32)dst_width;
            scale_y = src_height / (vx_float32)dst_height;
            cos_a = cosf(angle);
            sin_a = sinf(angle);

            mat[0][0] = cos_a * scale_x;
            mat[0][1] = sin_a * scale_y;

            mat[1][0] = -sin_a * scale_x;
            mat[1][1] = cos_a  * scale_y;

            mat[2][0] = 0.f;
            mat[2][1] = 0.f;
        }
        else
        {
            angle = M_PIF / RND_FLT(3.f, 6.f);
            scale_x = src_width / (vx_float32)dst_width;
            scale_y = src_height / (vx_float32)dst_height;
            cos_a = cosf(angle);
            sin_a = sinf(angle);

            mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
            mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);

            mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
            mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);

            mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
            mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
        }

        for (i = 0; i < (vx_uint32)dst_height; i++)
        {
            for (j = 0; j < (vx_uint32)dst_width; j++)
            {
                x = j * mat[0][0] + i * mat[1][0] + mat[2][0];
                y = j * mat[0][1] + i * mat[1][1] + mat[2][1];
                status = vxSetRemapPoint(map, j, i, x, y);
                if (VX_SUCCESS != status)
                    return 0;
            }
        }
    }

    return map;
}

static int remap_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_remap map, vx_bool do_report)
{
    vx_float32 _x0;
    vx_float32 _y0;
    vx_float64 x0, y0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    VX_CALL_RET(vxGetRemapPoint(map, x, y, &_x0, &_y0));

    x0 = (vx_float64)_x0;
    y0 = (vx_float64)_y0;

    if (VX_INTERPOLATION_NEAREST_NEIGHBOR == interp_type)
    {
        for (yi = (vx_int32)ceil(y0 - VX_NN_AREA_SIZE); (vx_float64)yi <= y0 + VX_NN_AREA_SIZE; yi++)
        {
            for (xi = (vx_int32)ceil(x0 - VX_NN_AREA_SIZE); (vx_float64)xi <= x0 + VX_NN_AREA_SIZE; xi++)
            {
                if (0 <= xi && 0 <= yi && xi < (vx_int32)input->width && yi < (vx_int32)input->height)
                {
                    candidate = *CT_IMAGE_DATA_PTR_8U(input, xi, yi);
                }
                else if (VX_BORDER_CONSTANT == border.mode)
                {
                    candidate = border.constant_value.U8;
                }
                else if (VX_BORDER_REPLICATE == border.mode)
                {
                    candidate = CT_IMAGE_DATA_REPLICATE_8U(input, xi, yi);
                }
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        if (do_report)
            CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
        else
            return 1;
    }
    else if (VX_INTERPOLATION_BILINEAR == interp_type)
    {
        xlower = floor(x0);
        ylower = floor(y0);

        s = x0 - xlower;
        t = y0 - ylower;

        xi = (vx_int32)xlower;
        yi = (vx_int32)ylower;

        candidate = -1;
        if (VX_BORDER_UNDEFINED == border.mode)
        {
            if (xi >= 0 && yi >= 0 && xi < (vx_int32)input->width - 1 && yi < (vx_int32)input->height - 1)
            {
                candidate = (int)((1. - s) * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi    ) +
                                        s  * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi    ) +
                                  (1. - s) *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi + 1) +
                                        s  *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi + 1));
            }
        }
        else if (VX_BORDER_CONSTANT == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float64)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float64)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float64)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float64)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        else if (VX_BORDER_REPLICATE == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float64)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi    ) +
                                    s  * (1. - t) * (vx_float64)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi    ) +
                              (1. - s) *       t  * (vx_float64)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi + 1) +
                                    s  *       t  * (vx_float64)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi + 1));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    if (do_report)
        CT_FAIL_(return 1, "Interpolation type undefined");
    else
        return 1;
}

static void remap_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_remap map)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += remap_check_pixel(input, output, x, y, interp_type, border, map, vx_true_e));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static vx_bool remap_is_equal(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_remap map)
{
    CT_FILL_IMAGE_8U(, output,
            {
                if (0 != remap_check_pixel(input, output, x, y, interp_type, border, map, vx_false_e))
                    return vx_false_e;
            });
    return vx_true_e;
}

static void remap_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_remap map)
{
    ASSERT(input && output);
    ASSERT( (interp_type == VX_INTERPOLATION_NEAREST_NEIGHBOR) ||
            (interp_type == VX_INTERPOLATION_BILINEAR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT) );

    remap_validate(input, output, interp_type, border, map);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
    }
}

static void remap_check_param(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_remap map)
{
    ASSERT(input && output);
    ASSERT( (interp_type == VX_INTERPOLATION_NEAREST_NEIGHBOR) ||
            (interp_type == VX_INTERPOLATION_BILINEAR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT) ||
            (border.mode == VX_BORDER_REPLICATE) );
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char*      fileName;
    int width, height;
    vx_border_t border;
    vx_enum border_policy;
    vx_enum interp_type;
    int map_type;
} Arg;

#define ADD_VX_BORDERS_REMAP_FULL(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=1", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 1 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=127", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 127 }} }))

#define ADD_VX_BORDERS_REMAP_SMALL(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} }))

#define ADD_VX_BORDERS_NO_POLICY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "", __VA_ARGS__, (vx_enum)0))

#define ADD_VX_INTERP_TYPE_REMAP(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_NEAREST_NEIGHBOR", __VA_ARGS__, VX_INTERPOLATION_NEAREST_NEIGHBOR))

#define ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_NEAREST_NEIGHBOR", __VA_ARGS__, VX_INTERPOLATION_NEAREST_NEIGHBOR))

#define ADD_VX_MAP_PARAM_REMAP_FULL(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_MAP_RANDOM", __VA_ARGS__,       VX_MAP_RANDOM))

#define REMAP_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_18x18, ADD_VX_BORDERS_REMAP_FULL, ADD_VX_BORDERS_NO_POLICY, ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, ADD_VX_MAP_PARAM_REMAP_FULL, ARG, remap_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_644x258, ADD_VX_BORDERS_REMAP_FULL, ADD_VX_BORDERS_NO_POLICY, ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, ADD_VX_MAP_PARAM_REMAP_FULL, ARG, remap_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_1600x1200, ADD_VX_BORDERS_REMAP_FULL, ADD_VX_BORDERS_NO_POLICY, ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, ADD_VX_MAP_PARAM_REMAP_FULL, ARG, remap_generate_random, NULL)

TEST_WITH_ARG(tivxRemap, testGraphProcessing, Arg,
    REMAP_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_node node3 = 0, node4 = 0;
    vx_image input_image = 0, output_image = 0, virt;
    vx_image input_image2 = 0, output_image2 = 0, int_image;
    vx_remap map1 = 0, map2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    CT_Image input = NULL, int_ctimage = NULL, output1 = NULL, output2 = NULL;

    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, SRC_WIDTH, SRC_HEIGHT));
    ASSERT_NO_FAILURE(output1 = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(output2 = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image2 = ct_image_to_vx_image(output2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(map1 = remap_generate_map(context, input->width, input->height, arg_->width, arg_->height, arg_->map_type), VX_TYPE_REMAP);
    ASSERT_VX_OBJECT(map2 = remap_generate_map(context, arg_->width, arg_->height, 16, 16, arg_->map_type), VX_TYPE_REMAP);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(int_image   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxRemapNode(graph, input_image, map1, arg_->interp_type, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxRemapNode(graph, virt, map2, arg_->interp_type, output_image), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxRemapNode(graph, input_image2, map1, arg_->interp_type, int_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxRemapNode(graph, int_image, map2, arg_->interp_type, output_image2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxGetValidRegionImage(input_image, &src_rect);
    vxGetValidRegionImage(output_image2, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), SRC_WIDTH);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), SRC_HEIGHT);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), 16);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), 16);

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_true_e);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(int_ctimage = ct_image_from_vx_image(int_image));
    ASSERT_NO_FAILURE(output1 = ct_image_from_vx_image(output_image));
    ASSERT_NO_FAILURE(output2 = ct_image_from_vx_image(output_image2));
    ASSERT_NO_FAILURE(remap_check(input, int_ctimage, arg_->interp_type, arg_->border, map1));
    ASSERT_NO_FAILURE(remap_check(int_ctimage, output1, arg_->interp_type, arg_->border, map2));
    ASSERT_NO_FAILURE(remap_check(int_ctimage, output2, arg_->interp_type, arg_->border, map2));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseRemap(&map1));
    VX_CALL(vxReleaseRemap(&map2));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&int_image));
    VX_CALL(vxReleaseImage(&output_image));
    VX_CALL(vxReleaseImage(&output_image2));
    VX_CALL(vxReleaseImage(&input_image));
    VX_CALL(vxReleaseImage(&input_image2));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);
    ASSERT(map1 == 0);
    ASSERT(map2 == 0);
    ASSERT(output_image == 0);
    ASSERT(output_image2 == 0);
    ASSERT(input_image == 0);
    ASSERT(input_image2 == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define NEGATIVE_REMAP_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_18x18, ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ADD_VX_BORDERS_NO_POLICY, ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, ADD_VX_MAP_PARAM_REMAP_FULL, ARG, remap_generate_random, NULL)
;
TEST_WITH_ARG(tivxRemap, negativeTestBorderMode, Arg,
    NEGATIVE_REMAP_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_node node3 = 0, node4 = 0;
    vx_image input_image = 0, output_image = 0, virt;
    vx_image input_image2 = 0, output_image2 = 0, int_image;
    vx_remap map1 = 0, map2 = 0;

    CT_Image input = NULL, int_ctimage = NULL, output1 = NULL, output2 = NULL;

    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, SRC_WIDTH, SRC_HEIGHT));
    ASSERT_NO_FAILURE(output1 = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(output2 = ct_allocate_image(16, 16, VX_DF_IMAGE_U8));

    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_image2 = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(output_image2 = ct_image_to_vx_image(output2, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(map1 = remap_generate_map(context, input->width, input->height, arg_->width, arg_->height, arg_->map_type), VX_TYPE_REMAP);
    ASSERT_VX_OBJECT(map2 = remap_generate_map(context, arg_->width, arg_->height, 16, 16, arg_->map_type), VX_TYPE_REMAP);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt   = vxCreateVirtualImage(graph, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(int_image   = vxCreateImage(context, arg_->width, arg_->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(node1 = vxRemapNode(graph, input_image, map1, arg_->interp_type, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxRemapNode(graph, virt, map2, arg_->interp_type, output_image), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxRemapNode(graph, input_image2, map1, arg_->interp_type, int_image), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxRemapNode(graph, int_image, map2, arg_->interp_type, output_image2), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node2, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node3, VX_NODE_BORDER, &border, sizeof(border)));
    VX_CALL(vxSetNodeAttribute(node4, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);;

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseRemap(&map1));
    VX_CALL(vxReleaseRemap(&map2));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&int_image));
    VX_CALL(vxReleaseImage(&output_image));
    VX_CALL(vxReleaseImage(&output_image2));
    VX_CALL(vxReleaseImage(&input_image));
    VX_CALL(vxReleaseImage(&input_image2));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(graph == 0);
    ASSERT(map1 == 0);
    ASSERT(map2 == 0);
    ASSERT(output_image == 0);
    ASSERT(output_image2 == 0);
    ASSERT(input_image == 0);
    ASSERT(input_image2 == 0);
}

TESTCASE_TESTS(tivxRemap,
        testGraphProcessing,
        negativeTestBorderMode
)
