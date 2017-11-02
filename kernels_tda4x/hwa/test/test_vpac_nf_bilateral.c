/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/tda4x.h>
#include "test_engine/test.h"
#include <string.h>


TESTCASE(tivxHwaVpacNfBilateral, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxHwaVpacNfBilateral, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    tivx_vpac_nf_bilateral_params_t params;
    tivx_vpac_nf_bilateral_sigmas_t sigmas;
    vx_enum params_type = VX_TYPE_INVALID;
    vx_enum sigmas_type = VX_TYPE_INVALID;
    vx_array param_array;
    vx_array sigma_array;
    vx_graph graph = 0;
    vx_node node = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_NF))
    {
        hwaLoadKernels(context);

        ASSERT_VX_OBJECT(src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        params_type = vxRegisterUserStruct(context, sizeof(tivx_vpac_nf_bilateral_params_t));
        ASSERT(params_type >= VX_TYPE_USER_STRUCT_START && params_type <= VX_TYPE_USER_STRUCT_END);
        memset(&params, 0, sizeof(tivx_vpac_nf_bilateral_params_t));
        ASSERT_VX_OBJECT(param_array = vxCreateArray(context, params_type, 1), VX_TYPE_ARRAY);

        sigmas_type = vxRegisterUserStruct(context, sizeof(tivx_vpac_nf_bilateral_sigmas_t));
        ASSERT(sigmas_type >= VX_TYPE_USER_STRUCT_START && sigmas_type <= VX_TYPE_USER_STRUCT_END);
        memset(&sigmas, 0, sizeof(tivx_vpac_nf_bilateral_sigmas_t));
        ASSERT_VX_OBJECT(sigma_array = vxCreateArray(context, sigmas_type, 1), VX_TYPE_ARRAY);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfBilateralNode(graph, param_array, src_image, sigma_array, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_NF));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseArray(&param_array));
        VX_CALL(vxReleaseArray(&sigma_array));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_array == 0);
        ASSERT(sigma_array == 0);

        hwaUnLoadKernels(context);
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
} Arg;


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
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIGMAS, ADD_NUMTABLES, ADD_CONV_SHIFT, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_64x64, ARG, convolve_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_SIGMAS, ADD_NUMTABLES, ADD_CONV_SHIFT, ADD_CONV_DST_FORMAT, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, convolve_read_image, "lena.bmp")

TEST_WITH_ARG(tivxHwaVpacNfBilateral, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    tivx_vpac_nf_bilateral_params_t params;
    tivx_vpac_nf_bilateral_sigmas_t sigmas;
    vx_enum params_type = VX_TYPE_INVALID;
    vx_enum sigmas_type = VX_TYPE_INVALID;
    vx_array param_array;
    vx_array sigma_array;
    vx_graph graph = 0;
    vx_node node = 0;
    int i;

    CT_Image src = NULL;
    vx_border_t border = arg_->border;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_NF))
    {
        hwaLoadKernels(context);

        ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, src->width, src->height, arg_->dst_format), VX_TYPE_IMAGE);

        params_type = vxRegisterUserStruct(context, sizeof(tivx_vpac_nf_bilateral_params_t));
        ASSERT(params_type >= VX_TYPE_USER_STRUCT_START && params_type <= VX_TYPE_USER_STRUCT_END);
        memset(&params, 0, sizeof(tivx_vpac_nf_bilateral_params_t));
        ASSERT_VX_OBJECT(param_array = vxCreateArray(context, params_type, 1), VX_TYPE_ARRAY);

        sigmas_type = vxRegisterUserStruct(context, sizeof(tivx_vpac_nf_bilateral_sigmas_t));
        ASSERT(sigmas_type >= VX_TYPE_USER_STRUCT_START && sigmas_type <= VX_TYPE_USER_STRUCT_END);
        memset(&sigmas, 0, sizeof(tivx_vpac_nf_bilateral_sigmas_t));
        ASSERT_VX_OBJECT(sigma_array = vxCreateArray(context, sigmas_type, 1), VX_TYPE_ARRAY);

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

        VX_CALL(vxAddArrayItems(param_array, 1, &params, sizeof(tivx_vpac_nf_bilateral_params_t)));
        VX_CALL(vxAddArrayItems(sigma_array, 1, &sigmas, sizeof(tivx_vpac_nf_bilateral_sigmas_t)));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacNfBilateralNode(graph, param_array, src_image, sigma_array, dst_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_NF));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&src_image));
        VX_CALL(vxReleaseArray(&param_array));
        VX_CALL(vxReleaseArray(&sigma_array));

        ASSERT(dst_image == 0);
        ASSERT(src_image == 0);
        ASSERT(param_array == 0);
        ASSERT(sigma_array == 0);

        hwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVpacNfBilateral, testNodeCreation, testGraphProcessing)
