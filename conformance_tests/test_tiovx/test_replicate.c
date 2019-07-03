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

TESTCASE(tivxReplicate, CT_VXContext, ct_setup_vx_context, 0)

/* test replicate node */

static CT_Image own_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

typedef enum
{
    ADD = 0,
    SUB,
    MUL

} OWN_OPERATION_TYPE;

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width;
    int height;
    vx_size levels;
    vx_float32 scale;
    vx_df_image format;
    OWN_OPERATION_TYPE op;
} Test_Replicate_Arg;

#define ADD_SIZE_SET(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=256x256", __VA_ARGS__, 256, 256)), \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ADD_VX_LEVELS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/REPLICAS=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/REPLICAS=4", __VA_ARGS__, 4))

#define ADD_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/PYRAMID:SCALE_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF)), \
    CT_EXPAND(nextmacro(testArgName "/PYRAMID:SCALE_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB)), \
    CT_EXPAND(nextmacro(testArgName "/OBJECT_ARRAY", __VA_ARGS__, -1))

#define ADD_VX_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8))

#define ADD_VX_OPERATION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/Add", __VA_ARGS__, ADD)),  \
    CT_EXPAND(nextmacro(testArgName "/Sub", __VA_ARGS__, SUB)),  \
    CT_EXPAND(nextmacro(testArgName "/Mul", __VA_ARGS__, MUL))

#define TEST_REPLICATE_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_SET, ADD_VX_LEVELS, ADD_VX_SCALE, ADD_VX_FORMAT, ADD_VX_OPERATION, ARG, own_generate_random, NULL)

static void ref_replicate_op(vx_context context, vx_reference input1, vx_reference input2, vx_reference input3, vx_reference output, OWN_OPERATION_TYPE op)
{
    vx_uint32 i, k;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    VX_CALL(vxQueryReference(input1, VX_REFERENCE_TYPE, &type, sizeof(type)));

    if (type == VX_TYPE_PYRAMID)
    {
        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size)));

        // add, sub, mul
        for (k = 0; k < levels; k++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            vx_image intermediate = 0;
            vx_image dst = 0;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_float32 scale_val = 1.0f;
            vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(intermediate = vxGetPyramidLevel((vx_pyramid)input3, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, k), VX_TYPE_IMAGE);

            switch (op)
            {
            case ADD:
                VX_CALL(vxuAdd(context, src1, src2, policy, intermediate));
                VX_CALL(vxuNot(context, intermediate, dst));
                break;

            case SUB:
                VX_CALL(vxuSubtract(context, src1, src2, policy, intermediate));
                VX_CALL(vxuNot(context, intermediate, dst));
                break;

            case MUL:
                VX_CALL(vxuMultiply(context, src1, src2, scale_val, policy, rounding, intermediate));
                VX_CALL(vxuNot(context, intermediate, dst));
                break;

            default:
                ASSERT(0);
            }

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
            VX_CALL(vxReleaseImage(&intermediate));
            VX_CALL(vxReleaseImage(&dst));
        }
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        VX_CALL(vxQueryObjectArray((vx_object_array)input1, VX_OBJECT_ARRAY_NUMITEMS, &levels, sizeof(vx_size)));

        // add, sub, mul
        for (k = 0; k < levels; k++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            vx_image intermediate = 0;
            vx_image dst = 0;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_float32 scale_val = 1.0f;
            vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;

            ASSERT_VX_OBJECT(src1 = (vx_image)vxGetObjectArrayItem((vx_object_array)input1, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = (vx_image)vxGetObjectArrayItem((vx_object_array)input2, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(intermediate = (vx_image)vxGetObjectArrayItem((vx_object_array)input3, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(dst = (vx_image)vxGetObjectArrayItem((vx_object_array)output, k), VX_TYPE_IMAGE);

            switch (op)
            {
            case ADD:
                VX_CALL(vxuAdd(context, src1, src2, policy, intermediate));
                VX_CALL(vxuNot(context, intermediate, dst));
                break;

            case SUB:
                VX_CALL(vxuSubtract(context, src1, src2, policy, intermediate));
                VX_CALL(vxuNot(context, intermediate, dst));
                break;

            case MUL:
                VX_CALL(vxuMultiply(context, src1, src2, scale_val, policy, rounding, intermediate));
                VX_CALL(vxuNot(context, intermediate, dst));
                break;

            default:
                ASSERT(0);
            }

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
            VX_CALL(vxReleaseImage(&intermediate));
            VX_CALL(vxReleaseImage(&dst));
        }
    }
    else
        ASSERT(0);

    return;
}

static void tst_replicate_op(vx_context context, vx_reference input1, vx_reference input2, vx_reference input3, vx_reference output, OWN_OPERATION_TYPE op, vx_float32 pyramid_scale)
{
    vx_graph graph = 0;
    vx_node node1, node2 = 0;
    vx_object_array object_array = 0;
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image intermediate = 0;
    vx_image dst = 0;
    vx_enum policy = VX_CONVERT_POLICY_SATURATE;
    vx_float32 scale_val = 1.0f;
    vx_scalar scale = 0;
    vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    VX_CALL(vxQueryReference(input1, VX_REFERENCE_TYPE, &type, sizeof(type)));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (type == VX_TYPE_PYRAMID)
    {
        ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intermediate = vxGetPyramidLevel((vx_pyramid)input3, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        // add, sub, mul
        switch (op)
        {
        case ADD:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node1 = vxAddNode(graph, src1, src2, policy, intermediate), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node1, replicate, 4));
            ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node2, replicate, 2));
            break;
        }

        case SUB:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node1 = vxSubtractNode(graph, src1, src2, policy, intermediate), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node1, replicate, 4));
            ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node2, replicate, 2));
            break;
        }

        case MUL:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(node1 = vxMultiplyNode(graph, src1, src2, scale, policy, rounding, intermediate), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node1, replicate, 6));
            ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node2, replicate, 2));
            VX_CALL(vxReleaseScalar(&scale));
            break;
        }

        default:
            ASSERT(0);
        }
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        ASSERT_VX_OBJECT(src1 = (vx_image)vxGetObjectArrayItem((vx_object_array)input1, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = (vx_image)vxGetObjectArrayItem((vx_object_array)input2, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intermediate = (vx_image)vxGetObjectArrayItem((vx_object_array)input3, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = (vx_image)vxGetObjectArrayItem((vx_object_array)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryObjectArray((vx_object_array)input1, VX_OBJECT_ARRAY_NUMITEMS, &levels, sizeof(levels)));

        // add, sub, mul
        switch (op)
        {
        case ADD:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node1 = vxAddNode(graph, src1, src2, policy, intermediate), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node1, replicate, 4));
            ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node2, replicate, 2));
            break;
        }

        case SUB:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node1 = vxSubtractNode(graph, src1, src2, policy, intermediate), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node1, replicate, 4));
            ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node2, replicate, 2));
            break;
        }

        case MUL:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(node1 = vxMultiplyNode(graph, src1, src2, scale, policy, rounding, intermediate), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node1, replicate, 6));
            ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intermediate, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node2, replicate, 2));
            VX_CALL(vxReleaseScalar(&scale));
            break;
        }

        default:
            ASSERT(0);
        }
    }
    else
        ASSERT(0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    if (intermediate)
        VX_CALL(vxReleaseImage(&intermediate));
    VX_CALL(vxReleaseImage(&dst));

    if (object_array)
        VX_CALL(vxReleaseObjectArray(&object_array));

    VX_CALL(vxReleaseNode(&node1));
    if (node2)
        VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    return;
}

static void check_replicas(vx_reference ref, vx_reference tst, vx_border_t border)
{
    vx_uint32 i;
    vx_size ref_levels = 0;
    vx_size tst_levels = 0;
    vx_enum type = VX_TYPE_INVALID;
    int roi_adj = 0;

    VX_CALL(vxQueryReference(ref, VX_REFERENCE_TYPE, &type, sizeof(type)));

    if (type == VX_TYPE_PYRAMID)
    {
        vx_float32 scale;
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_LEVELS, &ref_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)tst, VX_PYRAMID_LEVELS, &tst_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        EXPECT_EQ_INT(ref_levels, tst_levels);

        for (i = 0; i < ref_levels; i++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            CT_Image img1 = 0;
            CT_Image img2 = 0;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)ref, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)tst, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(img1 = ct_image_from_vx_image(src1));
            ASSERT_NO_FAILURE(img2 = ct_image_from_vx_image(src2));

            if (VX_BORDER_UNDEFINED == border.mode)
            {
                if (i > 0)
                {
                    int next_roi_adj = ceil((double)scale*(2+roi_adj));

                    if (next_roi_adj != roi_adj)
                    {
                        roi_adj = roi_adj + ceil((double)(scale) * 2);
                    }

                    ct_adjust_roi(img1, roi_adj, roi_adj, roi_adj, roi_adj);
                    ct_adjust_roi(img2, roi_adj, roi_adj, roi_adj, roi_adj);
                }
            }

            EXPECT_EQ_CTIMAGE(img1, img2);

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
        }
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        VX_CALL(vxQueryObjectArray((vx_object_array)ref, VX_OBJECT_ARRAY_NUMITEMS, &ref_levels, sizeof(vx_size)));
        VX_CALL(vxQueryObjectArray((vx_object_array)tst, VX_OBJECT_ARRAY_NUMITEMS, &tst_levels, sizeof(vx_size)));
        EXPECT_EQ_INT(ref_levels, tst_levels);

        for (i = 0; i < ref_levels; i++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            CT_Image img1 = 0;
            CT_Image img2 = 0;

            ASSERT_VX_OBJECT(src1 = (vx_image)vxGetObjectArrayItem((vx_object_array)ref, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = (vx_image)vxGetObjectArrayItem((vx_object_array)tst, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(img1 = ct_image_from_vx_image(src1));
            ASSERT_NO_FAILURE(img2 = ct_image_from_vx_image(src2));
            EXPECT_EQ_CTIMAGE(img1, img2);

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
        }
    }
    else
        ASSERT(0);

    return;
}

TEST_WITH_ARG(tivxReplicate, tivxReplicateNode, Test_Replicate_Arg, TEST_REPLICATE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    CT_Image src = 0;
    vx_reference src1 = 0;
    vx_reference src2 = 0;
    vx_reference src3 = 0;
    vx_reference ref = 0;
    vx_reference tst = 0;
    vx_image input1 = 0;
    vx_image input2 = 0;
    vx_pixel_value_t value = {{ 2 }};
    vx_border_t border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    if (arg_->scale < 0)
    {
        vx_size i;
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1 = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(src2 = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(src3 = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(ref = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(tst = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);

        /* Initialize array items */
        for (i = 0; i < arg_->levels; ++i)
        {
            vx_image item = 0;
            value.U8 = (vx_uint8)i;
            ASSERT_VX_OBJECT(input2 = vxCreateUniformImage(context, arg_->width, arg_->height, arg_->format, &value), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(item = (vx_image)vxGetObjectArrayItem((vx_object_array)src1, (vx_uint32)i), VX_TYPE_IMAGE);
            VX_CALL(vxuAdd(context, input1, input2, VX_CONVERT_POLICY_WRAP, item));
            VX_CALL(vxReleaseImage(&item));
            VX_CALL(vxReleaseImage(&input2));
        }

        VX_CALL(vxReleaseImage(&input1));
    }
    else
    {
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input2 = vxCreateUniformImage(context, arg_->width, arg_->height, arg_->format, &value), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1 = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(src2 = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(src3 = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(ref = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(tst = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);

        /* Initialize pyramids */
        VX_CALL(vxuGaussianPyramid(context, input1, (vx_pyramid)src1));
        VX_CALL(vxuGaussianPyramid(context, input2, (vx_pyramid)src2));
        VX_CALL(vxuGaussianPyramid(context, input2, (vx_pyramid)src3));

        VX_CALL(vxReleaseImage(&input1));
        VX_CALL(vxReleaseImage(&input2));
    }

    ref_replicate_op(context, src1, src2, src3, ref, arg_->op);
    tst_replicate_op(context, src1, src2, src3, tst, arg_->op, arg_->scale);

    VX_CALL(vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));
    check_replicas(ref, tst, border);

    VX_CALL(vxReleaseReference(&src1));
    VX_CALL(vxReleaseReference(&src2));
    VX_CALL(vxReleaseReference(&src3));
    VX_CALL(vxReleaseReference(&ref));
    VX_CALL(vxReleaseReference(&tst));
}

static void ref_replicate_op_2(vx_context context, vx_pyramid input1, vx_pyramid input2, vx_pyramid output, OWN_OPERATION_TYPE op)
{
    vx_uint32 i, k;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    {
        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size)));

        // add, sub, mul
        for (k = 0; k < levels; k++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            vx_image dst = 0;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_float32 scale_val = 1.0f;
            vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, k), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, k), VX_TYPE_IMAGE);

            switch (op)
            {
            case ADD:
                VX_CALL(vxuAdd(context, src1, src2, policy, dst));
                break;

            case SUB:
                VX_CALL(vxuSubtract(context, src1, src2, policy, dst));
                break;

            case MUL:
                VX_CALL(vxuMultiply(context, src1, src2, scale_val, policy, rounding, dst));
                break;

            default:
                ASSERT(0);
            }

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
            VX_CALL(vxReleaseImage(&dst));
        }
    }

    return;
}

static void tst_replicate_op_2(vx_context context, vx_image input1_img, vx_image input2_img, vx_pyramid input1, vx_pyramid input2, vx_pyramid output, OWN_OPERATION_TYPE op)
{
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0, node3 = 0;
    vx_object_array object_array = 0;
    vx_image src1 = 0;
    vx_image src2 = 0;
    vx_image dst = 0;
    vx_enum policy = VX_CONVERT_POLICY_SATURATE;
    vx_float32 scale_val = 1.0f;
    vx_scalar scale = 0;
    vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    {
        ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)input1, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)input2, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryPyramid((vx_pyramid)input1, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        ASSERT_VX_OBJECT(node1 = vxGaussianPyramidNode(graph, input1_img, input1), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxGaussianPyramidNode(graph, input2_img, input2), VX_TYPE_NODE);

        // add, sub, mul
        switch (op)
        {
        case ADD:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node3 = vxAddNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node3, replicate, 4));
            break;
        }

        case SUB:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(node3 = vxSubtractNode(graph, src1, src2, policy, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node3, replicate, 4));
            break;
        }

        case MUL:
        {
            vx_bool replicate[] = { vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e };
            ASSERT_VX_OBJECT(scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_val), VX_TYPE_SCALAR);
            ASSERT_VX_OBJECT(node3 = vxMultiplyNode(graph, src1, src2, scale, policy, rounding, dst), VX_TYPE_NODE);
            VX_CALL(vxReplicateNode(graph, node3, replicate, 6));
            VX_CALL(vxReleaseScalar(&scale));
            break;
        }

        default:
            ASSERT(0);
        }
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseImage(&src1));
    VX_CALL(vxReleaseImage(&src2));
    VX_CALL(vxReleaseImage(&dst));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseNode(&node3));
    VX_CALL(vxReleaseGraph(&graph));

    return;
}

static void check_replicas_2(vx_pyramid ref, vx_pyramid tst, vx_border_t border)
{
    vx_uint32 i;
    vx_size ref_levels = 0;
    vx_size tst_levels = 0;
    vx_enum type = VX_TYPE_INVALID;
    int roi_adj = 0;

    {
        vx_float32 scale;
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_LEVELS, &ref_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)tst, VX_PYRAMID_LEVELS, &tst_levels, sizeof(vx_size)));
        VX_CALL(vxQueryPyramid((vx_pyramid)ref, VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        EXPECT_EQ_INT(ref_levels, tst_levels);

        for (i = 0; i < ref_levels; i++)
        {
            vx_image src1 = 0;
            vx_image src2 = 0;
            CT_Image img1 = 0;
            CT_Image img2 = 0;

            ASSERT_VX_OBJECT(src1 = vxGetPyramidLevel((vx_pyramid)ref, i), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(src2 = vxGetPyramidLevel((vx_pyramid)tst, i), VX_TYPE_IMAGE);

            ASSERT_NO_FAILURE(img1 = ct_image_from_vx_image(src1));
            ASSERT_NO_FAILURE(img2 = ct_image_from_vx_image(src2));

            if (VX_BORDER_UNDEFINED == border.mode)
            {
                if (i > 0)
                {
                    int next_roi_adj = ceil((double)scale*(2+roi_adj));

                    if (next_roi_adj != roi_adj)
                    {
                        roi_adj = roi_adj + ceil((double)(scale) * 2);
                    }

                    ct_adjust_roi(img1, roi_adj, roi_adj, roi_adj, roi_adj);
                    ct_adjust_roi(img2, roi_adj, roi_adj, roi_adj, roi_adj);
                }
            }

            EXPECT_EQ_CTIMAGE(img1, img2);

            VX_CALL(vxReleaseImage(&src1));
            VX_CALL(vxReleaseImage(&src2));
        }
    }

    return;
}

TEST_WITH_ARG(tivxReplicate, tivxReplicateNode2, Test_Replicate_Arg, TEST_REPLICATE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    CT_Image src = 0;
    vx_pyramid src1 = 0, src1_0 = 0;
    vx_pyramid src2 = 0, src2_0 = 0;
    vx_pyramid ref = 0;
    vx_pyramid tst = 0;
    vx_image input1, input1_0 = 0;
    vx_image input2, input2_0 = 0;
    vx_pixel_value_t value = {{ 2 }};
    vx_border_t border;

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height));

    if ( (arg_->scale >= 0) && (VX_SCALE_PYRAMID_ORB != arg_->scale))
    {
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input1_0 = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input2 = vxCreateUniformImage(context, arg_->width, arg_->height, arg_->format, &value), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input2_0 = vxCreateUniformImage(context, arg_->width, arg_->height, arg_->format, &value), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src1_0 = vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(src2_0 = vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(ref = vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(tst = vxCreatePyramid(context, arg_->levels, arg_->scale, arg_->width, arg_->height, arg_->format), VX_TYPE_PYRAMID);

        tst_replicate_op_2(context, input1_0, input2_0, src1_0, src2_0, tst, arg_->op);
        ref_replicate_op_2(context, src1_0, src2_0, ref, arg_->op);

        VX_CALL(vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border)));
        check_replicas_2(ref, tst, border);

        VX_CALL(vxReleaseImage(&input1));
        VX_CALL(vxReleaseImage(&input2));
        VX_CALL(vxReleaseImage(&input1_0));
        VX_CALL(vxReleaseImage(&input2_0));

        VX_CALL(vxReleasePyramid(&src1_0));
        VX_CALL(vxReleasePyramid(&src2_0));
        VX_CALL(vxReleasePyramid(&ref));
        VX_CALL(vxReleasePyramid(&tst));
    }
}

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"

typedef enum _own_params_e
{
    OWN_PARAM_INPUT = 0,
    OWN_PARAM_OUTPUT,
} own_params_e;

static enum vx_type_e type = VX_TYPE_IMAGE;
static enum vx_type_e objarray_itemtype = VX_TYPE_INVALID;

static vx_size local_size = 0;
static vx_bool is_kernel_alloc = vx_false_e;
static vx_size local_size_auto_alloc = 0;
static vx_size local_size_kernel_alloc = 0;

static vx_status set_local_size_status_init = VX_SUCCESS;
static vx_status set_local_ptr_status_init = VX_SUCCESS;

static vx_status query_local_size_status_deinit = VX_SUCCESS;
static vx_status query_local_ptr_status_deinit = VX_SUCCESS;
static vx_status set_local_size_status_deinit = VX_SUCCESS;
static vx_status set_local_ptr_status_deinit = VX_SUCCESS;

static vx_status VX_CALLBACK own_set_image_valid_rect(
    vx_node node,
    vx_uint32 index,
    const vx_rectangle_t* const input_valid[],
    vx_rectangle_t* const output_valid[])
{
    vx_status status = VX_FAILURE;

    if (index == OWN_PARAM_OUTPUT)
    {
        output_valid[0]->start_x = input_valid[0]->start_x + 2;
        output_valid[0]->start_y = input_valid[0]->start_y + 2;
        output_valid[0]->end_x   = input_valid[0]->end_x - 2;
        output_valid[0]->end_y   = input_valid[0]->end_y - 2;

        status = VX_SUCCESS;
    }

    return status;
}

static vx_bool is_validator_called = vx_false_e;
static vx_status VX_CALLBACK own_ValidatorMetaFromRef(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT];
    ASSERT_VX_OBJECT_(return VX_FAILURE, input, type);
    vx_reference output = parameters[OWN_PARAM_OUTPUT];
    ASSERT_VX_OBJECT_(return VX_FAILURE, output, type);

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT];

    vx_enum in_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(input, VX_REFERENCE_TYPE, &in_ref_type, sizeof(vx_enum)));
    vx_enum out_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(output, VX_REFERENCE_TYPE, &out_ref_type, sizeof(vx_enum)));

    if (in_ref_type == out_ref_type)
    {
        vx_enum format = VX_DF_IMAGE_U8;
        vx_uint32 src_width = 128, src_height = 128;
        vx_uint32 dst_width = 256, dst_height = 256;
        vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
        vx_size capacity = 20;
        vx_size bins = 36;
        vx_int32 offset = 0;
        vx_uint32 range = 360;
        vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
        vx_size num_items = 100;
        vx_size m = 5, n = 5;

        vx_enum actual_format = VX_TYPE_INVALID;
        vx_uint32 actual_src_width = 128, actual_src_height = 128;
        vx_uint32 actual_dst_width = 256, actual_dst_height = 256;
        vx_enum actual_item_type = VX_TYPE_INVALID;
        vx_size actual_capacity = 0;
        vx_size actual_levels = 0;
        vx_float32 actual_scale = 0;
        vx_size actual_bins = 0;
        vx_int32 actual_offset = -1;
        vx_uint32 actual_range = 0;
        vx_enum actual_thresh_type = VX_TYPE_INVALID;
        vx_size actual_num_items = 0;
        vx_size actual_m = 0, actual_n = 0;
        switch (type)
        {
        case VX_TYPE_IMAGE:
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

            if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
                vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
            break;
        }

    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK own_ValidatorMetaFromAttr(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT];

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT];

    vx_enum format = VX_DF_IMAGE_U8;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;
    vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_size levels = 8;
    vx_float32 scale = 0.5f;
    vx_size bins = 36;
    vx_int32 offset = 0;
    vx_uint32 range = 360;
    vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
    vx_size num_items = 100;
    vx_size m = 5, n = 5;

    vx_enum actual_format = VX_TYPE_INVALID;
    vx_uint32 actual_src_width = 128, actual_src_height = 128;
    vx_uint32 actual_dst_width = 256, actual_dst_height = 256;
    vx_enum actual_item_type = VX_TYPE_INVALID;
    vx_size actual_capacity = 0;
    vx_size actual_levels = 0;
    vx_float32 actual_scale = 0;
    vx_size actual_bins = 0;
    vx_int32 actual_offset = -1;
    vx_uint32 actual_range = 0;
    vx_enum actual_thresh_type = VX_TYPE_INVALID;
    vx_size actual_num_items = 0;
    vx_size actual_m = 0, actual_n = 0;
    switch (type)
    {
    case VX_TYPE_IMAGE:
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_FORMAT, &actual_format, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, vxQueryImage((vx_image)input, VX_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));

        if (format == actual_format && src_width == actual_src_width && src_height == actual_src_height)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_FORMAT, &format, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_WIDTH, &src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_IMAGE_HEIGHT, &src_height, sizeof(vx_uint32)));
            vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    default:
        return VX_ERROR_INVALID_PARAMETERS;
        break;
    }

    return VX_SUCCESS;
}

static vx_bool is_kernel_called = vx_false_e;
static vx_status VX_CALLBACK own_Kernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    is_kernel_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }

    return VX_SUCCESS;
}

static vx_bool is_initialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void* ptr = NULL;
    is_initialize_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    if (local_size_kernel_alloc > 0)
    {
        size = local_size_kernel_alloc;
        ptr = ct_calloc(1, local_size_kernel_alloc);
    }
    return VX_SUCCESS;
}

static vx_bool is_deinitialize_called = vx_false_e;
static vx_status VX_CALLBACK own_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_size size = 0;
    void* ptr = NULL;
    is_deinitialize_called = vx_true_e;
    EXPECT(node != 0);
    EXPECT(parameters != NULL);
    EXPECT(num == 2);
    if (parameters != NULL && num == 2)
    {
        EXPECT_VX_OBJECT(parameters[0], type);
        EXPECT_VX_OBJECT(parameters[1], type);
    }
    query_local_size_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    query_local_ptr_status_deinit = vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    if (local_size_kernel_alloc > 0)
    {
        size = 0;
        if (ptr != NULL)
        {
            ct_free_mem(ptr);
            ptr = NULL;
        }
    }
    return VX_SUCCESS;
}

static void own_register_kernel(vx_context context)
{
    vx_kernel kernel = 0;
    vx_size size = local_size_auto_alloc;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
        VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
        own_Kernel,
        2,
        own_ValidatorMetaFromRef,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_INPUT, VX_INPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_INPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_INPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxAddParameterToKernel(kernel, OWN_PARAM_OUTPUT, VX_OUTPUT, type, VX_PARAMETER_STATE_REQUIRED));
    {
        vx_parameter parameter = 0;
        vx_enum direction = 0;
        ASSERT_VX_OBJECT(parameter = vxGetKernelParameterByIndex(kernel, OWN_PARAM_OUTPUT), VX_TYPE_PARAMETER);
        VX_CALL(vxQueryParameter(parameter, VX_PARAMETER_DIRECTION, &direction, sizeof(direction)));
        ASSERT(direction == VX_OUTPUT);
        VX_CALL(vxReleaseParameter(&parameter));
    }
    VX_CALL(vxSetKernelAttribute(kernel, VX_KERNEL_LOCAL_DATA_SIZE, &size, sizeof(size)));
    VX_CALL(vxFinalizeKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));
}


static void tst_userkernel_replicate_op(vx_context context, vx_reference input, vx_reference output, vx_kernel user_kernel, vx_float32 pyramid_scale)
{
    vx_graph graph = 0;
    vx_node node;
    vx_object_array object_array = 0;
    vx_image src = 0;
    vx_image dst = 0;
    vx_enum policy = VX_CONVERT_POLICY_SATURATE;
    vx_float32 scale_val = 1.0f;
    vx_scalar scale = 0;
    vx_enum rounding = VX_ROUND_POLICY_TO_ZERO;
    vx_size levels = 0;
    vx_enum type = VX_TYPE_INVALID;

    VX_CALL(vxQueryReference(input, VX_REFERENCE_TYPE, &type, sizeof(type)));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    if (type == VX_TYPE_PYRAMID)
    {
        ASSERT_VX_OBJECT(src = vxGetPyramidLevel((vx_pyramid)input, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = vxGetPyramidLevel((vx_pyramid)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryPyramid((vx_pyramid)input, VX_PYRAMID_LEVELS, &levels, sizeof(levels)));

        vx_bool replicate[] = { vx_true_e, vx_true_e };
        ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

        VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
        VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));
        VX_CALL(vxReplicateNode(graph, node, replicate, 2));
    }
    else if (type == VX_TYPE_OBJECT_ARRAY)
    {
        ASSERT_VX_OBJECT(src = (vx_image)vxGetObjectArrayItem((vx_object_array)input, 0), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst = (vx_image)vxGetObjectArrayItem((vx_object_array)output, 0), VX_TYPE_IMAGE);

        VX_CALL(vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_NUMITEMS, &levels, sizeof(levels)));

        vx_bool replicate[] = { vx_true_e, vx_true_e };
        ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

        VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
        VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));
        VX_CALL(vxReplicateNode(graph, node, replicate, 2));
    }
    else
        ASSERT(0);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));

    if (object_array)
        VX_CALL(vxReleaseObjectArray(&object_array));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    return;
}

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width;
    int height;
    vx_size levels;
    vx_float32 scale;
    vx_df_image format;
} Test_UserNode_Arg;

#define ADD_SIZE_SET(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=256x256", __VA_ARGS__, 256, 256)), \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define ADD_VX_LEVELS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/REPLICAS=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/REPLICAS=4", __VA_ARGS__, 4))

#define ADD_VX_SCALE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/PYRAMID:SCALE_HALF", __VA_ARGS__, VX_SCALE_PYRAMID_HALF)), \
    CT_EXPAND(nextmacro(testArgName "/PYRAMID:SCALE_ORB", __VA_ARGS__, VX_SCALE_PYRAMID_ORB)), \
    CT_EXPAND(nextmacro(testArgName "/OBJECT_ARRAY", __VA_ARGS__, -1))

#define ADD_VX_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8))

#define TEST_USERNODE_PARAMETERS \
    CT_GENERATE_PARAMETERS("random", ADD_SIZE_SET, ADD_VX_LEVELS, ADD_VX_SCALE, ADD_VX_FORMAT, ARG, own_generate_random, NULL)

TEST_WITH_ARG(tivxReplicate, tivxReplicateUserNode, Test_UserNode_Arg, TEST_USERNODE_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    CT_Image src_ctimage = 0, dst_ctimage = 0;
    vx_reference tst = 0;
    vx_image input1 = 0, input2 = 0;
    vx_pixel_value_t value = {{ 2 }};
    vx_border_t border;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 dst_width = 256, dst_height = 256;

    ASSERT_NO_FAILURE(src_ctimage = arg_->generator(arg_->fileName, src_width, src_height));
    ASSERT_NO_FAILURE(dst_ctimage = arg_->generator(arg_->fileName, dst_width, dst_height));

    vx_reference src = 0;
    vx_kernel user_kernel = 0;

    vx_enum format = VX_DF_IMAGE_U8;

    int phase = 0;

    local_size = 10;
    is_kernel_alloc = vx_false_e;

    if (is_kernel_alloc == vx_false_e)
    {
        local_size_auto_alloc = local_size;
        local_size_kernel_alloc = 0;
    }
    else
    {
        local_size_auto_alloc = 0;
        local_size_kernel_alloc = local_size;
    }

    is_validator_called = vx_false_e;
    is_kernel_called = vx_false_e;
    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;

    if (arg_->scale < 0)
    {
        vx_size i;
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src_ctimage, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input2 = ct_image_to_vx_image(dst_ctimage, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src = (vx_reference)vxCreateObjectArray(context, (vx_reference)input1, arg_->levels), VX_TYPE_OBJECT_ARRAY);
        ASSERT_VX_OBJECT(tst = (vx_reference)vxCreateObjectArray(context, (vx_reference)input2, arg_->levels), VX_TYPE_OBJECT_ARRAY);

        VX_CALL(vxReleaseImage(&input1));
        VX_CALL(vxReleaseImage(&input2));
    }
    else
    {
        ASSERT_VX_OBJECT(input1 = ct_image_to_vx_image(src_ctimage, context), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(src = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, src_width, src_height, arg_->format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(tst = (vx_reference)vxCreatePyramid(context, arg_->levels, arg_->scale, dst_width, dst_height, arg_->format), VX_TYPE_PYRAMID);

        VX_CALL(vxuGaussianPyramid(context, input1, (vx_pyramid)src));

        VX_CALL(vxReleaseImage(&input1));
    }

    ASSERT_NO_FAILURE(own_register_kernel(context));

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);

    tst_userkernel_replicate_op(context, src, tst, user_kernel, arg_->scale);

    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&tst));
    VX_CALL(vxReleaseReference(&src));

    ASSERT(tst == 0);
    ASSERT(src == 0);
}

static void channel_combine_fill_chanel(CT_Image src, vx_enum channel, CT_Image dst)
{
    uint8_t *dst_base = NULL;
    int x, y;

    int plane, component;

    int x_subsampling = ct_image_get_channel_subsampling_x(dst, channel);
    int y_subsampling = ct_image_get_channel_subsampling_y(dst, channel);

    int xstep = ct_image_get_channel_step_x(dst, channel);
    int ystep = ct_image_get_channel_step_y(dst, channel);

    int src_width = dst->width / x_subsampling;
    int src_height = dst->height / y_subsampling;

    // Check that src was subsampled (by spec)
    ASSERT_EQ_INT(src_width, src->width);
    ASSERT_EQ_INT(src_height, src->height);

    ASSERT_NO_FAILURE(plane = ct_image_get_channel_plane(dst, channel));
    ASSERT_NO_FAILURE(component = ct_image_get_channel_component(dst, channel));

    ASSERT(dst_base = ct_image_get_plane_base(dst, plane));

    for (y = 0; y < src_height; y++)
    {
        for (x = 0; x < src_width; x++)
        {
            uint8_t *src_data = CT_IMAGE_DATA_PTR_8U(src, x, y);
            uint8_t *dst_data = dst_base + (x * xstep) + (y * ystep);
            dst_data[component] = *src_data;
        }
    }

    return;
}

static CT_Image channel_combine_create_reference_image(CT_Image src1, CT_Image src2, CT_Image src3, CT_Image src4, vx_df_image format)
{
    CT_Image dst = NULL;

    ASSERT_(return NULL, src1);
    ASSERT_NO_FAILURE_(return NULL, dst = ct_allocate_image(src1->width, src1->height, format));

    switch (format)
    {
        case VX_DF_IMAGE_RGB:
        case VX_DF_IMAGE_RGBX:
            ASSERT_(return NULL, src1);
            ASSERT_(return NULL, src2);
            ASSERT_(return NULL, src3);
            if (format == VX_DF_IMAGE_RGB)
                ASSERT_(return NULL, src4 == NULL);
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src1, VX_CHANNEL_R, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src2, VX_CHANNEL_G, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src3, VX_CHANNEL_B, dst));
            if (format == VX_DF_IMAGE_RGBX)
                ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src4, VX_CHANNEL_A, dst));
            return dst;
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_IYUV:
        case VX_DF_IMAGE_YUV4:
            ASSERT_(return NULL, src1);
            ASSERT_(return NULL, src2);
            ASSERT_(return NULL, src3);
            ASSERT_(return NULL, src4 == NULL);
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src1, VX_CHANNEL_Y, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src2, VX_CHANNEL_U, dst));
            ASSERT_NO_FAILURE_(return NULL, channel_combine_fill_chanel(src3, VX_CHANNEL_V, dst));
            return dst;
    }

    CT_FAIL_(return NULL, "Not supported");
}

static void channel_combine_check(CT_Image src1, CT_Image src2, CT_Image src3, CT_Image src4, CT_Image dst)
{
    CT_Image dst_ref = NULL;

    ASSERT_NO_FAILURE(dst_ref = channel_combine_create_reference_image(src1, src2, src3, src4, dst->format));

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
}

TEST(tivxReplicate, tivxReplicateNodeOptionalInputChannelCombine)
{
    vx_context context = context_->vx_context_;
    vx_image src1_image_exemplar, src1_image[2] = {0, 0}, src2_image[2] = {0, 0}, src3_image[2] = {0, 0};
    vx_image dst_image_exemplar, dst_image[2] = {0, 0};
    vx_object_array src_image_array[3] = {0, 0, 0};
    vx_object_array dst_image_array = 0;
    vx_graph graph1 = 0;
    vx_node node1 = 0;
    vx_int32 width = 320;
    vx_int32 height = 240;

    int channels = 0, i;
    CT_Image src1[2] = {NULL, NULL};
    CT_Image src2[2] = {NULL, NULL};
    CT_Image src3[2] = {NULL, NULL};
    CT_Image dst[2] = {NULL, NULL}, dummy = NULL;
    vx_enum channel_ref;

    ASSERT_VX_OBJECT(src1_image_exemplar = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image_exemplar = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_image_array[0] = vxCreateObjectArray(context, (vx_reference)src1_image_exemplar, 2), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(src_image_array[1] = vxCreateObjectArray(context, (vx_reference)src1_image_exemplar, 2), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(src_image_array[2] = vxCreateObjectArray(context, (vx_reference)src1_image_exemplar, 2), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(dst_image_array = vxCreateObjectArray(context,(vx_reference) dst_image_exemplar, 2), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseImage(&src1_image_exemplar));
    VX_CALL(vxReleaseImage(&dst_image_exemplar));

    graph1 = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph1, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src1_image[0] = (vx_image)vxGetObjectArrayItem(src_image_array[0], 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_image[0] = (vx_image)vxGetObjectArrayItem(src_image_array[1], 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3_image[0] = (vx_image)vxGetObjectArrayItem(src_image_array[2], 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[0] = (vx_image)vxGetObjectArrayItem(dst_image_array, 0), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1_image[1] = (vx_image)vxGetObjectArrayItem(src_image_array[0], 1), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src2_image[1] = (vx_image)vxGetObjectArrayItem(src_image_array[1], 1), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(src3_image[1] = (vx_image)vxGetObjectArrayItem(src_image_array[2], 1), VX_TYPE_IMAGE);

    node1 = vxChannelCombineNode(graph1, src1_image[0], src2_image[0], src3_image[0], NULL, dst_image[0]);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    vx_bool replicate[] = { vx_true_e, vx_true_e, vx_true_e, vx_false_e, vx_true_e };
    VX_CALL(vxReplicateNode(graph1, node1, replicate, 5));


    ASSERT_NO_FAILURE(dummy = ct_allocate_image(4, 4, VX_DF_IMAGE_RGB));
    ASSERT_NO_FAILURE(channels = ct_get_num_channels(VX_DF_IMAGE_RGB));
    channel_ref = VX_CHANNEL_Y;

    int w, h;

    w = width / ct_image_get_channel_subsampling_x(dummy, channel_ref + 0);
    h = height / ct_image_get_channel_subsampling_y(dummy, channel_ref + 0);
    ASSERT_NO_FAILURE(src1[0] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src1_image[0], src1[0]));
    ASSERT_NO_FAILURE(src1[1] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src1_image[1], src1[1]));

    w = width / ct_image_get_channel_subsampling_x(dummy, channel_ref + 1);
    h = height / ct_image_get_channel_subsampling_y(dummy, channel_ref + 1);
    ASSERT_NO_FAILURE(src2[0] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src2_image[0], src2[0]));
    ASSERT_NO_FAILURE(src2[1] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src2_image[1], src2[1]));

    w = width / ct_image_get_channel_subsampling_x(dummy, channel_ref + 2);
    h = height / ct_image_get_channel_subsampling_y(dummy, channel_ref + 2);
    ASSERT_NO_FAILURE(src3[0] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src3_image[0], src3[0]));
    ASSERT_NO_FAILURE(src3[1] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src3_image[1], src3[1]));

    for (i = 0; i < 2; i++)
    {
        VX_CALL(vxReleaseImage(&src1_image[i]));
        VX_CALL(vxReleaseImage(&src2_image[i]));
        VX_CALL(vxReleaseImage(&src3_image[i]));
    }
    VX_CALL(vxReleaseImage(&dst_image[0]));

    VX_CALL(vxVerifyGraph(graph1));
    VX_CALL(vxProcessGraph(graph1));

    ASSERT_VX_OBJECT(dst_image[0] = (vx_image)vxGetObjectArrayItem(dst_image_array, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[1] = (vx_image)vxGetObjectArrayItem(dst_image_array, 1), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(dst[0] = ct_image_from_vx_image(dst_image[0]));
    ASSERT_NO_FAILURE(dst[1] = ct_image_from_vx_image(dst_image[1]));
    VX_CALL(vxReleaseImage(&dst_image[0]));
    VX_CALL(vxReleaseImage(&dst_image[1]));

    ASSERT_NO_FAILURE(channel_combine_check(src1[0], src2[0], src3[0], NULL, dst[0]));
    ASSERT_NO_FAILURE(channel_combine_check(src1[1], src2[1], src3[1], NULL, dst[1]));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node1 == 0);
    ASSERT(graph1 == 0);

    for (i = 0; i < 3; i++)
    {
        VX_CALL(vxReleaseObjectArray(&src_image_array[i]));
        ASSERT(src_image_array[i] == 0);
    }
    VX_CALL(vxReleaseObjectArray(&dst_image_array));
    ASSERT(dst_image_array == 0);
}

static void sobel3x3_check_y(CT_Image src, CT_Image dst_y, vx_border_t border)
{
    CT_Image dst_x_ref = NULL, dst_y_ref = NULL;

    ASSERT(src && dst_y);

    ASSERT_NO_FAILURE(tivx_sobel3x3_create_reference_image(src, border, &dst_x_ref, &dst_y_ref));

    ASSERT_NO_FAILURE(
        if (border.mode == VX_BORDER_UNDEFINED)
        {
            ct_adjust_roi(dst_y,  1, 1, 1, 1);
            ct_adjust_roi(dst_y_ref, 1, 1, 1, 1);
        }
    );

    EXPECT_EQ_CTIMAGE(dst_y_ref, dst_y);
}

TEST(tivxReplicate, tivxReplicateNodeOptionalOutputSobel)
{
    vx_context context = context_->vx_context_;
    vx_image src1_image_exemplar, src1_image[2] = {0, 0};
    vx_image dst_image_exemplar, dst_image[2] = {0, 0};
    vx_object_array src_image_array = 0;
    vx_object_array dst_image_array = 0;
    vx_graph graph1 = 0;
    vx_node node1 = 0;
    vx_int32 width = 320;
    vx_int32 height = 240;
    vx_border_t border = { VX_BORDER_UNDEFINED, {{ 0 }} };

    int channels = 0, i;
    CT_Image src1[2] = {NULL, NULL};
    CT_Image dst[2] = {NULL, NULL}, dummy = NULL;
    vx_enum channel_ref;

    ASSERT_VX_OBJECT(src1_image_exemplar = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image_exemplar = vxCreateImage(context, width, height, VX_DF_IMAGE_S16), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src_image_array = vxCreateObjectArray(context, (vx_reference)src1_image_exemplar, 2), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(dst_image_array = vxCreateObjectArray(context,(vx_reference) dst_image_exemplar, 2), VX_TYPE_OBJECT_ARRAY);

    VX_CALL(vxReleaseImage(&src1_image_exemplar));
    VX_CALL(vxReleaseImage(&dst_image_exemplar));

    graph1 = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph1, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(src1_image[0] = (vx_image)vxGetObjectArrayItem(src_image_array, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[0] = (vx_image)vxGetObjectArrayItem(dst_image_array, 0), VX_TYPE_IMAGE);

    ASSERT_VX_OBJECT(src1_image[1] = (vx_image)vxGetObjectArrayItem(src_image_array, 1), VX_TYPE_IMAGE);

    node1 = vxSobel3x3Node(graph1, src1_image[0], NULL, dst_image[0]);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
    VX_CALL(vxSetNodeAttribute(node1, VX_NODE_BORDER, &border, sizeof(border)));

    vx_bool replicate[] = { vx_true_e, vx_false_e, vx_true_e };
    VX_CALL(vxReplicateNode(graph1, node1, replicate, 3));

    int w = width, h = height;

    ASSERT_NO_FAILURE(src1[0] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src1_image[0], src1[0]));
    ASSERT_NO_FAILURE(src1[1] = own_generate_random(NULL, w, h));
    ASSERT_NO_FAILURE(ct_image_copyto_vx_image(src1_image[1], src1[1]));

    for (i = 0; i < 2; i++)
    {
        VX_CALL(vxReleaseImage(&src1_image[i]));
    }
    VX_CALL(vxReleaseImage(&dst_image[0]));

    VX_CALL(vxVerifyGraph(graph1));
    VX_CALL(vxProcessGraph(graph1));

    ASSERT_VX_OBJECT(dst_image[0] = (vx_image)vxGetObjectArrayItem(dst_image_array, 0), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(dst_image[1] = (vx_image)vxGetObjectArrayItem(dst_image_array, 1), VX_TYPE_IMAGE);
    ASSERT_NO_FAILURE(dst[0] = ct_image_from_vx_image(dst_image[0]));
    ASSERT_NO_FAILURE(dst[1] = ct_image_from_vx_image(dst_image[1]));
    VX_CALL(vxReleaseImage(&dst_image[0]));
    VX_CALL(vxReleaseImage(&dst_image[1]));

    ASSERT_NO_FAILURE(sobel3x3_check_y(src1[0], dst[0], border));
    ASSERT_NO_FAILURE(sobel3x3_check_y(src1[1], dst[1], border));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseGraph(&graph1));

    ASSERT(node1 == 0);
    ASSERT(graph1 == 0);

    VX_CALL(vxReleaseObjectArray(&src_image_array));
    ASSERT(src_image_array == 0);
    VX_CALL(vxReleaseObjectArray(&dst_image_array));
    ASSERT(dst_image_array == 0);
}

TESTCASE_TESTS(tivxReplicate,
        tivxReplicateNode,
        tivxReplicateNode2,
        tivxReplicateUserNode,
        tivxReplicateNodeOptionalInputChannelCombine,
        tivxReplicateNodeOptionalOutputSobel
)
