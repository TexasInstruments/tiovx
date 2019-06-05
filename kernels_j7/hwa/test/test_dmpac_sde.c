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

#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/j7.h>
#include "test_engine/test.h"
#include <string.h>

TESTCASE(tivxHwaDmpacSde, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxHwaDmpacSde, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image left_image = 0, right_image = 0, dst_image = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_SDE))
    {
        tivxHwaLoadKernels(context);

        ASSERT_VX_OBJECT(left_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

        memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacSdeNode(graph, param_obj, left_image, right_image, dst_image, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_SDE));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&right_image));
        VX_CALL(vxReleaseImage(&left_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(dst_image == 0);
        ASSERT(right_image == 0);
        ASSERT(left_image == 0);
        ASSERT(param_obj == 0);

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
    uint32_t dispMin, dispMax;
    vx_int32 median;
    vx_int32 texture;
    uint32_t hist_output;
    vx_border_t border;
    int width, height;
} Arg;


#define ADD_DISPMIN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dispMin=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dispMin=-3", __VA_ARGS__, 1))

#define ADD_DISPMAX(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dispMax=min+63", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dispMax=min+127", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dispMax=min+191", __VA_ARGS__, 2))

#define ADD_MEDIAN(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/median=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/median=ON", __VA_ARGS__, 1))

#define ADD_TEXTURE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/texture=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/texture=ON", __VA_ARGS__, 1))

#define ADD_OUTPUT_HISTOGRAM(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/hist_output=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/hist_output=ON", __VA_ARGS__, 1))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_DISPMIN, ADD_DISPMAX, ADD_MEDIAN, ADD_TEXTURE, ADD_OUTPUT_HISTOGRAM, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_256x256, ARG, convolve_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_DISPMIN, ADD_DISPMAX, ADD_MEDIAN, ADD_TEXTURE, ADD_OUTPUT_HISTOGRAM, ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY, ADD_SIZE_NONE, ARG, convolve_read_image, "lena.bmp")

TEST_WITH_ARG(tivxHwaDmpacSde, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_image left_image = 0, right_image = 0, dst_image = 0;
    vx_distribution histogram = 0;
    tivx_dmpac_sde_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    int i;

    CT_Image srcL = NULL;
    CT_Image srcR = NULL;
    vx_border_t border = arg_->border;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_SDE))
    {
        tivxHwaLoadKernels(context);

        ASSERT_NO_FAILURE(srcL = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(srcR = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_VX_OBJECT(left_image = ct_image_to_vx_image(srcL, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(right_image = ct_image_to_vx_image(srcR, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_image = vxCreateImage(context, srcL->width, srcL->height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

        if(arg_->hist_output) {
            ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 128, 0, 4096), VX_TYPE_DISTRIBUTION);
        }

        memset(&params, 0, sizeof(tivx_dmpac_sde_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t",
                                                            sizeof(tivx_dmpac_sde_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.median_filter_enable = arg_->median;
        params.disparity_min = arg_->dispMin;
        params.disparity_max = arg_->dispMax;
        params.texture_filter_enable = arg_->texture;
        for(i = 0; i < 8; i++) {
            params.confidence_score_map[i] = i*8;
        }
        params.threshold_left_right = 0;
        params.threshold_texture = 0;
        params.aggregation_penalty_p1 = 0;
        params.aggregation_penalty_p2 = 0;
        params.reduced_range_search_enable = 0;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_sde_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDmpacSdeNode(graph, param_obj, left_image, right_image, dst_image, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DMPAC_SDE));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(node == 0);
        ASSERT(graph == 0);

        VX_CALL(vxReleaseImage(&dst_image));
        VX_CALL(vxReleaseImage(&right_image));
        VX_CALL(vxReleaseImage(&left_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        if(arg_->hist_output) {
            VX_CALL(vxReleaseDistribution(&histogram));
        }

        ASSERT(dst_image == 0);
        ASSERT(right_image == 0);
        ASSERT(left_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(histogram == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaDmpacSde, testNodeCreation, testGraphProcessing)
