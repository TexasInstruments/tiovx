/*

 * Copyright (c) 2017 The Khronos Group Inc.
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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/j7.h>
#include "test_engine/test.h"
#include <string.h>


#define MAX_CONV_SIZE 5

TESTCASE(tivxHwaVpacNfGeneric, CT_VXContext, ct_setup_vx_context, 0)

static vx_convolution convolution_create(vx_context context, int cols, int rows, vx_int16* data, vx_uint32 scale)
{
    vx_convolution convolution = vxCreateConvolution(context, cols, rows);
    vx_size size = 0;

    ASSERT_VX_OBJECT_(return 0, convolution, VX_TYPE_CONVOLUTION);

    VX_CALL_(return 0, vxQueryConvolution(convolution, VX_CONVOLUTION_SIZE, &size, sizeof(size)));

    VX_CALL_(return 0, vxCopyConvolutionCoefficients(convolution, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    VX_CALL_(return 0, vxSetConvolutionAttribute(convolution, VX_CONVOLUTION_SCALE, &scale, sizeof(scale)));

    return convolution;
}

static void convolution_data_fill_identity(int cols, int rows, vx_int16* data)
{
    int x = cols / 2, y = rows / 2;
    ct_memset(data, 0, sizeof(vx_int16) * cols * rows);
    data[y * cols + x] = 1;
}

static void convolution_data_fill_random_128(int cols, int rows, vx_int16* data)
{
    uint64_t* seed = &CT()->seed_;
    int i;

    for (i = 0; i < cols * rows; i++)
        data[i] = (vx_uint8)CT_RNG_NEXT_INT(*seed, (uint32_t)-128, 128);
}


TEST(tivxHwaVpacNfGeneric, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    int cols = 3, rows = 3;
    tivx_vpac_nf_common_params_t params;
    vx_user_data_object param_obj;
    vx_int16 data[3 * 3] = { 0, 0, 0, 0, 1, 0, 0, 0, 0};
    vx_convolution convolution = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_NF))
    {
        tivxHwaLoadKernels(context);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(convolution = convolution_create(context, cols, rows, data, 1), VX_TYPE_CONVOLUTION);

        memset(&params, 0, sizeof(tivx_vpac_nf_common_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_common_params_t",
                                                            sizeof(tivx_vpac_nf_common_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfGenericNode(graph, param_obj, src_image, convolution, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_NF));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_obj == 0);

        VX_CALL(vxReleaseConvolution(&convolution));
        ASSERT(convolution == NULL);

        tivxHwaUnLoadKernels(context);
    }
}


static CT_Image convolve_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static CT_Image convolve_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}

static int32_t convolve_get(CT_Image src, int32_t x, int32_t y, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_int32 shift, vx_df_image dst_format)
{
    int i;
    int32_t sum = 0, datasum = 0, value = 0;
    int32_t src_data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };

    ASSERT_(return 0, cols <= MAX_CONV_SIZE);
    ASSERT_(return 0, rows <= MAX_CONV_SIZE);

    ASSERT_NO_FAILURE_(return 0,
            ct_image_read_rect_S32(src, src_data, x - cols / 2, y - rows / 2, x + cols / 2, y + rows / 2, border));

    for (i = 0; i < cols * rows; ++i) {
        sum += src_data[i] * data[cols * rows - 1 - i];
        datasum += data[cols * rows - 1 - i];
    }

    if ( shift > 0 ) {
        value = ((sum >> shift) / datasum) ;
    } else if ( shift < 0 ) {
        value = ((sum << (-1*shift)) / datasum) ;
    } else {
        value = (sum / datasum);
    }

    if (dst_format == VX_DF_IMAGE_U8)
    {
        if (value < 0) value = 0;
        else if (value > UINT8_MAX) value = UINT8_MAX;
    }
    else if (dst_format == VX_DF_IMAGE_S16)
    {
        if (value < INT16_MIN) value = INT16_MIN;
        else if (value > INT16_MAX) value = INT16_MAX;
    }

    return value;
}


static CT_Image convolve_create_reference_image(CT_Image src, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_int32 shift, vx_df_image dst_format)
{
    CT_Image dst;

    CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);

    dst = ct_allocate_image(src->width, src->height, dst_format);

    if (dst_format == VX_DF_IMAGE_U8)
    {
        CT_FILL_IMAGE_8U(return 0, dst,
                {
                    int32_t res = convolve_get(src, x, y, border, cols, rows, data, shift, dst_format);
                    *dst_data = (vx_uint8)res;
                });
    }
    else if (dst_format == VX_DF_IMAGE_S16)
    {
        CT_FILL_IMAGE_16S(return 0, dst,
                {
                    int32_t res = convolve_get(src, x, y, border, cols, rows, data, shift, dst_format);
                    *dst_data = (vx_int16)res;
                });
    }
    else
    {
        CT_FAIL_(return 0, "NOT IMPLEMENTED");
    }
    return dst;
}


static void convolve_check(CT_Image src, CT_Image dst, vx_border_t border,
        int cols, int rows, vx_int16* data, vx_int32 shift, vx_df_image dst_format)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = convolve_create_reference_image(src, border, cols, rows, data, shift, dst_format));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst, cols / 2, rows / 2, cols / 2, rows / 2);
            ct_adjust_roi(dst_ref, cols / 2, rows / 2, cols / 2, rows / 2);
        }
    );

    EXPECT_CTIMAGE_NEAR(dst_ref, dst, 1);
#if 0
    if (CT_HasFailure())
    {
        printf("=== SRC ===\n");
        ct_dump_image_info_ex(src, 16, 4);
        printf("=== DST ===\n");
        ct_dump_image_info_ex(dst, 16, 4);
        printf("=== EXPECTED ===\n");
        ct_dump_image_info_ex(dst_ref, 16, 4);
    }
#endif
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int cols, rows;
    vx_int32 shift;
    void (*convolution_data_generator)(int cols, int rows, vx_int16* data);
    vx_df_image dst_format;
    vx_border_t border;
    int width, height;
} Arg;

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int cols, rows;
    vx_int32 shift;
    void (*convolution_data_generator)(int cols, int rows, vx_int16* data);
    vx_df_image dst_format;
    vx_border_t border;
    int width, height;
    int negative_test;
    int condition;
} ArgNegative;

#define ADD_CONV_SIZE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv=3x3", __VA_ARGS__, 3, 3)), \
    CT_EXPAND(nextmacro(testArgName "/conv=5x3", __VA_ARGS__, 5, 3)), \
    CT_EXPAND(nextmacro(testArgName "/conv=3x5", __VA_ARGS__, 3, 5)), \
    CT_EXPAND(nextmacro(testArgName "/conv=5x5", __VA_ARGS__, 5, 5))

#define ADD_CONV_SHIFT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=7", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=-1", __VA_ARGS__, -1)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=-2", __VA_ARGS__, -2)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=-8", __VA_ARGS__, -8))

#define ADD_CONV_GENERATORS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_fill=identity", __VA_ARGS__, convolution_data_fill_identity)), \
    CT_EXPAND(nextmacro(testArgName "/conv_fill=random128", __VA_ARGS__, convolution_data_fill_random_128))

#define ADD_CONV_DST_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dst8U", __VA_ARGS__, VX_DF_IMAGE_U8))
#if 0
, \
    CT_EXPAND(nextmacro(testArgName "/dst16S", __VA_ARGS__, VX_DF_IMAGE_S16))
#endif

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_CONV_SIZE, ADD_CONV_SHIFT, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_64x64, ARG, convolve_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_CONV_SIZE, ADD_CONV_SHIFT, ADD_CONV_GENERATORS, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, convolve_read_image, "lena.bmp")

#define ADD_CONV_SIZE_NEGATIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv=5x5", __VA_ARGS__, 5, 5))

#define ADD_CONV_GENERATORS_NEGATIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_fill=identity", __VA_ARGS__, convolution_data_fill_identity))

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=input_interleaved", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_downshift", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_offset", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_pixel_skip", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_pixel_skip_odd", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=kern_ln_offset", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=kern_sz_height", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=src_ln_inc_2", __VA_ARGS__, 7))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=negative", __VA_ARGS__, 2))

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_CONV_SIZE_NEGATIVE, ADD_CONV_SHIFT, ADD_CONV_GENERATORS_NEGATIVE, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_64x64, ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ARG, convolve_generate_random, NULL)

TEST_WITH_ARG(tivxHwaVpacNfGeneric, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_convolution convolution = 0;
    vx_int16 data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    tivx_vpac_nf_common_params_t params;
    vx_user_data_object param_obj;
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_NF))
    {
        tivxHwaLoadKernels(context);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

        VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

        if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
        {
            printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
            return;
        }

        ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data));
        ASSERT_NO_FAILURE(convolution = convolution_create(context, arg_->cols, arg_->rows, data, 1));

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_common_params_t",
                                                            sizeof(tivx_vpac_nf_common_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_nf_common_params_init(&params);

        params.output_downshift = arg_->shift;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_nf_common_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfGenericNode(graph, param_obj, src_image, convolution, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_NF));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

        ASSERT_NO_FAILURE(convolve_check(src, dst, border, arg_->cols, arg_->rows, data, arg_->shift, arg_->dst_format));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_obj == 0);

        VX_CALL(vxReleaseConvolution(&convolution));
        ASSERT(convolution == NULL);

        tivxHwaUnLoadKernels(context);
    }
}
#if 0
TEST_WITH_ARG(tivxHWA_VPAC_NF_Generic, testImmediateProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_convolution convolution = 0;
    vx_int16 data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    vx_size conv_max_dim = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

    if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
    {
        printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
        return;
    }

    ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data));
    ASSERT_NO_FAILURE(convolution = convolution_create(context, arg_->cols, arg_->rows, data, arg_->scale));

    VX_CALL(vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));

    VX_CALL(vxuConvolve(context, src_image, convolution, dst_image));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(convolve_check(src, dst, border, arg_->cols, arg_->rows, data, arg_->scale, arg_->dst_format));

    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    VX_CALL(vxReleaseConvolution(&convolution));
    ASSERT(convolution == NULL);
}
#endif

TEST_WITH_ARG(tivxHwaVpacNfGeneric, testNegativeGraph, ArgNegative,
    PARAMETERS_NEGATIVE
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_convolution convolution = 0;
    vx_int16 data[MAX_CONV_SIZE * MAX_CONV_SIZE] = { 0 };
    tivx_vpac_nf_common_params_t params;
    vx_user_data_object param_obj;
    vx_size conv_max_dim = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    vx_border_t border = arg_->border;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_NF))
    {
        tivxHwaLoadKernels(context);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

        VX_CALL(vxQueryContext(context, VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &conv_max_dim, sizeof(conv_max_dim)));

        if ((vx_size)arg_->cols > conv_max_dim || (vx_size)arg_->rows > conv_max_dim)
        {
            printf("%dx%d convolution is not supported. Skip test\n", (int)arg_->cols, (int)arg_->rows);
            return;
        }

        ASSERT_NO_FAILURE(arg_->convolution_data_generator(arg_->cols, arg_->rows, data));
        ASSERT_NO_FAILURE(convolution = convolution_create(context, arg_->cols, arg_->rows, data, 1));
        
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_common_params_t",
                                                            sizeof(tivx_vpac_nf_common_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_nf_common_params_init(&params);

        params.output_downshift = arg_->shift;

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.input_interleaved = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.input_interleaved = 1;
                }
                else
                {
                    params.input_interleaved = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.output_downshift = -8;
                }
                else if (1U == arg_->condition)
                {
                    params.output_downshift = 7;
                }
                else
                {
                    params.output_downshift = 8;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.output_offset = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.output_offset = 4095;
                }
                else
                {
                    params.output_offset = 4096;
                }
                break;
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.output_pixel_skip = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.output_pixel_skip = 1;
                }
                else
                {
                    params.output_pixel_skip = 2;
                }
                break;
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.output_pixel_skip_odd = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.output_pixel_skip_odd = 1;
                }
                else
                {
                    params.output_pixel_skip_odd = 2;
                }
                break;
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    params.kern_ln_offset = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.kern_ln_offset = 4;
                }
                else
                {
                    params.kern_ln_offset = 5;
                }
                break;
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    params.kern_sz_height = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.kern_sz_height = 5;
                }
                else
                {
                    params.kern_sz_height = 6;
                }
                break;
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    params.src_ln_inc_2 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.src_ln_inc_2 = 1;
                }
                else
                {
                    params.src_ln_inc_2 = 2;
                }
                break;
            }
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_nf_common_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfGenericNode(graph, param_obj, src_image, convolution, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_NF));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        if(2 != arg_->condition)
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_obj == 0);

        VX_CALL(vxReleaseConvolution(&convolution));
        ASSERT(convolution == NULL);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVpacNfGeneric, testNodeCreation, testGraphProcessing, testNegativeGraph)
