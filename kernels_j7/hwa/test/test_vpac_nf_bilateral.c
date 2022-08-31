/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifdef BUILD_VPAC_NF

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>
#include "tivx_utils_checksum.h"
#include "test_hwa_common.h"

#define TEST_NUM_NODE_INSTANCE 2

/* #define TEST_NF_PERFORMANCE_LOGGING */

TESTCASE(tivxHwaVpacNfBilateral, CT_VXContext, ct_setup_vx_context, 0)

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char* target_string;

} SetTarget_Arg;

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF/TIVX_TARGET_VPAC2_NF", __VA_ARGS__, TIVX_TARGET_VPAC_NF, TIVX_TARGET_VPAC2_NF)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF", __VA_ARGS__, TIVX_TARGET_VPAC_NF, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_NF", __VA_ARGS__, TIVX_TARGET_VPAC2_NF, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF/TIVX_TARGET_VPAC_NF", __VA_ARGS__, TIVX_TARGET_VPAC_NF, TIVX_TARGET_VPAC_NF)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_NF/TIVX_TARGET_VPAC2_NF", __VA_ARGS__, TIVX_TARGET_VPAC2_NF, TIVX_TARGET_VPAC2_NF))
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_NF, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF/TIVX_TARGET_VPAC_NF", __VA_ARGS__, TIVX_TARGET_VPAC_NF, TIVX_TARGET_VPAC_NF))
#endif

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF", __VA_ARGS__, TIVX_TARGET_VPAC_NF)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_NF", __VA_ARGS__, TIVX_TARGET_VPAC2_NF))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_NF", __VA_ARGS__, TIVX_TARGET_VPAC_NF))
#endif

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacNfBilateral, testNodeCreation, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    tivx_vpac_nf_bilateral_params_t params;
    tivx_vpac_nf_bilateral_sigmas_t sigmas;
    vx_user_data_object param_obj;
    vx_user_data_object sigma_obj;
    vx_graph graph = 0;
    vx_node node = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        tivx_vpac_nf_bilateral_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_bilateral_params_t",
                                                            sizeof(tivx_vpac_nf_bilateral_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_nf_bilateral_sigmas_init(&sigmas);
        ASSERT_VX_OBJECT(sigma_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_bilateral_sigmas_t",
                                                            sizeof(tivx_vpac_nf_bilateral_sigmas_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfBilateralNode(graph, param_obj, src_image, sigma_obj, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&sigma_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(sigma_obj == 0);

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

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    double sigma_s, sigma_r;
    vx_int32 numTables;
    vx_int32 shift;
    vx_df_image dst_format;
    vx_border_t border;
    int width, height;
    char *target_string, *target_string_2;
} Arg;

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height);
    const char* fileName;
    double sigma_s, sigma_r;
    vx_int32 numTables;
    vx_int32 shift;
    vx_df_image dst_format;
    vx_border_t border;
    int width, height;
    int negative_test;
    int condition;
    char* target_string;
} ArgNegative;

static uint32_t nf_bilateral_checksums_ref[7*4] = {
    (uint32_t) 0xfba17165, (uint32_t) 0x98e85088, (uint32_t) 0xb912ec8e, (uint32_t) 0x9a9761de,
    (uint32_t) 0xe21b1231, (uint32_t) 0xe621c619, (uint32_t) 0xfe1fd400, (uint32_t) 0xcb17ed56,
    (uint32_t) 0xbf6d9f22, (uint32_t) 0x7ebd6360, (uint32_t) 0xa49d69ee, (uint32_t) 0xeb481747,
    (uint32_t) 0x2f3429bb, (uint32_t) 0xfe1fd3fe, (uint32_t) 0x20c52f33, (uint32_t) 0xc38e0a76,
    (uint32_t) 0x935d6a34, (uint32_t) 0x958e63e8, (uint32_t) 0x8b4e687b, (uint32_t) 0x7786a288,
    (uint32_t) 0xfe1fd3fb, (uint32_t) 0x8db682d9, (uint32_t) 0x4f60252,  (uint32_t) 0xfdcbe9d5,
    (uint32_t) 0x9e8b63d6, (uint32_t) 0x31bd5f38, (uint32_t) 0xef45f271, (uint32_t) 0xff09d3f0,
};

static uint32_t get_checksum(vx_int32 tables, vx_int32 shift)
{
    uint16_t a;
    uint16_t b;

    if (1 == tables)
    {
        a = 0U;
    }
    else if (2 == tables)
    {
        a = 1U;
    }
    else if (4 == tables)
    {
        a = 2U;
    }
    else
    {
        a = 3U;
    }

    if (0 == shift)
    {
        b = 0U;
    }
    else if (1 == shift)
    {
        b = 1U;
    }
    else if (2 == shift)
    {
        b = 2U;
    }
    else if (7 == shift)
    {
        b = 3U;
    }
    else if (-1 == shift)
    {
        b = 4U;
    }
    else if (-2 == shift)
    {
        b = 5U;
    }
    else
    {
        b = 6U;
    }

    return nf_bilateral_checksums_ref[(7U * a) + b];
}

#define ADD_SIGMAS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sigma_s=1.0/sigma_r=128.0", __VA_ARGS__, 1.0, 128.0))

#define ADD_NUMTABLES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/num_tables=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/num_tables=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/num_tables=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/num_tables=8", __VA_ARGS__, 8))

#define ADD_CONV_SHIFT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=7", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=-1", __VA_ARGS__, -1)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=-2", __VA_ARGS__, -2)), \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=-8", __VA_ARGS__, -8))

#define ADD_CONV_DST_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dst8U", __VA_ARGS__, VX_DF_IMAGE_U8))
#if 0
, \
    CT_EXPAND(nextmacro(testArgName "/dst16S", __VA_ARGS__, VX_DF_IMAGE_S16))
#endif

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("lena", ADD_SIGMAS, ADD_NUMTABLES, ADD_CONV_SHIFT, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG, convolve_read_image, "lena.bmp")

#define ADD_NUMTABLES_NEGATIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/num_tables=1", __VA_ARGS__, 1))

#define ADD_CONV_SHIFT_NEGATIVE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/conv_shift=0", __VA_ARGS__, 0))

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=input_interleaved", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_downshift", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_offset", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_pixel_skip", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=output_pixel_skip_odd", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=kern_ln_offset", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=kern_sz_height", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=src_ln_inc_2", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=adaptive_mode", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=sub_table_select", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=num_sigmas", __VA_ARGS__, 10))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIGMAS, ADD_NUMTABLES_NEGATIVE, ADD_CONV_SHIFT_NEGATIVE, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_64x64, ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ADD_SET_TARGET_PARAMETERS, ARG, convolve_generate_random, NULL)

TEST_WITH_ARG(tivxHwaVpacNfBilateral, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image[TEST_NUM_NODE_INSTANCE] = {0}, dst_image[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_nf_bilateral_params_t params;
    tivx_vpac_nf_bilateral_sigmas_t sigmas;
    vx_user_data_object param_obj[TEST_NUM_NODE_INSTANCE] = {0};
    vx_user_data_object sigma_obj[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};
    int i, j;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;

    CT_Image src = NULL;
    vx_border_t border = arg_->border;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(src_image[i] = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

                ASSERT_VX_OBJECT(dst_image[i] = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

                tivx_vpac_nf_bilateral_params_init(&params);
                ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_nf_bilateral_params_t",
                                                                    sizeof(tivx_vpac_nf_bilateral_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                tivx_vpac_nf_bilateral_sigmas_init(&sigmas);
                ASSERT_VX_OBJECT(sigma_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_nf_bilateral_sigmas_t",
                                                                    sizeof(tivx_vpac_nf_bilateral_sigmas_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                params.params.output_downshift = arg_->shift;

                sigmas.num_sigmas = arg_->numTables;
                for(j=0; j < arg_->numTables; j++)
                {
                    sigmas.sigma_space[j] = arg_->sigma_s + (0.2*j);
                    sigmas.sigma_range[j] = arg_->sigma_r + (20*j);
                }
                if(sigmas.num_sigmas > 1)
                {
                    params.adaptive_mode = 1;
                }

                VX_CALL(vxCopyUserDataObject(param_obj[i], 0, sizeof(tivx_vpac_nf_bilateral_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(vxCopyUserDataObject(sigma_obj[i], 0, sizeof(tivx_vpac_nf_bilateral_sigmas_t), &sigmas, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                ASSERT_VX_OBJECT(node[i] = tivxVpacNfBilateralNode(graph, param_obj[i], src_image[i], sigma_obj[i], dst_image[i]), VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                VX_CALL(vxSetNodeAttribute(node[i], VX_NODE_BORDER, &border, sizeof(border)));
            }
        }

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                checksum_expected = get_checksum(arg_->numTables, arg_->shift);
                checksum_actual = tivx_utils_simple_image_checksum(dst_image[i], 0, rect);
                ASSERT(checksum_expected == checksum_actual);
            }
        }

        if ((NULL != arg_->target_string) &&
            (NULL != arg_->target_string_2) )
        {
            vx_perf_t perf_node[TEST_NUM_NODE_INSTANCE], perf_graph;

            for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
            {
                vxQueryNode(node[i], VX_NODE_PERFORMANCE, &perf_node[i], sizeof(perf_node[i]));
            }
            vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

            if (strncmp(arg_->target_string, arg_->target_string_2, TIVX_TARGET_MAX_NAME) == 0)
            {
                #if defined(TEST_NF_PERFORMANCE_LOGGING)
                printf("targets are same\n");
                printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                #endif
                ASSERT(perf_graph.avg >= (perf_node[0].avg + perf_node[1].avg));
            }
            else
            {
                #if defined(TEST_NF_PERFORMANCE_LOGGING)
                printf("targets are different\n");
                printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                #endif
                ASSERT(perf_graph.avg < (perf_node[0].avg + perf_node[1].avg));
            }
        }

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&dst_image[i]));
                VX_CALL(vxReleaseImage(&src_image[i]));
                VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&sigma_obj[i]));
                ASSERT(node[i] == 0);
                ASSERT(dst_image[i] == 0);
                ASSERT(src_image[i] == 0);
                ASSERT(param_obj[i] == 0);
                ASSERT(sigma_obj[i] == 0);
            }
        }

        VX_CALL(vxReleaseGraph(&graph));
        ASSERT(graph == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacNfBilateral, testNegativeGraph, ArgNegative,
    PARAMETERS_NEGATIVE
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    tivx_vpac_nf_bilateral_params_t params;
    tivx_vpac_nf_bilateral_sigmas_t sigmas;
    vx_user_data_object param_obj;
    vx_user_data_object sigma_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    int i;

    CT_Image src = NULL;
    vx_border_t border = arg_->border;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_bilateral_params_t",
                                                            sizeof(tivx_vpac_nf_bilateral_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_nf_bilateral_params_init(&params);
        ASSERT_VX_OBJECT(sigma_obj = vxCreateUserDataObject(context, "tivx_vpac_nf_bilateral_sigmas_t",
                                                            sizeof(tivx_vpac_nf_bilateral_sigmas_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_nf_bilateral_sigmas_init(&sigmas);
        params.params.output_downshift = arg_->shift;

        sigmas.num_sigmas = arg_->numTables;
        for(i=0; i < arg_->numTables; i++)
        {
            sigmas.sigma_space[i] = arg_->sigma_s + (0.2*i);
            sigmas.sigma_range[i] = arg_->sigma_r + (20*i);
        }
        if(sigmas.num_sigmas > 1)
        {
            params.adaptive_mode = 1;
        }

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.params.input_interleaved = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.params.input_interleaved = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.params.input_interleaved = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.params.input_interleaved = 2;
                }
                else
                {
                    params.params.input_interleaved = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.params.output_downshift = -8;
                }
                else if (1U == arg_->condition)
                {
                    params.params.output_downshift = 7;
                }
                else if (2U == arg_->condition)
                {
                    params.params.output_downshift = -9;
                }
                else if (3U == arg_->condition)
                {
                    params.params.output_downshift = 8;
                }
                else
                {
                    params.params.output_downshift = 8;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.params.output_offset = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.params.output_offset = 4095;
                }
                else if (2U == arg_->condition)
                {
                    params.params.output_offset = 4096;
                }
                else if (3U == arg_->condition)
                {
                    params.params.output_offset = 4096;
                }
                else
                {
                    params.params.output_offset = 4096;
                }
                break;
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.params.output_pixel_skip = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.params.output_pixel_skip = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.params.output_pixel_skip = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.params.output_pixel_skip = 2;
                }
                else
                {
                    params.params.output_pixel_skip = 2;
                }
                break;
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.params.output_pixel_skip_odd = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.params.output_pixel_skip_odd = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.params.output_pixel_skip_odd = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.params.output_pixel_skip_odd = 2;
                }
                else
                {
                    params.params.output_pixel_skip_odd = 2;
                }
                break;
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    params.params.kern_ln_offset = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.params.kern_ln_offset = 4;
                }
                else if (2U == arg_->condition)
                {
                    params.params.kern_ln_offset = 5;
                }
                else if (3U == arg_->condition)
                {
                    params.params.kern_ln_offset = 5;
                }
                else
                {
                    params.params.kern_ln_offset = 5;
                }
                break;
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    params.params.kern_sz_height = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.params.kern_sz_height = 5;
                }
                else if (2U == arg_->condition)
                {
                    params.params.kern_sz_height = 0;
                }
                else if (3U == arg_->condition)
                {
                    params.params.kern_sz_height = 6;
                }
                else
                {
                    params.params.kern_sz_height = 6;
                }
                break;
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    params.params.src_ln_inc_2 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.params.src_ln_inc_2 = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.params.src_ln_inc_2 = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.params.src_ln_inc_2 = 2;
                }
                else
                {
                    params.params.src_ln_inc_2 = 2;
                }
                break;
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    params.adaptive_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.adaptive_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.adaptive_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.adaptive_mode = 2;
                }
                else
                {
                    params.adaptive_mode = 2;
                }
                break;
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    params.adaptive_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.adaptive_mode = 7;
                }
                else if (2U == arg_->condition)
                {
                    params.adaptive_mode = 8;
                }
                else if (3U == arg_->condition)
                {
                    params.adaptive_mode = 8;
                }
                else
                {
                    params.adaptive_mode = 8;
                }
                break;
            }
            case 10:
            {
                if (0U == arg_->condition)
                {
                    sigmas.num_sigmas = 1;
                }
                else if (1U == arg_->condition)
                {
                    sigmas.num_sigmas = 8;
                }
                else if (2U == arg_->condition)
                {
                    sigmas.num_sigmas = 0;
                }
                else if (3U == arg_->condition)
                {
                    sigmas.num_sigmas = 9;
                }
                else
                {
                    sigmas.num_sigmas = 5;
                }
                break;
            }
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_nf_bilateral_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(sigma_obj, 0, sizeof(tivx_vpac_nf_bilateral_sigmas_t), &sigmas, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfBilateralNode(graph, param_obj, src_image, sigma_obj, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        if(2 > arg_->condition)
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
        VX_CALL(vxReleaseUserDataObject(&sigma_obj));

        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(sigma_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVpacNfBilateral, testNodeCreation, testGraphProcessing, testNegativeGraph)

#endif /* BUILD_VPAC_NF */