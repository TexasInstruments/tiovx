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
#include <VX/vxu.h>
#include <string.h>

#include "shared_functions.h"

#define MAX_POINTS 100

TESTCASE(tivxOptFlowPyrLK, CT_VXContext, ct_setup_vx_context, 0)

static vx_array own_create_keypoint_array(vx_context context, vx_size count, vx_keypoint_t* keypoints)
{
    vx_array arr = 0;

    ASSERT_VX_OBJECT_(return 0, arr = vxCreateArray(context, VX_TYPE_KEYPOINT, count), VX_TYPE_ARRAY);

    VX_CALL_(return 0, vxAddArrayItems(arr, count, keypoints, sizeof(vx_keypoint_t)));

    return arr;
}

static CT_Image optflow_pyrlk_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static vx_size own_read_keypoints(const char* fileName, vx_keypoint_t** p_old_points, vx_keypoint_t** p_new_points)
{
    size_t sz = 0, read_sz;
    void* buf = 0;
    char file[MAXPATHLENGTH];

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return 0, (sz < MAXPATHLENGTH));
#if 1
    FILE* f = fopen(file, "rb");
    ASSERT_(return 0, f);
    fseek(f, 0, SEEK_END);

    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = ct_alloc_mem(sz + 1);
    if (NULL == buf)
    {
        fclose(f);
        return 0;
    }
    read_sz = fread(buf, 1, sz, f);
    if (read_sz != sz)
    {
        fclose(f);
        ct_free_mem(buf);
        return 0;
    }
    fclose(f); f = NULL;
    ((char*)buf)[sz] = 0;
#else
    sz = ...
    buf = ...
#endif

    ASSERT_(return 0, *p_old_points = ct_alloc_mem(sizeof(vx_keypoint_t) * MAX_POINTS));
    ASSERT_(return 0, *p_new_points = ct_alloc_mem(sizeof(vx_keypoint_t) * MAX_POINTS));

    {
        int num = 0;
        char* pos = buf;
        char* next = 0;
        while(pos && (next = strchr(pos, '\n')))
        {
            int id = 0, status = 0;
            float x1, y1, x2, y2;

            int res;

            *next = 0;
            res = sscanf(pos, "%d %d %g %g %g %g", &id, &status, &x1, &y1, &x2, &y2);
            pos = next + 1;
            if (res == 6)
            {
                (*p_old_points)[num].x = (vx_int32)x1;
                (*p_old_points)[num].y = (vx_int32)y1;
                (*p_old_points)[num].strength = 1;
                (*p_old_points)[num].scale = 0;
                (*p_old_points)[num].orientation = 0;
                (*p_old_points)[num].tracking_status = 1;
                (*p_old_points)[num].error = 0;

                (*p_new_points)[num].x = (vx_int32)x2;
                (*p_new_points)[num].y = (vx_int32)y2;
                (*p_new_points)[num].strength = 1;
                (*p_new_points)[num].scale = 0;
                (*p_new_points)[num].orientation = 0;
                (*p_new_points)[num].tracking_status = status;
                (*p_new_points)[num].error = 0;

                num++;
            }
            else
                break;
        }

        ct_free_mem(buf);

        return num;
    }
}

static void own_keypoints_check(vx_size num_points,
        vx_keypoint_t* old_points, vx_keypoint_t* new_points_ref, vx_keypoint_t* new_points)
{
    vx_size i;
    int num_valid_points = 0;
    int num_lost = 0;
    int num_errors = 0;
    int num_tracked_points = 0;

    for (i = 0; i < num_points; i++)
    {
        vx_int32 dx, dy;
        if (new_points_ref[i].tracking_status == 0)
            continue;
        num_valid_points++;
        if (new_points[i].tracking_status == 0)
        {
            num_lost++;
            continue;
        }
        num_tracked_points++;
        dx = new_points_ref[i].x - new_points[i].x;
        dy = new_points_ref[i].y - new_points[i].y;
        if ((dx * dx + dy * dy) > 2 * 2)
        {
            num_errors++;
        }
    }

    if (num_lost > (int)(num_valid_points * 0.05f))
        CT_ADD_FAILURE("Too many lost points: %d (threshold %d)\n",
                num_lost, (int)(num_valid_points * 0.05f));
    if (num_errors > (int)(num_tracked_points * 0.10f))
        CT_ADD_FAILURE("Too many bad points: %d (threshold %d, both tracked points %d)\n",
                num_errors, (int)(num_tracked_points * 0.10f), num_tracked_points);
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* src1_fileName;
    const char* src2_fileName;
    const char* points_fileName;
    vx_size winSize;
    int useReferencePyramid;
} Arg;


#define PARAMETERS \
    ARG("case1/5x5/ReferencePyramid", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_5x5.txt", 5, 1), \
    ARG("case1/9x9/ReferencePyramid", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_9x9.txt", 9, 1), \
    ARG("DISABLED_case1/5x5", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_5x5.txt", 5, 0), \
    ARG("DISABLED_case1/9x9", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_9x9.txt", 9, 0), \

TEST_WITH_ARG(tivxOptFlowPyrLK, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[2] = { 0, 0 };
    vx_pyramid src_pyr[2]   = { 0, 0 };
    vx_array old_points_arr = 0;
    vx_array virt_points_arr = 0;
    vx_array int_points_arr = 0;
    vx_array new_points_arr0 = 0;
    vx_array new_points_arr1 = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = arg_->winSize;
    vx_graph graph = 0;
    vx_node src_pyr_node[2] = { 0, 0 };
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect;
    vx_bool valid_rect;
    vx_size levels;
    vx_uint32 w, h, new_w, new_h, i;
    vx_image tmp_img[4];

    vx_size num_points = 0;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points0_ref = 0;
    vx_keypoint_t* new_points0 = 0;
    vx_size new_points0_size = 0;
    vx_keypoint_t* new_points1_ref = 0;
    vx_keypoint_t* new_points1 = 0;
    vx_size new_points1_size = 0;
    vx_keypoint_t* int_points_ref = 0;
    vx_keypoint_t* int_points = 0;
    vx_size int_points_size = 0;

    vx_size max_window_dim = 0;

    CT_Image src_ct_image[2] = {0, 0};

    VX_CALL(vxQueryContext(context, VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION, &max_window_dim, sizeof(max_window_dim)));
    if (winSize > max_window_dim)
    {
        printf("%d window dim is not supported. Skip test\n", (int)winSize);
        return;
    }

    ASSERT_NO_FAILURE(src_ct_image[0] = arg_->generator(arg_->src1_fileName, 0, 0));
    ASSERT_NO_FAILURE(src_ct_image[1] = arg_->generator(arg_->src2_fileName, 0, 0));

    ASSERT_VX_OBJECT(src_image[0] = ct_image_to_vx_image(src_ct_image[0], context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image[1] = ct_image_to_vx_image(src_ct_image[1], context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_pyr[0] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image[0]->width, src_ct_image[0]->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(src_pyr[1] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image[0]->width, src_ct_image[0]->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points = own_read_keypoints(arg_->points_fileName, &old_points, &int_points_ref));

    ASSERT_VX_OBJECT(old_points_arr = own_create_keypoint_array(context, num_points, old_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr0 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr1 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(int_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(virt_points_arr = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    if (arg_->useReferencePyramid)
    {
        vx_border_t border = { VX_BORDER_REPLICATE };
        ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image[0], src_pyr[0], 4, VX_SCALE_PYRAMID_HALF, border));
        ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image[1], src_pyr[1], 4, VX_SCALE_PYRAMID_HALF, border));
    }
    else
    {
        VX_CALL(vxuGaussianPyramid(context, src_image[0], src_pyr[0]));
        VX_CALL(vxuGaussianPyramid(context, src_image[1], src_pyr[1]));
    }

    ASSERT_VX_OBJECT(node1 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        old_points_arr, old_points_arr, virt_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        virt_points_arr, virt_points_arr, new_points_arr0,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        old_points_arr, old_points_arr, int_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        virt_points_arr, virt_points_arr, new_points_arr1,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryPyramid(src_pyr[0], VX_PYRAMID_LEVELS, &levels, sizeof(levels));
    vxQueryPyramid(src_pyr[0], VX_PYRAMID_WIDTH, &w, sizeof(w));
    vxQueryPyramid(src_pyr[0], VX_PYRAMID_HEIGHT, &h, sizeof(h));

    new_w = w;
    new_h = h;
    for (i = 0; i < levels; i++)
    {
        tmp_img[i] = vxGetPyramidLevel(src_pyr[0], i);

        vxGetValidRegionImage(tmp_img[i], &src_rect);

        ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), new_w);
        ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), new_h);

        new_w = new_w / 2;
        new_h = new_h / 2;
    }

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(int_points_arr, (void**)&int_points, 0, &int_points_size, 0));
    ASSERT(int_points_size == num_points);

    ASSERT_NO_FAILURE(own_keypoints_check(num_points, old_points, int_points_ref, int_points));

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr0, (void**)&new_points0, 0, &new_points0_size, 0));
    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr1, (void**)&new_points1, 0, &new_points1_size, 0));
    ASSERT(new_points0_size == new_points1_size);

    ASSERT_NO_FAILURE(own_keypoints_check(new_points1_size, NULL, new_points0, new_points1));

    ct_free_mem(new_points0);
    ct_free_mem(new_points0_ref);
    ct_free_mem(new_points1);
    ct_free_mem(new_points1_ref);
    ct_free_mem(int_points);
    ct_free_mem(int_points_ref);
    ct_free_mem(old_points);

    VX_CALL(vxReleaseImage(&tmp_img[0]));
    VX_CALL(vxReleaseImage(&tmp_img[1]));
    VX_CALL(vxReleaseImage(&tmp_img[2]));
    VX_CALL(vxReleaseImage(&tmp_img[3]));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    if(src_pyr_node[0])
        VX_CALL(vxReleaseNode(&src_pyr_node[0]));
    if(src_pyr_node[1])
        VX_CALL(vxReleaseNode(&src_pyr_node[1]));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseScalar(&eps));
    VX_CALL(vxReleaseScalar(&num_iter));
    VX_CALL(vxReleaseScalar(&use_estimations));
    VX_CALL(vxReleaseArray(&old_points_arr));
    VX_CALL(vxReleaseArray(&virt_points_arr));
    VX_CALL(vxReleaseArray(&int_points_arr));
    VX_CALL(vxReleaseArray(&new_points_arr1));
    VX_CALL(vxReleaseArray(&new_points_arr0));
    VX_CALL(vxReleasePyramid(&src_pyr[0]));
    VX_CALL(vxReleasePyramid(&src_pyr[1]));
    VX_CALL(vxReleaseImage(&src_image[0]));
    VX_CALL(vxReleaseImage(&src_image[1]));

    printPerformance(perf_node1, src_ct_image[0]->width*src_ct_image[0]->height, "N1");
    printPerformance(perf_node2, src_ct_image[0]->width*src_ct_image[0]->height, "N2");
    printPerformance(perf_graph, src_ct_image[0]->width*src_ct_image[0]->height, "G1");
}

#define NEG_PARAMETERS \
    ARG("case1/5x5/ReferencePyramid", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_5x5.txt", 5, 1), \
    ARG("case1/9x9/ReferencePyramid", optflow_pyrlk_read_image, "optflow_00.bmp", "optflow_01.bmp", "optflow_pyrlk_9x9.txt", 9, 1)

TEST_WITH_ARG(tivxOptFlowPyrLK, negativeTestBorderMode, Arg,
    NEG_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[2] = { 0, 0 };
    vx_pyramid src_pyr[2]   = { 0, 0 };
    vx_array old_points_arr = 0;
    vx_array virt_points_arr = 0;
    vx_array int_points_arr = 0;
    vx_array new_points_arr0 = 0;
    vx_array new_points_arr1 = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = arg_->winSize;
    vx_graph graph = 0;
    vx_node src_pyr_node[2] = { 0, 0 };
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;

    vx_size num_points = 0;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points0_ref = 0;
    vx_keypoint_t* new_points0 = 0;
    vx_size new_points0_size = 0;
    vx_keypoint_t* new_points1_ref = 0;
    vx_keypoint_t* new_points1 = 0;
    vx_size new_points1_size = 0;
    vx_keypoint_t* int_points_ref = 0;
    vx_keypoint_t* int_points = 0;
    vx_size int_points_size = 0;

    vx_size max_window_dim = 0;

    CT_Image src_ct_image[2] = {0, 0};

    vx_border_t border = { VX_BORDER_REPLICATE };

    if (arg_->winSize == 5)
        border.mode = VX_BORDER_CONSTANT;

    VX_CALL(vxQueryContext(context, VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION, &max_window_dim, sizeof(max_window_dim)));
    if (winSize > max_window_dim)
    {
        printf("%d window dim is not supported. Skip test\n", (int)winSize);
        return;
    }

    ASSERT_NO_FAILURE(src_ct_image[0] = arg_->generator(arg_->src1_fileName, 0, 0));
    ASSERT_NO_FAILURE(src_ct_image[1] = arg_->generator(arg_->src2_fileName, 0, 0));

    ASSERT_VX_OBJECT(src_image[0] = ct_image_to_vx_image(src_ct_image[0], context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src_image[1] = ct_image_to_vx_image(src_ct_image[1], context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_pyr[0] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image[0]->width, src_ct_image[0]->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    ASSERT_VX_OBJECT(src_pyr[1] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image[0]->width, src_ct_image[0]->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points = own_read_keypoints(arg_->points_fileName, &old_points, &int_points_ref));

    ASSERT_VX_OBJECT(old_points_arr = own_create_keypoint_array(context, num_points, old_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr0 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr1 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(int_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(virt_points_arr = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    if (arg_->useReferencePyramid)
    {
        vx_border_t border = { VX_BORDER_REPLICATE };
        ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image[0], src_pyr[0], 4, VX_SCALE_PYRAMID_HALF, border));
        ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image[1], src_pyr[1], 4, VX_SCALE_PYRAMID_HALF, border));
    }
    else
    {
        VX_CALL(vxuGaussianPyramid(context, src_image[0], src_pyr[0]));
        VX_CALL(vxuGaussianPyramid(context, src_image[1], src_pyr[1]));
    }

    ASSERT_VX_OBJECT(node1 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        old_points_arr, old_points_arr, virt_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        virt_points_arr, virt_points_arr, new_points_arr0,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        old_points_arr, old_points_arr, int_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxOpticalFlowPyrLKNode(
        graph,
        src_pyr[0], src_pyr[1],
        virt_points_arr, virt_points_arr, new_points_arr1,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    if(src_pyr_node[0])
        VX_CALL(vxReleaseNode(&src_pyr_node[0]));
    if(src_pyr_node[1])
        VX_CALL(vxReleaseNode(&src_pyr_node[1]));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseScalar(&eps));
    VX_CALL(vxReleaseScalar(&num_iter));
    VX_CALL(vxReleaseScalar(&use_estimations));
    VX_CALL(vxReleaseArray(&old_points_arr));
    VX_CALL(vxReleaseArray(&virt_points_arr));
    VX_CALL(vxReleaseArray(&int_points_arr));
    VX_CALL(vxReleaseArray(&new_points_arr1));
    VX_CALL(vxReleaseArray(&new_points_arr0));
    VX_CALL(vxReleasePyramid(&src_pyr[0]));
    VX_CALL(vxReleasePyramid(&src_pyr[1]));
    VX_CALL(vxReleaseImage(&src_image[0]));
    VX_CALL(vxReleaseImage(&src_image[1]));
}

TESTCASE_TESTS(tivxOptFlowPyrLK,
        testGraphProcessing,
        negativeTestBorderMode
        )
