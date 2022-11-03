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
#include <math.h>

#include "shared_functions.h"

#define VX_GAUSSIAN_PYRAMID_TOLERANCE 1
#define MAX_POINTS 100

TESTCASE(tivxGaussianPyramid, CT_VXContext, ct_setup_vx_context, 0)

#define LEVELS_COUNT_MAX    7

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
    if (sz != read_sz)
    {
        ct_free_mem(buf);
        fclose(f);
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

void tivx_gaussian_pyramid_fill_reference(CT_Image input, vx_pyramid pyr, vx_size levels, vx_float32 scale, vx_border_t border)
{
    vx_uint32 level = 0;
    vx_image  output_image = 0;
    CT_Image  output_prev  = NULL;
    CT_Image  output_cur   = NULL;
    vx_uint32 ref_width    = input->width;
    vx_uint32 ref_height   = input->height;

    ASSERT(input && pyr && (levels < sizeof(c_orbscale) / sizeof(vx_float64) ));
    ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, 0), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(output_prev = ct_image_from_vx_image(output_image));

    CT_FILL_IMAGE_8U(return, output_prev,
            {
                uint8_t res = *CT_IMAGE_DATA_PTR_8U(input, x, y);
                *dst_data = res;
            });
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output_image, output_prev));

    VX_CALL(vxReleaseImage(&output_image));
    ASSERT(output_image == 0);

    for (level = 1; level < levels; level++)
    {
        ASSERT_VX_OBJECT(output_image = vxGetPyramidLevel(pyr, level), VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(output_cur = ct_image_from_vx_image(output_image));

        if (VX_SCALE_PYRAMID_ORB == scale)
        {
            vx_float64 orb_scale = c_orbscale[level];
            if ( (output_cur->width  != ceil(ref_width  * orb_scale)) ||
                 (output_cur->height != ceil(ref_height * orb_scale)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }
        else
        {
            if ( (output_cur->width  != ceil(output_prev->width  * scale)) ||
                 (output_cur->height != ceil(output_prev->height * scale)))
            {
                CT_FAIL_(return, "Check failed for size of level: %d", level);
            }
        }

        ASSERT_NO_FAILURE(output_cur = gaussian_pyramid_create_reference_image(input, output_prev, border, scale, level));
        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(output_image, output_cur));

        VX_CALL(vxReleaseImage(&output_image));
        ASSERT(output_image == 0);

        output_prev = output_cur;
    }
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

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    vx_border_t border;
    int width, height;
    vx_float32 scale;
} Arg;

#define ADD_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF)), \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_640x480, ADD_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL)

TEST_WITH_ARG(tivxGaussianPyramid, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid pyr = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_pyramid src_pyr   = 0;
    vx_array old_points_arr = 0;
    vx_array new_points_arr = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = 5; // hardcoded from optflow test case
    vx_border_t border = arg_->border;
    vx_border_t border_rep = { VX_BORDER_REPLICATE };
    vx_bool valid_rect;
    vx_rectangle_t dst_rect;
    vx_uint32 w, h, new_w, new_h, i;
    vx_float32 new_wf, new_hf;
    vx_image tmp_img;

    vx_size num_points = 0;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points = 0;
    vx_size new_points_size = 0;

    vx_size max_window_dim = 0;

    CT_Image input = NULL, src_ct_image = NULL;

    ASSERT(arg_->scale < 1.0);

    ASSERT_NO_FAILURE(input = optflow_pyrlk_read_image( "optflow_00.bmp", 0, 0));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = gaussian_pyramid_calc_max_levels_count(input->width, input->height, arg_->scale);

    ASSERT_NO_FAILURE(src_ct_image = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points = own_read_keypoints("optflow_pyrlk_5x5.txt", &old_points, &new_points_ref));

    ASSERT_VX_OBJECT(old_points_arr = own_create_keypoint_array(context, num_points, old_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(pyr = vxCreatePyramid(context, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image, src_pyr, levels, arg_->scale, border_rep));

    ASSERT_VX_OBJECT(node1 = vxGaussianPyramidNode(graph, input_image, pyr), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        pyr, src_pyr,
        old_points_arr, old_points_arr, new_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    if (border.mode != VX_BORDER_UNDEFINED)
        VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryPyramid(pyr, VX_PYRAMID_LEVELS, &levels, sizeof(levels));
    vxQueryPyramid(pyr, VX_PYRAMID_WIDTH, &w, sizeof(w));
    vxQueryPyramid(pyr, VX_PYRAMID_HEIGHT, &h, sizeof(h));

    new_w = w;
    new_h = h;
    new_wf = w;
    new_hf = h;

    if (arg_->scale == VX_SCALE_PYRAMID_HALF)
    {
        new_w = w / 2;
        new_h = h / 2;
        new_wf = w / 2;
        new_hf = h / 2;

        for (i = 1; i < levels; i++)
        {
            tmp_img = vxGetPyramidLevel(pyr, i);

            vxGetValidRegionImage(tmp_img, &dst_rect);

            ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), new_w-2);
            ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), new_h-2);

            new_w = new_w / 2;
            new_h = new_h / 2;
            VX_CALL(vxReleaseImage(&tmp_img));
        }
    }
    else if (arg_->scale == VX_SCALE_PYRAMID_ORB) // 0.8408964f
    {
        vx_uint32 start_x = 2, start_y = 2;
        new_wf = new_wf*VX_SCALE_PYRAMID_ORB - 0.001; // Had to subtract due to precision
        new_hf = new_hf*VX_SCALE_PYRAMID_ORB - 0.001;
        new_w = ceil(new_wf) - 2;
        new_h = ceil(new_hf) - 2;

        for (i = 1; i < levels; i++)
        {
            tmp_img = vxGetPyramidLevel(pyr, i);

            vxGetValidRegionImage(tmp_img, &dst_rect);

            ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), new_w-start_x);
            ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), new_h-start_y);

            new_wf = new_wf*VX_SCALE_PYRAMID_ORB - 0.001; // Had to subtract due to precision
            new_hf = new_hf*VX_SCALE_PYRAMID_ORB - 0.001;
            new_w = ceil(new_wf) - 2;
            new_h = ceil(new_hf) - 2;
            start_x += 2;
            start_y += 2;
            VX_CALL(vxReleaseImage(&tmp_img));
        }
    }

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    CT_ASSERT_NO_FAILURE_(, gaussian_pyramid_check(input, pyr, levels, arg_->scale, arg_->border));

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr, (void**)&new_points, 0, &new_points_size, 0));
    ASSERT(new_points_size == num_points);

    ASSERT_NO_FAILURE(own_keypoints_check(num_points, old_points, new_points_ref, new_points));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseScalar(&eps));
    VX_CALL(vxReleaseScalar(&num_iter));
    VX_CALL(vxReleaseScalar(&use_estimations));
    VX_CALL(vxReleaseArray(&old_points_arr));
    VX_CALL(vxReleaseArray(&new_points_arr));
    VX_CALL(vxReleasePyramid(&src_pyr));
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleasePyramid(&pyr));
    VX_CALL(vxReleaseImage(&input_image));
    ASSERT(input_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

TEST_WITH_ARG(tivxGaussianPyramid, testVirtualPyramid, Arg,
    PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid virt_pyr = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;
    vx_pyramid src_pyr   = 0;
    vx_array old_points_arr = 0;
    vx_array new_points_arr = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = 5; // hardcoded from optflow test case
    vx_border_t border = arg_->border;
    vx_border_t border_rep = { VX_BORDER_REPLICATE };

    vx_size num_points = 0;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points = 0;
    vx_size new_points_size = 0;

    vx_size max_window_dim = 0;

    CT_Image input = NULL, src_ct_image = NULL;

    ASSERT(arg_->scale < 1.0);

    ASSERT_NO_FAILURE(input = optflow_pyrlk_read_image( "optflow_00.bmp", 0, 0));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = gaussian_pyramid_calc_max_levels_count(input->width, input->height, arg_->scale);

    ASSERT_NO_FAILURE(src_ct_image = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points = own_read_keypoints("optflow_pyrlk_5x5.txt", &old_points, &new_points_ref)); // create a new pts file

    ASSERT_VX_OBJECT(old_points_arr = own_create_keypoint_array(context, num_points, old_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt_pyr = vxCreateVirtualPyramid(graph, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(tivx_gaussian_pyramid_fill_reference(src_ct_image, src_pyr, levels, arg_->scale, border_rep));

    ASSERT_VX_OBJECT(node1 = vxGaussianPyramidNode(graph, input_image, virt_pyr), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        virt_pyr, src_pyr,
        old_points_arr, old_points_arr, new_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    if (border.mode != VX_BORDER_UNDEFINED)
        VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT(VX_TYPE_KEYPOINT == ct_read_array(new_points_arr, (void**)&new_points, 0, &new_points_size, 0));
    ASSERT(new_points_size == num_points);

    ASSERT_NO_FAILURE(own_keypoints_check(num_points, old_points, new_points_ref, new_points));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseScalar(&eps));
    VX_CALL(vxReleaseScalar(&num_iter));
    VX_CALL(vxReleaseScalar(&use_estimations));
    VX_CALL(vxReleaseArray(&old_points_arr));
    VX_CALL(vxReleaseArray(&new_points_arr));
    VX_CALL(vxReleasePyramid(&src_pyr));
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleasePyramid(&virt_pyr));
    VX_CALL(vxReleaseImage(&input_image));
    ASSERT(input_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");
    printPerformance(perf_node2, arg_->width*arg_->height, "N2");
    printPerformance(perf_graph, arg_->width*arg_->height, "G1");
}

#define NEG_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_SCALE_PYRAMID_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF))

#define NEGATIVE_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_REPLICATE_ONLY, ADD_SIZE_640x480, NEG_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_VX_BORDERS_REQUIRE_CONSTANT_ONLY, ADD_SIZE_640x480, NEG_VX_SCALE, ARG, gaussian_pyramid_generate_random, NULL)

TEST_WITH_ARG(tivxGaussianPyramid, negativeTestBorderMode, Arg,
    NEGATIVE_PARAMETERS
)
{
    vx_size levels;

    vx_context context = context_->vx_context_;
    vx_image input_image = 0;
    vx_pyramid virt_pyr = 0;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_pyramid src_pyr   = 0;
    vx_array old_points_arr = 0;
    vx_array new_points_arr = 0;
    vx_float32 eps_val      = 0.001f;
    vx_uint32  num_iter_val = 100;
    vx_bool   use_estimations_val = vx_true_e;
    vx_scalar eps                 = 0;
    vx_scalar num_iter            = 0;
    vx_scalar use_estimations     = 0;
    vx_size   winSize             = 5; // hardcoded from optflow test case
    vx_border_t border = arg_->border;
    vx_border_t border_rep = { VX_BORDER_REPLICATE };

    vx_size num_points = 0;
    vx_keypoint_t* old_points = 0;
    vx_keypoint_t* new_points_ref = 0;
    vx_keypoint_t* new_points = 0;
    vx_size new_points_size = 0;

    vx_size max_window_dim = 0;

    CT_Image input = NULL, src_ct_image = NULL;

    ASSERT(arg_->scale < 1.0);

    ASSERT_NO_FAILURE(input = optflow_pyrlk_read_image( "optflow_00.bmp", 0, 0));
    ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);

    levels = gaussian_pyramid_calc_max_levels_count(input->width, input->height, arg_->scale);

    ASSERT_NO_FAILURE(src_ct_image = optflow_pyrlk_read_image( "optflow_01.bmp", 0, 0));

    ASSERT_VX_OBJECT(src_pyr = vxCreatePyramid(context, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_NO_FAILURE(num_points = own_read_keypoints("optflow_pyrlk_5x5.txt", &old_points, &new_points_ref)); // create a new pts file

    ASSERT_VX_OBJECT(old_points_arr = own_create_keypoint_array(context, num_points, old_points), VX_TYPE_ARRAY);
    ASSERT_VX_OBJECT(new_points_arr = vxCreateArray(context, VX_TYPE_KEYPOINT, num_points), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(eps             = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(num_iter        = vxCreateScalar(context, VX_TYPE_UINT32, &num_iter_val), VX_TYPE_SCALAR);
    ASSERT_VX_OBJECT(use_estimations = vxCreateScalar(context, VX_TYPE_BOOL, &use_estimations_val), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt_pyr = vxCreateVirtualPyramid(graph, levels, arg_->scale, input->width, input->height, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);

    ASSERT_VX_OBJECT(node1 = vxGaussianPyramidNode(graph, input_image, virt_pyr), VX_TYPE_NODE);

    ASSERT_VX_OBJECT(node2 = vxOpticalFlowPyrLKNode(
        graph,
        virt_pyr, src_pyr,
        old_points_arr, old_points_arr, new_points_arr,
        VX_TERM_CRITERIA_BOTH, eps, num_iter, use_estimations, winSize), VX_TYPE_NODE);

    if (border.mode != VX_BORDER_UNDEFINED)
        VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    ASSERT_EQ_VX_STATUS(vxVerifyGraph(graph), VX_ERROR_NOT_SUPPORTED);

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseScalar(&eps));
    VX_CALL(vxReleaseScalar(&num_iter));
    VX_CALL(vxReleaseScalar(&use_estimations));
    VX_CALL(vxReleaseArray(&old_points_arr));
    VX_CALL(vxReleaseArray(&new_points_arr));
    VX_CALL(vxReleasePyramid(&src_pyr));
    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleasePyramid(&virt_pyr));
    VX_CALL(vxReleaseImage(&input_image));
    ASSERT(input_image == 0);
}

TESTCASE_TESTS(tivxGaussianPyramid,
        testGraphProcessing,
        testVirtualPyramid,
        negativeTestBorderMode
)
