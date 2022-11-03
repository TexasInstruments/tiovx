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

#define VX_GAUSSIAN_PYRAMID_TOLERANCE 1
#define PTS_SEARCH_RADIUS   5
#define MAX_POINTS 100
#define LEVELS_COUNT_MAX    7
#define HALFSCALEGAUSSIAN_TOLERANCE 1
#define WIDTH 16
#define HEIGHT 16
#define MAX_BINS 256

#define USE_OPENCV_GENERATED_REFERENCE
#define CANNY_ACCEPTANCE_THRESHOLD 0.95

#define CREF_EDGE 2
#define CREF_LINK 1
#define CREF_NONE 0

#define MASK_SIZE_MAX (5)

#ifndef MIN
#define MIN(_a,_b) (((_a) < (_b)) ? (_a) : (_b))
#endif
#ifndef MAX
#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#endif

TESTCASE(tivxMaxNodes, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    vx_size         num_corners;
    vx_float32      strength_thresh;
    vx_keypoint_t   *pts;
} TIVX_TruthData;

static CT_Image gaussian_pyramid_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image gaussian_pyramid_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static int32_t gaussian5x5_pyramid_get(int32_t *values)
{
    int32_t res = 1 * (values[ 0] + values[ 4] + values[20] + values[24]) +
                  4 * (values[ 1] + values[ 3] + values[ 5] + values[ 9] + values[15] + values[19] + values[21] + values[23]) +
                  6 * (values[ 2] + values[10] + values[14] + values[22]) +
                 16 * (values[ 6] + values[ 8] + values[16] + values[18]) +
                 24 * (values[ 7] + values[11] + values[13] + values[17]) +
                 36 * values[12];
    res = res >> 8;
    return res;
}

static uint8_t gaussian5x5_pyramid_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[25] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y - 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 2, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 2),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 2, y + 2),
    };
    return (uint8_t)gaussian5x5_pyramid_get(values);
}

static uint8_t gaussian5x5_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[25] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y - 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 2, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 2),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 2, y + 2),
    };
    return (uint8_t)gaussian5x5_pyramid_get(values);
}

static uint8_t gaussian5x5_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[25] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y - 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 2, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 2, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 2, y + 2, constant_value),
    };
    return (uint8_t)gaussian5x5_pyramid_get(values);
}

static vx_int32 gaussian_pyramid_get_pixel(CT_Image input, int x, int y, vx_border_t border, int level)
{
    if (border.mode == VX_BORDER_UNDEFINED)
    {
        if (x >= 2 + level && y >= 2 + level && x < (int)input->width - 2 - level && y < (int)input->height - 2 - level)
            return gaussian5x5_pyramid_calculate(input, x, y);
        else
            return -1;
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        return gaussian5x5_calculate_replicate(input, x, y);
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        return gaussian5x5_calculate_constant(input, x, y, border.constant_value.U32);
    }
    CT_FAIL_(return -1, "NOT IMPLEMENTED");
}

static void gaussian_pyramid_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_border_t border, int level)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    vx_float64 x_src = (((vx_float64)x + 0.5) * (vx_float64)input->width / (vx_float64)output->width) - 0.5;
    vx_float64 y_src = (((vx_float64)y + 0.5) * (vx_float64)input->height / (vx_float64)output->height) - 0.5;
    int x_min = (int)floor(x_src), y_min = (int)floor(y_src);
    int sx, sy;
    for (sy = 0; sy <= 1; sy++)
    {
        for (sx = 0; sx <= 1; sx++)
        {
            vx_int32 candidate = 0;
            ASSERT_NO_FAILURE_(return, candidate = gaussian_pyramid_get_pixel(input, x_min + sx, y_min + sy, border, level));
            if (candidate == -1 || abs(candidate - res) <= VX_GAUSSIAN_PYRAMID_TOLERANCE)
                return;
        }
    }
    CT_FAIL_(return, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
}

static void gaussian_pyramid_check_image(CT_Image input, CT_Image output, vx_border_t border, vx_size level)
{
    ASSERT(input && output);
    if (0 == level)
    {
        EXPECT_EQ_CTIMAGE(input, output);
    }
    else
    {
        CT_FILL_IMAGE_8U(, output,
                {
                    ASSERT_NO_FAILURE(gaussian_pyramid_check_pixel(input, output, x, y, border, (int)level));
                });
    }
}

static const vx_float64 c_orbscale[] =
{
    1.000000000000000000000000000000e+00,
    8.408964152537146130583778358414e-01,
    7.071067811865475727373109293694e-01,
    5.946035575013605134486738279520e-01,
    5.000000000000000000000000000000e-01,
    4.204482076268573065291889179207e-01,
    3.535533905932737308575042334269e-01,
    2.973017787506802567243369139760e-01,
    2.500000000000000000000000000000e-01,
    2.102241038134286532645944589603e-01,
    1.767766952966368654287521167134e-01,
    1.486508893753401283621684569880e-01,
    1.250000000000000000000000000000e-01,
    1.051120519067143266322972294802e-01,
    8.838834764831843271437605835672e-02,
    7.432544468767006418108422849400e-02,
    6.250000000000000000000000000000e-02,
};


static void gaussian_pyramid_check(CT_Image input, vx_pyramid pyr, vx_size levels, vx_float32 scale, vx_border_t border)
{
    vx_uint32 level = 0;
    vx_image output_image = 0;
    CT_Image output_prev = NULL, output_cur = NULL;
    vx_uint32 ref_width = input->width;
    vx_uint32 ref_height = input->height;

    ASSERT(input && pyr && (1 < levels) && (level < sizeof(c_orbscale) / sizeof(c_orbscale[0]) ));
    ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, 0), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(output_prev = ct_image_from_vx_image(output_image));
    VX_CALL(vxReleaseImage(&output_image));
    ASSERT(output_image == 0);

    gaussian_pyramid_check_image(input, output_prev, border, 0);
    if (CT_HasFailure())
    {
        printf("=== Input ===\n");
        ct_dump_image_info(input);
        printf("=== LEVEL %d ===\n", 0);
        ct_dump_image_info(output_prev);
        return;
    }

    for (level = 1; level < levels; level++)
    {
        ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, level), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(output_cur = ct_image_from_vx_image(output_image));
        VX_CALL(vxReleaseImage(&output_image));
        ASSERT(output_image == 0);

        if (VX_SCALE_PYRAMID_ORB == scale)
        {
            vx_float64 orb_scale = c_orbscale[level];
            if ( (output_cur->width  != ceil(orb_scale * ref_width)) ||
                 (output_cur->height != ceil(orb_scale * ref_height)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }
        else
        {
            if ( (output_cur->width != ceil(output_prev->width * scale)) ||
                 (output_cur->height != ceil(output_prev->height * scale)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }

        gaussian_pyramid_check_image(output_prev, output_cur, border, level);
        if (CT_HasFailure())
        {
            printf("=== Input ===\n");
            ct_dump_image_info(output_prev);
            printf("=== LEVEL %d ===\n", level);
            ct_dump_image_info(output_cur);
            return;
        }

        output_prev = output_cur;
    }
}



static vx_uint8 gaussian_pyramid_reference_get_pixel(CT_Image prevLevel, int dst_width, int dst_height, int x, int y, vx_border_t border, int level)
{
    vx_int32 candidate = -1;
    vx_float64 x_src = (((vx_float64)x + 0.5) * (vx_float64)prevLevel->width / (vx_float64)dst_width) - 0.5;
    vx_float64 y_src = (((vx_float64)y + 0.5) * (vx_float64)prevLevel->height / (vx_float64)dst_height) - 0.5;
    int x_int = (int)floor(x_src), y_int = (int)floor(y_src);
    vx_float64 x_f = x_src - x_int, y_f = y_src - y_int;
    if (x_f >= 0.5)
        x_int++;
    if (y_f >= 0.5)
        y_int++;
    if (x_int >= (int)prevLevel->width)
        x_int = prevLevel->width - 1;
    if (y_int >= (int)prevLevel->height)
        y_int = prevLevel->height - 1;
    ASSERT_NO_FAILURE_(return 0, candidate = gaussian_pyramid_get_pixel(prevLevel, x_int, y_int, border, level));
    if (candidate == -1)
        return 0;
    return CT_CAST_U8(candidate);
}

static CT_Image gaussian_pyramid_create_reference_image(CT_Image input, CT_Image prevLevel, vx_border_t border, vx_float32 scale, vx_size target_level)
{
    vx_uint32 level = 0;
    CT_Image dst;
    vx_uint32 ref_width = input->width;
    vx_uint32 ref_height = input->height;
    vx_uint32 dst_width = input->width;
    vx_uint32 dst_height = input->height;

    ASSERT_(return NULL, scale < 1.0);
    ASSERT_(return NULL, input && (level < (sizeof(c_orbscale) / sizeof(c_orbscale[0]))));

    ASSERT_(return NULL, input->format == VX_DF_IMAGE_U8);

    if (VX_SCALE_PYRAMID_HALF == scale)
    {
        dst_width = ref_width;
        dst_height = ref_height;
        for (level = 1; level <= target_level; level++)
        {
            dst_width = (vx_uint32)ceil(dst_width * scale);
            dst_height = (vx_uint32)ceil(dst_height * scale);
        }
    }
    else
    {
        vx_float64 orb_scale = c_orbscale[target_level];
        dst_width = (vx_uint32)ceil(orb_scale * ref_width);
        dst_height = (vx_uint32)ceil(orb_scale * ref_height);
    }

    dst = ct_allocate_image(dst_width, dst_height, input->format);

    if (target_level == 0)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = *CT_IMAGE_DATA_PTR_8U(input, x, y);
                    *dst_data = res;
                });
    }
    else
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian_pyramid_reference_get_pixel(prevLevel, dst_width, dst_height, x, y, border, (int)target_level);
                    *dst_data = res;
                });
    }

    return dst;
}

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

    memset(truth_data, 0, sizeof(*truth_data)); /* init it to zero */

    f = fopen(file_path, "rb");
    ASSERT(f);
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

static const int circle[][2] =
{
    {3, 0}, {3, -1}, {2, -2}, {1, -3}, {0, -3}, {-1, -3}, {-2, -2}, {-3, -1},
    {-3, 0}, {-3, 1}, {-2, 2}, {-1, 3}, {0, 3}, {1, 3}, {2, 2}, {3, 1},
    {3, 0}, {3, -1}, {2, -2}, {1, -3}, {0, -3}, {-1, -3}, {-2, -2}, {-3, -1},
    {-3, 0}, {-3, 1}, {-2, 2}, {-1, 3}, {0, 3}, {1, 3}, {2, 2}, {3, 1},
};

static int check_pt(const uint8_t* ptr, int32_t stride, int t)
{
    int cval = ptr[0];
    int max_up_count = 0, max_lo_count = 0;
    int i, up_count = 0, lo_count = 0;

    for( i = 0; i < 16+9; i++ )
    {
        int val = ptr[circle[i][0] + circle[i][1]*stride];
        if( val > cval + t )
            up_count++;
        else
        {
            max_up_count = CT_MAX(max_up_count, up_count);
            up_count = 0;
        }
        if( val < cval - t )
            lo_count++;
        else
        {
            max_lo_count = CT_MAX(max_lo_count, lo_count);
            lo_count = 0;
        }
    }
    max_up_count = CT_MAX(max_up_count, up_count);
    max_lo_count = CT_MAX(max_lo_count, lo_count);
    return max_up_count >= 9 || max_lo_count >= 9;
}

static uint32_t reference_fast(CT_Image src, CT_Image dst, CT_Image mask, int threshold, int nonmax_suppression)
{
    const int r = 3;
    int x, y, width, height;
    int32_t srcstride, dststride;
    uint32_t ncorners = 0;

    ASSERT_(return 0, src && dst);
    ASSERT_(return 0, src->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8);
    ASSERT_(return 0, src->width > 0 && src->height > 0 &&
           src->width == dst->width && src->height == dst->height);
    width = src->width;
    height = src->height;
    srcstride = (int32_t)ct_stride_bytes(src);
    dststride = (int32_t)ct_stride_bytes(dst);
    ct_memset( dst->data.y, 0, (vx_size)dststride*height );

    for( y = r; y < height - r; y++ )
    {
        const uint8_t* srcptr = src->data.y + y*srcstride;
        uint8_t* dstptr = dst->data.y + y*dststride;
        for( x = r; x < width - r; x++ )
        {
            int is_corner = check_pt(srcptr + x, srcstride, threshold);
            int strength = 0;

            if( is_corner )
            {
                // determine the corner strength using binary search
                int a = threshold;
                int b = 255;
                // loop invariant:
                //    1. point is corner with threshold=a
                //    2. point is not a corner with threshold=b
                while( b - a > 1 )
                {
                    int c = (b + a)/2;
                    is_corner = check_pt(srcptr + x, srcstride, c);
                    if( is_corner )
                        a = c;
                    else
                        b = c;
                }
                strength = a;
                ncorners++;
            }
            dstptr[x] = CT_CAST_U8(strength);
        }
    }

    if( nonmax_suppression )
    {
        int32_t maskstride = (int32_t)ct_stride_bytes(mask);

        for( y = r; y < height - r; y++ )
        {
            const uint8_t* dstptr = dst->data.y + y*dststride;
            uint8_t* mptr = mask->data.y + y*maskstride;
            for( x = r; x < width - r; x++ )
            {
                const uint8_t* ptr = dstptr + x;
                int cval = ptr[0];
                mptr[x] = cval >= ptr[-1-dststride] && cval >= ptr[-dststride] && cval >= ptr[-dststride+1] && cval >= ptr[-1] &&
                          cval >  ptr[-1+dststride] && cval >  ptr[ dststride] && cval >  ptr[ dststride+1] && cval >  ptr[ 1];
            }
        }

        ncorners = 0;
        for( y = r; y < height - r; y++ )
        {
            uint8_t* dstptr = dst->data.y + y*dststride;
            const uint8_t* mptr = mask->data.y + y*maskstride;
            for( x = r; x < width - r; x++ )
            {
                if( mptr[x] )
                    ncorners++;
                else
                    dstptr[x] = 0;
            }
        }
    }
    return ncorners;
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

static vx_array own_create_keypoint_array(vx_context context, vx_size count, vx_keypoint_t* keypoints)
{
    vx_array arr = 0;

    ASSERT_VX_OBJECT_(return 0, arr = vxCreateArray(context, VX_TYPE_KEYPOINT, count), VX_TYPE_ARRAY);

    VX_CALL_(return 0, vxAddArrayItems(arr, count, keypoints, sizeof(vx_keypoint_t)));

    return arr;
}

static int get_yuv_params(CT_Image img, uint8_t** ptrY, uint8_t** ptrU, uint8_t** ptrV,
                           uint32_t* strideY, uint32_t* deltaY,
                           uint32_t* strideC, uint32_t* deltaC,
                           uint32_t* shiftX, uint32_t* shiftY, int* code)
{
    int format = img->format;
    int is_yuv = 0;
    uint32_t stride = ct_stride_bytes(img);
    uint32_t height = img->height;

    *ptrY = img->data.y;
    *strideY = *strideC = stride;
    *deltaY = *deltaC = 1;
    *shiftX = *shiftY = 0;

    if( format == VX_DF_IMAGE_YUV4 )
    {
        *ptrU = *ptrY + stride*height;
        *ptrV = *ptrU + stride*height;
        *shiftX = *shiftY = 0;
        is_yuv = 1;
    }
    else if( format == VX_DF_IMAGE_IYUV )
    {
        *ptrU = *ptrY + stride*height;
        *ptrV = *ptrU + (stride*height)/4;
        *strideC = stride/2;
        *shiftX = *shiftY = 1;
        is_yuv = 1;
    }
    else if( format == VX_DF_IMAGE_NV12 || format == VX_DF_IMAGE_NV21 )
    {
        if( format == VX_DF_IMAGE_NV12 )
        {
            *ptrU = *ptrY + stride*height;
            *ptrV = *ptrU + 1;
        }
        else
        {
            *ptrV = *ptrY + stride*height;
            *ptrU = *ptrV + 1;
        }
        *deltaC = 2;
        *shiftX = *shiftY = 1;
        is_yuv = 1;
    }
    else if( format == VX_DF_IMAGE_YUYV || format == VX_DF_IMAGE_UYVY )
    {
        if( format == VX_DF_IMAGE_YUYV )
        {
            *ptrU = *ptrY + 1;
        }
        else
        {
            *ptrU = *ptrY;
            *ptrY = *ptrU + 1;
        }
        *ptrV = *ptrU + 2;
        *deltaY = 2;
        *deltaC = 4;
        *shiftX = 1;
        *shiftY = 0;
        is_yuv = 1;
    }
    *code = *shiftX == 0 ? 444 : *shiftY == 0 ? 422 : 420;
    return is_yuv;
}

static void rgb2yuv_bt709(uint8_t r, uint8_t g, uint8_t b, uint8_t* y, uint8_t* u, uint8_t* v)
{
    int yval = (int)(r*0.2126f + g*0.7152f + b*0.0722f + 0.5f);
    int uval = (int)(-r*0.1146f - g*0.3854 + b*0.5f + 128.5f);
    int vval = (int)(r*0.5f - g*0.4542f - b*0.0458f + 128.5f);
    *y = CT_CAST_U8(yval);
    *u = CT_CAST_U8(uval);
    *v = CT_CAST_U8(vval);
}

static void yuv2rgb_bt709(uint8_t y, uint8_t u, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b)
{
    int rval = (int)(y + 1.5748f*(v-128) + 0.5f);
    int gval = (int)(y - 0.1873f*(u-128) - 0.4681f*(v-128) + 0.5f);
    int bval = (int)(y + 1.8556f*(u-128) + 0.5f);
    *r = CT_CAST_U8(rval);
    *g = CT_CAST_U8(gval);
    *b = CT_CAST_U8(bval);
}

static void reference_colorconvert(CT_Image src, CT_Image dst)
{
    uint32_t x, y, width, height, srcstride, dststride;
    int srcformat = src->format;
    int dstformat = dst->format;
    uint8_t *srcptrY=0, *srcptrU=0, *srcptrV=0;
    uint8_t *dstptrY=0, *dstptrU=0, *dstptrV=0;
    uint32_t srcstrideY=0, srcdeltaY=1, srcstrideC=0, srcdeltaC=1;
    uint32_t dststrideY=0, dstdeltaY=1, dststrideC=0, dstdeltaC=1;
    uint32_t srcshiftX = 1, srcshiftY = 1;
    uint32_t dstshiftX = 1, dstshiftY = 1;
    int srcYUV, dstYUV;
    int srccode=0, dstcode=0;

    ASSERT(src && dst);
    ASSERT(src->width > 0 && src->height > 0 &&
           src->width == dst->width && src->height == dst->height);

    width = src->width;
    height = src->height;
    srcstride = ct_stride_bytes(src);
    dststride = ct_stride_bytes(dst);

    srcYUV = get_yuv_params(src, &srcptrY, &srcptrU, &srcptrV, &srcstrideY,
                            &srcdeltaY, &srcstrideC, &srcdeltaC,
                            &srcshiftX, &srcshiftY, &srccode);
    dstYUV = get_yuv_params(dst, &dstptrY, &dstptrU, &dstptrV, &dststrideY,
                            &dstdeltaY, &dststrideC, &dstdeltaC,
                            &dstshiftX, &dstshiftY, &dstcode);

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = ct_channels(srcformat);
        if( dstformat == VX_DF_IMAGE_RGB || dstformat == VX_DF_IMAGE_RGBX )
        {
            int dcn = ct_channels(dstformat);

            for( y = 0; y < height; y++ )
            {
                const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                uint8_t* dstptr = (uint8_t*)(dst->data.y + y*dststride);
                for( x = 0; x < width; x++, srcptr += scn, dstptr += dcn )
                {
                    dstptr[0] = srcptr[0];
                    dstptr[1] = srcptr[1];
                    dstptr[2] = srcptr[2];
                    if(dcn == 4)
                        dstptr[3] = 255;
                }
            }

        }
        else if( dstYUV )
        {
            if( dstcode == 444 )
            {
                for( y = 0; y < height; y++ )
                {
                    const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                    for( x = 0; x < width; x++, srcptr += scn )
                    {
                        rgb2yuv_bt709(srcptr[0], srcptr[1], srcptr[2],
                                      dstptrY + dststrideY*y + dstdeltaY*x,
                                      dstptrU + dststrideC*y + dstdeltaC*x,
                                      dstptrV + dststrideC*y + dstdeltaC*x);
                    }
                }
            }
            else if( dstcode == 422 )
            {
                for( y = 0; y < height; y++ )
                {
                    const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                    for( x = 0; x < width; x += 2, srcptr += scn*2 )
                    {
                        uint8_t u0 = 0, v0 = 0, u1 = 0, v1 = 0;
                        rgb2yuv_bt709(srcptr[0], srcptr[1], srcptr[2],
                                      dstptrY + dststrideY*y + dstdeltaY*x, &u0, &v0);
                        rgb2yuv_bt709(srcptr[scn], srcptr[scn+1], srcptr[scn+2],
                                      dstptrY + dststrideY*y + dstdeltaY*(x+1), &u1, &v1);
                        dstptrU[dststrideC*y + dstdeltaC*(x/2)] = (uint8_t)((u0 + u1) >> 1);
                        dstptrV[dststrideC*y + dstdeltaC*(x/2)] = (uint8_t)((v0 + v1) >> 1);
                    }
                }
            }
            else if( dstcode == 420 )
            {
                for( y = 0; y < height; y += 2 )
                {
                    const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                    for( x = 0; x < width; x += 2, srcptr += scn*2 )
                    {
                        uint8_t u[4], v[4];
                        rgb2yuv_bt709(srcptr[0], srcptr[1], srcptr[2],
                                      dstptrY + dststrideY*y + dstdeltaY*x, &u[0], &v[0]);
                        rgb2yuv_bt709(srcptr[scn], srcptr[scn+1], srcptr[scn+2],
                                      dstptrY + dststrideY*y + dstdeltaY*(x+1), &u[1], &v[1]);
                        rgb2yuv_bt709(srcptr[srcstride+0], srcptr[srcstride+1], srcptr[srcstride+2],
                                      dstptrY + dststrideY*(y+1) + dstdeltaY*x, &u[2], &v[2]);
                        rgb2yuv_bt709(srcptr[srcstride+scn], srcptr[srcstride+scn+1], srcptr[srcstride+scn+2],
                                      dstptrY + dststrideY*(y+1) + dstdeltaY*(x+1), &u[3], &v[3]);
                        dstptrU[dststrideC*(y/2) + dstdeltaC*(x/2)] = (uint8_t)((u[0] + u[1] + u[2] + u[3]) >> 2);
                        dstptrV[dststrideC*(y/2) + dstdeltaC*(x/2)] = (uint8_t)((v[0] + v[1] + v[2] + v[3]) >> 2);
                    }
                }
            }
        }
    }
    else if( srcYUV )
    {
        if( dstformat == VX_DF_IMAGE_RGB || dstformat == VX_DF_IMAGE_RGBX )
        {
            int dcn = ct_channels(dstformat);

            for( y = 0; y < height; y++ )
            {
                uint8_t* dstptr = (uint8_t*)(dst->data.y + y*dststride);
                for( x = 0; x < width; x++, dstptr += dcn )
                {
                    int xc = x >> srcshiftX, yc = y >> srcshiftY;
                    yuv2rgb_bt709(srcptrY[srcstrideY*y + srcdeltaY*x],
                                  srcptrU[srcstrideC*yc + srcdeltaC*xc],
                                  srcptrV[srcstrideC*yc + srcdeltaC*xc],
                                  dstptr, dstptr + 1, dstptr + 2);
                    if( dcn == 4 )
                        dstptr[3] = 255;
                }
            }
        }
        else if( dstYUV )
        {
            if( srccode <= dstcode )
            {
                // if both src and dst are YUV formats and
                // the source image chroma resolution
                // is smaller then we just replicate the chroma components
                for( y = 0; y < height; y++ )
                {
                    for( x = 0; x < width; x++ )
                    {
                        int dstYC = y >> dstshiftY, dstXC = x >> dstshiftX;
                        int srcYC = y >> srcshiftY, srcXC = x >> srcshiftX;
                        dstptrY[dststrideY*y + dstdeltaY*x] = srcptrY[srcstrideY*y + srcdeltaY*x];
                        dstptrU[dststrideC*dstYC + dstdeltaC*dstXC] = srcptrU[srcstrideC*srcYC + srcdeltaC*srcXC];
                        dstptrV[dststrideC*dstYC + dstdeltaC*dstXC] = srcptrV[srcstrideC*srcYC + srcdeltaC*srcXC];
                    }
                }
            }
            else if( srccode == 422 && dstcode == 420 )
            {
                // if both src and dst are YUV formats and
                // the source image chroma resolution
                // is larger then we have to average chroma samples
                for( y = 0; y < height; y += 2 )
                {
                    for( x = 0; x < width; x++ )
                    {
                        int dstYC = y >> dstshiftY, dstXC = x >> dstshiftX;
                        int srcYC = y >> srcshiftY, srcXC = x >> srcshiftX;
                        dstptrY[dststrideY*y + dstdeltaY*x] = srcptrY[srcstrideY*y + srcdeltaY*x];
                        dstptrY[dststrideY*(y+1) + dstdeltaY*x] = srcptrY[srcstrideY*(y+1) + srcdeltaY*x];

                        dstptrU[dststrideC*dstYC + dstdeltaC*dstXC] =
                            (uint8_t)((srcptrU[srcstrideC*srcYC + srcdeltaC*srcXC] +
                                       srcptrU[srcstrideC*(srcYC+1) + srcdeltaC*srcXC]) >> 1);

                        dstptrV[dststrideC*dstYC + dstdeltaC*dstXC] =
                            (uint8_t)((srcptrV[srcstrideC*srcYC + srcdeltaC*srcXC] +
                                       srcptrV[srcstrideC*(srcYC+1) + srcdeltaC*srcXC]) >> 1);
                    }
                }
            }
        }
    }
}

static int32_t dilate_get(int32_t *values)
{
    int i;
    int32_t v = values[0];
    for (i = 1; i < 9; i++)
        v = (v < values[i]) ? values[i] : v;
    return v;
}

static uint8_t dilate3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)dilate_get(values);
}

static uint8_t dilate3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)dilate_get(values);
}

static uint8_t dilate3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)dilate_get(values);
}


static CT_Image dilate3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = dilate3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = dilate3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = dilate3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}


static void dilate3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = dilate3x3_create_reference_image(src, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

static int cmp_color_images(CT_Image img0, CT_Image img1, int ythresh, int cthresh)
{
    uint32_t x, y, width, height, stride0, stride1;
    int format0 = img0->format;
    int format1 = img1->format;
    uint8_t *ptrY0=0, *ptrU0=0, *ptrV0=0;
    uint8_t *ptrY1=0, *ptrU1=0, *ptrV1=0;
    uint32_t strideY0=0, deltaY0=1, strideC0=0, deltaC0=1;
    uint32_t strideY1=0, deltaY1=1, strideC1=0, deltaC1=1;
    uint32_t shiftX0 = 1, shiftY0 = 1;
    uint32_t shiftX1 = 1, shiftY1 = 1;
    int YUV0, YUV1;
    int code0=0, code1=0;

    ASSERT_(return -1, img0 && img1);
    ASSERT_(return -1, img0->width > 0 && img0->height > 0 &&
           img0->width == img1->width && img0->height == img1->height &&
           format0 == format1);

    width = img0->width;
    height = img0->height;
    stride0 = ct_stride_bytes(img0);
    stride1 = ct_stride_bytes(img1);

    YUV0 = get_yuv_params(img0, &ptrY0, &ptrU0, &ptrV0, &strideY0,
                            &deltaY0, &strideC0, &deltaC0,
                            &shiftX0, &shiftY0, &code0);
    YUV1 = get_yuv_params(img1, &ptrY1, &ptrU1, &ptrV1, &strideY1,
                          &deltaY1, &strideC1, &deltaC1,
                          &shiftX1, &shiftY1, &code1);

    if( format0 == VX_DF_IMAGE_RGB || format0 == VX_DF_IMAGE_RGBX )
    {
        int cn = ct_channels(format0);
        for( y = 0; y < height; y++ )
        {
            const uint8_t* ptr0 = (const uint8_t*)(img0->data.y + y*stride0);
            const uint8_t* ptr1 = (const uint8_t*)(img1->data.y + y*stride1);
            for( x = 0; x < width*cn; x++ )
            {
                if( abs(ptr0[x] - ptr1[x]) > ythresh )
                {
                    printf("images are very different at (%d, %d): %d vs %d\n", x, y, ptr0[x], ptr1[x]);
                    return -1;
                }
            }
        }
    }
    else
    {
        ASSERT_(return -1, YUV0 != 0 && YUV1 != 0 && code0 == code1);
        for( y = 0; y < height; y++ )
        {
            const uint8_t* tempptrY0 = (const uint8_t*)(ptrY0 + y*strideY0);
            const uint8_t* tempptrY1 = (const uint8_t*)(ptrY1 + y*strideY1);
            const uint8_t* tempptrU0_row = (const uint8_t*)(ptrU0 + (y>>shiftY0)*strideC0);
            const uint8_t* tempptrU1_row = (const uint8_t*)(ptrU1 + (y>>shiftY1)*strideC1);
            const uint8_t* tempptrV0_row = (const uint8_t*)(ptrV0 + (y>>shiftY0)*strideC0);
            const uint8_t* tempptrV1_row = (const uint8_t*)(ptrV1 + (y>>shiftY1)*strideC1);
            for( x = 0; x < width; x++, tempptrY0 += deltaY0, tempptrY1 += deltaY1 )
            {
                const uint8_t* tempptrU0 = tempptrU0_row + (x >> shiftX0)*deltaC0;
                const uint8_t* tempptrU1 = tempptrU1_row + (x >> shiftX1)*deltaC1;
                const uint8_t* tempptrV0 = tempptrV0_row + (x >> shiftX0)*deltaC0;
                const uint8_t* tempptrV1 = tempptrV1_row + (x >> shiftX1)*deltaC1;

                if( abs(tempptrY0[0] - tempptrY1[0]) > ythresh ||
                    abs(tempptrU0[0] - tempptrU1[0]) > cthresh ||
                    abs(tempptrV0[0] - tempptrV1[0]) > cthresh )
                {
                    printf("images are very different at (%d, %d): (%d, %d, %d) vs (%d, %d, %d)\n",
                           x, y, tempptrY0[0], tempptrU0[0], tempptrV0[0], tempptrY1[0], tempptrU1[0], tempptrV1[0]);
                    return -1;
                }
            }
        }
    }
    return 0;
}


static int32_t erode_get(int32_t *values)
{
    int i;
    int32_t v = values[0];
    for (i = 1; i < 9; i++)
        v = (v > values[i]) ? values[i] : v;
    return v;
}

static uint8_t erode3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)erode_get(values);
}

static uint8_t erode3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)erode_get(values);
}

static uint8_t erode3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)erode_get(values);
}


static CT_Image erode3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = erode3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = erode3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = erode3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}


static void erode3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = erode3x3_create_reference_image(src, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}


static int32_t gaussian_get(int32_t *values)
{
    int32_t res = (values[0] + values[2] + values[6] + values[8]) * 1 +
                (values[1] + values[3] + values[5] + values[7]) * 2 +
                (values[4]) * 4;
    res = res >> 4;
    return res;
}

static uint8_t gaussian3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)gaussian_get(values);
}

static uint8_t gaussian3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)gaussian_get(values);
}

static uint8_t gaussian3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)gaussian_get(values);
}


static CT_Image gaussian3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = gaussian3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = gaussian3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}


static void gaussian3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = gaussian3x3_create_reference_image(src, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}


static int compare_for_median_get(const void * a, const void * b)
{
    return *(int*)a - *(int*)b;
}

static int32_t median_get(int32_t *values)
{
    qsort(values, 9, sizeof(values[0]), compare_for_median_get);
    return values[4];
}

static uint8_t median3x3_calculate(CT_Image src, uint32_t x, uint32_t y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    return (uint8_t)median_get(values);
}

static uint8_t median3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    return (uint8_t)median_get(values);
}

static uint8_t median3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    return (uint8_t)median_get(values);
}


static CT_Image median3x3_create_reference_image(CT_Image src, vx_border_t border)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, src->format);

    if (border.mode == VX_BORDER_UNDEFINED)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                if (x >= 1 && y >= 1 && x < src->width - 1 && y < src->height - 1)
                {
                    uint8_t res = median3x3_calculate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = median3x3_calculate_replicate(src, x, y);
                    *dst_data = res;
                });
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        vx_uint32 constant_value = border.constant_value.U32;
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    uint8_t res = median3x3_calculate_constant(src, x, y, constant_value);
                    *dst_data = res;
                });
    }
    else
    {
        ASSERT_(return 0, 0);
    }
    return dst;
}


static void median3x3_check(CT_Image src, CT_Image dst, vx_border_t border)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = median3x3_create_reference_image(src, border));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst,  1, 1, 1, 1);
            ct_adjust_roi(dst_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

static int32_t simple_get(CT_Image src, int32_t x, int32_t y, vx_border_t border,
        vx_df_image dst_format)
{
    int32_t value = 0;
    int valid_values = 0;

    ASSERT_NO_FAILURE_(return -1,
            valid_values = ct_image_read_rect_S32(src, &value, x, y, x, y, border));

    if (valid_values == 0)
        return INT32_MIN;

    if (dst_format == VX_DF_IMAGE_U8)
    {
        if (value < 0) value = 0;
        else if (value > UINT8_MAX) value = UINT8_MAX;
    }

    return value;
}

#define MAX_CONV_SIZE 15

static int32_t convolve_get(CT_Image src, int32_t x, int32_t y, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_int32 scale, vx_df_image dst_format)
{
    int i;
    int32_t sum = 0, value = 0;
    int32_t src_data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    int valid_values = 0;

    ASSERT_(return -1, cols <= MAX_CONV_SIZE);
    ASSERT_(return -1, rows <= MAX_CONV_SIZE);

    ASSERT_NO_FAILURE_(return -1,
            valid_values = ct_image_read_rect_S32(src, src_data, x - cols / 2, y - rows / 2, x + cols / 2, y + rows / 2, border));

    if (valid_values == 0)
        return INT32_MIN;

    for (i = 0; i < cols * rows; ++i)
        sum += src_data[i] * data[i];

    value = sum / scale;

    if (dst_format == VX_DF_IMAGE_U8)
    {
        if (value < 0) value = 0;
        else if (value > UINT8_MAX) value = UINT8_MAX;
    }

    return value;
}

static vx_int16 gaussian3x3_kernel[9] = {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
};
static const vx_int32 gaussian3x3_scale = 16;

static vx_int16 gaussian5x5_kernel[25] = {
        1,  4,  6,  4, 1,
        4, 16, 24, 16, 4,
        6, 24, 36, 24, 6,
        4, 16, 24, 16, 4,
        1,  4,  6,  4, 1
};
static const vx_int32 gaussian5x5_scale = 256;

static int32_t halfScaleGaussian_get_pixel(CT_Image src, int x, int y, vx_int32 kernel_size, vx_border_t border)
{
    if (kernel_size == 1)
    {
        int32_t res = simple_get(src, x, y, border, VX_DF_IMAGE_U8);
        return res;
    }
    else if (kernel_size == 3)
    {
        int32_t res = convolve_get(src, x, y, border, 3, 3, gaussian3x3_kernel, gaussian3x3_scale, VX_DF_IMAGE_U8);
        return res;
    }
    else if (kernel_size == 5)
    {
        int32_t res = convolve_get(src, x, y, border, 5, 5, gaussian5x5_kernel, gaussian5x5_scale, VX_DF_IMAGE_U8);
        return res;
    }
    CT_FAIL_(return -1, "Invalid kernel size");
}

static int halfScaleGaussian_check_pixel(CT_Image src, CT_Image dst, int x, int y, vx_int32 kernel_size, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float64 x_src = (((vx_float64)x + 0.5f) * (vx_float64)src->width / (vx_float64)dst->width) - 0.5f;
    vx_float64 y_src = (((vx_float64)y + 0.5f) * (vx_float64)src->height / (vx_float64)dst->height) - 0.5f;
    int x_min = (int)floor(x_src), y_min = (int)floor(y_src);
    int sx, sy;
    for (sy = -1; sy <= 1; sy++)
    {
        for (sx = -1; sx <= 1; sx++)
        {
            vx_int32 candidate = 0;
            ASSERT_NO_FAILURE_(return 0, candidate = halfScaleGaussian_get_pixel(src, x_min + sx, y_min + sy, kernel_size, border));
            if (candidate < 0 || abs(candidate - res) <= HALFSCALEGAUSSIAN_TOLERANCE)
                return 1;
        }
    }
#if 0
    for (sy = -1; sy <= 1; sy++)
    {
        for (sx = -1; sx <= 1; sx++)
        {
            vx_int32 candidate = 0;
            ASSERT_NO_FAILURE_(return 0, candidate = halfScaleGaussian_get_pixel(src, x_min + sx, y_min + sy, kernel_size, border));
            printf("Check failed for pixel (%d+%d, %d+%d): %d\n", x, sx, y, sy, (int)candidate);
        }
    }
#endif
    CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
}

static void halfScaleGaussian_validate(CT_Image src, CT_Image dst, vx_int32 kernel_size, vx_border_t border)
{
    ASSERT(src && dst);
    CT_FILL_IMAGE_8U(, dst,
            {
                ASSERT_NO_FAILURE(halfScaleGaussian_check_pixel(src, dst, x, y, kernel_size, border));
            });
}

static int16_t sobel_x_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[2]) +
                  (-values[3] * 2) + (values[5] * 2) +
                  (-values[6])     + (values[8]);
    return (int16_t)res;
}

static int16_t sobel_y_get(int32_t *values)
{
    int32_t res = (-values[0])     + (values[6]) +
                  (-values[1] * 2) + (values[7] * 2) +
                  (-values[2])     + (values[8]);
    return (int16_t)res;
}

static void sobel3x3_calculate(CT_Image src, uint32_t x, uint32_t y, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t values[9] = {
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y - 0),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x - 1, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 0, y + 1),
        (int32_t)*CT_IMAGE_DATA_PTR_8U(src, x + 1, y + 1)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_replicate(CT_Image src, uint32_t x_, uint32_t y_, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y - 0),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x - 1, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 0, y + 1),
        (int32_t)CT_IMAGE_DATA_REPLICATE_8U(src, x + 1, y + 1)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_calculate_constant(CT_Image src, uint32_t x_, uint32_t y_, vx_uint32 constant_value, int16_t *sobel_x, int16_t *sobel_y)
{
    int32_t x = (int)x_;
    int32_t y = (int)y_;
    int32_t values[9] = {
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y - 0, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x - 1, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 0, y + 1, constant_value),
        (int32_t)CT_IMAGE_DATA_CONSTANT_8U(src, x + 1, y + 1, constant_value)
    };
    *sobel_x = sobel_x_get(values);
    *sobel_y = sobel_y_get(values);
}

static void sobel3x3_check_x(CT_Image src, CT_Image dst_x, vx_border_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL;

    ASSERT(src && dst_x);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst_x,  1, 1, 1, 1);
            ct_adjust_roi(dst_x_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_x_ref, dst_x);
}

enum CT_AffineMatrixType {
    VX_MATRIX_IDENT = 0,
    VX_MATRIX_ROTATE_90,
    VX_MATRIX_SCALE,
    VX_MATRIX_SCALE_ROTATE,
    VX_MATRIX_RANDOM
};

#ifndef M_PI
#define M_PIF   3.14159265358979323846f
#else
#define M_PIF   (vx_float32)M_PI
#endif

#define VX_NN_AREA_SIZE         1.5
#define VX_BILINEAR_TOLERANCE   1

static CT_Image warp_affine_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);
static void warp_affine_generate_matrix(vx_float32* m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][2];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_ROTATE_90 == type)
    {
        mat[0][0] = 0.f;
        mat[0][1] = 1.f;

        mat[1][0] = -1.f;
        mat[1][1] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
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
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_affine_create_matrix(vx_context context, vx_float32 *m)
{
    vx_matrix matrix;
    matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);
    if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
    {
        if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
        {
            VX_CALL_(return 0, vxReleaseMatrix(&matrix));
        }
    }
    return matrix;
}

static int warp_affine_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_float64 x0, y0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    x0 = (vx_float64)m[2 * 0 + 0] * (vx_float64)x + (vx_float64)m[2 * 1 + 0] * (vx_float64)y + (vx_float64)m[2 * 2 + 0];
    y0 = (vx_float64)m[2 * 0 + 1] * (vx_float64)x + (vx_float64)m[2 * 1 + 1] * (vx_float64)y + (vx_float64)m[2 * 2 + 1];

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
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
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
            if (0 <= xi && 0 <= yi && xi < (vx_int32)input->width - 1 && yi < (vx_int32)input->height - 1)
            {
                candidate = (int)((1. - s) * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi    ) +
                                        s  * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi    ) +
                                  (1. - s) *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi + 1) +
                                        s  *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi + 1));
            }
        }
        else if (VX_BORDER_CONSTANT == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    CT_FAIL_(return 1, "Interpolation type undefined");
}

static void warp_affine_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += warp_affine_check_pixel(input, output, x, y, interp_type, border, m));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static void warp_affine_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    ASSERT(input && output);
    ASSERT( (interp_type == VX_INTERPOLATION_NEAREST_NEIGHBOR) ||
            (interp_type == VX_INTERPOLATION_BILINEAR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT));

    warp_affine_validate(input, output, interp_type, border, m);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
        printf("Matrix:\n%g %g %g\n%g %g %g\n",
                m[0], m[2], m[4],
                m[1], m[3], m[5]);
    }
}


#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);
static void warp_perspective_generate_matrix(vx_float32 *m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][3];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;
        mat[0][2] = 0.f;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
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
        mat[0][2] = RND_FLT(0.f, 0.1f);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);
        mat[1][2] = RND_FLT(0.f, 0.1f);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][2] = 1.f;
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_perspective_create_matrix(vx_context context, vx_float32 *m)
{
    vx_matrix matrix;
    matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
    if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
    {
        if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
        {
            VX_CALL_(return 0, vxReleaseMatrix(&matrix));
        }
    }
    return matrix;
}

static int warp_perspective_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_float64 x0, y0, z0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    x0 = (vx_float64)m[3 * 0 + 0] * (vx_float64)x + (vx_float64)m[3 * 1 + 0] * (vx_float64)y + (vx_float64)m[3 * 2 + 0];
    y0 = (vx_float64)m[3 * 0 + 1] * (vx_float64)x + (vx_float64)m[3 * 1 + 1] * (vx_float64)y + (vx_float64)m[3 * 2 + 1];
    z0 = (vx_float64)m[3 * 0 + 2] * (vx_float64)x + (vx_float64)m[3 * 1 + 2] * (vx_float64)y + (vx_float64)m[3 * 2 + 2];
    if (fabs(z0) < DBL_MIN)
        return 0;

    x0 = x0 / z0;
    y0 = y0 / z0;
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
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
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
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    CT_FAIL_(return 1, "Interpolation type undefined");
}

static void warp_perspective_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += warp_perspective_check_pixel(input, output, x, y, interp_type, border, m));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static void warp_perspective_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    ASSERT(input && output);
    ASSERT( (interp_type == VX_INTERPOLATION_NEAREST_NEIGHBOR) ||
            (interp_type == VX_INTERPOLATION_BILINEAR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT));

    warp_perspective_validate(input, output, interp_type, border, m);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
        printf("Matrix:\n%g %g %g\n%g %g %g\n%g %g %g\n",
                m[0], m[3], m[6],
                m[1], m[4], m[7],
                m[2], m[5], m[8]);
    }
}

static void referenceMultiply(CT_Image src0, CT_Image src1, CT_Image dst, CT_Image dst_plus_1, vx_float32 scale, enum vx_convert_policy_e policy)
{
    int32_t min_bound, max_bound;
    uint32_t i, j;

    ASSERT(src0 && src1 && dst && dst_plus_1);
    ASSERT(src0->width  == src1->width  && src0->width  == dst->width  && src0->width  == dst_plus_1->width);
    ASSERT(src0->height == src1->height && src0->height == dst->height && src0->height == dst_plus_1->height);
    ASSERT(dst->format == dst_plus_1->format);

    switch (policy)
    {
        case VX_CONVERT_POLICY_SATURATE:
            if (dst->format == VX_DF_IMAGE_U8)
            {
                min_bound = 0;
                max_bound = 255;
            }
            else if (dst->format == VX_DF_IMAGE_S16)
            {
                min_bound = -32768;
                max_bound =  32767;
            }
            else
                FAIL("Unsupported result format: (%.4s)", &dst->format);
            break;
        case VX_CONVERT_POLICY_WRAP:
            min_bound = INT32_MIN;
            max_bound = INT32_MAX;
            break;
        default: FAIL("Unknown owerflow policy"); break;
    };

#define MULTIPLY_LOOP(s0, s1, r)                                                                                \
    do{                                                                                                         \
        for (i = 0; i < dst->height; ++i)                                                                       \
            for (j = 0; j < dst->width; ++j)                                                                    \
            {                                                                                                   \
                int32_t val0 = src0->data.s0[i * src0->stride + j];                                             \
                int32_t val1 = src1->data.s1[i * src1->stride + j];                                             \
                /* use double precision because in S16*S16 case (val0*val1) can be not representable as float */\
                int32_t res0 = (int32_t)floor(((double)(val0 * val1)) * scale);                                 \
                int32_t res1 = res0 + 1;                                                                        \
                dst->data.r[i * dst->stride + j] = (res0 < min_bound ? min_bound :                              \
                                                                        (res0 > max_bound ? max_bound : res0)); \
                dst_plus_1->data.r[i * dst_plus_1->stride + j] = (res1 < min_bound ? min_bound :                \
                                                                        (res1 > max_bound ? max_bound : res1)); \
            }                                                                                                   \
    }while(0)

    if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_U8)
        MULTIPLY_LOOP(y, y, y);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(y, y, s16);
    else if (src0->format == VX_DF_IMAGE_U8 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(y, s16, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_U8 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(s16, y, s16);
    else if (src0->format == VX_DF_IMAGE_S16 && src1->format == VX_DF_IMAGE_S16 && dst->format == VX_DF_IMAGE_S16)
        MULTIPLY_LOOP(s16, s16, s16);
    else
        FAIL("Unsupported combination of argument formats: %.4s + %.4s = %.4s", &src0->format, &src1->format, &dst->format);

#undef MULTIPLY_LOOP
}

static vx_size lut_count(vx_enum data_type)
{
    vx_size count = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        count = 256;
        break;
    case VX_TYPE_INT16:
        count = 65536;
        break;
    }

    return count;
}

static vx_size lut_size(vx_enum data_type)
{
    vx_size size = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        size = 256*sizeof(vx_uint8);
        break;
    case VX_TYPE_INT16:
        size = 65536*sizeof(vx_int16);
        break;
    }

    return size;
}

static vx_lut lut_create(vx_context context, void* data, vx_enum data_type)
{
    vx_size count = lut_count(data_type);
    vx_size size = lut_size(data_type);

    vx_lut lut = vxCreateLUT(context, data_type, count);
    void* ptr = NULL;

    ASSERT_VX_OBJECT_(return 0, lut, VX_TYPE_LUT);

    vx_map_id map_id;
    VX_CALL_(return 0, vxMapLUT(lut, &map_id, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT_(return 0, ptr);
    memcpy(ptr, data, size);
    VX_CALL_(return 0, vxUnmapLUT(lut, map_id));
    return lut;
}

static void lut_data_fill_random(void* data, vx_enum data_type)
{
    uint64_t* seed = &CT()->seed_;
    int i;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        {
            vx_uint8* data8 = (vx_uint8*)data;
            for (i = 0; i < 256; ++i)
                data8[i] = (vx_uint8)CT_RNG_NEXT_INT(*seed, 0, 256);
        }
        break;
    case VX_TYPE_INT16:
        {
            vx_int16* data16 = (vx_int16*)data;
            for (i = 0; i < 65536; ++i)
                data16[i] = (vx_int16)CT_RNG_NEXT_INT(*seed, (uint32_t)-32768, 32768);
        }
        break;
    }
}

// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image lut_image_generate_random(const char* fileName, int width, int height, vx_enum data_type)
{
    CT_Image image = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256);
        break;
    case VX_TYPE_INT16:
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_S16, &CT()->seed_, -32768, 32768);
        break;
    }
    ASSERT_(return 0, image != 0);

    return image;
}

static CT_Image generate_random(int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

// data_type == VX_TYPE_UINT8
static vx_uint8 lut_calculate_u8(CT_Image src, uint32_t x, uint32_t y, void* lut_data)
{
    vx_uint8* lut_data8 = (vx_uint8*)lut_data;
    vx_int32 offset = 0;
    vx_uint8 value = *CT_IMAGE_DATA_PTR_8U(src, x, y);
    vx_uint8 res = lut_data8[offset + value];
    return res;
}

// data_type == VX_TYPE_INT16
static vx_int16 lut_calculate_s16(CT_Image src, uint32_t x, uint32_t y, void* lut_data)
{
    vx_int16* lut_data16 = (vx_int16*)lut_data;
    vx_int32 offset = 65536/2;
    vx_int16 value = *CT_IMAGE_DATA_PTR_16S(src, x, y);
    vx_int16 res = lut_data16[offset + value];
    return res;
}

static CT_Image lut_create_reference_image(CT_Image src, void* lut_data, vx_enum data_type)
{
    CT_Image dst;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);
        break;
    case VX_TYPE_INT16:
        CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_S16);
        break;
    }

    dst = ct_allocate_image(src->width, src->height, src->format);

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        CT_FILL_IMAGE_8U(return 0, dst,
            {
                uint8_t res = lut_calculate_u8(src, x, y, lut_data);
                *dst_data = res;
            });
        break;
    case VX_TYPE_INT16:
        CT_FILL_IMAGE_16S(return 0, dst,
            {
                int16_t res = lut_calculate_s16(src, x, y, lut_data);
                *dst_data = res;
            });
        break;
    }

    return dst;
}


static void lut_check(CT_Image src, CT_Image dst, void* lut_data, vx_enum data_type)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = lut_create_reference_image(src, lut_data, data_type));

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

static void reference_mag(CT_Image dx, CT_Image dy, CT_Image mag)
{
    uint32_t x, y, width, height, dxstride, dystride, magstride;

    ASSERT(dx && dy && mag);
    ASSERT(dx->format == VX_DF_IMAGE_S16 && dy->format == VX_DF_IMAGE_S16 && mag->format == VX_DF_IMAGE_S16);
    ASSERT(dx->width > 0 && dx->height > 0 &&
           dx->width == dy->width && dx->height == dy->height &&
           dx->width == mag->width && dx->height == mag->height);
    width = dx->width;
    height = dy->height;
    dxstride = ct_stride_bytes(dx);
    dystride = ct_stride_bytes(dy);
    magstride = ct_stride_bytes(mag);

    for( y = 0; y < height; y++ )
    {
        const int16_t* dxptr = (const int16_t*)(dx->data.y + y*dxstride);
        const int16_t* dyptr = (const int16_t*)(dy->data.y + y*dystride);
        int16_t* magptr = (int16_t*)(mag->data.y + y*magstride);
        for( x = 0; x < width; x++ )
        {
            // the specification says - use double in the test implementation
            double val = sqrt((double)dxptr[x]*dxptr[x] + (double)dyptr[x]*dyptr[x]);
            int ival = (int)floor(val + 0.5);
            magptr[x] = CT_CAST_S16(ival);
        }
    }
}

static void reference_phase(CT_Image dx, CT_Image dy, CT_Image phase)
{
    uint32_t x, y, width, height, dxstride, dystride, phasestride;

    ASSERT(dx && dy && phase);
    ASSERT(dx->format == VX_DF_IMAGE_S16 && dy->format == VX_DF_IMAGE_S16 && phase->format == VX_DF_IMAGE_U8);
    ASSERT(dx->width > 0 && dx->height > 0 &&
           dx->width == dy->width && dx->height == dy->height &&
           dx->width == phase->width && dx->height == phase->height);
    width = dx->width;
    height = dy->height;
    dxstride = ct_stride_bytes(dx);
    dystride = ct_stride_bytes(dy);
    phasestride = ct_stride_bytes(phase);

    for( y = 0; y < height; y++ )
    {
        const int16_t* dxptr = (const int16_t*)(dx->data.y + y*dxstride);
        const int16_t* dyptr = (const int16_t*)(dy->data.y + y*dystride);
        uint8_t* phaseptr = (uint8_t*)(phase->data.y + y*phasestride);
        for( x = 0; x < width; x++ )
        {
            double val = atan2(dyptr[x], dxptr[x])*256/(3.1415926535897932384626433832795*2);
            int ival;
            if( val < 0 )
                val += 256.;
            ival = (int)floor(val + 0.5);
            if( ival >= 256 )
                ival -= 256;
            phaseptr[x] = CT_CAST_U8(ival);
        }
    }
}

static vx_int32 ct_image_get_pixel_8u(CT_Image img, int x, int y, vx_border_t border)
{
    if (border.mode == VX_BORDER_UNDEFINED)
    {
        if (x < 0 || x >= (int)img->width || y < 0 || y >= (int)img->height)
            return -1; //border
        return *CT_IMAGE_DATA_PTR_8U(img, x, y);
    }
    else if (border.mode == VX_BORDER_REPLICATE)
    {
        return CT_IMAGE_DATA_REPLICATE_8U(img, x, y);
    }
    else if (border.mode == VX_BORDER_CONSTANT)
    {
        return CT_IMAGE_DATA_CONSTANT_8U(img, x, y, border.constant_value.U8);
    }
    else
    {
        CT_FAIL_(return -1, "Invalid border type");
    }
}

static int scale_check_pixel(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float32 x_src = (((vx_float32)x + 0.5f) * (vx_float32)src->width / (vx_float32)dst->width) - 0.5f;
    vx_float32 y_src = (((vx_float32)y + 0.5f) * (vx_float32)src->height / (vx_float32)dst->height) - 0.5f;
    int x_min = (int)floorf(x_src), y_min = (int)floorf(y_src);
    if (interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        int sx, sy;
        for (sy = -1; sy <= 1; sy++)
        {
            for (sx = -1; sx <= 1; sx++)
            {
                vx_int32 candidate = 0;
                ASSERT_NO_FAILURE_(return 0, candidate = ct_image_get_pixel_8u(src, x_min + sx, y_min + sy, border));
                if (candidate == -1 || candidate == res)
                    return 1;
            }
        }
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        vx_float32 s = x_src - x_min;
        vx_float32 t = y_src - y_min;
        vx_int32 p00 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 0, border);
        vx_int32 p01 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 1, border);
        vx_int32 p10 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 0, border);
        vx_int32 p11 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 1, border);
        vx_float32 ref_float;
        vx_int32 ref;

        // We don't compare the output when an input pixel is undefined (UNDEFINED border mode)
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e) {
            return 1;
        }

        // Compute the expected result (float)
        ref_float = (1 - s) * (1 - t) * p00 +
                    (    s) * (1 - t) * p10 +
                    (1 - s) * (    t) * p01 +
                    (    s) * (    t) * p11;

        // Take the nearest integer to avoid problems with casts in case of float rounding errors
        // (e.g: 30.999999 should give 31, not 30)
        ref = (vx_int32)(ref_float + 0.5f);

        // A difference of 1 is allowed
        if (abs(res - ref) <= 1) {
            return 1;
        }

        return 0; // don't generate failure, we will check num failed pixels later
    }
    if (interpolation == VX_INTERPOLATION_AREA)
    {
        vx_int32 v_min = 256, v_max = -1;
        int sx, sy;
        // check values at 5x5 area
        for (sy = -2; sy <= 2; sy++)
        {
            for (sx = -2; sx <= 2; sx++)
            {
                vx_int32 candidate = 0;
                ASSERT_NO_FAILURE_(return 0, candidate = ct_image_get_pixel_8u(src, x_min + sx, y_min + sy, border));
                if (candidate == -1)
                    return 1;
                if (v_min > candidate)
                    v_min = candidate;
                if (v_max < candidate)
                    v_max = candidate;
            }
            if (v_min <= res && v_max >= res)
                return 1;
        }
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    CT_FAIL_(return 0, "NOT IMPLEMENTED");
}

static int scale_check_pixel_exact(CT_Image src, CT_Image dst, int x, int y, vx_enum interpolation, vx_border_t border)
{
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(dst, x, y);
    vx_float32 x_src = (((vx_float32)x + 0.5f) * (vx_float32)src->width / (vx_float32)dst->width) - 0.5f;
    vx_float32 y_src = (((vx_float32)y + 0.5f) * (vx_float32)src->height / (vx_float32)dst->height) - 0.5f;
    vx_float32 x_minf = floorf(x_src);
    vx_float32 y_minf = floorf(y_src);
    int x_min = (vx_int32)x_minf;
    int y_min = (vx_int32)y_minf;
    int x_ref = x_min;
    int y_ref = y_min;
    if (x_src - x_minf >= 0.5f)
        x_ref++;
    if (y_src - y_minf >= 0.5f)
        y_ref++;
    if (interpolation == VX_INTERPOLATION_NEAREST_NEIGHBOR)
    {
        vx_int32 ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border);
        if (ref == -1 || ref == res)
            return 1;
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        vx_float32 s = x_src - x_minf;
        vx_float32 t = y_src - y_minf;
        vx_int32 p00 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 0, border);
        vx_int32 p01 = ct_image_get_pixel_8u(src, x_min + 0, y_min + 1, border);
        vx_int32 p10 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 0, border);
        vx_int32 p11 = ct_image_get_pixel_8u(src, x_min + 1, y_min + 1, border);
        vx_float32 ref_float;
        vx_int32 ref;

        // We don't compare the output when an input pixel is undefined (UNDEFINED border mode)
        vx_bool defined = (vx_bool)((p00 != -1) && (p10 != -1) && (p01 != -1) && (p11 != -1));
        if (defined == vx_false_e) {
            return 1;
        }

        // Compute the expected result (float)
        ref_float = (1 - s) * (1 - t) * p00 +
                    (    s) * (1 - t) * p10 +
                    (1 - s) * (    t) * p01 +
                    (    s) * (    t) * p11;

        // Take the nearest integer to avoid problems with casts in case of float rounding errors
        // (e.g: 30.999999 should give 31, not 30)
        ref = (vx_int32)(ref_float + 0.5f);

        // The result must be exact
        if (ref == res) {
            return 1;
        }

        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    if (interpolation == VX_INTERPOLATION_AREA)
    {
        vx_int32 ref;
        ASSERT_(return 0, dst->width % src->width == 0 && dst->height % src->height == 0);
        ref = ct_image_get_pixel_8u(src, x_ref, y_ref, border);
        if (ref == -1)
            return 1;
        if (ref == res)
            return 1;
        CT_FAIL_(return 0, "Check failed for pixel (%d, %d): %d (expected %d)", x, y, (int)res, (int)ref);
    }
    CT_FAIL_(return 0, "NOT IMPLEMENTED");
}

static void scale_validate(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    int num_failed = 0;
    if (src->width == dst->width && src->height == dst->height) // special case for scale=1.0
    {
        ASSERT_EQ_CTIMAGE(src, dst);
        return;
    }
    CT_FILL_IMAGE_8U(, dst,
            {
                int check;
                if (exact == 0)
                    ASSERT_NO_FAILURE(check = scale_check_pixel(src, dst, x, y, interpolation, border));
                else
                    ASSERT_NO_FAILURE(check = scale_check_pixel_exact(src, dst, x, y, interpolation, border));
                if (check == 0) {
                    num_failed++;
                }
            });
    if (interpolation == VX_INTERPOLATION_BILINEAR)
    {
        int total = dst->width * dst->height;
        if (num_failed * 100 > total * 2) // 98% should be valid
        {
            CT_FAIL("Check failed: %g (%d) pixels are wrong", (float)num_failed / total, num_failed);
        }
    }
}

static void scale_check(CT_Image src, CT_Image dst, vx_enum interpolation, vx_border_t border, int exact)
{
    ASSERT(src && dst);
    scale_validate(src, dst, interpolation, border, exact);
}

static void reference_histogram(CT_Image src, int32_t* hist, int nbins, int offset, int range)
{
    int i, hist0[256];
    uint32_t x, y, width = src->width, height = src->height, stride;

    ASSERT(src);
    ASSERT(src->format == VX_DF_IMAGE_U8);
    ASSERT(src->width > 0 && src->height > 0);
    stride = ct_stride_bytes(src);

    for( i = 0; i < 256; i++ )
        hist0[i] = 0;

    for( y = 0; y < height; y++ )
    {
        const uint8_t* ptr = src->data.y + y*stride;
        for( x = 0; x < width; x++ )
            hist0[ptr[x]]++;
    }

    for( i = 0; i < nbins; i++ )
        hist[i] = 0;

    for( i = offset; i < offset + range; i++ )
    {
        int j = (i - offset)*nbins/range;
        hist[j] = (int32_t)(hist[j] + hist0[i]);
    }
}

static vx_uint32 reference_window(vx_uint32 range, vx_size nbins)
{
    vx_uint32 test_window = (vx_uint32)(range / nbins);
    if (test_window*nbins == range)
        return test_window;
    else
        return 0;
}

static void reverse_histogram(int nbins, int32_t hist[])
{
    int i, j;
    for (i = 0, j = nbins-1; i < j; ++i, --j)
    {
        int32_t a = hist[i];
        int32_t b = hist[j];
        hist[i] = b;
        hist[j] = a;
    }
}

static int vx_uint8_compare(const void *p1, const void *p2)
{
    vx_uint8 a = *(vx_uint8 *)p1;
    vx_uint8 b = *(vx_uint8 *)p2;
    if (a > b)
        return 1;
    else if (a == b)
        return 0;
    else
        return -1;
}

static void filter_calculate(vx_enum function, CT_Image src, vx_coordinates2d_t* origin, vx_int32 cols, vx_int32 rows, vx_uint8* mask, vx_border_t* border, int32_t x, int32_t y, uint8_t *data)
{
    vx_uint8 values[MASK_SIZE_MAX * MASK_SIZE_MAX];

    vx_int32 i, j, ci, cj, m = 0, v = 0;
    vx_int32 cx = origin->x;
    vx_int32 cy = origin->y;

    for (j = y - cy; j < y - cy + rows; ++j)
    {
        for (i = x - cx; i < x - cx + cols; ++i, ++m)
        {
            if (mask[m])
            {
                ci = MAX(0, MIN(i, (vx_int32)src->width - 1));
                cj = MAX(0, MIN(j, (vx_int32)src->height - 1));

                values[v++] = (border->mode == VX_BORDER_CONSTANT && (i != ci || j != cj)) ? border->constant_value.U8 : *CT_IMAGE_DATA_PTR_8U(src, ci, cj);
            }
        }
    }

    qsort(values, v, sizeof(vx_uint8), vx_uint8_compare);

    switch (function)
    {
    case VX_NONLINEAR_FILTER_MIN: *data = values[0]; break; /* minimal value */
    case VX_NONLINEAR_FILTER_MAX: *data = values[v - 1]; break; /* maximum value */
    case VX_NONLINEAR_FILTER_MEDIAN: *data = values[v / 2]; break; /* pick the middle value */
    }
}

static void pattern_check(vx_uint8* mask, vx_size cols, vx_size rows, vx_enum pattern)
{
    vx_size x, y;

    ASSERT(pattern != VX_PATTERN_OTHER);

    for (y = 0; y < rows; ++y)
    {
        for (x = 0; x < cols; ++x)
        {
            vx_uint8 value = mask[x + y * cols];
            vx_uint8 ref = 0;
            switch (pattern)
            {
            case VX_PATTERN_BOX: ref = 255; break;
            case VX_PATTERN_CROSS: ref = ((y == rows / 2) || (x == cols / 2)) ? 255 : 0; break;
            case VX_PATTERN_DISK:
                ref = (((y - rows / 2.0 + 0.5) * (y - rows / 2.0 + 0.5)) / ((rows / 2.0) * (rows / 2.0)) +
                    ((x - cols / 2.0 + 0.5) * (x - cols / 2.0 + 0.5)) / ((cols / 2.0) * (cols / 2.0)))
                    <= 1 ? 255 : 0;
                break;
            }

            ASSERT(value == ref);
        }
    }
}

static void filter_check(vx_enum function, CT_Image src, vx_matrix mask, CT_Image dst, vx_border_t* border)
{
    CT_Image dst_ref = NULL;
    ASSERT(src && dst && mask && border);

    vx_size rows, cols;
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_ROWS, &rows, sizeof(rows)));
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_COLUMNS, &cols, sizeof(cols)));

    vx_coordinates2d_t origin;
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_ORIGIN, &origin, sizeof(origin)));

    vx_enum pattern = 0;
    vx_uint8 m[MASK_SIZE_MAX * MASK_SIZE_MAX];
    VX_CALL(vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern)));
    VX_CALL(vxCopyMatrix(mask, m, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    ASSERT_NO_FAILURE(pattern_check(m, cols, rows, pattern));

    ASSERT_NO_FAILURE(tivx_filter_create_reference_image(function, src, &origin, cols, rows, m, &dst_ref, border));

    ASSERT_NO_FAILURE(
    if (border->mode == VX_BORDER_UNDEFINED)
    {
        vx_int32 left = origin.x;
        vx_int32 top = origin.y;
        vx_int32 right = (vx_int32)(cols - origin.x - 1);
        vx_int32 bottom = (vx_int32)(rows - origin.y - 1);

        ct_adjust_roi(dst, left, top, right, bottom);
        ct_adjust_roi(dst_ref, left, top, right, bottom);
    }
    );

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

static int32_t offsets[][2] =
{
    { -1, -1}, {  0, -1}, {  1, -1},
    { -1,  0},            {  1,  0},
    { -1,  1}, {  0,  1}, {  1,  1}
};

#ifndef USE_OPENCV_GENERATED_REFERENCE
static uint64_t magnitude(CT_Image img, uint32_t x, uint32_t y, int32_t k, vx_enum type, int32_t* dx_out, int32_t* dy_out)
{
    static int32_t dim1[][7] = { { 1, 2, 1}, { 1,  4, 6, 4, 1}, { 1,  6, 15, 20, 15, 6, 1}};
    static int32_t dim2[][7] = { {-1, 0, 1}, {-1, -2, 0, 2, 1}, {-1, -4, -5,  0,  5, 4, 1}};
    int32_t dx = 0, dy = 0;
    int32_t i,j;

    int32_t* w1 = dim1[k/2-1];
    int32_t* w2 = dim2[k/2-1];

    x -= k/2;
    y -= k/2;

    for (i = 0; i < k; ++i)
    {
        int32_t xx = 0, yy = 0;
        for (j = 0; j < k; ++j)
        {
            vx_int32 v = img->data.y[(y + i) * img->stride + (x + j)];
            xx +=  v * w2[j];
            yy +=  v * w1[j];
        }

        dx += xx * w1[i];
        dy += yy * w2[i];
    }

    if (dx_out) *dx_out = dx;
    if (dy_out) *dy_out = dy;

    if (type == VX_NORM_L2)
        return dx * (int64_t)dx + dy * (int64_t)dy;
    else
        return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

static void follow_edge(CT_Image img, uint32_t x, uint32_t y)
{
    uint32_t i;
    img->data.y[y * img->stride + x] = 255;
    for (i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
        if (img->data.y[(y + offsets[i][0]) * img->stride + x + offsets[i][1]] == CREF_LINK)
            follow_edge(img, x + offsets[i][1], y + offsets[i][0]);
}

static void reference_canny(CT_Image src, CT_Image dst, int32_t low_thresh, int32_t high_thresh, uint32_t gsz, vx_enum norm)
{
    uint64_t lo = norm == VX_NORM_L1 ? low_thresh  : low_thresh*low_thresh;
    uint64_t hi = norm == VX_NORM_L1 ? high_thresh : high_thresh*high_thresh;
    uint32_t i, j;
    uint32_t bsz = gsz/2 + 1;

    ASSERT(src && dst);
    ASSERT(src->width == dst->width);
    ASSERT(src->height == dst->height);
    ASSERT(src->format == dst->format && src->format == VX_DF_IMAGE_U8);

    ASSERT(low_thresh <= high_thresh);
    ASSERT(low_thresh >= 0);
    ASSERT(gsz == 3 || gsz == 5 || gsz == 7);
    ASSERT(norm == VX_NORM_L2 || norm == VX_NORM_L1);
    ASSERT(src->width >= gsz && src->height >= gsz);

    // zero border pixels
    for (j = 0; j < bsz; ++j)
        for (i = 0; i < dst->width; ++i)
            dst->data.y[j * dst->stride + i] = dst->data.y[(dst->height - 1 - j) * dst->stride + i] = 255;
    for (j = bsz; j < dst->height - bsz; ++j)
        for (i = 0; i < bsz; ++i)
            dst->data.y[j * dst->stride + i] = dst->data.y[j * dst->stride + dst->width - 1 - i] = 255;

    // threshold + nms
    for (j = bsz; j < dst->height - bsz; ++j)
    {
        for (i = bsz; i < dst->width - bsz; ++i)
        {
            int32_t dx, dy, e = CREF_NONE;
            uint64_t m1, m2;

            uint64_t m = magnitude(src, i, j, gsz, norm, &dx, &dy);

            if (m > lo)
            {
                uint64_t l1 = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

                if (l1 * l1 < (uint64_t)(2 * dx * (int64_t)dx)) // |y| < |x| * tan(pi/8)
                {
                    m1 = magnitude(src, i-1, j, gsz, norm, 0, 0);
                    m2 = magnitude(src, i+1, j, gsz, norm, 0, 0);
                }
                else if (l1 * l1 < (uint64_t)(2 * dy * (int64_t)dy)) // |x| < |y| * tan(pi/8)
                {
                    m1 = magnitude(src, i, j-1, gsz, norm, 0, 0);
                    m2 = magnitude(src, i, j+1, gsz, norm, 0, 0);
                }
                else
                {
                    int32_t s = (dx ^ dy) < 0 ? -1 : 1;
                    m1 = magnitude(src, i-s, j-1, gsz, norm, 0, 0);
                    m2 = magnitude(src, i+s, j+1, gsz, norm, 0, 0) + 1; // (+1) is OpenCV's gotcha
                }

                if (m > m1 && m >= m2)
                    e = (m > hi ? CREF_EDGE : CREF_LINK);
            }

            dst->data.y[j * src->stride + i] = e;
        }
    }

    // trace edges
    for (j = bsz; j < dst->height - bsz; ++j)
        for (i = bsz; i < dst->width - bsz; ++i)
            if(dst->data.y[j * dst->stride + i] == CREF_EDGE)
                follow_edge(dst, i, j);

    // clear non-edges
    for (j = bsz; j < dst->height - bsz; ++j)
        for (i = bsz; i < dst->width - bsz; ++i)
            if(dst->data.y[j * dst->stride + i] < 255)
                dst->data.y[j * dst->stride + i] = 0;
}
#endif

// computes count(disttransform(src) >= 2, where dst != 0)
static uint32_t disttransform2_metric(CT_Image src, CT_Image dst, CT_Image dist, uint32_t* total_edge_pixels, uint8_t value)
{
    uint32_t i, j, k, count, total;
    ASSERT_(return 0, src && dst && dist && total_edge_pixels);
    ASSERT_(return 0, src->width == dst->width);
    ASSERT_(return 0, src->width == dist->width);
    ASSERT_(return 0, src->height == dst->height && src->height == dist->height);
    ASSERT_(return 0, src->format == dst->format && src->format == dist->format && src->format == VX_DF_IMAGE_U8);

    // fill borders with 1 (or 0 for edges)
    for (i = 0; i < dst->width; ++i)
    {
        dist->data.y[i] = src->data.y[i] == 0 ? 1 : 0;
        dist->data.y[(dist->height - 1) * dist->stride + i] = src->data.y[(dist->height - 1) * src->stride + i] == 0 ? 1 : 0;
    }
    for (j = 1; j < dst->height - 1; ++j)
    {
        dist->data.y[j * dist->stride] = src->data.y[j * src->stride] == 0 ? 1 : 0;
        dist->data.y[j * dist->stride + dist->width - 1] = src->data.y[j * src->stride + dist->width - 1] == 0 ? 1 : 0;
    }

    // minimalistic variant of disttransform:
    // 0   ==>      disttransform(src) == 0
    // 1   ==> 1 <= disttransform(src) < 2
    // 255 ==>      disttransform(src) >= 2
    for (j = 1; j < src->height-1; ++j)
    {
        for (i = 1; i < src->width-1; ++i)
        {
            if (src->data.y[j * src->stride + i] != 0)
                dist->data.y[j * dist->stride + i] = 0;
            else
            {
                int has_edge = 0;
                for (k = 0; k < sizeof(offsets)/sizeof(offsets[0]); ++k)
                {
                    if (src->data.y[(j + offsets[k][1]) * src->stride + i + offsets[k][0]] != 0)
                    {
                        has_edge = 1;
                        break;
                    }
                }

                dist->data.y[j * dist->stride + i] = (has_edge ? 1 : 255);
            }
        }
    }

    // count pixels where disttransform(src) < 2 and dst != 0
    total = count = 0;
    for (j = 0; j < dst->height; ++j)
    {
        for (i = 0; i < dst->width; ++i)
        {
            if (dst->data.y[j * dst->stride + i] != value) // Must be 255 when using vxNot
            {
                total += 1;
                count += (dist->data.y[j * dist->stride + i] < 2) ? 1 : 0;
            }
        }
    }

    *total_edge_pixels = total;

    return count;
}

#ifndef USE_OPENCV_GENERATED_REFERENCE
// own blur to not depend on OpenVX borders handling
static CT_Image gaussian5x5(CT_Image img)
{
    CT_Image res;
    uint32_t i, j, k, n;
    uint32_t ww[] = {1, 4, 6, 4, 1};

    ASSERT_(return 0, img);
    ASSERT_(return 0, img->format == VX_DF_IMAGE_U8);

    res = ct_allocate_image(img->width, img->height, img->format);
    ASSERT_(return 0, res);

    for (j = 0; j < img->height; ++j)
    {
        for (i = 0; i < img->width; ++i)
        {
            uint32_t r = 0;
            for (k = 0; k < 5; ++k)
            {
                uint32_t rr = 0;
                uint32_t kj = k + j < 2 ? 0 : (k + j - 2 >= img->width ? img->width -1 :  k + j - 2);
                for (n = 0; n < 5; ++n)
                {
                    uint32_t ni = n + i < 2 ? 0 : (n + i - 2 >= img->width ? img->width -1 :  n + i - 2);
                    rr += ww[n] * img->data.y[kj * img->stride + ni];
                }

                r += rr * ww[k];
            }
            res->data.y[j * res->stride + i] = (r + (1<<7)) >> 8;
        }
    }

    return res;
}
#endif

static CT_Image get_source_image(const char* filename)
{
#ifndef USE_OPENCV_GENERATED_REFERENCE
    if (strncmp(filename, "blurred_", 8) == 0)
        return gaussian5x5(ct_read_image(filename + 8, 1));
    else
#endif
        return ct_read_image(filename, 1);
}

static CT_Image get_reference_result(const char* src_name, CT_Image src, int32_t low_thresh, int32_t high_thresh, uint32_t gsz, vx_enum norm)
{
#ifdef USE_OPENCV_GENERATED_REFERENCE
    char buff[1024];
    sprintf(buff, "canny_%ux%u_%d_%d_%s_%s", gsz, gsz, low_thresh, high_thresh, norm == VX_NORM_L1 ? "L1" : "L2", src_name);
    // printf("reading: %s\n", buff);
    return ct_read_image(buff, 1);
#else
    CT_Image dst;
    ASSERT_(return 0, src);
    if (dst = ct_allocate_image(src->width, src->height, VX_DF_IMAGE_U8))
        reference_canny(src, dst, low_thresh, high_thresh, gsz, norm);
    return dst;
#endif
}

typedef struct {
    const char* testName;
    const char* filePrefix;
    vx_float32 min_distance;
    vx_float32 sensitivity;
    vx_int32  gradient_size;
    vx_int32  block_size;
    vx_int32 kernel_size;
    vx_enum interp_type;
} Arg;

#define ADD_VX_MIN_DISTANCE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MIN_DISTANCE=3.0", __VA_ARGS__, 3.0f))

#define ADD_VX_SENSITIVITY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/SENSITIVITY=0.10", __VA_ARGS__, 0.10f))

#define ADD_VX_GRADIENT_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/GRADIENT_SIZE=3", __VA_ARGS__, 3))

#define ADD_VX_BLOCK_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/BLOCK_SIZE=5", __VA_ARGS__, 5))

#define ADD_KERNEL_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/k=3", __VA_ARGS__, 3))

#define ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_INTERPOLATION_NEAREST_NEIGHBOR", __VA_ARGS__, VX_INTERPOLATION_NEAREST_NEIGHBOR))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("few_strong_corners",  ADD_VX_MIN_DISTANCE, ADD_VX_SENSITIVITY, ADD_VX_GRADIENT_SIZE, ADD_VX_BLOCK_SIZE, ADD_KERNEL_SIZE, ADD_VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, ARG, "hc_fsc")
; // Remove this semicolon
TEST_WITH_ARG(tivxMaxNodes, testMaxNodes, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    // Graph declaration
    vx_graph graph1 = 0, graph2 = 0, graph3 = 0, graph4 = 0, graph5 = 0, graph6 = 0, graph7 = 0, graph8 = 0;

    // Node declaration
    vx_node node1_graph1 = 0, node2_graph1 = 0, node3_graph1 = 0, node4_graph1 = 0, node5_graph1 = 0, node6_graph1 = 0;
    vx_node node1_graph2 = 0, node2_graph2 = 0, node3_graph2 = 0, node4_graph2 = 0;
    vx_node node1_graph3 = 0, node2_graph3 = 0;
    vx_node node1_graph4 = 0, node2_graph4 = 0, node3_graph4 = 0, node4_graph4 = 0, node5_graph4 = 0, node6_graph4 = 0, node7_graph4 = 0, node8_graph4 = 0, node9_graph4 = 0;
    vx_node node1_graph5 = 0, node2_graph5 = 0, node3_graph5 = 0, node4_graph5 = 0, node5_graph5 = 0;
    vx_node node1_graph6 = 0, node2_graph6 = 0;
    vx_node node1_graph7 = 0, node2_graph7 = 0, node3_graph7 = 0;
    vx_node node1_graph8 = 0;

    // Performance Structs declaration
    vx_perf_t perf_node1_graph1, perf_node2_graph1, perf_node3_graph1, perf_node4_graph1, perf_node5_graph1, perf_node6_graph1, perf_graph1;
    vx_perf_t perf_node1_graph2, perf_node2_graph2, perf_node3_graph2, perf_node4_graph2, perf_graph2;
    vx_perf_t perf_node1_graph3, perf_node2_graph3, perf_graph3;
    vx_perf_t perf_node1_graph4, perf_node2_graph4, perf_node3_graph4, perf_node4_graph4, perf_node5_graph4, perf_node6_graph4;
    vx_perf_t perf_node7_graph4, perf_node8_graph4, perf_node9_graph4, perf_graph4;
    vx_perf_t perf_node1_graph5, perf_node2_graph5, perf_node3_graph5, perf_node4_graph5, perf_node5_graph5, perf_graph5;
    vx_perf_t perf_node1_graph6, perf_node2_graph6, perf_graph6;
    vx_perf_t perf_node1_graph7, perf_node2_graph7, perf_node3_graph7, perf_graph7;
    vx_perf_t perf_node1_graph8, perf_graph8;

// Graph 1 parameters
    // Harris Corners Params
    vx_image input_image_graph1 = 0;
    vx_float32 strength_thresh;
    vx_float32 min_distance = arg_->min_distance + FLT_EPSILON;
    vx_float32 sensitivity = arg_->sensitivity;
    vx_size num_corners;
    size_t sz;
    vx_scalar strength_thresh_scalar, min_distance_scalar, sensitivity_scalar, num_corners_scalar0, num_corners_scalar1;
    vx_array corners, corners_virt;
    char filepath[MAXPATHLENGTH];
    CT_Image input_ctimage_graph1 = NULL;
    TIVX_TruthData truth_data;
    vx_array new_points_arr0 = 0, new_points_arr1 = 0;
    vx_pyramid pyr_node2_graph1 = 0, src_pyr_node2_graph1   = 0;
    vx_border_t gaussian_border = { VX_BORDER_REPLICATE };
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points0 = 0;
    vx_keypoint_t* new_points1 = 0;
    vx_size new_points_size0 = 0, new_points_size1 = 0;
    vx_size corners_data_size = 0;
    vx_keypoint_t* corners_data = 0;

    // Fast Corners Params
    vx_image src0_image_graph1 = 0;
    CT_Image src0_graph1, dst0_graph1, mask0_graph1, dst1_graph1;
    vx_size levels;
    int threshold = 10; // Hardcoded from test case
    vx_float32 threshold_f = (vx_float32)threshold;
    vx_array corners1;
    int nonmax = 0; // Hardcoded
    vx_pyramid pyr_node4_graph1 = 0, src_pyr_node4_graph1   = 0;
    uint32_t ncorners0, ncorners;
    uint32_t i, dst1stride;
    CT_Image src_ct_image = NULL;

    // Gaussian Pyramid
    vx_pyramid pyr_node6_graph1 = 0, src_pyr_node6_graph1   = 0;
    vx_array corners2 = 0;
    vx_array new_points_arr2 = 0;
    vx_size num_points_gausspyr = 0;
    vx_keypoint_t* old_points_gausspyr = 0;
    vx_keypoint_t* new_points_ref_gausspyr = 0;
    vx_keypoint_t* new_points_gausspyr = 0;
    vx_size new_points_size_gausspyr = 0;
    vx_image input_image_gausspyr_graph1 = 0;
    CT_Image input_gausspyr_graph1 = NULL, src_ctimage_gausspyr_graph1 = NULL;
    vx_border_t gaussianpyr_border = { VX_BORDER_UNDEFINED };

    // Graph 1 Common:
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = 5; // hardcoded from optflow test case
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar sthresh;

// Graph 2 Parameters:
    // Channel Combine Parameters
    vx_image src_image_graph2[3] = {0, 0, 0};
    vx_image dst_image_graph2[3] = {0, 0, 0};
    vx_image virt_graph2;
    CT_Image src_graph2[3] = {NULL, NULL, NULL};
    CT_Image dst_graph2[3] = {NULL, NULL, NULL};
    CT_Image dst_dummy_graph2 = NULL;
    vx_enum channel_ref;

// Graph 3 Parameters:
    int srcformat_graph3 = VX_DF_IMAGE_NV12;
    int dstformat_graph3 = VX_DF_IMAGE_RGB;
    int ythresh_graph3 = 1;
    int cthresh_graph3 = 1;
    vx_image src_graph3=0, dst_graph3=0, virt_graph3=0;
    CT_Image src0_graph3, dst0_graph3, dst1_graph3, virt_ctimage_graph3, virt_orig_graph3;
    uint64_t rng = CT()->seed_;
    vx_enum range = VX_CHANNEL_RANGE_FULL;
    vx_enum space = VX_COLOR_SPACE_BT709;

// Graph 4 Parameters:
    vx_image src1_graph4, src2_graph4, add_result_graph4;
    CT_Image ref1_graph4, ref2_graph4;
    CT_Image ref_add_result_graph4, vx_add_result_graph4;
    vx_border_t border_graph4 = { VX_BORDER_UNDEFINED };
    vx_image dilate_out, erode_out, gaussian_out, median_out, sobel_out;
    vx_image warpaffine_out, warpperspective_out, halfscalegaussian_out;
    CT_Image dilate_ref, erode_ref, gaussian_ref, median_ref, sobel_ref;
    CT_Image warpaffine_ref, warpperspective_ref, halfscalegaussian_ref;
    vx_float32 warpaffine_m[6], warpperspective_m[9];
    vx_matrix warpaffine_matrix, warpperspective_matrix;

// Graph 5 Parameters:
    vx_image mult_src1, mult_src2, mult_dst;
    vx_scalar mult_scale = 0;
    vx_float32 mult_scalef = 1.0f;
    CT_Image mult_ref1, mult_ref2, mult_refdst, mult_refdst_plus_1, mult_vxdst;
    vx_int32 convdepth_shift = 2;
    vx_scalar convdepth_scalar_shift;
    vx_int32 convdepth_tmp = 0;
    vx_image convdepth_dst;
    CT_Image convdepth_vxdst, convdepth_refdst;
    vx_image absdiff_src, absdiff_dst;
    CT_Image absdiff_ref, absdiff_refdst, absdiff_vxdst;
    // Magnitude
    vx_image mag_dx=0, mag_dy=0, mag=0;
    CT_Image mag_dx0, mag_dy0, mag0, mag1;
    int dxmin = -32768, dxmax = 32768;
    vx_border_t mag_border;
    mag_border.mode = VX_BORDER_REPLICATE;
    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                        vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER,
                                              &mag_border, sizeof(mag_border)));
    vx_image phase_dx=0, phase_dy=0, phase=0;
    CT_Image phase_dx0, phase_dy0, phase0, phase1;

// Graph 6 Parameters:
    vx_image not_dst;
    CT_Image not_refdst, not_vxdst;
    vx_lut lut;
    vx_image lut_dst = 0;
    vx_size size;
    void* lut_data;
    size = lut_size(VX_TYPE_UINT8);
    lut_data = ct_alloc_mem(size);
    CT_Image lut_ctdst = NULL;

// Graph 7 Parameters:
    vx_image scale_dst;
    vx_border_t scale_border = { VX_BORDER_UNDEFINED };
    CT_Image scale_dst0;
    vx_distribution dist;
    int val0 = CT_RNG_NEXT_INT(rng, 0, (MAX_BINS-1)), val1 = CT_RNG_NEXT_INT(rng, 0, (MAX_BINS-1));
    int offset = CT_MIN(val0, val1), hist_range = CT_MAX(val0, val1) - offset + 1;
    int nbins = CT_RNG_NEXT_INT(rng, 1, hist_range+1);
    int32_t hist0[MAX_BINS];
    vx_image nonlinear_src = 0, nonlinear_dst;
    vx_matrix nonlinear_mask = 0;
    vx_enum nonlinear_pattern = 0;
    CT_Image nonlinear_src0 = NULL, nonlinear_dst0 = NULL;

// Graph 8 Parameters:
    uint32_t canny_total, canny_count;
    vx_image canny_src, canny_dst;
    vx_threshold hyst;
    CT_Image lena, canny_vxdst, canny_refdst, canny_dist;
    vx_int32 low_thresh  = 100;
    vx_int32 high_thresh = 120;
    vx_border_t canny_border = { VX_BORDER_UNDEFINED, {{ 0 }} };
    vx_int32 canny_border_width = 3/2 + 1;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

// Create graphs
    ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph3 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph4 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph5 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph6 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph7 = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(graph8 = vxCreateGraph(context), VX_TYPE_GRAPH);

    double scale = 1.0 / ((1 << (arg_->gradient_size - 1)) * arg_->block_size * 255.0);
    scale = scale * scale * scale * scale;

    sz = snprintf(filepath, MAXPATHLENGTH, "%s/harriscorners/%s_%0.2f_%0.2f_%d_%d.txt", ct_get_test_file_path(), arg_->filePrefix, arg_->min_distance, arg_->sensitivity, arg_->gradient_size, arg_->block_size);
    ASSERT(sz < MAXPATHLENGTH);
    ASSERT_NO_FAILURE(harris_corner_read_truth_data(filepath, &truth_data, (float)scale));

    strength_thresh = truth_data.strength_thresh;

    sprintf(filepath, "harriscorners/%s.bmp", arg_->filePrefix);

    ASSERT_NO_FAILURE(input_ctimage_graph1 = ct_read_image(filepath, 1));
    ASSERT(input_ctimage_graph1 && (input_ctimage_graph1->format == VX_DF_IMAGE_U8));
    ASSERT_VX_OBJECT(input_image_graph1 = ct_image_to_vx_image(input_ctimage_graph1, context), VX_TYPE_IMAGE);

    num_corners = input_ctimage_graph1->width * input_ctimage_graph1->height / 10;

    ASSERT_NO_FAILURE(src_ct_image = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    ASSERT_VX_OBJECT(src_pyr_node2_graph1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image->width, src_ct_image->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(new_points_arr0 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr1 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(pyr_node2_graph1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, src_ct_image->width, src_ct_image->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image, src_pyr_node2_graph1, 4, VX_SCALE_PYRAMID_HALF, gaussian_border));

    ASSERT_VX_OBJECT(strength_thresh_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &strength_thresh), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(min_distance_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &min_distance), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(sensitivity_scalar = vxCreateScalar(context, VX_TYPE_FLOAT32, &sensitivity), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners_scalar0 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_corners_scalar1 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);

    // Fast corners
    ASSERT_NO_FAILURE(src0_graph1 = ct_read_image("optflow_00.bmp", 1));
    ASSERT(src0_graph1->format == VX_DF_IMAGE_U8);

    src0_image_graph1 = ct_image_to_vx_image(src0_graph1, context);
    sthresh = vxCreateScalar(context, VX_TYPE_FLOAT32, &threshold_f);

    levels = gaussian_pyramid_calc_max_levels_count(src0_graph1->width, src0_graph1->height, VX_SCALE_PYRAMID_HALF);

    ASSERT_VX_OBJECT(src_pyr_node4_graph1 = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, src0_graph1->width, src0_graph1->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(pyr_node4_graph1 = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, src0_graph1->width, src0_graph1->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image, src_pyr_node4_graph1, levels, VX_SCALE_PYRAMID_HALF, gaussian_border));

    corners1 = vxCreateArray(context, VX_TYPE_KEYPOINT, 80000);

    // Gaussian Pyramid
    ASSERT_NO_FAILURE(input_gausspyr_graph1 = optflow_pyrlk_read_image( "optflow_00.bmp", 0, 0));
    ASSERT_VX_OBJECT(input_image_gausspyr_graph1 = ct_image_to_vx_image(input_gausspyr_graph1, context), VX_TYPE_IMAGE);

    levels = gaussian_pyramid_calc_max_levels_count(input_gausspyr_graph1->width, input_gausspyr_graph1->height, VX_SCALE_PYRAMID_HALF);

    ASSERT_NO_FAILURE(src_ctimage_gausspyr_graph1 = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    ASSERT_VX_OBJECT(src_pyr_node6_graph1 = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, input_gausspyr_graph1->width, input_gausspyr_graph1->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points_gausspyr = own_read_keypoints("optflow_pyrlk_5x5.txt", &old_points_gausspyr, &new_points_ref_gausspyr));

    ASSERT_VX_OBJECT(corners2 = own_create_keypoint_array(context, num_points_gausspyr, old_points_gausspyr), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr2 = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points_gausspyr), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(pyr_node6_graph1 = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, input_gausspyr_graph1->width, input_gausspyr_graph1->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ctimage_gausspyr_graph1, src_pyr_node6_graph1, levels, VX_SCALE_PYRAMID_HALF, gaussian_border));

    // Channel Combine/Extract
    ASSERT_NO_FAILURE(dst_dummy_graph2 = ct_allocate_image(4, 4, VX_DF_IMAGE_RGB));
    channel_ref = VX_CHANNEL_R;
    for (i = 0; i < 3; i++)
    {
        int w = 640 / ct_image_get_channel_subsampling_x(dst_dummy_graph2, channel_ref + i);
        int h = 480 / ct_image_get_channel_subsampling_y(dst_dummy_graph2, channel_ref + i);
        ASSERT_NO_FAILURE(src_graph2[i] = channel_extract_image_generate_random(w, h, VX_DF_IMAGE_U8));
        ASSERT_VX_OBJECT(src_image_graph2[i] = ct_image_to_vx_image(src_graph2[i], context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image_graph2[i] = vxCreateImage(context, w, h, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(virt_graph2 = vxCreateVirtualImage(graph2, 640, 480, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    // Color Convert
    ASSERT_NO_FAILURE(src0_graph3 = ct_allocate_ct_image_random(640, 480, srcformat_graph3, &rng, 0, 256));
    ASSERT_NO_FAILURE(src_graph3 = ct_image_to_vx_image(src0_graph3, context));
    ASSERT_VX_OBJECT(src_graph3, VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src_graph3, VX_IMAGE_SPACE, &space, sizeof(space)));
    ASSERT_VX_OBJECT(dst_graph3 = vxCreateImage(context, 640, 480, srcformat_graph3), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_graph3, VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst_graph3, VX_IMAGE_SPACE, &space, sizeof(space)));
    ASSERT_VX_OBJECT(virt_graph3  = vxCreateImage(context, 640, 480, dstformat_graph3), VX_TYPE_IMAGE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(virt_graph3, VX_IMAGE_SPACE, &space, sizeof(space)));

// Graph 4 Create
    // Add
    ASSERT_VX_OBJECT(add_result_graph4   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dilate_out   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(erode_out   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(gaussian_out   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(median_out   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(sobel_out   = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(warpaffine_out   = vxCreateImage(context, 1280, 960, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(warpperspective_out  = vxCreateImage(context, 1280, 960, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(halfscalegaussian_out   = vxCreateImage(context, 320, 240, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(warp_affine_generate_matrix(warpaffine_m, 640, 480, 1280, 960, VX_MATRIX_RANDOM));
    ASSERT_VX_OBJECT(warpaffine_matrix = warp_affine_create_matrix(context, warpaffine_m), VX_TYPE_MATRIX);

    ASSERT_NO_FAILURE(warp_perspective_generate_matrix(warpperspective_m, 640, 480, 1280, 960, VX_MATRIX_RANDOM));
    ASSERT_VX_OBJECT(warpperspective_matrix = warp_perspective_create_matrix(context, warpperspective_m), VX_TYPE_MATRIX);

    ASSERT_VX_OBJECT(src1_graph4 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_graph4 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(src1_graph4, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(src2_graph4, &CT()->seed_));

// Graph 5 Create:
    ASSERT_VX_OBJECT(mult_dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mult_scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &mult_scalef), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(mult_src1 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mult_src2 = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(ct_fill_image_random(mult_src1, &CT()->seed_));
    ASSERT_NO_FAILURE(ct_fill_image_random(mult_src2, &CT()->seed_));

    ASSERT_VX_OBJECT(convdepth_scalar_shift = vxCreateScalar(context, VX_TYPE_INT32, &convdepth_tmp), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(convdepth_dst = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(absdiff_dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(absdiff_src = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8),   VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(ct_fill_image_random(absdiff_src, &CT()->seed_));

    // Magnitude
    ASSERT_NO_FAILURE(mag_dx0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, dxmin, dxmax));
    ASSERT_NO_FAILURE(mag_dy0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, dxmin, dxmax));
    mag_dx = ct_image_to_vx_image(mag_dx0, context);
    ASSERT_VX_OBJECT(mag_dx, VX_TYPE_IMAGE);
    mag_dy = ct_image_to_vx_image(mag_dy0, context);
    ASSERT_VX_OBJECT(mag_dy, VX_TYPE_IMAGE);
    mag = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_S16);
    ASSERT_VX_OBJECT(mag, VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(phase_dx0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, dxmin, dxmax));
    ASSERT_NO_FAILURE(phase_dy0 = ct_allocate_ct_image_random(WIDTH, HEIGHT, VX_DF_IMAGE_S16, &rng, dxmin, dxmax));
    phase_dx = ct_image_to_vx_image(phase_dx0, context);
    ASSERT_VX_OBJECT(phase_dx, VX_TYPE_IMAGE);
    phase_dy = ct_image_to_vx_image(phase_dy0, context);
    ASSERT_VX_OBJECT(phase_dy, VX_TYPE_IMAGE);
    phase = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(phase, VX_TYPE_IMAGE);

// Graph 6 Create:
    ASSERT_VX_OBJECT(not_dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(lut_data_fill_random(lut_data, VX_TYPE_UINT8));
    ASSERT_VX_OBJECT(lut = lut_create(context, lut_data, VX_TYPE_UINT8), VX_TYPE_LUT);

    ASSERT_VX_OBJECT(lut_dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

// Graph 7 Create:
    ASSERT_VX_OBJECT(scale_dst = vxCreateImage(context, 2*WIDTH, 2*HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    dist = vxCreateDistribution(context, nbins, offset, hist_range);
    ASSERT_VX_OBJECT(dist, VX_TYPE_DISTRIBUTION);

    ASSERT_NO_FAILURE(nonlinear_src0 = generate_random(WIDTH, HEIGHT));

    ASSERT_VX_OBJECT(nonlinear_src = ct_image_to_vx_image(nonlinear_src0, context), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(nonlinear_dst   = vxCreateImage(context, WIDTH, HEIGHT, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    nonlinear_mask = vxCreateMatrixFromPattern(context, VX_PATTERN_BOX, 5, 5);
    ASSERT_VX_OBJECT(nonlinear_mask, VX_TYPE_MATRIX);
    VX_CALL(vxQueryMatrix(nonlinear_mask, VX_MATRIX_PATTERN, &nonlinear_pattern, sizeof(nonlinear_pattern)));
    ASSERT_EQ_INT(VX_PATTERN_BOX, nonlinear_pattern);

// Graph 8 Create:
    ASSERT_NO_FAILURE(lena = get_source_image("lena_gray.bmp"));
    ASSERT_NO_FAILURE(canny_src = ct_image_to_vx_image(lena, context));
    ASSERT_VX_OBJECT(canny_dst = vxCreateImage(context, lena->width, lena->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(hyst = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8), VX_TYPE_THRESHOLD);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_LOWER, &low_thresh,  sizeof(low_thresh)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetThresholdAttribute(hyst, VX_THRESHOLD_THRESHOLD_UPPER, &high_thresh, sizeof(high_thresh)));

// Nodes
    // Graph 1 Nodes
    ASSERT_VX_OBJECT(corners_virt = vxCreateVirtualArray(graph1, VX_TYPE_KEYPOINT, num_corners), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(node1_graph1 = vxHarrisCornersNode(graph1, input_image_graph1, strength_thresh_scalar, min_distance_scalar,
                                                sensitivity_scalar, arg_->gradient_size, arg_->block_size, corners,
                                                num_corners_scalar0), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph1 = vxOpticalFlowPyrLKNode(
        graph1,
        pyr_node2_graph1, src_pyr_node2_graph1,
        corners, corners, new_points_arr0,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2_graph1, VX_TYPE_NODE);

    node3_graph1 = vxFastCornersNode(graph1, src0_image_graph1, sthresh, nonmax ? vx_true_e : vx_false_e, corners1, 0);
    ASSERT_VX_OBJECT(node3_graph1, VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node4_graph1 = vxOpticalFlowPyrLKNode(
        graph1,
        pyr_node4_graph1, src_pyr_node4_graph1,
        corners1, corners1, new_points_arr1,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node4_graph1, VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node5_graph1 = vxGaussianPyramidNode(graph1, input_image_gausspyr_graph1, pyr_node6_graph1), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node6_graph1 = vxOpticalFlowPyrLKNode(
        graph1,
        pyr_node6_graph1, src_pyr_node6_graph1,
        corners2, corners2, new_points_arr2,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    // Graph 2 Nodes
    node1_graph2 = vxChannelCombineNode(graph2, src_image_graph2[0], src_image_graph2[1], src_image_graph2[2], NULL, virt_graph2);
    ASSERT_VX_OBJECT(node1_graph2, VX_TYPE_NODE);

    node2_graph2 = vxChannelExtractNode(graph2, virt_graph2, VX_CHANNEL_R, dst_image_graph2[0]);
    ASSERT_VX_OBJECT(node2_graph2, VX_TYPE_NODE);

    node3_graph2 = vxChannelExtractNode(graph2, virt_graph2, VX_CHANNEL_G, dst_image_graph2[1]);
    ASSERT_VX_OBJECT(node3_graph2, VX_TYPE_NODE);

    node4_graph2 = vxChannelExtractNode(graph2, virt_graph2, VX_CHANNEL_B, dst_image_graph2[2]);
    ASSERT_VX_OBJECT(node4_graph2, VX_TYPE_NODE);

    // Graph 3 Nodes
    ASSERT_VX_OBJECT(node1_graph3 = vxColorConvertNode(graph3, src_graph3, virt_graph3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node1_graph3, VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2_graph3 = vxColorConvertNode(graph3, virt_graph3, dst_graph3), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2_graph3, VX_TYPE_NODE);

    // Graph 4 Nodes
    ASSERT_VX_OBJECT(node1_graph4 = vxAddNode(graph4, src1_graph4, src2_graph4, VX_CONVERT_POLICY_SATURATE, add_result_graph4), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph4 = vxDilate3x3Node(graph4, add_result_graph4, dilate_out), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node2_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node3_graph4 = vxErode3x3Node(graph4, add_result_graph4, erode_out), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node3_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node4_graph4 = vxGaussian3x3Node(graph4, add_result_graph4, gaussian_out), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node4_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node5_graph4 = vxMedian3x3Node(graph4, add_result_graph4, median_out), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node5_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node6_graph4 = vxHalfScaleGaussianNode(graph4, add_result_graph4, halfscalegaussian_out, arg_->kernel_size), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node6_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node7_graph4 = vxSobel3x3Node(graph4, add_result_graph4, sobel_out, NULL), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node7_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node8_graph4 = vxWarpAffineNode(graph4, add_result_graph4, warpaffine_matrix, arg_->interp_type, warpaffine_out), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node8_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    ASSERT_VX_OBJECT(node9_graph4 = vxWarpPerspectiveNode(graph4, add_result_graph4, warpperspective_matrix, arg_->interp_type, warpperspective_out), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node9_graph4, VX_NODE_BORDER, &border_graph4, sizeof(border_graph4)));

    // Graph 5 Nodes
    ASSERT_VX_OBJECT(node1_graph5 = vxMultiplyNode(graph5, mult_src1, mult_src2, mult_scale, VX_CONVERT_POLICY_SATURATE, VX_ROUND_POLICY_TO_ZERO, mult_dst), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph5 = vxConvertDepthNode(graph5, mult_dst, convdepth_dst, VX_CONVERT_POLICY_WRAP, convdepth_scalar_shift), VX_TYPE_NODE);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxCopyScalar(convdepth_scalar_shift, &convdepth_shift, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    ASSERT_VX_OBJECT(node3_graph5 = vxAbsDiffNode(graph5, convdepth_dst, absdiff_src, absdiff_dst), VX_TYPE_NODE);

    node4_graph5 = vxMagnitudeNode(graph5, mag_dx, mag_dy, mag);
    ASSERT_VX_OBJECT(node4_graph5, VX_TYPE_NODE);

    node5_graph5 = vxPhaseNode(graph5, phase_dx, phase_dy, phase);
    ASSERT_VX_OBJECT(node5_graph5, VX_TYPE_NODE);

    // Graph 6 Nodes
    ASSERT_VX_OBJECT(node1_graph6 = vxNotNode(graph6, absdiff_dst, not_dst), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2_graph6 = vxTableLookupNode(graph6, not_dst, lut, lut_dst), VX_TYPE_NODE);

    // Graph 7 Nodes
    ASSERT_VX_OBJECT(node1_graph7 = vxScaleImageNode(graph7, phase, scale_dst, VX_INTERPOLATION_NEAREST_NEIGHBOR), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node1_graph7, VX_NODE_BORDER, &scale_border, sizeof(scale_border)));

    ASSERT_VX_OBJECT(node2_graph7 = vxHistogramNode(graph7, scale_dst, dist), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node3_graph7 = vxNonLinearFilterNode(graph7, VX_NONLINEAR_FILTER_MEDIAN, nonlinear_src, nonlinear_mask, nonlinear_dst), VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node3_graph7, VX_NODE_BORDER, &scale_border, sizeof(scale_border)));

    // Graph 8 Nodes
    ASSERT_VX_OBJECT(node1_graph8 = vxCannyEdgeDetectorNode(graph8, canny_src, hyst, 3, VX_NORM_L1, canny_dst), VX_TYPE_NODE);

    // Verify and Process Graphs
    VX_CALL(vxVerifyGraph(graph1));
    VX_CALL(vxProcessGraph(graph1));
    VX_CALL(vxVerifyGraph(graph2));
    VX_CALL(vxProcessGraph(graph2));
    VX_CALL(vxVerifyGraph(graph3));
    VX_CALL(vxProcessGraph(graph3));
    VX_CALL(vxVerifyGraph(graph4));
    VX_CALL(vxProcessGraph(graph4));
    VX_CALL(vxVerifyGraph(graph5));
    VX_CALL(vxProcessGraph(graph5));
    VX_CALL(vxVerifyGraph(graph6));
    VX_CALL(vxProcessGraph(graph6));
    VX_CALL(vxVerifyGraph(graph7));
    VX_CALL(vxProcessGraph(graph7));
    VX_CALL(vxVerifyGraph(graph8));
    VX_CALL(vxProcessGraph(graph8));

    // Query Node Performances
    vxQueryNode(node1_graph1, VX_NODE_PERFORMANCE, &perf_node1_graph1, sizeof(perf_node1_graph1));
    vxQueryNode(node2_graph1, VX_NODE_PERFORMANCE, &perf_node2_graph1, sizeof(perf_node2_graph1));
    vxQueryNode(node3_graph1, VX_NODE_PERFORMANCE, &perf_node3_graph1, sizeof(perf_node3_graph1));
    vxQueryNode(node4_graph1, VX_NODE_PERFORMANCE, &perf_node4_graph1, sizeof(perf_node4_graph1));
    vxQueryNode(node5_graph1, VX_NODE_PERFORMANCE, &perf_node5_graph1, sizeof(perf_node5_graph1));
    vxQueryNode(node6_graph1, VX_NODE_PERFORMANCE, &perf_node6_graph1, sizeof(perf_node6_graph1));
    vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));

    vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
    vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
    vxQueryNode(node3_graph2, VX_NODE_PERFORMANCE, &perf_node3_graph2, sizeof(perf_node3_graph2));
    vxQueryNode(node4_graph2, VX_NODE_PERFORMANCE, &perf_node4_graph2, sizeof(perf_node4_graph2));
    vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

    vxQueryNode(node1_graph3, VX_NODE_PERFORMANCE, &perf_node1_graph3, sizeof(perf_node1_graph3));
    vxQueryNode(node2_graph3, VX_NODE_PERFORMANCE, &perf_node2_graph3, sizeof(perf_node2_graph3));
    vxQueryGraph(graph3, VX_GRAPH_PERFORMANCE, &perf_graph3, sizeof(perf_graph3));

    vxQueryNode(node1_graph4, VX_NODE_PERFORMANCE, &perf_node1_graph4, sizeof(perf_node1_graph4));
    vxQueryNode(node2_graph4, VX_NODE_PERFORMANCE, &perf_node2_graph4, sizeof(perf_node2_graph4));
    vxQueryNode(node3_graph4, VX_NODE_PERFORMANCE, &perf_node3_graph4, sizeof(perf_node3_graph4));
    vxQueryNode(node4_graph4, VX_NODE_PERFORMANCE, &perf_node4_graph4, sizeof(perf_node4_graph4));
    vxQueryNode(node5_graph4, VX_NODE_PERFORMANCE, &perf_node5_graph4, sizeof(perf_node5_graph4));
    vxQueryNode(node6_graph4, VX_NODE_PERFORMANCE, &perf_node6_graph4, sizeof(perf_node6_graph4));
    vxQueryNode(node7_graph4, VX_NODE_PERFORMANCE, &perf_node7_graph4, sizeof(perf_node7_graph4));
    vxQueryNode(node8_graph4, VX_NODE_PERFORMANCE, &perf_node8_graph4, sizeof(perf_node8_graph4));
    vxQueryNode(node9_graph4, VX_NODE_PERFORMANCE, &perf_node9_graph4, sizeof(perf_node9_graph4));
    vxQueryGraph(graph4, VX_GRAPH_PERFORMANCE, &perf_graph4, sizeof(perf_graph4));

    vxQueryNode(node1_graph5, VX_NODE_PERFORMANCE, &perf_node1_graph5, sizeof(perf_node1_graph5));
    vxQueryNode(node2_graph5, VX_NODE_PERFORMANCE, &perf_node2_graph5, sizeof(perf_node2_graph5));
    vxQueryNode(node3_graph5, VX_NODE_PERFORMANCE, &perf_node3_graph5, sizeof(perf_node3_graph5));
    vxQueryNode(node4_graph5, VX_NODE_PERFORMANCE, &perf_node4_graph5, sizeof(perf_node4_graph5));
    vxQueryNode(node5_graph5, VX_NODE_PERFORMANCE, &perf_node5_graph5, sizeof(perf_node5_graph5));
    vxQueryGraph(graph5, VX_GRAPH_PERFORMANCE, &perf_graph5, sizeof(perf_graph5));

    vxQueryNode(node1_graph6, VX_NODE_PERFORMANCE, &perf_node1_graph6, sizeof(perf_node1_graph6));
    vxQueryNode(node2_graph6, VX_NODE_PERFORMANCE, &perf_node2_graph6, sizeof(perf_node2_graph6));
    vxQueryGraph(graph6, VX_GRAPH_PERFORMANCE, &perf_graph6, sizeof(perf_graph6));

    vxQueryNode(node1_graph7, VX_NODE_PERFORMANCE, &perf_node1_graph7, sizeof(perf_node1_graph7));
    vxQueryNode(node2_graph7, VX_NODE_PERFORMANCE, &perf_node2_graph7, sizeof(perf_node2_graph7));
    vxQueryNode(node3_graph7, VX_NODE_PERFORMANCE, &perf_node3_graph7, sizeof(perf_node3_graph7));
    vxQueryGraph(graph7, VX_GRAPH_PERFORMANCE, &perf_graph7, sizeof(perf_graph7));

    vxQueryNode(node1_graph8, VX_NODE_PERFORMANCE, &perf_node1_graph8, sizeof(perf_node1_graph8));
    vxQueryGraph(graph8, VX_GRAPH_PERFORMANCE, &perf_graph8, sizeof(perf_graph8));

// Verify Graph 1 Results
    // Harris corners check
    CT_ASSERT_NO_FAILURE_(, harris_corner_check(corners, &truth_data));
    ct_read_array(corners, (void**)&corners_data, 0, &corners_data_size, 0);
    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr0, (void**)&new_points0, 0, &new_points_size0, 0));
    ASSERT(new_points_size0 == corners_data_size);

    // Gaussian Pyramid check
    CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input_gausspyr_graph1, pyr_node6_graph1, levels, VX_SCALE_PYRAMID_HALF, gaussianpyr_border));

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr2, (void**)&new_points_gausspyr, 0, &new_points_size_gausspyr, 0));
    ASSERT(new_points_size_gausspyr == num_points_gausspyr);

    ASSERT_NO_FAILURE(own_keypoints_check(num_points_gausspyr, old_points_gausspyr, new_points_ref_gausspyr, new_points_gausspyr));

    // Fast corners check
    ASSERT_NO_FAILURE(dst0_graph1 = ct_allocate_image(src0_graph1->width, src0_graph1->height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(mask0_graph1 = ct_allocate_image(src0_graph1->width, src0_graph1->height, VX_DF_IMAGE_U8));
    ASSERT_NO_FAILURE(dst1_graph1 = ct_allocate_image(src0_graph1->width, src0_graph1->height, VX_DF_IMAGE_U8));
    dst1stride = ct_stride_bytes(dst1_graph1);
    ct_memset(dst1_graph1->data.y, 0, (vx_size)dst1stride*src0_graph1->height);

    ncorners0 = reference_fast(src0_graph1, dst0_graph1, mask0_graph1, threshold, nonmax);
    ct_read_array(corners1, (void**)&corners_data, 0, &corners_data_size, 0);
    ncorners = (uint32_t)corners_data_size;

    for( i = 0; i < ncorners; i++ )
    {
        vx_keypoint_t* pt = &corners_data[i];
        int ix, iy;
        ASSERT( 0.f <= pt->x && pt->x < (float)src0_graph1->width &&
                0.f <= pt->y && pt->y < (float)src0_graph1->height );
        ASSERT(pt->tracking_status == 1);
        ix = (int)(pt->x + 0.5f);
        iy = (int)(pt->y + 0.5f);
        ix = CT_MIN(ix, (int)src0_graph1->width-1);
        iy = CT_MIN(iy, (int)src0_graph1->height-1);
        ASSERT( !nonmax || (0 < pt->strength && pt->strength <= 255) );
        dst1_graph1->data.y[dst1stride*iy + ix] = nonmax ? (uint8_t)(pt->strength + 0.5f) : 1;
    }

    {
    const uint32_t border = 3;
    int32_t stride0 = (int32_t)ct_stride_bytes(dst0_graph1), stride1 = (int32_t)ct_stride_bytes(dst1_graph1);
    uint32_t x, y;
    uint32_t missing0 = 0, missing1 = 0;

    for( y = border; y < src0_graph1->height - border; y++ )
    {
        const uint8_t* ptr0 = dst0_graph1->data.y + stride0*y;
        const uint8_t* ptr1 = dst1_graph1->data.y + stride1*y;

        for( x = border; x < src0_graph1->width - border; x++ )
        {
            if( ptr0[x] > 0 && ptr1[x] == 0 )
                missing0++;
            else if( ptr0[x] == 0 && ptr1[x] > 0 )
                missing1++;
            else if( nonmax && ptr0[x] > 0 && ptr1[x] > 0 && fabs(log10((double)ptr0[x]/ptr1[x])) >= 1 )
            {
                missing0++;
                missing1++;
            }
        }
    }

    ASSERT( missing0 <= 0.02*ncorners0 );
    ASSERT( missing1 <= 0.02*ncorners );
    }

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr1, (void**)&new_points1, 0, &new_points_size1, 0));
    ASSERT(new_points_size1 == corners_data_size);

    /* This is invalid check since one set of outputs comes from harris, the other from fast.  Disabling this check */
    //ASSERT_NO_FAILURE(own_keypoints_check(new_points_size0, NULL, new_points0, new_points1));

    ct_free_mem(corners_data);
    ct_free_mem(new_points1);
    ct_free_mem(new_points0);
    ct_free_mem(new_points_ref);
    ct_free_mem(old_points);

// Graph 2 Verify
    ASSERT_NO_FAILURE(dst_graph2[0] = ct_image_from_vx_image(dst_image_graph2[0]));
    ASSERT_NO_FAILURE(dst_graph2[1] = ct_image_from_vx_image(dst_image_graph2[1]));
    ASSERT_NO_FAILURE(dst_graph2[2] = ct_image_from_vx_image(dst_image_graph2[2]));

    EXPECT_EQ_CTIMAGE(src_graph2[0], dst_graph2[0]);
    EXPECT_EQ_CTIMAGE(src_graph2[1], dst_graph2[1]);
    EXPECT_EQ_CTIMAGE(src_graph2[2], dst_graph2[2]);

// Graph 3 Verify
    dst1_graph3 = ct_image_from_vx_image(dst_graph3);

    virt_ctimage_graph3 = ct_allocate_image(640, 480, dstformat_graph3);
    ASSERT_NO_FAILURE(dst0_graph3 = ct_allocate_image(640, 480, srcformat_graph3));
    virt_orig_graph3 = ct_image_from_vx_image(virt_graph3);
    reference_colorconvert(src0_graph3, virt_ctimage_graph3);
    reference_colorconvert(virt_orig_graph3, dst0_graph3);

    ASSERT(cmp_color_images(virt_orig_graph3, virt_ctimage_graph3, ythresh_graph3, cthresh_graph3) >= 0);
    ASSERT(cmp_color_images(dst0_graph3, dst1_graph3, ythresh_graph3, cthresh_graph3) >= 0);

// Graph 4 Verify
    ref1_graph4  = ct_image_from_vx_image(src1_graph4);
    ref2_graph4  = ct_image_from_vx_image(src2_graph4);
    vx_add_result_graph4 = ct_image_from_vx_image(add_result_graph4);
    ref_add_result_graph4 = ct_allocate_image(640, 480, VX_DF_IMAGE_U8);

    // Add verify
    referenceAddSingle(ref1_graph4, ref2_graph4, ref_add_result_graph4, VX_CONVERT_POLICY_SATURATE);
    ASSERT_EQ_CTIMAGE(vx_add_result_graph4, ref_add_result_graph4);

    // Dilate verify
    ASSERT_NO_FAILURE(dilate_ref = ct_image_from_vx_image(dilate_out));
    dilate3x3_check(vx_add_result_graph4, dilate_ref, border_graph4);

    // Erode verify
    ASSERT_NO_FAILURE(erode_ref = ct_image_from_vx_image(erode_out));
    erode3x3_check(vx_add_result_graph4, erode_ref, border_graph4);

    // Gaussian verify
    ASSERT_NO_FAILURE(gaussian_ref = ct_image_from_vx_image(gaussian_out));
    gaussian3x3_check(vx_add_result_graph4, gaussian_ref, border_graph4);

    // Median verify
    ASSERT_NO_FAILURE(median_ref = ct_image_from_vx_image(median_out));
    median3x3_check(vx_add_result_graph4, median_ref, border_graph4);

    // Halfscale Gaussian verify
    ASSERT_NO_FAILURE(halfscalegaussian_ref = ct_image_from_vx_image(halfscalegaussian_out));
    halfScaleGaussian_validate(vx_add_result_graph4, halfscalegaussian_ref, arg_->kernel_size, border_graph4);

    // Sobel verify
    ASSERT_NO_FAILURE(sobel_ref = ct_image_from_vx_image(sobel_out));
    sobel3x3_check_x(vx_add_result_graph4, sobel_ref, border_graph4);

    // Warp Affine verify
    ASSERT_NO_FAILURE(warpaffine_ref = ct_image_from_vx_image(warpaffine_out));
    warp_affine_check(vx_add_result_graph4, warpaffine_ref, arg_->interp_type, border_graph4, warpaffine_m);

    // Warp Perspective verify
    ASSERT_NO_FAILURE(warpperspective_ref = ct_image_from_vx_image(warpperspective_out));
    warp_perspective_check(vx_add_result_graph4, warpperspective_ref, arg_->interp_type, border_graph4, warpperspective_m);

// Graph 5 Verify
    // Multiply
    mult_ref1  = ct_image_from_vx_image(mult_src1);
    mult_ref2  = ct_image_from_vx_image(mult_src2);
    mult_vxdst = ct_image_from_vx_image(mult_dst);
    mult_refdst = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_S16);
    mult_refdst_plus_1 = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_S16);
    referenceMultiply(mult_ref1, mult_ref2, mult_refdst, mult_refdst_plus_1, mult_scalef, VX_CONVERT_POLICY_SATURATE);
    EXPECT_CTIMAGE_NEAR(mult_refdst, mult_vxdst, 1);
    EXPECT_CTIMAGE_NEAR(mult_refdst_plus_1, mult_vxdst, 1);

    // Convert Depth
    convdepth_vxdst = ct_image_from_vx_image(convdepth_dst);
    convdepth_refdst = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_U8);
    referenceConvertDepth(mult_vxdst, convdepth_refdst, convdepth_shift, VX_CONVERT_POLICY_WRAP);
    EXPECT_EQ_CTIMAGE(convdepth_refdst, convdepth_vxdst);

    // AbsDiff
    absdiff_ref  = ct_image_from_vx_image(absdiff_src);
    absdiff_vxdst = ct_image_from_vx_image(absdiff_dst);
    absdiff_refdst = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_U8);
    referenceAbsDiffSingle(convdepth_vxdst, absdiff_ref, absdiff_refdst);
    ASSERT_EQ_CTIMAGE(absdiff_refdst, absdiff_vxdst);

    // Magnitude
    ASSERT_NO_FAILURE(mag0 = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_S16));
    mag1 = ct_image_from_vx_image(mag);
    reference_mag(mag_dx0, mag_dy0, mag0);
    ASSERT_CTIMAGE_NEAR(mag0, mag1, 1);

    // Phase
    ASSERT_NO_FAILURE(phase0 = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_U8));
    phase1 = ct_image_from_vx_image(phase);
    reference_phase(phase_dx0, phase_dy0, phase0);
    ASSERT_CTIMAGE_NEARWRAP(phase0, phase1, 1, 0);

// Graph 6 Verify
    not_vxdst = ct_image_from_vx_image(not_dst);
    not_refdst = ct_allocate_image(WIDTH, HEIGHT, VX_DF_IMAGE_U8);
    referenceNot(absdiff_vxdst, not_refdst);
    ASSERT_EQ_CTIMAGE(not_refdst, not_vxdst);

    ASSERT_NO_FAILURE(lut_ctdst = ct_image_from_vx_image(lut_dst));
    ASSERT_NO_FAILURE(lut_check(not_vxdst, lut_ctdst, lut_data, VX_TYPE_UINT8));

// Graph 7 Verify
    // Scale
    ASSERT_NO_FAILURE(scale_dst0 = ct_image_from_vx_image(scale_dst));
    ASSERT_NO_FAILURE(scale_check(phase1, scale_dst0, VX_INTERPOLATION_NEAREST_NEIGHBOR, scale_border, 2));

    // Histogram
    ASSERT_NO_FAILURE(reference_histogram(scale_dst0, hist0, nbins, offset, hist_range));

    {
        /* smoke tests for query distribution attributes */
        vx_size   attr_dims = 0;
        vx_int32  attr_offset = 0;
        vx_uint32 attr_range = 0;
        vx_size   attr_bins = 0;
        vx_uint32 attr_window = 0;
        vx_size   attr_size = 0;
        VX_CALL(vxQueryDistribution(dist, VX_DISTRIBUTION_DIMENSIONS, &attr_dims, sizeof(attr_dims)));
        if (1 != attr_dims)
            CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_DIMENSIONS failed\n");

        VX_CALL(vxQueryDistribution(dist, VX_DISTRIBUTION_OFFSET, &attr_offset, sizeof(attr_offset)));
        if (attr_offset != offset)
            CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_OFFSET failed\n");

        VX_CALL(vxQueryDistribution(dist, VX_DISTRIBUTION_RANGE, &attr_range, sizeof(attr_range)));
        if (attr_range != hist_range)
            CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_RANGE failed\n");

        VX_CALL(vxQueryDistribution(dist, VX_DISTRIBUTION_BINS, &attr_bins, sizeof(attr_bins)));
        if (attr_bins != nbins)
            CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_BINS failed\n");

        VX_CALL(vxQueryDistribution(dist, VX_DISTRIBUTION_WINDOW, &attr_window, sizeof(attr_window)));
        /*The attribute is specified as valid only when the range is a multiple of nbins,
         * in other cases, its value shouldn't be checked */
        if (((hist_range % nbins) == 0) && (attr_window != reference_window(hist_range, nbins)))
            CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_WINDOW failed\n");

        VX_CALL(vxQueryDistribution(dist, VX_DISTRIBUTION_SIZE, &attr_size, sizeof(attr_size)));
        if (attr_size < nbins*sizeof(hist0[0]))
            CT_FAIL("check for query distribution attribute VX_DISTRIBUTION_SIZE failed\n");
    }

    // Nonlinear filter
    ASSERT_NO_FAILURE(nonlinear_dst0 = ct_image_from_vx_image(nonlinear_dst));
    ASSERT_NO_FAILURE(filter_check(VX_NONLINEAR_FILTER_MEDIAN, nonlinear_src0, nonlinear_mask, nonlinear_dst0, &scale_border));

// Graph 8 Verify
    ASSERT_NO_FAILURE(canny_vxdst = ct_image_from_vx_image(canny_dst));
    ASSERT_NO_FAILURE(canny_refdst = get_reference_result("lena_gray.bmp", lena, low_thresh, high_thresh, 3, VX_NORM_L1));

    ASSERT_NO_FAILURE(ct_adjust_roi(canny_vxdst, canny_border_width, canny_border_width, canny_border_width, canny_border_width));
    ASSERT_NO_FAILURE(ct_adjust_roi(canny_refdst, canny_border_width, canny_border_width, canny_border_width, canny_border_width));

    ASSERT_NO_FAILURE(canny_dist = ct_allocate_image(canny_refdst->width, canny_refdst->height, VX_DF_IMAGE_U8));

    // disttransform(x,y) < tolerance for all (x,y) such that output(x,y) = 255,
    // where disttransform is the distance transform image with Euclidean distance
    // of the reference(x,y) (canny edge ground truth). This condition should be
    // satisfied by 98% of output edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(canny_count = disttransform2_metric(canny_refdst, canny_vxdst, canny_dist, &canny_total, 0));
    if (canny_count < CANNY_ACCEPTANCE_THRESHOLD * canny_total)
    {
        CT_RecordFailureAtFormat("disttransform(reference) < 2 only for %u of %u pixels of output edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            canny_count, canny_total, canny_count/(double)canny_total*100, CANNY_ACCEPTANCE_THRESHOLD*100);
    }

    // And the inverse: disttransform(x,y) < tolerance for all (x,y) such that
    // reference(x,y) = 255, where disttransform is the distance transform image
    // with Euclidean distance of the output(x,y) (canny edge ground truth). This
    // condition should be satisfied by 98% of reference edge pixels, tolerance = 2.
    ASSERT_NO_FAILURE(canny_count = disttransform2_metric(canny_vxdst, canny_refdst, canny_dist, &canny_total, 0));
    if (canny_count < CANNY_ACCEPTANCE_THRESHOLD * canny_total)
    {
        CT_RecordFailureAtFormat("disttransform(output) < 2 only for %u of %u pixels of reference edges which is %.2f%% < %.2f%%", __FUNCTION__, __FILE__, __LINE__,
            canny_count, canny_total, canny_count/(double)canny_total*100, CANNY_ACCEPTANCE_THRESHOLD*100);
    }

// Release Graph 1 and Assert
    // Release Harris Corners
    VX_CALL(vxReleaseNode(&node6_graph1));
    VX_CALL(vxReleaseNode(&node5_graph1));
    VX_CALL(vxReleaseNode(&node4_graph1));
    VX_CALL(vxReleaseNode(&node3_graph1));
    VX_CALL(vxReleaseNode(&node2_graph1));
    VX_CALL(vxReleaseNode(&node1_graph1));
    VX_CALL(vxReleaseGraph(&graph1));
    ASSERT(node4_graph1 == 0);
    ASSERT(node3_graph1 == 0);
    ASSERT(node2_graph1 == 0);
    ASSERT(node1_graph1 == 0);
    ASSERT(graph1 == 0);
    VX_CALL(vxReleaseScalar(&eps));
    ASSERT(eps == 0);
    VX_CALL(vxReleaseScalar(&num_iter));
    ASSERT(num_iter == 0);
    VX_CALL(vxReleaseScalar(&use_estimations));
    ASSERT(use_estimations == 0);
    VX_CALL(vxReleasePyramid(&src_pyr_node2_graph1));
    ASSERT(src_pyr_node2_graph1 == 0);
    VX_CALL(vxReleasePyramid(&pyr_node2_graph1));
    ASSERT(pyr_node2_graph1 == 0);
    ct_free_mem(truth_data.pts); truth_data.pts = 0;
    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseArray(&corners_virt));
    VX_CALL(vxReleaseScalar(&num_corners_scalar0));
    VX_CALL(vxReleaseScalar(&num_corners_scalar1));
    VX_CALL(vxReleaseScalar(&sensitivity_scalar));
    VX_CALL(vxReleaseScalar(&min_distance_scalar));
    VX_CALL(vxReleaseScalar(&strength_thresh_scalar));
    VX_CALL(vxReleaseImage(&input_image_graph1));
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
    ASSERT(input_image_graph1 == 0);
    // Release Fast Corners
    VX_CALL(vxReleaseImage(&src0_image_graph1));
    ASSERT(src0_image_graph1 == 0);
    VX_CALL(vxReleaseScalar(&sthresh));
    ASSERT(sthresh == 0);
    VX_CALL(vxReleasePyramid(&src_pyr_node4_graph1));
    ASSERT(src_pyr_node4_graph1 == 0);
    VX_CALL(vxReleasePyramid(&pyr_node4_graph1));
    ASSERT(pyr_node4_graph1 == 0);
    VX_CALL(vxReleaseArray(&corners1));
    ASSERT(corners1 == 0);
    // Release Gaussian Pyramid
    VX_CALL(vxReleasePyramid(&src_pyr_node6_graph1));
    ASSERT(src_pyr_node6_graph1 == 0);
    VX_CALL(vxReleasePyramid(&pyr_node6_graph1));
    ASSERT(pyr_node6_graph1 == 0);
    VX_CALL(vxReleaseArray(&corners2));
    ASSERT(corners2 == 0);
    VX_CALL(vxReleaseArray(&new_points_arr2));
    ASSERT(new_points_arr2 == 0);
    VX_CALL(vxReleaseImage(&input_image_gausspyr_graph1));
    ASSERT(input_image_gausspyr_graph1 == 0);

// Release Graph 2
    VX_CALL(vxReleaseNode(&node4_graph2));
    VX_CALL(vxReleaseNode(&node3_graph2));
    VX_CALL(vxReleaseNode(&node2_graph2));
    VX_CALL(vxReleaseNode(&node1_graph2));
    VX_CALL(vxReleaseGraph(&graph2));
    VX_CALL(vxReleaseImage(&virt_graph2));

    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseImage(&src_image_graph2[i]));
        VX_CALL(vxReleaseImage(&dst_image_graph2[i]));
        ASSERT(src_image_graph2[i] == 0);
        ASSERT(dst_image_graph2[i] == 0);
    }

// Release Graph 3
    VX_CALL(vxReleaseImage(&src_graph3));
    VX_CALL(vxReleaseImage(&virt_graph3));
    VX_CALL(vxReleaseImage(&dst_graph3));
    VX_CALL(vxReleaseNode(&node2_graph3));
    VX_CALL(vxReleaseNode(&node1_graph3));
    VX_CALL(vxReleaseGraph(&graph3));

// Release Graph 4
    VX_CALL(vxReleaseMatrix(&warpperspective_matrix));
    VX_CALL(vxReleaseMatrix(&warpaffine_matrix));
    VX_CALL(vxReleaseImage(&dilate_out));
    VX_CALL(vxReleaseImage(&erode_out));
    VX_CALL(vxReleaseImage(&gaussian_out));
    VX_CALL(vxReleaseImage(&median_out));
    VX_CALL(vxReleaseImage(&sobel_out));
    VX_CALL(vxReleaseImage(&warpaffine_out));
    VX_CALL(vxReleaseImage(&warpperspective_out));
    VX_CALL(vxReleaseImage(&halfscalegaussian_out));
    VX_CALL(vxReleaseImage(&add_result_graph4));
    VX_CALL(vxReleaseImage(&src2_graph4));
    VX_CALL(vxReleaseImage(&src1_graph4));
    VX_CALL(vxReleaseNode(&node9_graph4));
    VX_CALL(vxReleaseNode(&node8_graph4));
    VX_CALL(vxReleaseNode(&node7_graph4));
    VX_CALL(vxReleaseNode(&node6_graph4));
    VX_CALL(vxReleaseNode(&node5_graph4));
    VX_CALL(vxReleaseNode(&node4_graph4));
    VX_CALL(vxReleaseNode(&node3_graph4));
    VX_CALL(vxReleaseNode(&node2_graph4));
    VX_CALL(vxReleaseNode(&node1_graph4));
    VX_CALL(vxReleaseGraph(&graph4));

// Release Graph 5
    VX_CALL(vxReleaseImage(&phase));
    VX_CALL(vxReleaseImage(&phase_dy));
    VX_CALL(vxReleaseImage(&phase_dx));
    VX_CALL(vxReleaseImage(&mag));
    VX_CALL(vxReleaseImage(&mag_dy));
    VX_CALL(vxReleaseImage(&mag_dx));
    VX_CALL(vxReleaseImage(&absdiff_dst));
    VX_CALL(vxReleaseImage(&absdiff_src));
    VX_CALL(vxReleaseImage(&convdepth_dst));
    VX_CALL(vxReleaseImage(&mult_src1));
    VX_CALL(vxReleaseImage(&mult_src2));
    VX_CALL(vxReleaseImage(&mult_dst));
    VX_CALL(vxReleaseScalar(&convdepth_scalar_shift));
    VX_CALL(vxReleaseScalar(&mult_scale));
    VX_CALL(vxReleaseNode(&node5_graph5));
    VX_CALL(vxReleaseNode(&node4_graph5));
    VX_CALL(vxReleaseNode(&node3_graph5));
    VX_CALL(vxReleaseNode(&node2_graph5));
    VX_CALL(vxReleaseNode(&node1_graph5));
    VX_CALL(vxReleaseGraph(&graph5));

// Release Graph 6
    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseImage(&not_dst));
    VX_CALL(vxReleaseImage(&lut_dst));
    VX_CALL(vxReleaseNode(&node2_graph6));
    VX_CALL(vxReleaseNode(&node1_graph6));
    VX_CALL(vxReleaseGraph(&graph6));
    ct_free_mem(lut_data);

// Release Graph 7
    VX_CALL(vxReleaseMatrix(&nonlinear_mask));
    VX_CALL(vxReleaseImage(&nonlinear_dst));
    VX_CALL(vxReleaseImage(&nonlinear_src));
    VX_CALL(vxReleaseDistribution(&dist));
    VX_CALL(vxReleaseImage(&scale_dst));
    VX_CALL(vxReleaseNode(&node3_graph7));
    VX_CALL(vxReleaseNode(&node2_graph7));
    VX_CALL(vxReleaseNode(&node1_graph7));
    VX_CALL(vxReleaseGraph(&graph7));

// Release Graph 8
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseThreshold(&hyst));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&canny_src));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseImage(&canny_dst));
    VX_CALL(vxReleaseNode(&node1_graph8));
    VX_CALL(vxReleaseGraph(&graph8));

    // Performance print out
    printPerformance(perf_node1_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "N1");
    printPerformance(perf_node2_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "N2");
    printPerformance(perf_node3_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "N3");
    printPerformance(perf_node4_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "N4");
    printPerformance(perf_node5_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "N5");
    printPerformance(perf_node6_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "N6");
    printPerformance(perf_graph1, input_ctimage_graph1->width*input_ctimage_graph1->height, "G1");
    printPerformance(perf_node1_graph2, 640*480, "N1");
    printPerformance(perf_node2_graph2, 640*480, "N2");
    printPerformance(perf_node3_graph2, 640*480, "N3");
    printPerformance(perf_node4_graph2, 640*480, "N4");
    printPerformance(perf_graph2, 640*480, "G2");
    printPerformance(perf_node1_graph3, 640*480, "N1");
    printPerformance(perf_node2_graph3, 640*480, "N2");
    printPerformance(perf_graph3, 640*480, "G3");
    printPerformance(perf_node1_graph4, 640*480, "N1");
    printPerformance(perf_node2_graph4, 640*480, "N2");
    printPerformance(perf_node3_graph4, 640*480, "N3");
    printPerformance(perf_node4_graph4, 640*480, "N4");
    printPerformance(perf_node5_graph4, 640*480, "N5");
    printPerformance(perf_node6_graph4, 640*480, "N6");
    printPerformance(perf_node7_graph4, 640*480, "N7");
    printPerformance(perf_node8_graph4, 640*480, "N8");
    printPerformance(perf_node9_graph4, 640*480, "N9");
    printPerformance(perf_graph4, 640*480, "G4");
    printPerformance(perf_node1_graph5, WIDTH*HEIGHT, "N1");
    printPerformance(perf_node2_graph5, WIDTH*HEIGHT, "N2");
    printPerformance(perf_node3_graph5, WIDTH*HEIGHT, "N3");
    printPerformance(perf_node4_graph5, WIDTH*HEIGHT, "N4");
    printPerformance(perf_node5_graph5, WIDTH*HEIGHT, "N5");
    printPerformance(perf_graph5, WIDTH*HEIGHT, "G5");
    printPerformance(perf_node1_graph6, WIDTH*HEIGHT, "N1");
    printPerformance(perf_node2_graph6, WIDTH*HEIGHT, "N2");
    printPerformance(perf_graph6, WIDTH*HEIGHT, "G6");
    printPerformance(perf_node1_graph7, WIDTH*HEIGHT, "N1");
    printPerformance(perf_node2_graph7, WIDTH*HEIGHT, "N2");
    printPerformance(perf_node3_graph7, WIDTH*HEIGHT, "N2");
    printPerformance(perf_graph7, WIDTH*HEIGHT, "G7");
    printPerformance(perf_node1_graph8, WIDTH*HEIGHT, "N1");
    printPerformance(perf_graph8, WIDTH*HEIGHT, "G8");
}

TESTCASE_TESTS(tivxMaxNodes,
        testMaxNodes
)
