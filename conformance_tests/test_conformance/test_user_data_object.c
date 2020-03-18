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

#ifdef OPENVX_USE_USER_DATA_OBJECT

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_khr_user_data_object.h>

#define VX_KERNEL_CONFORMANCE_TEST_OWN_BAD (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 0)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_BAD_NAME "org.khronos.openvx.test.own_bad"

#define VX_KERNEL_CONFORMANCE_TEST_OWN (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 1)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_NAME "org.khronos.openvx.test.own"

#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER (VX_KERNEL_BASE(VX_ID_DEFAULT, 0) + 2)
#define VX_KERNEL_CONFORMANCE_TEST_OWN_USER_NAME "org.khronos.openvx.test.own_user"

TESTCASE(UserDataObject, CT_VXContext, ct_setup_vx_context, 0)

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

static const vx_char user_data_object_name[] = "wb_t";

typedef struct
{
    vx_int32 mode;
    vx_int32 gain[4];
    vx_int32 offset[4];
} wb_t;

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

        vx_enum actual_item_type = VX_TYPE_INVALID;
        vx_size actual_capacity = 0;
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
        case VX_TYPE_USER_DATA_OBJECT:
            {
                char actual_name[VX_MAX_REFERENCE_NAME];
                vx_size actual_size;

                VX_CALL_(return VX_FAILURE, vxQueryUserDataObject((vx_user_data_object)input, VX_USER_DATA_OBJECT_NAME, &actual_name, sizeof(actual_name)));
                VX_CALL_(return VX_FAILURE, vxQueryUserDataObject((vx_user_data_object)input, VX_USER_DATA_OBJECT_SIZE, &actual_size, sizeof(vx_size)));

                if ((strcmp(user_data_object_name, actual_name) == 0) && (actual_size == sizeof(wb_t)))
                {
                    VX_CALL_(return VX_FAILURE, vxSetMetaFormatFromReference(meta, input));
                }
                else
                {
                    return VX_ERROR_INVALID_PARAMETERS;
                }
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

    vx_enum actual_item_type = VX_TYPE_INVALID;
    vx_size actual_capacity = 0;
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
    case VX_TYPE_USER_DATA_OBJECT:
        {
            vx_size actual_size;
            vx_size user_data_size = sizeof(wb_t);
            char actual_name[VX_MAX_REFERENCE_NAME];

            VX_CALL_(return VX_FAILURE, vxQueryUserDataObject((vx_user_data_object)input, VX_USER_DATA_OBJECT_NAME, &actual_name, sizeof(actual_name)));
            VX_CALL_(return VX_FAILURE, vxQueryUserDataObject((vx_user_data_object)input, VX_USER_DATA_OBJECT_SIZE, &actual_size, sizeof(vx_size)));

            if ((strcmp(user_data_object_name, actual_name) == 0) && (actual_size == sizeof(wb_t)))
            {
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_USER_DATA_OBJECT_NAME, &user_data_object_name, sizeof(user_data_object_name)));
                VX_CALL_(return VX_FAILURE, vxSetMetaFormatAttribute(meta, VX_USER_DATA_OBJECT_SIZE, &user_data_size, sizeof(vx_size)));
            }
            else
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }
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
    CT_EXPAND(nextmacro(testArgName "USER_DATA_OBJECT", __VA_ARGS__, VX_TYPE_USER_DATA_OBJECT)) \

#define ADD_FROM_FLAG(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "_FROM_REF", __VA_ARGS__, vx_true_e)), \
    CT_EXPAND(nextmacro(testArgName "_FROM_ATTR", __VA_ARGS__, vx_false_e))

#define ADD_LOCAL_SIZE_AND_ALLOC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=0", __VA_ARGS__, 0, vx_false_e)), \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=10/ALLOC=AUTO", __VA_ARGS__, 10, vx_false_e)), \
    CT_EXPAND(nextmacro(testArgName "/LOCAL_SIZE=10/ALLOC=KERNEL", __VA_ARGS__, 10, vx_true_e))

#define USERKERNEL_PARAMETERS \
    CT_GENERATE_PARAMETERS("", ADD_TYPE, ADD_FROM_FLAG, ADD_LOCAL_SIZE_AND_ALLOC, ARG)

TEST_WITH_ARG(UserDataObject, testUserKernel, type_arg, USERKERNEL_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_reference src = 0, dst = 0;
    vx_graph graph = 0;
    vx_kernel user_kernel = 0;
    vx_node node = 0;
    vx_bool is_meta_from_ref = arg_->is_meta_from_ref;

    int phase = 0;

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

    case VX_TYPE_USER_DATA_OBJECT:
        {
            ASSERT_VX_OBJECT(src = (vx_reference)vxCreateUserDataObject(context, (const vx_char*)&user_data_object_name, sizeof(wb_t), NULL), (enum vx_type_e)type);
            ASSERT_VX_OBJECT(dst = (vx_reference)vxCreateUserDataObject(context, (const vx_char*)&user_data_object_name, sizeof(wb_t), NULL), (enum vx_type_e)type);
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

TEST_WITH_ARG(UserDataObject, testUserKernelObjectArray, type_arg,
    ARG("USER_DATA_OBJECT_FROM_REF", VX_TYPE_USER_DATA_OBJECT, vx_true_e),
    ARG("USER_DATA_OBJECT_FROM_ATTR",VX_TYPE_USER_DATA_OBJECT, vx_false_e)
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

    switch (objarray_itemtype)
    {
    case VX_TYPE_USER_DATA_OBJECT:
        ASSERT_VX_OBJECT(exemplar = (vx_reference)vxCreateUserDataObject(context, (const vx_char*)&user_data_object_name, sizeof(wb_t), NULL), (enum vx_type_e)objarray_itemtype);
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

TEST(UserDataObject, testRemoveKernel)
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

TEST(UserDataObject, testOutDelay)
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



TEST(UserDataObject, test_vxCreateUserDataObject)
{
    vx_context context = context_->vx_context_;
    char actual_name[VX_MAX_REFERENCE_NAME];
    vx_size actual_size = 0;
    vx_user_data_object user_data_object = 0;

    /* 1. check if user data object can be created with empty type_name and not initialized */
    ASSERT_VX_OBJECT(user_data_object = vxCreateUserDataObject(context, NULL, sizeof(wb_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* 2. check if user data object actual name is a string with a null termination */
    VX_CALL(vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_NAME, &actual_name, sizeof(actual_name)));
    ASSERT(strncmp("", actual_name, VX_MAX_REFERENCE_NAME) == 0);

    /* 3. check if user data object actual size corresponds to requested size */
    VX_CALL(vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_SIZE, &actual_size, sizeof(vx_size)));
    ASSERT_EQ_INT(sizeof(wb_t), actual_size);

    /* 4. Initialize empty user data object after creation */
	{
		wb_t *p = NULL;
        vx_map_id map_id;
        vx_int32 i;

		/* Initialize data using WRITE ONLY MAP */
        VX_CALL(vxMapUserDataObject(user_data_object, 0, sizeof(wb_t), &map_id, (void **)&p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
        ASSERT(p != NULL);
		p->mode = 2;
		for (i = 0; i < 4; i++)
		{
			p->gain[i] = i;
			p->offset[i] = i+4;
		}
        VX_CALL(vxUnmapUserDataObject(user_data_object, map_id));

    /* 5. check data in user data object */

        VX_CALL(vxMapUserDataObject(user_data_object, 0, sizeof(wb_t), &map_id, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
		ASSERT(2 == p->mode);
		for (i = 0; i < 4; i++)
		{
			ASSERT(p->gain[i] == i);
			ASSERT(p->offset[i] == i+4);
		}
        VX_CALL(vxUnmapUserDataObject(user_data_object, map_id));
	}

    VX_CALL(vxReleaseUserDataObject(&user_data_object));
    ASSERT(user_data_object == 0);
}

TEST(UserDataObject, test_vxCopyUserDataObjectWrite)
{
    vx_context context = context_->vx_context_;
    wb_t localUserDataObjectInit;
    wb_t localUserDataObject;
    vx_user_data_object user_data_object;
    int i;

    /* Initialization */
    localUserDataObjectInit.mode = 0;
    localUserDataObject.mode = 1;

    for (i = 0; i < 4; i++)
    {
        localUserDataObjectInit.gain[i] = 0;
        localUserDataObjectInit.offset[i] = 0;

        localUserDataObject.gain[i] = i;
        localUserDataObject.offset[i] = i+4;
    }

    ASSERT_VX_OBJECT(user_data_object = vxCreateUserDataObject(context, user_data_object_name, sizeof(wb_t), &localUserDataObjectInit), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* Write, COPY gains */
    {
        vx_size local_offset = offsetof(wb_t, gain);
        vx_size local_bytes = sizeof(vx_int32)*4;
        vx_int32 *p = &localUserDataObject.gain[0];
        VX_CALL(vxCopyUserDataObject(user_data_object, local_offset, local_bytes, (void *)p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    }

    /* Check (MAP) */
    {
        vx_int32 *p = NULL;
        vx_size local_offset = offsetof(wb_t, gain);
        vx_size local_bytes = sizeof(vx_int32)*4;
        vx_map_id map_id;
        VX_CALL(vxMapUserDataObject(user_data_object, local_offset, local_bytes, &map_id, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));

        ASSERT(p != NULL);
        for (i = 0; i<4; i++)
        {
            ASSERT(p[i] == i);
        }

        VX_CALL(vxUnmapUserDataObject(user_data_object, map_id));
    }

    VX_CALL(vxReleaseUserDataObject(&user_data_object));
    ASSERT(user_data_object == 0);
}

TEST(UserDataObject, test_vxCopyUserDataObjectRead)
{
    vx_context context = context_->vx_context_;
    wb_t localUserDataObjectInit;
    wb_t localUserDataObject;
    vx_user_data_object user_data_object;
    int i;

    /* Initialization */
    localUserDataObjectInit.mode = 1;
    localUserDataObject.mode =0;

    for (i = 0; i < 4; i++)
    {
        localUserDataObjectInit.gain[i] = i;
        localUserDataObjectInit.offset[i] = i+4;

        localUserDataObject.gain[i] = 0;
        localUserDataObject.offset[i] = 0;
    }

    ASSERT_VX_OBJECT(user_data_object = vxCreateUserDataObject(context, user_data_object_name, sizeof(wb_t), &localUserDataObjectInit), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    /* READ, COPY offsets */
    {
        vx_size local_offset = offsetof(wb_t, offset);
        vx_size local_bytes = sizeof(vx_int32)*4;
        vx_int32 *p = &localUserDataObject.offset[0];
        VX_CALL(vxCopyUserDataObject(user_data_object, local_offset, local_bytes, (void *)p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    }
    /* Check */
    for (i = 0; i < 4; i++)
    {
        ASSERT(localUserDataObject.offset[i] == i+4);
    }

    VX_CALL(vxReleaseUserDataObject(&user_data_object));
    ASSERT(user_data_object == 0);
}

TEST(UserDataObject, test_vxMapUserDataObjectWrite)
{
    vx_context context = context_->vx_context_;
    wb_t localUserDataObjectInit;
    vx_user_data_object user_data_object;
    int i;

    /* Initialization */
    localUserDataObjectInit.mode = 1;

    for (i = 0; i < 4; i++)
    {
        localUserDataObjectInit.gain[i] = i+0x10000000;
        localUserDataObjectInit.offset[i] = i+0x10000004;
    }

    ASSERT_VX_OBJECT(user_data_object = vxCreateUserDataObject(context, user_data_object_name, sizeof(wb_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    {
		wb_t *p = NULL;
        vx_map_id map_id;

		/* Map, WRITE_ONLY mode*/
        VX_CALL(vxMapUserDataObject(user_data_object, 0, sizeof(wb_t), &map_id, (void **)&p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
        ASSERT(p != NULL);
        memcpy(p, &localUserDataObjectInit, sizeof(wb_t));
        VX_CALL(vxUnmapUserDataObject(user_data_object, map_id));

		/* Map, READ_AND_WRITE mode*/

        VX_CALL(vxMapUserDataObject(user_data_object, 0, sizeof(wb_t), &map_id, (void **)&p, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
        ASSERT(p != NULL);
        /* Check */
		ASSERT(localUserDataObjectInit.mode == p->mode);
		for (i = 0; i < 4; i++)
		{
			ASSERT(localUserDataObjectInit.gain[i] == p->gain[i]);
			ASSERT(localUserDataObjectInit.offset[i] == p->offset[i]);
		}

        /* Write into user data object */
		p->mode = 2;
		for (i = 0; i < 4; i++)
		{
			p->gain[i] = i;
			p->offset[i] = i+4;
		}
        VX_CALL(vxUnmapUserDataObject(user_data_object, map_id));

        /* Check */
        VX_CALL(vxMapUserDataObject(user_data_object, 0, sizeof(wb_t), &map_id, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
		ASSERT(2 == p->mode);
		for (i = 0; i < 4; i++)
		{
			ASSERT(p->gain[i] == i);
			ASSERT(p->offset[i] == i+4);
		}
        VX_CALL(vxUnmapUserDataObject(user_data_object, map_id));
    }

    VX_CALL(vxReleaseUserDataObject(&user_data_object));
    ASSERT(user_data_object == 0);
}

TESTCASE_TESTS(UserDataObject,
        test_vxCreateUserDataObject,
        test_vxCopyUserDataObjectRead,
        test_vxCopyUserDataObjectWrite,
        test_vxMapUserDataObjectWrite,
        testUserKernel,
        testUserKernelObjectArray,
        testRemoveKernel,
        testOutDelay
        )

#endif
