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
#include <float.h>
#include <math.h>
#include "shared_functions.h"

#define PTS_SEARCH_RADIUS   5
#define MAX_POINTS 100
#define LEVELS_COUNT_MAX    7
#define MAX_NODES 10

TESTCASE(tivxHarrisCorners, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    vx_size         num_corners;
    vx_float32      strength_thresh;
    vx_keypoint_t   *pts;
} TIVX_TruthData;

static vx_size gaussian_pyramid_calc_max_levels_count(int width, int height, vx_float32 scale)
{
    vx_size level = 1;
    while ((16 <= width) && (16 <= height) && level < LEVELS_COUNT_MAX)
    {
        level++;
        width = (int)ceil((vx_float64)width * scale);
        height = (int)ceil((vx_float64)height * scale);
    }
    return level;
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

static vx_size harris_corner_read_line(const char *data, char *line)
{
    const char* ptr = data;
    int pos_temp = 0;
    while (*ptr && *ptr != '\n')
    {
        line[pos_temp] = *ptr;
        pos_temp++; ptr++;
    }
    line[pos_temp] = 0;
    return (ptr - data);
}

static void harris_corner_read_truth_data(const char *file_path, TIVX_TruthData *truth_data, float strengthScale)
{
    FILE* f;
    long sz, read_sz;
    void* buf; char* ptr;
    char temp[1024];
    vx_size ln_size = 0;
    vx_size pts_count = 0;
    vx_keypoint_t *pt;

    ASSERT(truth_data && file_path);

    f = fopen(file_path, "rb");
    if (NULL == f)
    {
        return;
    }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (0 == sz)
    {
        fclose(f);
        return;
    }
    fseek(f, 0, SEEK_SET);

    buf = ct_alloc_mem(sz + 1);
    if (NULL == buf)
    {
        fclose(f);
        return;
    }
    read_sz = fread(buf, 1, sz, f);
    if (read_sz != sz)
    {
        fclose(f);
        ct_free_mem(buf);
        return;
    }

    fclose(f);

    ptr = (char *)buf;
    ptr[sz] = 0;
    ln_size = harris_corner_read_line(ptr, temp);
    ASSERT(ln_size);
    truth_data->num_corners = atoi(temp);
    ASSERT(truth_data->num_corners);
    ptr+= ln_size + 1;

    ASSERT(truth_data->pts = (vx_keypoint_t *)ct_alloc_mem(truth_data->num_corners * sizeof(vx_keypoint_t)));
    pt = truth_data->pts;
    for (;pts_count < truth_data->num_corners; ptr += ln_size + 1, pt++, pts_count++)
    {
        ln_size = harris_corner_read_line(ptr, temp);
        if (0 == ln_size)
            break;
        sscanf(temp, "%d %d %f", &pt->x, &pt->y, &pt->strength);
        pt->strength *= strengthScale;
    }
    ct_free_mem(buf);

    ASSERT(pts_count == truth_data->num_corners);
    truth_data->strength_thresh = truth_data->pts[truth_data->num_corners - 1].strength - FLT_EPSILON;
}

static int harris_corner_search_truth_point(vx_int32 x, vx_int32 y, vx_float32 strength, const TIVX_TruthData *truth_data)
{
    vx_int32 xmin = x - PTS_SEARCH_RADIUS;
    vx_int32 xmax = x + PTS_SEARCH_RADIUS;
    vx_int32 ymin = y - PTS_SEARCH_RADIUS;
    vx_int32 ymax = y + PTS_SEARCH_RADIUS;
    vx_size num;
    if (FLT_MIN >= strength)
        return 1;
    for (num = 0; num < truth_data->num_corners; num++)
    {
        if ((xmin <= truth_data->pts[num].x) && (truth_data->pts[num].x <= xmax) &&
            (ymin <= truth_data->pts[num].y) && (truth_data->pts[num].y <= ymax))
        {
            if (fabs(log10(truth_data->pts[num].strength/strength)) < 1.1)
                return 0;
        }
    }
    return 1;
}

static int harris_corner_search_test_point(vx_int32 x, vx_int32 y, vx_float32 strength, char *test_pts, vx_size pts_stride, vx_size num_pts)
{
    vx_int32 xmin = x - PTS_SEARCH_RADIUS;
    vx_int32 xmax = x + PTS_SEARCH_RADIUS;
    vx_int32 ymin = y - PTS_SEARCH_RADIUS;
    vx_int32 ymax = y + PTS_SEARCH_RADIUS;
    vx_size num;
    vx_keypoint_t *pt = NULL;
    if (FLT_MIN >= strength)
        return 1;
    for (num = 0; num < num_pts; num++, test_pts += pts_stride)
    {
        pt = (vx_keypoint_t *)test_pts;
        if ((xmin <= pt->x) && (pt->x <= xmax) &&
            (ymin <= pt->y) && (pt->y <= ymax))
        {
            if (fabs(log10(pt->strength / strength)) < 1.1)
                return 0;
        }
    }
    return 1;
}

static void harris_corner_check(vx_array corners, const TIVX_TruthData *truth_data)
{
    vx_enum type;
    vx_size i, num_corners, stride;
    char *pts = NULL;
    char *pts_ptr = NULL;
    vx_keypoint_t *pt = NULL;
    vx_int32 fail_count = 0;
    vx_map_id map_id;

    ASSERT(corners && truth_data);
    ASSERT(VX_SUCCESS == vxQueryArray(corners, VX_ARRAY_ITEMTYPE, &type, sizeof(type)));
    ASSERT(VX_TYPE_KEYPOINT == type);

    ASSERT(VX_SUCCESS == vxQueryArray(corners, VX_ARRAY_NUMITEMS, &num_corners, sizeof(num_corners)));

    ASSERT(num_corners);
    ASSERT(VX_SUCCESS == vxMapArrayRange(corners, 0, num_corners, &map_id, &stride, (void**)&pts, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

    pts_ptr = pts;
    for (i = 0; i < num_corners; i++, pts_ptr += stride)
    {
        pt = (vx_keypoint_t *)pts_ptr;
        ASSERT(pt->tracking_status == 1);
        if (harris_corner_search_truth_point(pt->x, pt->y, pt->strength, truth_data))
            fail_count++;
    }
    if (100 * fail_count > 10 * (vx_int32)i)
    {
        CT_FAIL_(goto cleanup, "Too much (%d) test points, which don't have corresponding truth data points", fail_count);
    }
    fail_count = 0;
    for (i = 0; i < truth_data->num_corners; i++)
    {
        if (harris_corner_search_test_point(truth_data->pts[i].x, truth_data->pts[i].y, truth_data->pts[i].strength, pts, stride, num_corners))
            fail_count++;
    }
    if (100 * fail_count > 10 * (vx_int32)i)
    {
        CT_FAIL_(goto cleanup, "Too much (%d) truth data points, which don't have corresponding test points", fail_count);
    }

cleanup:
    vxUnmapArrayRange(corners, map_id);
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
    const char* filePrefix;
    vx_float32 min_distance;
    vx_float32 sensitivity;
    vx_int32  gradient_size;
    vx_int32  block_size;
} Arg;

#define ADD_VX_MIN_DISTANCE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MIN_DISTANCE=3.0", __VA_ARGS__, 3.0f))

#define ADD_VX_SENSITIVITY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SENSITIVITY=0.10", __VA_ARGS__, 0.10f))

#define ADD_VX_GRADIENT_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/GRADIENT_SIZE=3", __VA_ARGS__, 3))

#define ADD_VX_BLOCK_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/BLOCK_SIZE=5", __VA_ARGS__, 5))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("few_strong_corners",  ADD_VX_MIN_DISTANCE, ADD_VX_SENSITIVITY, ADD_VX_GRADIENT_SIZE, ADD_VX_BLOCK_SIZE, ARG, "hc_fsc"), \
    CT_GENERATE_PARAMETERS("many_strong_corners", ADD_VX_MIN_DISTANCE, ADD_VX_SENSITIVITY, ADD_VX_GRADIENT_SIZE, ADD_VX_BLOCK_SIZE, ARG, "hc_msc")

TEST_WITH_ARG(tivxHarrisCorners, testMultipleGraphs, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;

    vx_image input_image0 = 0, input_image1 = 0;
    vx_float32 strength_thresh0, strength_thresh1;
    vx_float32 min_distance0 = arg_->min_distance + FLT_EPSILON;
    vx_float32 min_distance1 = arg_->min_distance + 2 + FLT_EPSILON;
    vx_float32 sensitivity0 = arg_->sensitivity;
    vx_float32 sensitivity1 = arg_->sensitivity + 0.05f;
    vx_size num_corners0, num_corners1;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    size_t sz0, sz1;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    vx_scalar strength_thresh0_scalar, min_distance0_scalar, sensitivity0_scalar, num_corners0_scalar;
    vx_scalar strength_thresh1_scalar, min_distance1_scalar, sensitivity1_scalar, num_corners1_scalar;
    vx_array corners0, corners1;

    char filepath0[MAXPATHLENGTH], filepath1[MAXPATHLENGTH];

    CT_Image input0 = NULL, input1 = NULL;
    TIVX_TruthData truth_data0, truth_data1;

    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    scale = scale * scale * scale * scale;

    sz0 = snprintf(filepath0, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz0 < MAXPATHLENGTH);
    sz1 = snprintf(filepath1, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, (arg_->min_distance + 2), (arg_->sensitivity + 0.05f), arg_->gradient_size, arg_->block_size);
    ASSERT(sz1 < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath0, &truth_data0, (float)scale));
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath1, &truth_data1, (float)scale));

    strength_thresh0 = truth_data0.strength_thresh;
    strength_thresh1 = truth_data1.strength_thresh;

    sprintf(filepath0, "harriscorners/%s.bmp", arg_->filePrefix);
    sprintf(filepath1, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input0 = ct_read_image(filepath0, 1));
    ASSERT(input0 && (input0->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image0 = ct_image_to_vx_image(input0, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(input1 = ct_read_image(filepath1, 1));
    ASSERT(input1 && (input1->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);

    num_corners0 = input0->width * input0->height / 10;
    num_corners1 = input1->width * input1->height / 10;

    ASSERT_VX_OBJECT(strength_thresh0_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance0_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity0_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners0_scalar = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners0), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(strength_thresh1_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance1_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity1_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners1_scalar = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners1), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners0 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners0), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(corners1 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners1), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxHarrisCornersNode(graph, input_image0, strength_thresh0_scalar, min_distance0_scalar,
                                                sensitivity0_scalar, arg_->gradient_size, arg_->block_size, corners0,
                                                num_corners0_scalar), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxHarrisCornersNode(graph, input_image1, strength_thresh1_scalar, min_distance1_scalar,
                                                sensitivity1_scalar, arg_->gradient_size, arg_->block_size, corners1,
                                                num_corners1_scalar), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners0, &truth_data0));
    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners1, &truth_data1));

    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    ct_free_mem(truth_data0.pts); truth_data0.pts = 0;
    ct_free_mem(truth_data1.pts); truth_data1.pts = 0;
    VX_CALL(vxReleaseArray(&corners1));
    VX_CALL(vxReleaseArray(&corners0));
    VX_CALL(vxReleaseScalar(&num_corners0_scalar));
    VX_CALL(vxReleaseScalar(&sensitivity0_scalar));
    VX_CALL(vxReleaseScalar(&min_distance0_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh0_scalar));
    VX_CALL(vxReleaseScalar(&num_corners1_scalar));
    VX_CALL(vxReleaseScalar(&sensitivity1_scalar));
    VX_CALL(vxReleaseScalar(&min_distance1_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh1_scalar));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseImage(&input_image0));

    ASSERT(truth_data0.pts == 0);
    ASSERT(truth_data1.pts == 0);
    ASSERT(corners0 == 0);
    ASSERT(corners1 == 0);
    ASSERT(num_corners0_scalar == 0);
    ASSERT(sensitivity0_scalar == 0);
    ASSERT(min_distance0_scalar == 0);
    ASSERT(strength_thresh0_scalar == 0);
    ASSERT(num_corners1_scalar == 0);
    ASSERT(sensitivity1_scalar == 0);
    ASSERT(min_distance1_scalar == 0);
    ASSERT(strength_thresh1_scalar == 0);
    ASSERT(input_image0 == 0);
    ASSERT(input_image1 == 0);

    printPerformance(perf_node1, input0->width*input0->height, "N1");
    printPerformance(perf_node2, input1->width*input1->height, "N2");
    printPerformance(perf_graph, input0->width*input0->height, "G1");
}

TEST_WITH_ARG(tivxHarrisCorners, testVirtualImage, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;

    vx_image input_image = 0;
    vx_float32 strength_thresh;
    vx_float32 min_distance = arg_->min_distance + FLT_EPSILON;
    vx_float32 sensitivity = arg_->sensitivity;
    vx_size num_corners;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0;
    size_t sz;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_rectangle_t src_rect;
    vx_bool valid_rect;

    vx_scalar strength_thresh_scalar, min_distance_scalar, sensitivity_scalar, num_corners_scalar0, num_corners_scalar1;
    vx_array corners, corners_virt;

    char filepath[MAXPATHLENGTH];

    CT_Image input = NULL;
    TIVX_TruthData truth_data;

    vx_array new_points_arr0 = 0, new_points_arr1 = 0;
    vx_pyramid pyr = 0, src_pyr   = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = 5; // hardcoded from optflow test case
    vx_border_t border = { VX_BORDER_REPLICATE };
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points0 = 0;
    vx_keypoint_t* new_points1 = 0;
    vx_size new_points_size0 = 0, new_points_size1 = 0;
    vx_size corners_data_size = 0;
    vx_keypoint_t* corners_data = 0;

    CT_Image src_ct_image = NULL;

    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    scale = scale * scale * scale * scale;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath, &truth_data, (float)scale));

    strength_thresh = truth_data.strength_thresh;

    sprintf(filepath, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input = ct_read_image(filepath, 1));
    ASSERT(input && (input->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    num_corners = input->width * input->height / 10;

    ASSERT_NO_FAILURE(src_ct_image = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    if (NULL != src_ct_image)
    {
        ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image->width, src_ct_image->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
    }

    ASSERT_VX_OBJECT(new_points_arr0 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr1 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    if (NULL != src_ct_image)
    {
        ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image->width, src_ct_image->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image, src_pyr, 4, VX_SCALE_PYRAMID_HALF, border));
    }

    ASSERT_VX_OBJECT(strength_thresh_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners_scalar0 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners_scalar1 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(corners_virt = vxCreateVirtualArray(graph, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(node1 = vxHarrisCornersNode(graph, input_image, strength_thresh_scalar, min_distance_scalar,
                                                sensitivity_scalar, arg_->gradient_size, arg_->block_size, corners,
                                                num_corners_scalar0), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        pyr, src_pyr,
        corners, corners, new_points_arr0,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3 = vxHarrisCornersNode(graph, input_image, strength_thresh_scalar, min_distance_scalar,
                                                sensitivity_scalar, arg_->gradient_size, arg_->block_size, corners_virt,
                                                num_corners_scalar1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4 = vxOpticalFlowPyrLKNode(
        graph,
        pyr, src_pyr,
        corners_virt, corners_virt, new_points_arr1,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(input_image, &src_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), input->width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), input->height);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners, &truth_data));

    ct_read_array(corners, (void**)&corners_data, 0, &corners_data_size, 0);
    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr0, (void**)&new_points0, 0, &new_points_size0, 0));
    ASSERT(new_points_size0 == corners_data_size);
    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr1, (void**)&new_points1, 0, &new_points_size1, 0));
    ASSERT(new_points_size1 == corners_data_size);

    ASSERT_NO_FAILURE(own_keypoints_check(new_points_size0, NULL, new_points0, new_points1));

    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node4 == 0);
    ASSERT(node3 == 0);
    ASSERT(node2 == 0);
    ASSERT(node1 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseScalar(&eps));
    ASSERT(eps == 0);
    VX_CALL(vxReleaseScalar(&num_iter));
    ASSERT(num_iter == 0);
    VX_CALL(vxReleaseScalar(&use_estimations));
    ASSERT(use_estimations == 0);
    VX_CALL(vxReleasePyramid(&src_pyr));
    ASSERT(src_pyr == 0);
    VX_CALL(vxReleasePyramid(&pyr));
    ASSERT(pyr == 0);

    ct_free_mem(truth_data.pts); truth_data.pts = 0;
    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseArray(&corners_virt));
    VX_CALL(vxReleaseScalar(&num_corners_scalar0));
    VX_CALL(vxReleaseScalar(&num_corners_scalar1));
    VX_CALL(vxReleaseScalar(&sensitivity_scalar));
    VX_CALL(vxReleaseScalar(&min_distance_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh_scalar));
    VX_CALL(vxReleaseImage(&input_image));

    VX_CALL(vxReleaseArray(&new_points_arr0));
    ASSERT(new_points_arr0 == 0);
    VX_CALL(vxReleaseArray(&new_points_arr1));
    ASSERT(new_points_arr1 == 0);

    ASSERT(truth_data.pts == 0);
    ASSERT(corners == 0);
    ASSERT(num_corners_scalar1 == 0);
    ASSERT(num_corners_scalar0 == 0);
    ASSERT(sensitivity_scalar == 0);
    ASSERT(min_distance_scalar == 0);
    ASSERT(strength_thresh_scalar == 0);
    ASSERT(input_image == 0);

    printPerformance(perf_node1, input->width*input->height, "N1");
    printPerformance(perf_node2, input->width*input->height, "N2");
    printPerformance(perf_graph, input->width*input->height, "G1");
}

TEST_WITH_ARG(tivxHarrisCorners, testOptionalParameters, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;

    vx_image input_image = 0;
    vx_float32 strength_thresh;
    vx_float32 min_distance = arg_->min_distance + FLT_EPSILON;
    vx_float32 sensitivity = arg_->sensitivity;
    vx_size num_corners;
    vx_graph graph = 0;
    vx_node node = 0;
    size_t sz;
    vx_perf_t perf_node1, perf_graph;

    vx_scalar strength_thresh_scalar, min_distance_scalar, sensitivity_scalar;
    vx_array corners;

    char filepath[MAXPATHLENGTH];

    CT_Image input = NULL;
    TIVX_TruthData truth_data;

    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    scale = scale * scale * scale * scale;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath, &truth_data, (float)scale));

    strength_thresh = truth_data.strength_thresh;

    sprintf(filepath, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input = ct_read_image(filepath, 1));
    ASSERT(input && (input->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    num_corners = input->width * input->height / 10;

    ASSERT_VX_OBJECT(strength_thresh_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxHarrisCornersNode(graph, input_image, strength_thresh_scalar, min_distance_scalar,
                                                sensitivity_scalar, arg_->gradient_size, arg_->block_size, corners,
                                                NULL), VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners, &truth_data));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);

    ct_free_mem(truth_data.pts); truth_data.pts = 0;
    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseScalar(&sensitivity_scalar));
    VX_CALL(vxReleaseScalar(&min_distance_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh_scalar));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(truth_data.pts == 0);
    ASSERT(corners == 0);
    ASSERT(sensitivity_scalar == 0);
    ASSERT(min_distance_scalar == 0);
    ASSERT(strength_thresh_scalar == 0);
    ASSERT(input_image == 0);

    printPerformance(perf_node1, input->width*input->height, "N1");
    printPerformance(perf_graph, input->width*input->height, "G1");
}

#define NEG_VX_BLOCK_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/BLOCK_SIZE=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/BLOCK_SIZE=5", __VA_ARGS__, 5))

#define NEG_PARAMETERS \
    CT_GENERATE_PARAMETERS("few_strong_corners",  ADD_VX_MIN_DISTANCE, ADD_VX_SENSITIVITY, ADD_VX_GRADIENT_SIZE, NEG_VX_BLOCK_SIZE, ARG, "hc_fsc")

TEST_WITH_ARG(tivxHarrisCorners, negativeTestBorderMode, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;

    vx_image input_image = 0;
    vx_float32 strength_thresh;
    vx_float32 min_distance = arg_->min_distance + FLT_EPSILON;
    vx_float32 sensitivity = arg_->sensitivity;
    vx_size num_corners;
    vx_graph graph = 0;
    vx_node node = 0;
    size_t sz;
    vx_border_t border = { VX_BORDER_REPLICATE };

    if (arg_->block_size == 4)
        border.mode = VX_BORDER_CONSTANT;

    vx_scalar strength_thresh_scalar, min_distance_scalar, sensitivity_scalar;
    vx_array corners;

    char filepath[MAXPATHLENGTH];

    CT_Image input = NULL;
    TIVX_TruthData truth_data;

    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    scale = scale * scale * scale * scale;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath, &truth_data, (float)scale));

    strength_thresh = truth_data.strength_thresh;

    sprintf(filepath, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input = ct_read_image(filepath, 1));
    ASSERT(input && (input->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    num_corners = input->width * input->height / 10;

    ASSERT_VX_OBJECT(strength_thresh_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxHarrisCornersNode(graph, input_image, strength_thresh_scalar, min_distance_scalar,
                                                sensitivity_scalar, arg_->gradient_size, arg_->block_size, corners,
                                                NULL), VX_TYPE_NODE);

    if (border.mode != VX_BORDER_UNDEFINED)
        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);

    ct_free_mem(truth_data.pts); truth_data.pts = 0;
    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseScalar(&sensitivity_scalar));
    VX_CALL(vxReleaseScalar(&min_distance_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh_scalar));
    VX_CALL(vxReleaseImage(&input_image));

    ASSERT(truth_data.pts == 0);
    ASSERT(corners == 0);
    ASSERT(sensitivity_scalar == 0);
    ASSERT(min_distance_scalar == 0);
    ASSERT(strength_thresh_scalar == 0);
    ASSERT(input_image == 0);
}


TEST_WITH_ARG(tivxHarrisCorners, testHarrisCornersSupernode, Arg,
    PARAMETERS
)
{
    int node_count = 3;
    vx_context context = context_->vx_context_;
    vx_image input_image0 = 0, input_image1 = 0, input_virt1 = 0, input_virt2 = 0, input_virt3 = 0, input_virt4 = 0;
    vx_float32 strength_thresh0, strength_thresh1;
    vx_float32 min_distance0 = arg_->min_distance + FLT_EPSILON;
    vx_float32 min_distance1 = arg_->min_distance + 2 + FLT_EPSILON;
    vx_float32 sensitivity0 = arg_->sensitivity;
    vx_float32 sensitivity1 = arg_->sensitivity + 0.05f;
    vx_size num_corners0, num_corners1;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0, node4 = 0, node5 = 0, node6 = 0;
    size_t sz0, sz1;
    vx_perf_t perf_super_node_1, perf_super_node_2, perf_graph;
    tivx_super_node super_node_1 = 0, super_node_2 = 0;
    vx_node node_list_1[MAX_NODES], node_list_2[MAX_NODES];

    vx_scalar strength_thresh0_scalar, min_distance0_scalar, sensitivity0_scalar, num_corners0_scalar;
    vx_scalar strength_thresh1_scalar, min_distance1_scalar, sensitivity1_scalar, num_corners1_scalar;
    vx_array corners0, corners1;

    char filepath0[MAXPATHLENGTH], filepath1[MAXPATHLENGTH];

    CT_Image input0 = NULL, input1 = NULL;
    TIVX_TruthData truth_data0, truth_data1;

    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    scale = scale * scale * scale * scale;

    sz0 = snprintf(filepath0, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz0 < MAXPATHLENGTH);
    sz1 = snprintf(filepath1, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, (arg_->min_distance + 2), (arg_->sensitivity + 0.05f), arg_->gradient_size, arg_->block_size);
    ASSERT(sz1 < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath0, &truth_data0, (float)scale));
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath1, &truth_data1, (float)scale));

    strength_thresh0 = truth_data0.strength_thresh;
    strength_thresh1 = truth_data1.strength_thresh;

    sprintf(filepath0, "harriscorners/%s.bmp", arg_->filePrefix);
    sprintf(filepath1, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input0 = ct_read_image(filepath0, 1));
    ASSERT(input0 && (input0->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image0 = ct_image_to_vx_image(input0, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_virt1 = vxCreateImage(context, input0->width,input0->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_virt2 = vxCreateImage(context, input0->width,input0->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(input1 = ct_read_image(filepath1, 1));
    ASSERT(input1 && (input1->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image1 = ct_image_to_vx_image(input1, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_virt3 = vxCreateImage(context, input1->width,input1->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(input_virt4 = vxCreateImage(context, input1->width,input1->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    num_corners0 = input0->width * input0->height / 10;
    num_corners1 = input1->width * input1->height / 10;

    ASSERT_VX_OBJECT(strength_thresh0_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance0_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity0_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity0), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners0_scalar = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners0), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(strength_thresh1_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance1_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity1_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity1), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners1_scalar = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners1), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners0 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners0), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(corners1 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners1), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, input_image0, input_virt1), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, input_virt1, input_virt2), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node3 = vxHarrisCornersNode(graph, input_virt2, strength_thresh0_scalar, min_distance0_scalar,
                                                sensitivity0_scalar, arg_->gradient_size, arg_->block_size, corners0,
                                                num_corners0_scalar), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4 = vxNotNode(graph, input_image1, input_virt3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node5 = vxNotNode(graph, input_virt3, input_virt4), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node6 = vxHarrisCornersNode(graph, input_virt4, strength_thresh1_scalar, min_distance1_scalar,
                                                sensitivity1_scalar, arg_->gradient_size, arg_->block_size, corners1,
                                                num_corners1_scalar), VX_TYPE_NODE);
    ASSERT_NO_FAILURE(node_list_1[0] = node1);
    ASSERT_NO_FAILURE(node_list_1[1] = node2);
    ASSERT_NO_FAILURE(node_list_1[2] = node3);
    ASSERT_VX_OBJECT(super_node_1 = tivxCreateSuperNode(graph, node_list_1, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node_1));

    ASSERT_NO_FAILURE(node_list_2[0] = node4);
    ASSERT_NO_FAILURE(node_list_2[1] = node5);
    ASSERT_NO_FAILURE(node_list_2[2] = node6);
    ASSERT_VX_OBJECT(super_node_2 = tivxCreateSuperNode(graph, node_list_2, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node_1));
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(tivxQuerySuperNode(super_node_1, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node_1, sizeof(perf_super_node_1)));
    VX_CALL(tivxQuerySuperNode(super_node_2, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node_2, sizeof(perf_super_node_2)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners0, &truth_data0));
    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners1, &truth_data1));

    VX_CALL(tivxReleaseSuperNode(&super_node_1));
    VX_CALL(tivxReleaseSuperNode(&super_node_2));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseNode(&node4));
    VX_CALL(vxReleaseNode(&node5));
    VX_CALL(vxReleaseNode(&node6));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(super_node_1 == 0);
    ASSERT(super_node_2 == 0);
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(node3 == 0);
    ASSERT(node4 == 0);
    ASSERT(node5 == 0);
    ASSERT(node6 == 0);
    ASSERT(graph == 0);

    ct_free_mem(truth_data0.pts); truth_data0.pts = 0;
    ct_free_mem(truth_data1.pts); truth_data1.pts = 0;
    VX_CALL(vxReleaseArray(&corners1));
    VX_CALL(vxReleaseArray(&corners0));
    VX_CALL(vxReleaseScalar(&num_corners0_scalar));
    VX_CALL(vxReleaseScalar(&sensitivity0_scalar));
    VX_CALL(vxReleaseScalar(&min_distance0_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh0_scalar));
    VX_CALL(vxReleaseScalar(&num_corners1_scalar));
    VX_CALL(vxReleaseScalar(&sensitivity1_scalar));
    VX_CALL(vxReleaseScalar(&min_distance1_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh1_scalar));
    VX_CALL(vxReleaseImage(&input_image1));
    VX_CALL(vxReleaseImage(&input_image0));
    VX_CALL(vxReleaseImage(&input_virt1));
    VX_CALL(vxReleaseImage(&input_virt2));
    VX_CALL(vxReleaseImage(&input_virt3));
    VX_CALL(vxReleaseImage(&input_virt4));

    ASSERT(truth_data0.pts == 0);
    ASSERT(truth_data1.pts == 0);
    ASSERT(corners0 == 0);
    ASSERT(corners1 == 0);
    ASSERT(num_corners0_scalar == 0);
    ASSERT(sensitivity0_scalar == 0);
    ASSERT(min_distance0_scalar == 0);
    ASSERT(strength_thresh0_scalar == 0);
    ASSERT(num_corners1_scalar == 0);
    ASSERT(sensitivity1_scalar == 0);
    ASSERT(min_distance1_scalar == 0);
    ASSERT(strength_thresh1_scalar == 0);
    ASSERT(input_image0 == 0);
    ASSERT(input_image1 == 0);
    ASSERT(input_virt1 == 0);
    ASSERT(input_virt2 == 0);
    ASSERT(input_virt3 == 0);
    ASSERT(input_virt4 == 0);
    printPerformance(perf_super_node_1, input0->width * input0->height, "SN1");
    printPerformance(perf_super_node_2, input1->width * input1->height, "SN2");
    printPerformance(perf_graph, input0->width * input0->height, "G");
}

#ifdef BUILD_BAM
#define testHarrisCornersSupernode DISABLED_testHarrisCornersSupernode
#else
#define testHarrisCornersSupernode DISABLED_testHarrisCornersSupernode
#endif

TESTCASE_TESTS(tivxHarrisCorners,
        testMultipleGraphs,
        testVirtualImage,
        testOptionalParameters,
        negativeTestBorderMode,
        testHarrisCornersSupernode
)
