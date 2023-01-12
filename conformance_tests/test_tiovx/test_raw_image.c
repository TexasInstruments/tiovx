/*

 * Copyright (c) 2012-2018 The Khronos Group Inc.
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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx_ext_raw_image.h>

#define VX_KERNEL_CONFORMANCE_TEST_OWN_BAD (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 0)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME "org.khronos.openvx.test.own_bad"

#define VX_KERNEL_CONFORMANCE_TEST_OWN (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 1)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_NAME "org.khronos.openvx.test.own"

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"

TESTCASE(tivxRawImage, CT_VXContext, ct_setup_vx_context, 0)

typedef enum _own_params_e
{
    OWN_PARAM_INPUT = 0,
    OWN_PARAM_OUTPUT,
} own_params_e;

static uint32_t type = VX_TYPE_INVALID;
static uint32_t objarray_itemtype = VX_TYPE_INVALID;

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

#if 0
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
#endif
static vx_bool is_validator_called = vx_false_e;
static vx_status VX_CALLBACK own_ValidatorMetaFromRef(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    is_validator_called = vx_true_e;
    ASSERT_VX_OBJECT_(return VX_FAILURE, node, VX_TYPE_NODE);

    vx_reference input = parameters[OWN_PARAM_INPUT];
    ASSERT_VX_OBJECT_(return VX_FAILURE, input, (enum vx_type_e)type);
    vx_reference output = parameters[OWN_PARAM_OUTPUT];
    ASSERT_VX_OBJECT_(return VX_FAILURE, output, (enum vx_type_e)type);

    vx_meta_format meta = metas[OWN_PARAM_OUTPUT];

    vx_enum in_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(input, VX_REFERENCE_TYPE, &in_ref_type, sizeof(vx_enum)));
    vx_enum out_ref_type = VX_TYPE_INVALID;
    VX_CALL_(return VX_ERROR_INVALID_PARAMETERS, vxQueryReference(output, VX_REFERENCE_TYPE, &out_ref_type, sizeof(vx_enum)));

    if (in_ref_type == out_ref_type)
    {
        vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
        vx_size capacity = 20;
        vx_uint32 src_width = 128, src_height = 128;
        vx_uint32 num_exposures = 3;
        vx_bool line_interleaved = vx_false_e;
        vx_uint32 meta_height_before = 5, meta_height_after = 0;
        tivx_raw_image_format_t format[3];
        vx_imagepatch_addressing_t image_addr[3];

        vx_enum actual_item_type = VX_TYPE_INVALID;
        vx_size actual_capacity = 0;
        vx_uint32 actual_src_width = 0, actual_src_height = 0;
        vx_uint32 actual_num_exposures = 0;
        vx_bool actual_line_interleaved = vx_true_e;
        vx_uint32 actual_meta_height_before = 0, actual_meta_height_after = 0;
        tivx_raw_image_format_t actual_format[3];
        vx_imagepatch_addressing_t actual_image_addr[3];

        format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        format[0].msb = 12;
        format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
        format[1].msb = 7;
        format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
        format[2].msb = 11;

        image_addr[0].dim_x = 128;
        image_addr[0].dim_y = 128;
        image_addr[0].stride_x = 2;
        image_addr[0].stride_y = 256;
        image_addr[0].scale_x = 1024;
        image_addr[0].scale_y = 1024;
        image_addr[0].step_x = 1;
        image_addr[0].step_y = 1;

        image_addr[1].dim_x = 128;
        image_addr[1].dim_y = 128;
        image_addr[1].stride_x = 1;
        image_addr[1].stride_y = 128;
        image_addr[1].scale_x = 1024;
        image_addr[1].scale_y = 1024;
        image_addr[1].step_x = 1;
        image_addr[1].step_y = 1;

        image_addr[2].dim_x = 128;
        image_addr[2].dim_y = 128;
        image_addr[2].stride_x = 0;
        image_addr[2].stride_y = 192;
        image_addr[2].scale_x = 1024;
        image_addr[2].scale_y = 1024;
        image_addr[2].step_x = 1;
        image_addr[2].step_y = 1;

        switch (type)
        {
        case VX_TYPE_OBJECT_ARRAY:
            VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_ITEMTYPE, &actual_item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_NUMITEMS, &actual_capacity, sizeof(vx_size)));

            if (actual_item_type == item_type && actual_capacity == capacity)
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case TIVX_TYPE_RAW_IMAGE:
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_NUM_EXPOSURES, &actual_num_exposures, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &actual_line_interleaved, sizeof(vx_bool)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_FORMAT, &actual_format, sizeof(actual_format)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_META_HEIGHT_BEFORE, &actual_meta_height_before, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_META_HEIGHT_AFTER, &actual_meta_height_after, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_IMAGEPATCH_ADDRESSING, &actual_image_addr, sizeof(actual_image_addr)));

            if (src_width == actual_src_width &&
                src_height == actual_src_height &&
                num_exposures == actual_num_exposures &&
                line_interleaved == actual_line_interleaved &&
                meta_height_before == actual_meta_height_before &&
                meta_height_after == actual_meta_height_after &&
                memcmp(format, actual_format, sizeof(format)) == 0 &&
                memcmp(image_addr, actual_image_addr, sizeof(image_addr)) == 0
                )
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
                //vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
                //VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
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

    vx_enum item_type = (type == VX_TYPE_OBJECT_ARRAY) ? objarray_itemtype : VX_TYPE_UINT8;
    vx_size capacity = 20;
    vx_uint32 src_width = 128, src_height = 128;
    vx_uint32 num_exposures = 3;
    vx_bool line_interleaved = vx_false_e;
    vx_uint32 meta_height_before = 5, meta_height_after = 0;
    tivx_raw_image_format_t format[3];
    vx_imagepatch_addressing_t image_addr[3];

    vx_enum actual_item_type = VX_TYPE_INVALID;
    vx_size actual_capacity = 0;
    vx_uint32 actual_src_width = 0, actual_src_height = 0;
    vx_uint32 actual_num_exposures = 0;
    vx_bool actual_line_interleaved = vx_true_e;
    vx_uint32 actual_meta_height_before = 0, actual_meta_height_after = 0;
    tivx_raw_image_format_t actual_format[3];
    vx_imagepatch_addressing_t actual_image_addr[3];

    format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    format[0].msb = 12;
    format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    format[1].msb = 7;
    format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    format[2].msb = 11;

    image_addr[0].dim_x = 128;
    image_addr[0].dim_y = 128;
    image_addr[0].stride_x = 2;
    image_addr[0].stride_y = 256;
    image_addr[0].scale_x = 1024;
    image_addr[0].scale_y = 1024;
    image_addr[0].step_x = 1;
    image_addr[0].step_y = 1;

    image_addr[1].dim_x = 128;
    image_addr[1].dim_y = 128;
    image_addr[1].stride_x = 1;
    image_addr[1].stride_y = 128;
    image_addr[1].scale_x = 1024;
    image_addr[1].scale_y = 1024;
    image_addr[1].step_x = 1;
    image_addr[1].step_y = 1;

    image_addr[2].dim_x = 128;
    image_addr[2].dim_y = 128;
    image_addr[2].stride_x = 0;
    image_addr[2].stride_y = 192;
    image_addr[2].scale_x = 1024;
    image_addr[2].scale_y = 1024;
    image_addr[2].step_x = 1;
    image_addr[2].step_y = 1;

    switch (type)
    {

    case VX_TYPE_OBJECT_ARRAY:
        VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_ITEMTYPE, &actual_item_type, sizeof(vx_enum)));
        VX_CALL_(return VX_FAILURE, vxQueryObjectArray((vx_object_array)input, VX_OBJECT_ARRAY_NUMITEMS, &actual_capacity, sizeof(vx_size)));

        if (actual_item_type == item_type && actual_capacity == capacity)
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_OBJECT_ARRAY_ITEMTYPE, &item_type, sizeof(vx_enum)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_OBJECT_ARRAY_NUMITEMS, &capacity, sizeof(vx_size)));
        }
        else
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        break;
    case TIVX_TYPE_RAW_IMAGE:
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_WIDTH, &actual_src_width, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_HEIGHT, &actual_src_height, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_NUM_EXPOSURES, &actual_num_exposures, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &actual_line_interleaved, sizeof(vx_bool)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_FORMAT, &actual_format, sizeof(actual_format)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_META_HEIGHT_BEFORE, &actual_meta_height_before, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_META_HEIGHT_AFTER, &actual_meta_height_after, sizeof(vx_uint32)));
        VX_CALL_(return VX_FAILURE, tivxQueryRawImage((tivx_raw_image)input, TIVX_RAW_IMAGE_IMAGEPATCH_ADDRESSING, &actual_image_addr, sizeof(actual_image_addr)));

        if (src_width == actual_src_width &&
            src_height == actual_src_height &&
            num_exposures == actual_num_exposures &&
            line_interleaved == actual_line_interleaved &&
            meta_height_before == actual_meta_height_before &&
            meta_height_after == actual_meta_height_after &&
            memcmp(format, actual_format, sizeof(format)) == 0 &&
            memcmp(image_addr, actual_image_addr, sizeof(image_addr)) == 0
            )
        {
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_WIDTH, &src_width, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_HEIGHT, &src_height, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_NUM_EXPOSURES, &num_exposures, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &line_interleaved, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_META_HEIGHT_BEFORE, &meta_height_before, sizeof(vx_uint32)));
            VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, TIVX_RAW_IMAGE_META_HEIGHT_AFTER, &meta_height_after, sizeof(vx_uint32)));
            //vx_kernel_image_valid_rectangle_f callback = &own_set_image_valid_rect;
            //VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_VALID_RECT_CALLBACK, &callback, sizeof(callback)));
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
        EXPECT_VX_OBJECT(parameters[0], (enum vx_type_e)type);
        EXPECT_VX_OBJECT(parameters[1], (enum vx_type_e)type);
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
        EXPECT_VX_OBJECT(parameters[0], (enum vx_type_e)type);
        EXPECT_VX_OBJECT(parameters[1], (enum vx_type_e)type);
    }
    if (local_size_kernel_alloc > 0)
    {
        size = local_size_kernel_alloc;
        ptr = ct_calloc(1, local_size_kernel_alloc);
    }
    set_local_size_status_init = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    set_local_ptr_status_init = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
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
        EXPECT_VX_OBJECT(parameters[0], (enum vx_type_e)type);
        EXPECT_VX_OBJECT(parameters[1], (enum vx_type_e)type);
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
    set_local_size_status_deinit = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size));
    set_local_ptr_status_deinit = vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr));
    return VX_SUCCESS;
}

static void own_register_kernel(vx_context context, vx_bool is_meta_from_ref)
{
    vx_kernel kernel = 0;
    vx_size size = local_size_auto_alloc;

    if (is_meta_from_ref)
    {
        ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
            own_Kernel,
            2,
            own_ValidatorMetaFromRef,
            own_Initialize,
            own_Deinitialize), VX_TYPE_KERNEL);
    }
    else
    {
        ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME,
            VX_KERNEL_CONFORMANCE_TEST_OWN_USER,
            own_Kernel,
            2,
            own_ValidatorMetaFromAttr,
            own_Initialize,
            own_Deinitialize), VX_TYPE_KERNEL);
    }

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

typedef struct {
    const char* name;
    vx_enum type;
    vx_bool is_meta_from_ref;
    vx_size local_size;
    vx_bool is_kernel_alloc;
} type_arg;

#define ADD_TYPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "RAW_IMAGE", __VA_ARGS__, TIVX_TYPE_RAW_IMAGE)) \

#define ADD_FROM_FLAG(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "_FROM_REF", __VA_ARGS__, vx_true_e)), \
    CT_EXPAND(nextmacro(testArgName "_FROM_ATTR", __VA_ARGS__, vx_false_e))

#define ADD_LOCAL_SIZE_AND_ALLOC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=0", __VA_ARGS__, 0, vx_false_e)), \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=10/ALLOC=AUTO", __VA_ARGS__, 10, vx_false_e)), \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=10/ALLOC=KERNEL", __VA_ARGS__, 10, vx_true_e))

#define USERKERNEL_PARAMETERS \
    CT_GENERATE_PARAMETERS("", ADD_TYPE, ADD_FROM_FLAG, ADD_LOCAL_SIZE_AND_ALLOC, ARG)

TEST_WITH_ARG(tivxRawImage, testUserKernel, type_arg, USERKERNEL_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_reference src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_bool is_meta_from_ref = arg_->is_meta_from_ref;

    int phase = 0;
    tivx_raw_image_create_params_t params;
    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_false_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    type = (enum vx_type_e)arg_->type;
    local_size = arg_->local_size;
    is_kernel_alloc = arg_->is_kernel_alloc;

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

    switch (type)
    {

    case TIVX_TYPE_RAW_IMAGE:
        {
            ASSERT_VX_OBJECT(src = (vx_reference)tivxCreateRawImage(context, &params), (enum vx_type_e)type);
            ASSERT_VX_OBJECT(dst = (vx_reference)tivxCreateRawImage(context, &params), (enum vx_type_e)type);
        }
        break;

    default:
        break;
    }

    ASSERT_NO_FAILURE(own_register_kernel(context, is_meta_from_ref));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    // graph verification, first-time, and re-verify

    for (phase = 0; phase < 2; ++phase)
    {
        vx_size size = 0;
        void* ptr = NULL;

        is_initialize_called = vx_false_e;
        is_deinitialize_called = vx_false_e;
        is_validator_called = vx_false_e;
        set_local_size_status_init = VX_FAILURE;
        set_local_ptr_status_init = VX_FAILURE;
        query_local_size_status_deinit = VX_FAILURE;
        query_local_ptr_status_deinit = VX_FAILURE;
        set_local_size_status_deinit = VX_FAILURE;
        set_local_ptr_status_deinit = VX_FAILURE;

        VX_CALL(vxVerifyGraph(graph));

        ASSERT(is_initialize_called == vx_true_e);
        if (phase == 0)
            ASSERT(is_deinitialize_called == vx_false_e);
        else
            ASSERT(is_deinitialize_called == vx_true_e);
        ASSERT(is_validator_called == vx_true_e);

        VX_CALL(vxQueryNode(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size)));
        VX_CALL(vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr)));

        ASSERT(VX_SUCCESS != vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_SIZE, &size, sizeof(size)));
        ASSERT(VX_SUCCESS != vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &ptr, sizeof(ptr)));

        ASSERT(size == local_size);
        if (size > 0)
            ASSERT(ptr != NULL);
        else
            ASSERT(ptr == NULL);

        if (local_size_auto_alloc == 0) // change allowed
        {
            ASSERT(set_local_size_status_init == VX_SUCCESS);
            ASSERT(set_local_ptr_status_init == VX_SUCCESS);
            if (is_deinitialize_called)
            {
                ASSERT(set_local_size_status_deinit == VX_SUCCESS);
                ASSERT(set_local_ptr_status_deinit == VX_SUCCESS);
            }
        }
        else // change does not allowed: error must be generated
        {
            ASSERT(set_local_size_status_init != VX_SUCCESS);
            ASSERT(set_local_ptr_status_init != VX_SUCCESS);
            if (is_deinitialize_called)
            {
                ASSERT(set_local_size_status_deinit != VX_SUCCESS);
                ASSERT(set_local_ptr_status_deinit != VX_SUCCESS);
            }
        }
    }

    // execute graph

    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;
    is_validator_called = vx_false_e;
    is_kernel_called = vx_false_e;

    VX_CALL(vxProcessGraph(graph));

    ASSERT(is_initialize_called == vx_false_e);
    ASSERT(is_deinitialize_called == vx_false_e);
    ASSERT(is_validator_called == vx_false_e);
    ASSERT(is_kernel_called == vx_true_e);

    // finalization

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst == 0);
    ASSERT(src == 0);
}

TEST_WITH_ARG(tivxRawImage, testUserKernelObjectArray, type_arg,
    ARG("USER_DATA_OBJECT_FROM_REF", TIVX_TYPE_RAW_IMAGE, vx_true_e),
    ARG("USER_DATA_OBJECT_FROM_ATTR",TIVX_TYPE_RAW_IMAGE, vx_false_e)
)
{
    vx_context context = context_->vx_context_;
    vx_reference exemplar = 0, src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_bool is_meta_from_ref = arg_->is_meta_from_ref;
    objarray_itemtype = (enum vx_type_e)arg_->type;
    type = VX_TYPE_OBJECT_ARRAY;

    is_validator_called = vx_false_e;
    is_kernel_called = vx_false_e;
    is_initialize_called = vx_false_e;
    is_deinitialize_called = vx_false_e;

    vx_size capacity = 20;

    tivx_raw_image_create_params_t params;
    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_false_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    switch (objarray_itemtype)
    {
    case TIVX_TYPE_RAW_IMAGE:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)tivxCreateRawImage(context, &params), (enum vx_type_e)objarray_itemtype);
        break;
    default:
        break;
    }

    ASSERT_VX_OBJECT(src = (vx_reference)vxCreateObjectArray(context, exemplar, capacity), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateObjectArray(context, exemplar, capacity), VX_TYPE_OBJECT_ARRAY);

    ASSERT_NO_FAILURE(own_register_kernel(context, is_meta_from_ref));

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(user_kernel = vxGetKernelByName(context, VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME), VX_TYPE_KERNEL);
    ASSERT_VX_OBJECT(node = vxCreateGenericNode(graph, user_kernel), VX_TYPE_NODE);

    VX_CALL(vxSetParameterByIndex(node, 0, (vx_reference)src));
    VX_CALL(vxSetParameterByIndex(node, 1, (vx_reference)dst));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    /* user kernel should be removed only after all references to it released */
    /* Note, vxRemoveKernel doesn't zeroing kernel ref */
    VX_CALL(vxRemoveKernel(user_kernel));

    VX_CALL(vxReleaseReference(&dst));
    VX_CALL(vxReleaseReference(&src));
    VX_CALL(vxReleaseReference(&exemplar));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(dst == 0);
    ASSERT(src == 0);
    ASSERT(exemplar == 0);

    ASSERT(is_validator_called == vx_true_e);
    ASSERT(is_kernel_called == vx_true_e);
    ASSERT(is_initialize_called == vx_true_e);
    ASSERT(is_deinitialize_called == vx_true_e);
}

TEST(tivxRawImage, testRemoveKernel)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel = 0;

    EXPECT_VX_OBJECT(kernel = vxGetKernelByEnum(context, VX_KERNEL_ADD), VX_TYPE_KERNEL);
    // Only kernels added through vxAddUserKernel can be removed
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxRemoveKernel(kernel));
    VX_CALL(vxReleaseKernel(&kernel));

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
            context,
            VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME,
            VX_KERNEL_CONFORMANCE_TEST_OWN_BAD,
            own_Kernel,
            2,
            own_ValidatorMetaFromRef,
            own_Initialize,
            own_Deinitialize), VX_TYPE_KERNEL);

    VX_CALL(vxRemoveKernel(kernel));
}

TEST(tivxRawImage, testOutDelay)
{
    vx_context context = context_->vx_context_;
    vx_kernel kernel = 0;

    ASSERT_VX_OBJECT(kernel = vxAddUserKernel(
        context,
        VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME,
        VX_KERNEL_CONFORMANCE_TEST_OWN_BAD,
        own_Kernel,
        2,
        own_ValidatorMetaFromRef,
        own_Initialize,
        own_Deinitialize), VX_TYPE_KERNEL);

    ASSERT_NE_VX_STATUS(vxAddParameterToKernel(kernel, 0, VX_OUTPUT, VX_TYPE_DELAY, VX_PARAMETER_STATE_REQUIRED), VX_SUCCESS);

    VX_CALL(vxRemoveKernel(kernel));
}



TEST(tivxRawImage, test_tivxCreateRawImage)
{
    vx_context context = context_->vx_context_;
    tivx_raw_image raw_image = 0;
    vx_uint32 actual_line_interleaved;
    vx_uint32 actual_width;

    tivx_raw_image_create_params_t params;
    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_true_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    /* 1. check if raw image can be created */
    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    /* 2. check if raw image can be queried */
    VX_CALL(tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &actual_line_interleaved, sizeof(vx_uint32)));
    ASSERT(actual_line_interleaved == vx_true_e);

    /* 3. check if raw image can be queried */
    VX_CALL(tivxQueryRawImage(raw_image, TIVX_RAW_IMAGE_WIDTH, &actual_width, sizeof(vx_uint32)));
    ASSERT_EQ_INT(128, actual_width);

    /* 4. Initialize empty raw image after creation */
    {
        vx_map_id map_id;
        vx_int32 i;
        vx_rectangle_t rect;
        vx_imagepatch_addressing_t addr;
        uint16_t *ptr = NULL;
        uint16_t *ptr2 = NULL;

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 128;
        rect.end_y = 128;

        /* Initialize data using WRITE ONLY MAP */
        VX_CALL(tivxMapRawImagePatch(raw_image, &rect, 0, &map_id, &addr, (void **)&ptr,
                                     VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));
        ASSERT(ptr != NULL);

        for (i = 0; i < 128*128; i++)
        {
            ptr[i] = i;
        }
        VX_CALL(tivxUnmapRawImagePatch(raw_image, map_id));

    /* 5. check data in raw image */

        VX_CALL(tivxMapRawImagePatch(raw_image, &rect, 0, &map_id, &addr, (void **)&ptr2,
                                      VX_READ_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));

        for (i = 0; i < 128*128; i++)
        {
            ASSERT(ptr[i] == i);
        }
        VX_CALL(tivxUnmapRawImagePatch(raw_image, map_id));
    }

    VX_CALL(tivxReleaseRawImage(&raw_image));
    ASSERT(raw_image == 0);
}

TEST(tivxRawImage, test_tivxCopyRawImageWrite)
{
    vx_context context = context_->vx_context_;
    tivx_raw_image raw_image;
    int i, j;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t addr, addr1;
    uint8_t *ptr = NULL;

    rect.start_x = 16;
    rect.start_y = 19;
    rect.end_x = 16+16;
    rect.end_y = 19+21;

    addr.dim_x = 16;
    addr.dim_y = 21;
    addr.stride_x = 1;
    addr.stride_y = 16;
    addr.step_x = 1;
    addr.step_y = 1;

    uint8_t img[16*21];

    tivx_raw_image_create_params_t params;
    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_true_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    /* Initialization */
    for (i = 0; i < 16*21; i++)
    {
        img[i] = i%256;
    }

    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    /* Write, COPY gains */
    VX_CALL(tivxCopyRawImagePatch(raw_image, &rect, 1, &addr, (void *)&img,
                                      VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));

    /* Check (MAP) */
    {
        vx_map_id map_id;
        VX_CALL(tivxMapRawImagePatch(raw_image, &rect, 1, &map_id, &addr1, (void **)&ptr,
                                      VX_READ_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER));

        ASSERT(ptr != NULL);
        for (j = 0; j < 21; j++)
        {
            for (i = 0; i < 16; i++)
            {
                ASSERT(ptr[j*addr1.stride_y + i] == (i+j*16)%256);
            }
        }

        VX_CALL(tivxUnmapRawImagePatch(raw_image, map_id));
    }

    VX_CALL(tivxReleaseRawImage(&raw_image));
    ASSERT(raw_image == 0);
}

TEST(tivxRawImage, test_tivxCopyRawImageRead)
{
    vx_context context = context_->vx_context_;
    tivx_raw_image raw_image;
    int i;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t addr, addrMap;
    uint16_t *ptr = NULL;
    vx_map_id map_id;

    rect.start_x = 16;
    rect.start_y = 19;
    rect.end_x = 16+16;
    rect.end_y = 19+21;

    addr.dim_x = 16*sizeof(uint16_t);

    uint16_t img[16];

    tivx_raw_image_create_params_t params;
    params.width = 128;
    params.height = 128;
    params.num_exposures = 3;
    params.line_interleaved = vx_false_e;
    params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    params.format[0].msb = 12;
    params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    params.format[1].msb = 7;
    params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    params.format[2].msb = 11;
    params.meta_height_before = 5;
    params.meta_height_after = 0;

    ASSERT_VX_OBJECT(raw_image = tivxCreateRawImage(context, &params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

    /* Initialize data using WRITE ONLY MAP */
    VX_CALL(tivxMapRawImagePatch(raw_image, NULL, 0, &map_id, &addrMap, (void **)&ptr,
                                 VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_META_BEFORE_BUFFER));

    /* Initialization */
    for (i = 0; i < addrMap.dim_x/2; i++)
    {
        ptr[i] = i;
    }

    VX_CALL(tivxUnmapRawImagePatch(raw_image, map_id));

    /* READ, COPY offsets */
    {
        VX_CALL(tivxCopyRawImagePatch(raw_image, NULL, 0, &addr, (void *)img,
                                      VX_READ_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_META_BEFORE_BUFFER));
    }
    /* Check */
    for (i = 0; i < 16; i++)
    {
        ASSERT(img[i] == i);
    }

    VX_CALL(tivxReleaseRawImage(&raw_image));
    ASSERT(raw_image == 0);
}

TESTCASE_TESTS(tivxRawImage,
        test_tivxCreateRawImage,
        test_tivxCopyRawImageRead,
        test_tivxCopyRawImageWrite,
        testUserKernel,
        testUserKernelObjectArray,
        testRemoveKernel,
        testOutDelay
        )

