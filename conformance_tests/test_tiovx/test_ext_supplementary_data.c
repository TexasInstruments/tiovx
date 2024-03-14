/*
 * Copyright (c) 2023 ETAS Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

/* supplementary_data.c
 * Functional tests for supplementary data extension.
 *
 * Test registration of bad types / exemplars:
 * Exemplar that itself has supplementary data
 * Unsupported type (VX_TYPE_REFERENCE, VX_TYPE_DELAY)
 * 
 * Tests creation and operation of supplementary data for:
 * VX_TYPE_USER_DATA_OBJECT
 * VX_TYPE_LUT
 * VX_TYPE_DISTRIBUTION
 * VX_TYPE_PYRAMID
 * VX_TYPE_THRESHOLD
 * VX_TYPE_MATRIX
 * VX_TYPE_CONVOLUTION
 * VX_TYPE_SCALAR
 * VX_TYPE_ARRAY
 * VX_TYPE_IMAGE
 * VX_TYPE_REMAP
 * VX_TYPE_OBJECT_ARRAY
 * VX_TYPE_TENSOR
 * TIVX_TYPE_RAW_IMAGE
 * 
 * Test supplementary data types of:
 * VX_TYPE_SCALAR - deprecated
 * VX_TYPE_ARRAY - deprecated
 * VX_TYPE_USER_DATA_OBJECT
 * 
 * Test inheritance of supplementary data for:
 * sub-images (ROI & Channel) alone & in object arrays
 * sub-tensors (View, ROI & Channel) alone & in object arrays
 * 
 * Test target kernel operation of object descriptor for tensors
 * and images
 * 
 * Test Copy & Swap of objects with supplementary data:
 * Data correctly copied/ swapped
 * Data not copied / swapped if types don't match (without error)
 * Data not copied if one object doesn't have any supplementary data (without error)
 */

#include "test_engine/test.h"

#include "VX/vx.h"
#include "TI/tivx.h"
#include "enumstring.h"
#include <TI/tivx_obj_desc.h>
#include <TI/tivx_obj_desc.h>
#include <VX/vx_khr_supplementary_data.h>
#include <VX/vx_khr_swap_move.h>
#include <RB/vx_rb_ref_data.h>
#include <VX/vxu.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <TI/tivx_target_kernel.h>


/*******************************************************************/
/* test : Functional tests for supplementary data extension        */
/*******************************************************************/
TESTCASE(supplementary_data, CT_VXContext, ct_setup_vx_context, 0)

#define ERROR_EXPECT_STATUS_BAD( status, tag ) { \
        if((status) == VX_SUCCESS) { \
            printf("ERROR: failed with unexpected VX_SUCCESS at " __FILE__ "#%d;\n", __LINE__ ); \
        } \
        EXPECT_NE_VX_STATUS(VX_SUCCESS, status);\     
    }

#define ERROR_EXPECT_STATUS( status, expected, tag ) { \
        vx_status status_ = (status); \
        if(status_ != (expected)) { \
            printf("ERROR: failed with status = (%d) at " __FILE__ "#%d when %d was expected\n", status_, __LINE__, expected); \
        } \
        EXPECT_EQ_VX_STATUS(expected, status_);\
    }    

#define ERROR_CHECK_STATUS(status, tag) ERROR_EXPECT_STATUS( status, VX_SUCCESS, tag )

static const vx_enum type_ids[] = 
{
    VX_TYPE_ARRAY,
    VX_TYPE_CONVOLUTION,
    VX_TYPE_DISTRIBUTION,
    VX_TYPE_IMAGE,
    VX_TYPE_LUT,
    VX_TYPE_MATRIX,
    VX_TYPE_OBJECT_ARRAY,
    VX_TYPE_PYRAMID,
    VX_TYPE_REMAP,
    VX_TYPE_SCALAR,
    VX_TYPE_TENSOR,
    VX_TYPE_THRESHOLD,
    VX_TYPE_USER_DATA_OBJECT,
    TIVX_TYPE_RAW_IMAGE
};
typedef struct _user_data
{
    vx_uint32 numbers[4];
} user_data_t;


#define NUM_TYPES (sizeof(type_ids) / sizeof(vx_enum))

enum user_library_e
{
    USER_LIBRARY_EXAMPLE = 1
};

enum user_kernel_e
{
    MY_USER_TARGET_KERNEL = VX_KERNEL_BASE( VX_ID_USER, USER_LIBRARY_EXAMPLE ) + 0x001,
    MY_USER_TEST_KERNEL = VX_KERNEL_BASE(VX_ID_USER, USER_LIBRARY_EXAMPLE) + 0x002,
    MY_USER_TEST_TARGET_KERNEL = VX_KERNEL_BASE(VX_ID_USER, USER_LIBRARY_EXAMPLE) + 0x003,
    MY_DUMB_IMAGE_COPY_KERNEL = VX_KERNEL_BASE(VX_ID_USER, USER_LIBRARY_EXAMPLE) + 0x004,
};

static vx_status VX_CALLBACK myGeneralKernelCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK myGeneralKernelDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return VX_SUCCESS;
}
static tivx_target_kernel target_kernel = NULL;
static vx_kernel general_kernel = NULL;
static tivx_target_kernel target_test_kernel = NULL;
static vx_kernel my_dumb_copy_kernel = NULL;
static vx_kernel user_test_kernel = NULL;
static vx_kernel general_test_kernel = NULL;

/* Function to get a parameter, add it to a graph and then release it */
static vx_status addParameterToGraph(vx_graph graph, vx_node node, vx_uint32 num)
{
    vx_parameter p = vxGetParameterByIndex(node, num);
    vx_status status = vxAddParameterToGraph(graph, p);
    vxReleaseParameter(&p);
    return status;
}

vx_status myGeneralKernelValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    // verification is ensured by design
    return VX_SUCCESS;
}

vx_status myUserTestKernelValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    vx_user_data_object supp = vxGetSupplementaryUserDataObject(parameters[0], NULL, NULL);
    /* Successfully got supplementary data object with NULL status" */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)supp));
    /* Successfully created supplementary on virtual object data in validator */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject(parameters[3], supp));
    VX_CALL(vxReleaseUserDataObject(&supp));
    supp = vxGetSupplementaryUserDataObject(parameters[0], NULL, NULL);
    /* Successfully got supplementary data from virtual object in validator */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)supp));
    VX_CALL(vxReleaseUserDataObject(&supp));
    return VX_SUCCESS;
}

vx_status myDumbImageCopyValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_user_data_object supp = vxGetSupplementaryUserDataObject(parameters[0], NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject(parameters[1], supp));
    vxSetMetaFormatFromReference(metas[1], parameters[0]);
    vxReleaseUserDataObject(&supp);
    return VX_SUCCESS;
}

vx_status myDumbImageCopyKernel(vx_node node, const vx_reference params[], vx_uint32 num_params)
{
    vxuCopy(vxGetContext((vx_reference)node), params[0], params[1]);
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK  myGeneralKernelFunction(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg)
{
    /* Just get the object descriptor from input and copy it to output with a value incremented
    * We assume there are just 2 parameters.
    */
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t * udo_in = tivxGetSupplementaryDataObjDesc(obj_desc[0], NULL);
    if (udo_in)
    {
        /* we have user data type supplementary data. In this case they should be the same user type */
        tivx_obj_desc_user_data_object_t * udo_out = tivxGetSupplementaryDataObjDesc(obj_desc[1], NULL);
        if (udo_out)
        {
            /* same types, we can process... the data type is user_data_t */
            user_data_t * src_desc_target_ptr = (user_data_t *)tivxMemShared2TargetPtr(&udo_in->mem_ptr);
            user_data_t * dst_desc_target_ptr = (user_data_t *)tivxMemShared2TargetPtr(&udo_out->mem_ptr);

            /* Map all buffers, which invalidates the cache */
            status = tivxMemBufferMap(src_desc_target_ptr, udo_in->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
            status = tivxMemBufferMap(dst_desc_target_ptr, udo_out->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
            *dst_desc_target_ptr = *src_desc_target_ptr;
            dst_desc_target_ptr->numbers[0]++;
            dst_desc_target_ptr->numbers[3]++;
            status = tivxMemBufferUnmap(dst_desc_target_ptr, udo_out->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
        }
    }
    else
    {
        /* we don't have any supplementary data that we recognise; do nothing. */
        printf("No Supplementary data!\n");
    }
    return status;
}

static vx_status VX_CALLBACK myUserTestKernelFunction(vx_node node, const vx_reference params[], vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_user_data_object input = vxGetSupplementaryUserDataObject(params[0], NULL, &status);
    /* Got supplementary data of input in kernel function" */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    status = VX_SUCCESS;
    vx_user_data_object childInput = vxGetSupplementaryUserDataObject(params[1], NULL, &status);
    /* "Got supplementary data of parent from child in kernel function" */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,status);
    /* ".. and the data object is good" */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)(childInput)));
    vx_size size = 4;
    vx_map_id map_id;
    void * ptr;
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(tivxSetUserDataObjectAttribute(input, TIVX_USER_DATA_OBJECT_VALID_SIZE, &size, sizeof(size)), "Cannot set valid size of input supplementary data");
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(vxCopyUserDataObject(input, 0, sizeof(size), &size, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST), "Cannot copy to the supplementary data of input object");
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(vxMapUserDataObject(input, 0, sizeof(size), &map_id, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0), "Cannot map supplementary data of input for writing");
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(vxSetSupplementaryUserDataObject(params[2], input), "Cannot create supplementary data in kernel function");
    vx_user_data_object output = vxGetSupplementaryUserDataObject(params[3], NULL, &status);
    ERROR_CHECK_STATUS(tivxSetUserDataObjectAttribute(output, TIVX_USER_DATA_OBJECT_VALID_SIZE, &size, sizeof(size)), "Can set valid size of output supplementary data");
    ERROR_CHECK_STATUS(vxCopyUserDataObject(output, 0, sizeof(size), &size, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST), "Can copy to the supplementary data of output object");
    ERROR_CHECK_STATUS(vxMapUserDataObject(output, 0, sizeof(size), &map_id, &ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0), "Can map supplementary data of output for writing");
    ERROR_CHECK_STATUS(vxUnmapUserDataObject(output, map_id), "Successfully unmapped supplementary data of output");
    vxReleaseUserDataObject(&input);
    vxReleaseUserDataObject(&output);
    vxReleaseUserDataObject(&childInput);
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK  myUserTestTargetFunction(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg)
{
    /* Parameter 0: input with supplementary data equal to {1, 2, 3, 4}
       Parameter 1: input without supplementary data, child of parameter[0]
       parameter 2: input without supplementary data
       Parameter 3: output with supplementary data
    */
    user_data_t test_data = {.numbers = {90, 80, 70, 60}};
    tivx_obj_desc_user_data_object_t * supp = tivxGetSupplementaryDataObjDesc(NULL, NULL);
    ERROR_CHECK_STATUS(NULL != supp, "tivxGetSupplementaryDataObjDesc fails when passed a NULL");
    supp = tivxGetSupplementaryDataObjDesc(obj_desc[0], "invalid type");
    ERROR_CHECK_STATUS(NULL != supp, "tivxGetSupplementaryDataObjDesc fails when passed the wrong type");
    supp = tivxGetSupplementaryDataObjDesc(obj_desc[2], NULL);
    ERROR_CHECK_STATUS(NULL != supp, "tivxGetSupplementaryDataObjDesc fails when there is no supplementary data");
    supp = tivxGetSupplementaryDataObjDesc(obj_desc[0], NULL);
    ERROR_CHECK_STATUS(NULL == supp,"tivxGetSupplementaryDataObjDesc success with NULL type name");
    supp = tivxGetSupplementaryDataObjDesc(obj_desc[1], "user_data_t");
    ERROR_CHECK_STATUS(NULL == supp, "tivxGetSupplementaryDataObjDesc success with correct type name from child object");
    supp = tivxGetSupplementaryDataObjDesc(obj_desc[0], "user_data_t");
    ERROR_CHECK_STATUS(NULL == supp, "tivxGetSupplementaryDataObjDesc success with correct type name");
    ERROR_EXPECT_STATUS(tivxSetSupplementaryDataObjDesc(obj_desc[3], NULL), VX_ERROR_INVALID_REFERENCE, "tivxGetSupplementaryDataObjDesc returns VX_ERROR_INVALID_REFERENCE when passed NULL source");
    ERROR_EXPECT_STATUS(tivxSetSupplementaryDataObjDesc(NULL, supp), VX_ERROR_INVALID_REFERENCE, "tivxGetSupplementaryDataObjDesc returns VX_ERROR_INVALID_REFERENCE when passed NULL destination");
    ERROR_EXPECT_STATUS(tivxSetSupplementaryDataObjDesc(obj_desc[1], supp), VX_FAILURE, "tivxGetSupplementaryDataObjDesc returns VX_FAILURE when destination has no supplementary data even though parent has");
    ERROR_EXPECT_STATUS(tivxSetSupplementaryDataObjDesc(obj_desc[2], supp), VX_FAILURE, "tivxGetSupplementaryDataObjDesc returns VX_FAILURE when destination has no supplementary data");
    ERROR_EXPECT_STATUS(tivxSetSupplementaryDataObjDesc(obj_desc[3], supp), VX_SUCCESS, "tivxGetSupplementaryDataObjDesc returns VX_SUCCESS with correct parameters");
    supp->type_name[0]++;
    ERROR_EXPECT_STATUS(tivxSetSupplementaryDataObjDesc(obj_desc[3], supp), VX_ERROR_INVALID_TYPE, "tivxGetSupplementaryDataObjDesc returns VX_ERROR_INVALID_TYPE when type does not match");
    ERROR_EXPECT_STATUS(tivxExtendSupplementaryDataObjDesc(obj_desc[3], supp, &test_data, sizeof(test_data)), VX_ERROR_INVALID_TYPE, "tivxExtendSupplementaryDataObjDesc returns VX_ERROR_INVALID_TYPE when type does not match");
    supp->type_name[0]--;
    supp->valid_mem_size = sizeof(test_data) / 2;
    ERROR_EXPECT_STATUS(tivxExtendSupplementaryDataObjDesc(NULL, supp, &test_data, sizeof(test_data)), VX_ERROR_INVALID_REFERENCE, "tivxExtendSupplementaryDataObjDesc returns VX_ERROR_INVALID_REFERENCE when destination is NULL");
    ERROR_EXPECT_STATUS(tivxExtendSupplementaryDataObjDesc(obj_desc[3], NULL, NULL, sizeof(test_data)), VX_ERROR_INVALID_PARAMETERS, "tivxExtendSupplementaryDataObjDesc returns VX_ERROR_INVALID_REFERENCE when source and user_data are NULL");
    ERROR_EXPECT_STATUS(tivxExtendSupplementaryDataObjDesc(obj_desc[3], supp, &test_data, sizeof(test_data) + 1), VX_ERROR_INVALID_VALUE, "tivxExtendSupplementaryDataObjDesc returns VX_ERROR_INVALID_REFERENCE when size is too large");
    ERROR_EXPECT_STATUS(tivxExtendSupplementaryDataObjDesc(obj_desc[2], supp, &test_data, sizeof(test_data)), VX_FAILURE, "tivxExtendSupplementaryDataObjDesc returns VX_FAILURE when destination has no supplementary data");
    ERROR_EXPECT_STATUS(tivxExtendSupplementaryDataObjDesc(obj_desc[3], supp, &test_data, 3 * sizeof(test_data) / 4), VX_SUCCESS, "tivxExtendSupplementaryDataObjDesc returns VX_SUCCESS with correct parameters");
    supp->valid_mem_size = sizeof(test_data);
    supp = tivxGetSupplementaryDataObjDesc(obj_desc[3], NULL);
    ERROR_EXPECT_STATUS(supp->valid_mem_size, (vx_uint32)(3U * sizeof(test_data) / 4U), "tivxExtendSupplementaryDataObjDesc sets valid_mem_size correctly");
    if (VX_SUCCESS == tivxMemBufferMap((void *)(uintptr_t)(supp->mem_ptr.host_ptr ), supp->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY))
    {
        tivx_obj_desc_memcpy((void *)&test_data, (void *)(uintptr_t)(supp->mem_ptr.host_ptr), supp->mem_size);
        tivxMemBufferUnmap((void *)(uintptr_t)(supp->mem_ptr.host_ptr), supp->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        ERROR_CHECK_STATUS(test_data.numbers[0] != 1 ||
                           test_data.numbers[1] != 2 ||
                           test_data.numbers[2] != 70, "tivxExtendSupplementaryDataObjDesc copies correct data");
    }
    else
    {
        FAIL("ERROR - Could not map the supplementary data!");
    }
    return VX_SUCCESS;
}

void tivxAddTargetKernelGeneral(vx_context context)
{
    /* section to remove */
    target_kernel = tivxAddTargetKernel(MY_USER_TARGET_KERNEL, TIVX_TARGET_HOST, myGeneralKernelFunction, myGeneralKernelCreate, myGeneralKernelDelete, NULL, NULL);
    general_kernel = vxAddUserKernel(context, "myUserTargetKernel", MY_USER_TARGET_KERNEL, NULL, 2, myGeneralKernelValidator, NULL, NULL);
    EXPECT_VX_OBJECT(general_kernel, VX_TYPE_KERNEL);
    ERROR_CHECK_STATUS(tivxAddKernelTarget(general_kernel, TIVX_TARGET_HOST), NULL);;
    ERROR_CHECK_STATUS(vxAddParameterToKernel(general_kernel, 0, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(general_kernel, 1, VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxFinalizeKernel(general_kernel), NULL);
    /* end of section to remove */
    target_test_kernel = tivxAddTargetKernel(MY_USER_TEST_TARGET_KERNEL, TIVX_TARGET_HOST, myUserTestTargetFunction, myGeneralKernelCreate, myGeneralKernelDelete, NULL, NULL);
    general_test_kernel = vxAddUserKernel(context, "myUserTestTargetKernel", MY_USER_TEST_TARGET_KERNEL, NULL, 4, myGeneralKernelValidator, NULL, NULL);
    EXPECT_VX_OBJECT(general_test_kernel, VX_TYPE_KERNEL);
    ERROR_CHECK_STATUS(tivxAddKernelTarget(general_test_kernel, TIVX_TARGET_HOST), NULL);;
    ERROR_CHECK_STATUS(vxAddParameterToKernel(general_test_kernel, 0, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(general_test_kernel, 1, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(general_test_kernel, 2, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(general_test_kernel, 3, VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxFinalizeKernel(general_test_kernel), NULL);

    user_test_kernel = vxAddUserKernel(context, "myUserTestKernel", MY_USER_TEST_KERNEL, myUserTestKernelFunction, 4, myUserTestKernelValidator, NULL, NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(user_test_kernel, 0, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(user_test_kernel, 1, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(user_test_kernel, 2, VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(user_test_kernel, 3, VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxFinalizeKernel(user_test_kernel), NULL);

    my_dumb_copy_kernel = vxAddUserKernel(context, "MY_DUMB_IMAGE_COPY", MY_DUMB_IMAGE_COPY_KERNEL, myDumbImageCopyKernel, 2, myDumbImageCopyValidator, NULL, NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(my_dumb_copy_kernel, 0, VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxAddParameterToKernel(my_dumb_copy_kernel, 1, VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED), NULL);
    ERROR_CHECK_STATUS(vxFinalizeKernel(my_dumb_copy_kernel), NULL);
}

void tivxRemoveTargetKernelGeneral(void)
{
    if (target_kernel != NULL)
    {
        if (VX_SUCCESS == tivxRemoveTargetKernel(target_kernel))
        {
            target_kernel = NULL;
        }
    }
    vxRemoveKernel(general_kernel);
    if (target_test_kernel != NULL)
    {
        if (VX_SUCCESS == tivxRemoveTargetKernel(target_test_kernel))
        {
            target_test_kernel = NULL;
        }
    }
    vxRemoveKernel(general_test_kernel);
    vxRemoveKernel(user_test_kernel);
    vxRemoveKernel(my_dumb_copy_kernel);
}

/* Write values into appropriate places of the given reference & type 
  Supports just the supplementary data types scalar, array, user data*/
vx_status writeValues(vx_reference ref, vx_enum type, vx_uint8 a, vx_uint8 b)
{
    vx_status status = VX_SUCCESS;
    switch (type)
    {
        case VX_TYPE_ARRAY:
        {
            vx_uint8 items[4] = {a, b, b, a};
            vx_size n = 0;
            ERROR_CHECK_STATUS(vxQueryArray((vx_array)ref, VX_ARRAY_NUMITEMS, &n, sizeof(n)), NULL);
            if (n)
            {
                (void)vxTruncateArray((vx_array)ref, 0);
            }
            status = vxAddArrayItems((vx_array)ref, 4, items, sizeof(vx_uint8));
            break;
        }
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = (a << 16) + b;
            status = vxCopyScalar((vx_scalar)ref, &value, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data = {.numbers = {a, b, b, a}};
            status = vxCopyUserDataObject((vx_user_data_object)ref, 0, sizeof(user_data_t), &user_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            break;
        }
        default:
            printf("ERROR in TEST! Found unexpected type %d (%s)\n", type, enumToString(type));
    }
    return status;
}

/* check values in a reference of the given type  
  Supports just the supplementary data types scalar, array, user data*/
vx_status checkValues(vx_reference ref, vx_enum type, vx_uint8 a, vx_uint8 b)
{
    vx_status status = VX_SUCCESS;
    vx_enum reftype = VX_TYPE_INVALID;
    ERROR_CHECK_STATUS(vxQueryReference(ref, VX_REFERENCE_TYPE, &reftype, sizeof(reftype)), NULL);
    if (reftype != type)
    {
        printf("Found reference type %s when %s was expected\n", enumToString(reftype), enumToString(type));
        status =  VX_FAILURE;
    }
    else switch (type)
    {
        case VX_TYPE_ARRAY:
        {
            vx_uint8 items[4];
            status = vxCopyArrayRange((vx_array)ref, 0, 4, sizeof(vx_uint8), &items, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status)
            {
                if ((items[0] != a) ||
                    (items[1] != b) ||
                    (items[2] != b) ||
                    (items[3] != a))
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = 0;
            status = vxCopyScalar((vx_scalar)ref, &value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status && ((a << 16) + b) != value)
            {
                status = VX_FAILURE;
            }
            break;
        }
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data;
            status = vxCopyUserDataObject((vx_user_data_object)ref, 0, sizeof(user_data_t), &user_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS == status)
            {
                if ((user_data.numbers[0] != a) ||
                    (user_data.numbers[1] != b) ||
                    (user_data.numbers[2] != b) ||
                    (user_data.numbers[3] != a))
                {
                    status = VX_FAILURE;
                }
            }
            break;
        }
        default:
            printf("ERROR in TEST! Found unexpected type %d (%s)\n", type, enumToString(type));
    }
    return status;
}

/* Create a non-virtual example reference of the given type */
vx_reference createReference(vx_context context, vx_enum type)
{
    vx_reference ref = NULL;
    switch (type)
    {
        case VX_TYPE_ARRAY:
            ref = (vx_reference)vxCreateArray(context, VX_TYPE_UINT8, 4);
            break;
        case VX_TYPE_CONVOLUTION:
            ref = (vx_reference)vxCreateConvolution(context, 3, 3);
            break;;
        case VX_TYPE_DISTRIBUTION:
            ref = (vx_reference)vxCreateDistribution(context, 4, 0, 256);
            break;
        case VX_TYPE_IMAGE:
            ref = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            break;
        case VX_TYPE_LUT:
            ref = (vx_reference)vxCreateLUT(context, VX_TYPE_UINT8, 4);
            break;
        case VX_TYPE_MATRIX:
            ref = (vx_reference)vxCreateMatrix(context, VX_TYPE_INT32, 3, 3);
            break;
        case VX_TYPE_OBJECT_ARRAY:
        {
            vx_image image = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8);
            ref = (vx_reference)vxCreateObjectArray(context, (vx_reference)image, 4);
            vxReleaseImage(&image);
            break;
        }
        case VX_TYPE_PYRAMID:
            ref = (vx_reference)vxCreatePyramid(context, 2, VX_SCALE_PYRAMID_HALF, 16, 16, VX_DF_IMAGE_U8);
            break;
        case VX_TYPE_SCALAR:
        {
            vx_uint32 value = 0;
            ref = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &value);
            break;
        }
        case VX_TYPE_TENSOR:
        {
            vx_size dims[3] = {4, 4, 4};
            ref = (vx_reference)vxCreateTensor(context, 3, dims, VX_TYPE_INT32, 0);
            break;
        }
        case VX_TYPE_USER_DATA_OBJECT:
        {
            user_data_t user_data = {0};
            ref = (vx_reference)vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
            break;
        }
        case VX_TYPE_REMAP:
            ref = (vx_reference)vxCreateRemap(context, 16, 16, 16, 16);
            break;
        case VX_TYPE_THRESHOLD:
            ref = (vx_reference)vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8); /* This will need updating!!)*/
            break;
        case TIVX_TYPE_RAW_IMAGE:
            {
                tivx_raw_image_create_params_t params = {
                    .width = 16,
                    .height = 16,
                    .line_interleaved = vx_false_e,
                    .meta_height_before = 1,
                    .meta_height_after = 1,
                    .num_exposures = 2,
                    .format = {
                        {.msb = 7, .pixel_container = TIVX_RAW_IMAGE_8_BIT},
                        {.msb = 7, .pixel_container = TIVX_RAW_IMAGE_8_BIT},
                        {.msb = 7, .pixel_container = TIVX_RAW_IMAGE_8_BIT}
                    }
                };
                ref = (vx_reference)tivxCreateRawImage(context, &params);
                break;
            }
        default:
            printf("ERROR in TEST! Found unknown type %d (%s)\n", type, enumToString(type));
    }
    return ref;
}

TEST (supplementary_data, testRefCount)
{
    vx_context context = context_->vx_context_;

    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_status status = (vx_status)VX_SUCCESS;

    /* First, set supplementary data on an object */
    vx_reference test_object = (vx_image)vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject(test_object, exemplar));
    VX_CALL(vxReleaseUserDataObject(&exemplar));

    vx_reference test_object_copy = test_object;
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxRetainReference(test_object_copy));
    vx_reference supp_data = (vx_reference)(vxGetSupplementaryUserDataObject(test_object, NULL, &status));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    vx_reference supp_data_copy = (vx_reference)(vxGetSupplementaryUserDataObject(test_object, NULL, &status));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    vx_uint32 ref_count1, ref_count2;
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryReference(supp_data, VX_REFERENCE_COUNT, &ref_count1, sizeof(ref_count1)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&test_object_copy));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryReference(supp_data_copy, VX_REFERENCE_COUNT, &ref_count2, sizeof(ref_count2)));
    /* Supplementary data reference count is not affected by decrementing release count of owner above zero */
    EXPECT_EQ_VX_STATUS(ref_count2, ref_count1);
    
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&test_object));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryReference(supp_data, VX_REFERENCE_COUNT, &ref_count1, sizeof(ref_count1)));
    /* Supplementary data reference count is affected by decrementing release count of owner to zero*/
    EXPECT_NE_VX_STATUS(ref_count2, ref_count1);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&supp_data));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryReference(supp_data_copy, VX_REFERENCE_COUNT, &ref_count2, sizeof(ref_count2)));
    /* Supplementary data reference count equal to one before final release*/
    EXPECT_NE_VX_STATUS(ref_count2, 1);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&supp_data_copy));
}

/* Test creation getting and setting and getting again of supplementary data for one type */
void testOneType(vx_context context, vx_enum type)
{
    vx_reference ref = createReference(context, type);
    vx_status status = VX_SUCCESS;
    vx_user_data_object supp = vxGetSupplementaryUserDataObject(ref, NULL, &status);
    if ( status != (vx_status)VX_SUCCESS)
    {
        printf("No supplementary data retrieved for type %s \n", enumToString(type));
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    status = vxSetSupplementaryUserDataObject(ref, exemplar);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("vxSetSupplementaryUserDataObject for type %s \n", enumToString(type));
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseUserDataObject(&exemplar));

    supp = vxGetSupplementaryUserDataObject(ref, NULL, &status);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("vxGetSupplementaryUserDataObject for type %s \n", enumToString(type));
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);    

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp, 0, sizeof(user_data.numbers[0]), &user_data.numbers[3], VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if (user_data.numbers[3] != user_data.numbers[0])
    {
        printf("Correct value read from supplementary data for type %s", enumToString(type));
        status = (vx_status)VX_FAILURE;
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseUserDataObject(&supp));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseReference(&ref));
}

TEST(supplementary_data, testAllGetSet)
{
    vx_uint32 i;
    vx_context context = context_->vx_context_;
    printf("\nTesting Creation and retrieval of supplementary data for all supported types\n");
    for (i = 0; i < NUM_TYPES; ++i)
    {
        testOneType(context, type_ids[i]);
    }
}

TEST(supplementary_data, testInvalidTypes)
{
    /* This is a negative test for types that should never ever be supported: vx_context, vx_graph
       Basically it covers some error code in the implementation
    */
    vx_context context = context_->vx_context_;
    printf("\nNegative tests for disallowed types\n");

    vx_graph graph = vxCreateGraph(context);
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_status status = VX_SUCCESS;
    vx_user_data_object supp = vxGetSupplementaryUserDataObject((vx_reference)context, NULL, &status);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("Cannot get supplementary data from context\n");
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    
    status = vxSetSupplementaryUserDataObject((vx_reference)context, exemplar);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("Cannot set supplementary data on context\n");
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    supp = vxGetSupplementaryUserDataObject((vx_reference)graph, NULL, &status);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("Cannot get supplementary data from graph\n");
    }    
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    status = vxSetSupplementaryUserDataObject((vx_reference)graph, exemplar);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("Cannot set supplementary data on graph\n");
    }     
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseUserDataObject(&exemplar));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseGraph(&graph));
}

/* test get and set error conditions */
TEST(supplementary_data, testGetSetErrors)
{
    vx_context context = context_->vx_context_;
    printf("\nChecking error codes for vxGetSupplementaryUserDataObject and vxSetSupplementaryUserDataObject\n");
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    /* First, set supplementary data on an object */
    vx_image image = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)image, exemplar));

    /* Now, check that we can't retrieve from an invalid reference */
    vx_status status = VX_SUCCESS;
    vx_user_data_object supp = vxGetSupplementaryUserDataObject((vx_reference)&user_data, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, status);

    /* Now, check for virtual objects */
    vx_graph graph = vxCreateGraph(context);
    vx_image virtualImage = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8);
    supp = vxGetSupplementaryUserDataObject((vx_reference)virtualImage, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_ERROR_OPTIMIZED_AWAY, status);

    /* Check for VX_FAILURE if no supplementary data*/
    supp = vxGetSupplementaryUserDataObject((vx_reference)exemplar, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_FAILURE, status);

    /* Check for invalid type */
    supp = vxGetSupplementaryUserDataObject((vx_reference)image, "This is the wrong type", &status);
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_TYPE, status);

    /* Check for NULL */
    supp = vxGetSupplementaryUserDataObject(NULL, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, status);

    /* Check for NULL status */
    status = VX_ERROR_INVALID_DIMENSION;
    supp = vxGetSupplementaryUserDataObject(NULL, NULL, NULL);
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_DIMENSION, status);

    supp = vxGetSupplementaryUserDataObject((vx_reference)image, "user_data_t", &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    /* Now check vxSetSupplementaryUserDataObject */
    vx_user_data_object wrong_data =  vxCreateUserDataObject(context, "wrong data", sizeof(user_data_t), &user_data);
    vx_user_data_object invalid_user_data_object = (vx_user_data_object)image;
    vx_reference invalid_reference = (vx_reference)&user_data;
    /* Source not a vx_user_data_object" */
    status = vxSetSupplementaryUserDataObject((vx_reference)image, invalid_user_data_object);
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, status);

    /* Destination not a vx_reference */
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetSupplementaryUserDataObject(invalid_reference, exemplar));
    /* Types do not match */
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_TYPE, vxSetSupplementaryUserDataObject((vx_reference)(image), wrong_data));
     /* Destination must not be supplementary data of something else */
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetSupplementaryUserDataObject((vx_reference)(supp), exemplar));
    /* Supplementary data of virtual images is inaccessible */
    EXPECT_EQ_VX_STATUS(VX_ERROR_OPTIMIZED_AWAY, vxSetSupplementaryUserDataObject((vx_reference)(virtualImage), exemplar));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)(exemplar), exemplar));
    /* Source must not have supplementary data itself */
    EXPECT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxSetSupplementaryUserDataObject((vx_reference)(image), exemplar));
    /* Correct action for NULL destination */
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetSupplementaryUserDataObject(NULL, exemplar));
    /* Correct action for NULL source */
    EXPECT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxSetSupplementaryUserDataObject((vx_reference)image, NULL));
    VX_CALL(vxReleaseUserDataObject(&wrong_data));
    VX_CALL(vxReleaseUserDataObject(&supp));
    VX_CALL(vxReleaseUserDataObject(&exemplar));
    VX_CALL(vxReleaseImage(&virtualImage));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseImage(&image));
}

/* Test inherited data from parent.
  Should be possible to get supplementary data from parent and from child.
  Changing data in parent should see change reflected in child
  Should not be possible to change supplementary data from child
  Setting new supplementary data on child will not affect parent
  After this operation they are independent
  */
void testParentAndChild(vx_reference parent, vx_reference child, const char * msg)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object parent_data = vxGetSupplementaryUserDataObject(parent, "user_data_t", &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    vx_user_data_object child_data = vxGetSupplementaryUserDataObject(child, "user_data_t", &status);
    if ((vx_status)VX_SUCCESS != status)
    {
        printf("Can't get supplementary data from %s \n", msg);
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    user_data_t test1 = {.numbers={9, 8, 7, 6}};
    user_data_t orig;
    user_data_t test2 = {.numbers={0}};
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(parent_data, 0, sizeof(orig), &orig, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(parent_data, 0, sizeof(test1), &test1, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(child_data, 0, sizeof(test2), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if (test1.numbers[2] == test2.numbers[2])
    {
      printf("Changing parent data doe not affects data for %s \n", msg);
      status = (vx_enum) VX_FAILURE;
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    /* Should not be able to copy to the child supplementary data! */
    EXPECT_NE_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(child_data, 0, sizeof(test2), &test2, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    /* return parent to original data */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(parent_data, 0, sizeof(orig), &orig, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    /* Now actually set the child user data object & re-get the supplementary data */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject(child, child_data));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseUserDataObject(&child_data));
    child_data = vxGetSupplementaryUserDataObject(child, "user_data_t", &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    test2.numbers[2] = 45;
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(child_data, 0, sizeof(test2), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if (test1.numbers[2] == test2.numbers[2])
    {
      printf("Can not read data back correctly for %s \n", msg);
      status = (vx_enum) VX_FAILURE;
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    /* Should be able to copy to it now */
    status = vxCopyUserDataObject(child_data, 0, sizeof(test1), &test1, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    if ((vx_enum)VX_SUCCESS != status)
    {
        printf("Can not write to the new supplementary data object for %s \n", msg);
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    test2.numbers[2] = 45;
    status = vxCopyUserDataObject(child_data, 0, sizeof(test1), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    if (test1.numbers[2] == test2.numbers[2])
    {
        printf("Can not read changed data back correctly for %s \n", msg);
        status = (vx_enum) VX_FAILURE;
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    test2.numbers[2] = 45;
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(parent_data, 0, sizeof(test2), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if (orig.numbers[2] == test2.numbers[2])
    {
        printf("changed data does affect parent for%s \n", msg);
        status = (vx_enum) VX_FAILURE;
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    VX_CALL(vxReleaseUserDataObject(&child_data));
    VX_CALL(vxReleaseUserDataObject(&parent_data));
}

/* Test child image of image, child tensor of image, child image of tensor, child tensor of tensor, grandchild image */
TEST(supplementary_data, testChildren)
{
    vx_context context = context_->vx_context_;
    vx_status status = VX_SUCCESS;
    printf("\nTesting inheritance for images and tensors\n");
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_size dims[] = {10, 10, 3};
    vx_tensor tensor = vxCreateTensor(context, 3, dims, VX_TYPE_UINT8, 0);
    vx_image image = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = 9, .end_y = 9};
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    vx_image imageFromROI = vxCreateImageFromROI(image, &rect);
    vx_image grandchildImageFromImage = vxCreateImageFromROI(imageFromROI, &rect);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    /* That's all the objects created, they do not have supplementary data, add it to the parents */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)tensor, exemplar));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)image, exemplar));
    /* Now all children and grand children should have read-only supplementary data from the parent, check it all */
    testParentAndChild((vx_reference)image, (vx_reference)grandchildImageFromImage, "image created from child of image");
    testParentAndChild((vx_reference)image, (vx_reference)imageFromROI, "image created from image");
    VX_CALL(vxReleaseUserDataObject(&exemplar));
    VX_CALL(vxReleaseImage(&image));
    VX_CALL(vxReleaseImage(&grandchildImageFromImage));
    VX_CALL(vxReleaseImage(&imageFromROI));
    VX_CALL(vxReleaseTensor(&tensor));
}

vx_node dumbCopyNode(vx_graph graph, vx_image in, vx_image out)
{
    vx_node node = vxCreateGenericNode(graph, my_dumb_copy_kernel);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 0, (vx_reference)in));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetParameterByIndex(node, 1, (vx_reference)out));
    return node;
}

/* Test object arrays, delays, pipelined interstitial objects */
TEST(supplementary_data, testExemplars)
{
    vx_context context = context_->vx_context_;
    printf("\nTesting exemplars\n");
    vx_status status = VX_SUCCESS;
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_image images[] = {
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8),
        vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8)
    };
    int i;
    for (i = 0; i < 4; ++i)
    {
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_image)images[i], exemplar));
    }
    vx_object_array array = vxCreateObjectArray(context, (vx_reference)(images[0]), 3);
    /* Each of the objects should have been created with supplementary data from the exemplar */
    status = VX_SUCCESS;
    for (i = 0; (i < 3) && ((vx_enum)VX_SUCCESS == status); ++i)
    {
        vx_reference ref = vxGetObjectArrayItem(array, i);
        vx_user_data_object supp = vxGetSupplementaryUserDataObject(ref, "user_data_t", &status);
        user_data_t read_data = {0};
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp, 0, sizeof(read_data), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        if (read_data.numbers[3] != user_data.numbers[3])
        {
            status = VX_ERROR_INVALID_VALUE;
        }
        VX_CALL(vxReleaseReference(&ref));
        VX_CALL(vxReleaseUserDataObject(&supp));
    }
    /* All object array items created and initialised with supplementary data */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    VX_CALL(vxReleaseObjectArray(&array));
    
    vx_delay delay = vxCreateDelay(context, (vx_reference)(images[0]), 3);
    /* Each of the objects should have been created with supplementary data from the exemplar */
    status = (vx_enum)VX_SUCCESS;
    for (i = 0; (i < 3) && ((vx_enum)VX_SUCCESS == status); ++i)
    {
        vx_reference ref = vxGetReferenceFromDelay(delay, i);
        vx_user_data_object supp = vxGetSupplementaryUserDataObject(ref, "user_data_t", &status);
        user_data_t read_data = {0};
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp, 0, sizeof(read_data), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        if (read_data.numbers[3] != user_data.numbers[3])
        {
            status = VX_ERROR_INVALID_VALUE;
        }
        VX_CALL(vxReleaseUserDataObject(&supp));
    }
     /* All delay slots created and initialised with supplementary data */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    
    VX_CALL(vxReleaseDelay(&delay));
    /* Now check that objects are correctly created as required for pipelined or batched graphs */
    vx_graph graph = vxCreateGraph(context);
    vx_image virt = vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8);
    vx_node node1 = dumbCopyNode(graph, images[0], virt);
    vx_node node2 = dumbCopyNode(graph, virt, images[2]);
    addParameterToGraph(graph, node1, 0);
    addParameterToGraph(graph, node2, 1);
    vx_graph_parameter_queue_params_t q_params[2] = {
        {.graph_parameter_index = 0, .refs_list = (vx_reference*)(&images[0]), .refs_list_size = 2},
        {.graph_parameter_index = 1, .refs_list = (vx_reference*)(&images[2]), .refs_list_size = 2}
    };
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetGraphScheduleConfig(graph, VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL, 2, q_params));
    user_data_t test2 = {.numbers = {101, 102, 103, 104}};
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(exemplar, 0, sizeof(test2), &test2, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_reference)(images[1]), exemplar));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, 0, q_params[0].refs_list, q_params[0].refs_list_size));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterEnqueueReadyRef(graph, 1, q_params[1].refs_list, q_params[1].refs_list_size));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxScheduleGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxWaitGraph(graph));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, 0, q_params[0].refs_list, q_params[0].refs_list_size, &q_params[0].refs_list_size));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGraphParameterDequeueDoneRef(graph, 1, q_params[1].refs_list, q_params[1].refs_list_size, &q_params[1].refs_list_size));
    VX_CALL(vxReleaseUserDataObject(&exemplar));
    exemplar = vxGetSupplementaryUserDataObject((vx_reference)(images[3]), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(exemplar, 0, sizeof(user_data) / 4, &user_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    VX_CALL(vxReleaseUserDataObject(&exemplar));
    exemplar = vxGetSupplementaryUserDataObject((vx_reference)(images[2]), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(exemplar, sizeof(user_data) / 4, sizeof(user_data) / 4, &user_data.numbers[1], VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if ((user_data.numbers[0] + user_data.numbers[1]) != 103)
    {
        printf("Virtual interstitial node not correctly created and initialised \n");
        status = VX_FAILURE;
    }

    /*Special check for object arrays of pyramids. The supplementary data of each level should be created*/
    vx_pyramid pyr = vxCreatePyramid(context, 3, VX_SCALE_PYRAMID_HALF, 64, 64, VX_DF_IMAGE_U8);
    EXPECT_VX_OBJECT(pyr, VX_TYPE_PYRAMID);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_pyramid)pyr, exemplar));
    for (i = 0; i < 3; ++i)
    {
        vx_image img = vxGetPyramidLevel(pyr, i);
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxSetSupplementaryUserDataObject((vx_image)img, exemplar));
        VX_CALL(vxReleaseImage(&img));
    }
    array = vxCreateObjectArray(context, (vx_pyramid)pyr, 3);
    EXPECT_VX_OBJECT(array, VX_TYPE_ARRAY);
    /* Each of the pyramids should have been created with supplementary data at each of their levels as well as in the pyramid */
    status = VX_SUCCESS;
    for (i = 0; (i < 3) && ((vx_enum)VX_SUCCESS == status); ++i)
    {
        vx_reference ref = vxGetObjectArrayItem(array, i);
        EXPECT_VX_REFERENCE(ref);
        vx_user_data_object supp = vxGetSupplementaryUserDataObject(ref, "user_data_t", &status);
        user_data_t read_data = {0};
        EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp, 0, sizeof(read_data), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        if (read_data.numbers[3] != user_data.numbers[3])
        {
            status = VX_ERROR_INVALID_VALUE;
        }
        for (int j = 0; (j < 3) && ((vx_enum)VX_SUCCESS == status); ++j)
        {
            vx_image img = vxGetPyramidLevel((vx_pyramid)ref, j);
            EXPECT_VX_OBJECT(img, VX_TYPE_IMAGE);
            vx_user_data_object supp = vxGetSupplementaryUserDataObject((vx_reference)img, "user_data_t", &status);
            user_data_t read_data = {0};
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp, 0, sizeof(read_data), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
            if (read_data.numbers[3] != user_data.numbers[3])
            {
                status = VX_ERROR_INVALID_VALUE;
            }
            VX_CALL(vxReleaseImage(&img));
            VX_CALL(vxReleaseUserDataObject(&supp));
        }
        VX_CALL(vxReleaseReference(&ref));
        VX_CALL(vxReleaseUserDataObject(&supp));
    }
    /* All object array pyramids and levels created and initialised with supplementary data */
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    VX_CALL(vxReleasePyramid(&pyr));
    VX_CALL(vxReleaseObjectArray(&array));

    for (i = 0; i < 4; ++i)
        VX_CALL(vxReleaseImage(&images[i]));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseUserDataObject(&exemplar));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));
}

#if 0
/* Test Copy, Swap, Move */
void testCopySwapMove(vx_context context)
{
    printf("\nTesting copy and swap\n");
    vx_status status = VX_SUCCESS;
    user_data_t user_data1 = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar1 = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data1);
    user_data_t user_data2 = {.numbers = {11, 12, 13, 14}};
    vx_user_data_object exemplar2 = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data2);
    vx_matrix matrix1 = vxCreateMatrixFromPattern(context, VX_PATTERN_BOX, 3, 3);
    vx_matrix matrix2 = vxCreateMatrixFromPattern(context, VX_PATTERN_BOX, 3, 3);
    
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject(vxCastFromMatrix(matrix1), exemplar1), NULL);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject(vxCastFromMatrix(matrix2), exemplar2), NULL);

    vx_user_data_object supp1 = vxGetSupplementaryUserDataObject(vxCastFromMatrix(matrix1), NULL, &status);
    ERROR_CHECK_STATUS(status , NULL);
    vx_user_data_object supp2 = vxGetSupplementaryUserDataObject(vxCastFromMatrix(matrix2), NULL, &status);
    ERROR_CHECK_STATUS(status , NULL);
    user_data_t test1 = {0};
    user_data_t test2 = {0};
    /* Now try a swap */
    ERROR_CHECK_STATUS(vxuSwap(context, vxCastFromMatrix(matrix1), vxCastFromMatrix(matrix2)), NULL);
    /* Now if we look at the supplementary data it should be the other way around */
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data swapped when objects swapped");
    /* Now try a Copy */
    ERROR_CHECK_STATUS(vxuCopy(context, vxCastFromMatrix(matrix1), vxCastFromMatrix(matrix2)), NULL);
    /* And now supp2 should have the same data as supp1, i.e. user_data_2*/
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test2.numbers[i] != user_data2.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data copied when objects copied");
    
    /* Now test copy & swap in a graph */
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &user_data1, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST), NULL);

    vx_graph graph = vxCreateGraph(context);
    vx_node node = vxSwapNode(graph, vxCastFromMatrix(matrix1), vxCastFromMatrix(matrix2));
    ERROR_CHECK_STATUS(vxProcessGraph(graph), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data swapped when objects swapped in a graph");
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    graph = vxCreateGraph(context);
    node = vxCopyNode(graph, vxCastFromMatrix(matrix1), vxCastFromMatrix(matrix2));
    ERROR_CHECK_STATUS(vxProcessGraph(graph), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test2.numbers[i] != user_data2.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data copied when objects copied in a graph");
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    vxReleaseUserDataObject(&supp1);
    vxReleaseUserDataObject(&supp2);
    vxReleaseMatrix(&matrix1);
    vxReleaseMatrix(&matrix2);

    /* Now check object arrays of pyramids */
    vx_pyramid pyr_ex1 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 32, 32, VX_DF_IMAGE_U8);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject(vxCastFromPyramid(pyr_ex1), exemplar1), NULL);
    for (int i = 0; i < 4; ++i)
    {
        vx_image img = vxGetPyramidLevel(pyr_ex1, i);
        vxSetSupplementaryUserDataObject((vx_reference)(img), exemplar1);
        vxReleaseImage(&img);
    }
    vx_object_array arr1 = vxCreateObjectArray(context, vxCastFromPyramid(pyr_ex1), 4);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject(vxCastFromObjectArray(arr1), exemplar1), NULL);
    vx_pyramid pyr_ex2 = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 32, 32, VX_DF_IMAGE_U8);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject(vxCastFromPyramid(pyr_ex2), exemplar2), NULL);
    for (int i = 0; i < 4; ++i)
    {
        vx_image img = vxGetPyramidLevel(pyr_ex2, i);
        vxSetSupplementaryUserDataObject((vx_reference)(img), exemplar2);
        vxReleaseImage(&img);
    }
    vx_object_array arr2 = vxCreateObjectArray(context, vxCastFromPyramid(pyr_ex2), 4);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject(vxCastFromObjectArray(arr2), exemplar2), NULL);
    supp1 = vxGetSupplementaryUserDataObject(vxCastFromObjectArray(arr1), NULL, &status);
    ERROR_CHECK_STATUS(status , NULL);
    supp2 = vxGetSupplementaryUserDataObject(vxCastFromObjectArray(arr2), NULL, &status);
    ERROR_CHECK_STATUS(status , NULL);
    /* Now try a swap */
    ERROR_CHECK_STATUS(vxuSwap(context, vxCastFromObjectArray(arr1), vxCastFromObjectArray(arr2)), NULL);
    /* Now if we look at the supplementary data it should be the other way around */
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data swapped when object arrays swapped");

    /* now check the supplementary data of an object in the array */
    vx_reference ref1 = vxGetObjectArrayItem(arr1, 2);
    vx_reference ref2 = vxGetObjectArrayItem(arr2, 2);
    ERROR_CHECK_STATUS(vxReleaseUserDataObject(&supp1), NULL);
    ERROR_CHECK_STATUS(vxReleaseUserDataObject(&supp2), NULL);
    status = VX_SUCCESS;
    supp1 = vxGetSupplementaryUserDataObject(ref1, NULL, &status);
    supp2 = vxGetSupplementaryUserDataObject(ref2, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data of array element swapped when object arrays swapped");

    /* now check the supplementary data of a level of a pyramid in the array */
    vx_reference ref3 = (vx_reference)(vxGetPyramidLevel(vxCastAsPyramid(ref1, &status), 2));
    vx_reference ref4 = (vx_reference)(vxGetPyramidLevel(vxCastAsPyramid(ref2, &status), 2));
    ERROR_CHECK_STATUS(vxReleaseUserDataObject(&supp1), NULL);
    ERROR_CHECK_STATUS(vxReleaseUserDataObject(&supp2), NULL);
    status = VX_SUCCESS;
    supp1 = vxGetSupplementaryUserDataObject(ref3, NULL, &status);
    supp2 = vxGetSupplementaryUserDataObject(ref4, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    ERROR_CHECK_STATUS(status, "Supplementary data of pyramid level swapped when object arrays of pyramids swapped");
    ERROR_CHECK_STATUS(vxReleaseReference(&ref1), NULL);
    ERROR_CHECK_STATUS(vxReleaseReference(&ref2), NULL);
    ERROR_CHECK_STATUS(vxReleaseReference(&ref3), NULL);
    ERROR_CHECK_STATUS(vxReleaseReference(&ref4), NULL);
    ERROR_CHECK_STATUS(vxReleaseUserDataObject(&supp1), NULL);
    ERROR_CHECK_STATUS(vxReleaseUserDataObject(&supp2), NULL);
    vxReleasePyramid(&pyr_ex1);
    vxReleasePyramid(&pyr_ex2);
    vxReleaseObjectArray(&arr1);
    vxReleaseObjectArray(&arr2);
    vxReleaseUserDataObject(&exemplar1);
    vxReleaseUserDataObject(&exemplar2);
}

/* Test vxExtendSupplementaryUserDataObject
   Check operation with source NULL, user_data NULL
   Different values for byte counts, starting with
   no supplementary data, starting with inherited
   supplementary data, check errors.
 */
void testExtend(vx_context context)
{
    printf("\nTesting vxExtendSupplementaryUserDataObject\n");
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_image image = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = 9, .end_y = 9};
    vx_image subimage = vxCreateImageFromROI(image, &rect);
    user_data_t new_data = {.numbers = {10, 11, 12, 13}};
    user_data_t read_data = {0};
    vx_status status = VX_SUCCESS;
    ERROR_EXPECT_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), NULL, &user_data, sizeof(user_data), sizeof(user_data)), VX_ERROR_INVALID_REFERENCE, "Source may not be NULL if there is no supplementary data");
    ERROR_EXPECT_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), NULL, NULL, sizeof(user_data), sizeof(user_data)), VX_ERROR_INVALID_REFERENCE, "Both source and user_data may not be NULL");
    ERROR_EXPECT_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, NULL, sizeof(user_data), sizeof(user_data) + 1), VX_ERROR_INVALID_VALUE, "Correct error for num_bytes > size");
    ERROR_EXPECT_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, NULL, sizeof(user_data) + 1, sizeof(user_data)), VX_ERROR_INVALID_VALUE, "Correct error for source_bytes > size");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, NULL, sizeof(user_data) / 2, sizeof(user_data)), "vxExtendSupplementaryUserDataObject with half source data");
    vx_user_data_object supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == user_data.numbers[0] &&
                           read_data.numbers[1] == user_data.numbers[1] &&
                           read_data.numbers[2] == 0 &&
                           read_data.numbers[3] == 0, "Correctly initialised first half of the data from source");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), NULL, &new_data, sizeof(user_data), sizeof(user_data) / 2), "vxExtendSupplementaryUserDataObject with half user data");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == new_data.numbers[0] &&
                           read_data.numbers[1] == new_data.numbers[1] &&
                           read_data.numbers[2] == 0 &&
                           read_data.numbers[3] == 0, "Correctly initialised first half of the data from user_data");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, &new_data, sizeof(user_data), sizeof(user_data) / 2), "vxExtendSupplementaryUserDataObject with half user data then half source data");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == new_data.numbers[0] &&
                           read_data.numbers[1] == new_data.numbers[1] &&
                           read_data.numbers[2] == user_data.numbers[2] &&
                           read_data.numbers[3] == user_data.numbers[3], "Correctly initialised first half of the data from user_data, second from source");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, &new_data, sizeof(user_data) / 2, sizeof(user_data)), "vxExtendSupplementaryUserDataObject with half source data then half user data");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == user_data.numbers[0] &&
                           read_data.numbers[1] == user_data.numbers[1] &&
                           read_data.numbers[2] == new_data.numbers[2] &&
                           read_data.numbers[3] == new_data.numbers[3], "Correctly initialised first half of the data from source, second from user_data");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, &new_data, sizeof(user_data), 0), "vxExtendSupplementaryUserDataObject with source");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == user_data.numbers[0] &&
                           read_data.numbers[1] == user_data.numbers[1] &&
                           read_data.numbers[2] == user_data.numbers[2] &&
                           read_data.numbers[3] == user_data.numbers[3], "Correctly initialised data from source");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(image), exemplar, &new_data, 0, sizeof(user_data)), "vxExtendSupplementaryUserDataObject with user data");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == new_data.numbers[0] &&
                           read_data.numbers[1] == new_data.numbers[1] &&
                           read_data.numbers[2] == new_data.numbers[2] &&
                           read_data.numbers[3] == new_data.numbers[3], "Correctly initialised data from user data");
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(subimage), supp, &user_data, sizeof(user_data) / 4, (sizeof(user_data) * 3)/ 4 ), "vxExtendSupplementaryUserDataObject of child with user data");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(image), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == new_data.numbers[0] &&
                           read_data.numbers[1] == new_data.numbers[1] &&
                           read_data.numbers[2] == new_data.numbers[2] &&
                           read_data.numbers[3] == new_data.numbers[3], "Setting child object supplementary data did not affect parent");
    vxReleaseUserDataObject(&supp);
    supp = vxGetSupplementaryUserDataObject((vx_reference)(subimage), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == new_data.numbers[0] &&
                           read_data.numbers[1] == user_data.numbers[1] &&
                           read_data.numbers[2] == user_data.numbers[2] &&
                           read_data.numbers[3] == 0, "Correctly initialised supplementary data of child object");
    /* Test extending when source valid size is less than source bytes */
    user_data_t empty = {0};
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(empty), &empty, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    vx_size valid_size = sizeof(user_data) / 4;
    ERROR_CHECK_STATUS(vxSetUserDataObjectAttribute(exemplar, VX_USER_DATA_OBJECT_VALID_SIZE, &valid_size, sizeof(valid_size)), NULL);
    ERROR_CHECK_STATUS(vxExtendSupplementaryUserDataObject((vx_reference)(subimage), exemplar, &new_data,  sizeof(user_data)/ 2,  3 * sizeof(user_data) / 4), NULL);
    ERROR_CHECK_STATUS(vxCopyUserDataObject(supp, 0, sizeof(user_data_t), &read_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), NULL);
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(read_data.numbers[0] == user_data.numbers[0] &&
                           read_data.numbers[1] == 0 &&
                           read_data.numbers[2] == new_data.numbers[2] &&
                           read_data.numbers[3] == 0, "Correctly terminated copy when valid size was less than source bytes");
    vxReleaseImage(&image);
    vxReleaseImage(&subimage);
    vxReleaseUserDataObject(&exemplar);
    vxReleaseUserDataObject(&supp);
}

/* Test object descriptor functions */
void testObjectDescriptors(vx_context context)
{
    printf("\nTesting Object Descriptors\n");
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_image image = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_image image1 = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = 9, .end_y = 9};
    vx_image subimage = vxCreateImageFromROI(image, &rect);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(image), exemplar), NULL);
    vx_image output = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(output), exemplar), NULL);
    vx_graph graph = vxCreateGraph(context);
    vx_kernel kernel = vxGetKernelByEnum(context, MY_USER_TEST_TARGET_KERNEL);
    ERROR_CHECK_OBJECT(kernel, NULL);
    vx_node node = vxCreateGenericNode(graph, kernel);
    ERROR_CHECK_OBJECT(node, NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 0, (vx_reference)(image)), NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 1, (vx_reference)(subimage)), NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 2, (vx_reference)(image1)), NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 3, (vx_reference)(output)), NULL);
    ERROR_CHECK_STATUS(vxProcessGraph(graph), NULL);
    vxReleaseKernel(&kernel);
    vxReleaseImage(&subimage);
    vxReleaseImage(&image);
    vxReleaseImage(&image1);
    vxReleaseImage(&output);
    vxReleaseUserDataObject(&exemplar);
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
}

/* Test read-only input parameters and child objects
    Attempting to call vxCopyUserDataObject for read-only supplementary data of all
    types has already been tested. Here we also test vxMapUserDataObject and vxSetUserDataObjectAttribute */
void testReadOnlyObjects(vx_context context)
{
    printf("\nTesting read-only input parameters and child objects\n");
    vx_status status = VX_SUCCESS;
    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    vx_user_data_object exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data);
    vx_image image = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_rectangle_t rect = {.start_x = 0, .start_y = 0, .end_x = 9, .end_y = 9};
    vx_image subimage = vxCreateImageFromROI(image, &rect);
    ERROR_CHECK_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(image), exemplar), NULL);
    vx_user_data_object childSupp = vxGetSupplementaryUserDataObject((vx_reference)(subimage), NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    vx_size size = sizeof(user_data_t);
    vx_map_id map_id;
    void *ptr;
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(tivxSetUserDataObjectAttribute(childSupp, TIVX_USER_DATA_OBJECT_VALID_SIZE, &size, sizeof(size)), "Cannot set attribute of read-only supplementary data");
    ERROR_EXPECT_STATUS_BAD(VX_SUCCESS, status);(vxMapUserDataObject(childSupp, 0, size, &map_id, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0), "Cannot map read-only supplementary data for writing");
    vx_image output1 = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_image output2 = vxCreateImage(context, 10, 10, VX_DF_IMAGE_U8);
    vx_graph graph = vxCreateGraph(context);
    vx_image virt = vxCreateVirtualImage(graph, 10, 10, VX_DF_IMAGE_U8);
    vx_kernel kernel = vxGetKernelByEnum(context, MY_USER_TEST_KERNEL);
    ERROR_CHECK_OBJECT(kernel, NULL);
    vx_node node = vxCreateGenericNode(graph, kernel);
    ERROR_CHECK_OBJECT(node, NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 0, (vx_reference)(image)), NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 1, (vx_reference)(subimage)), NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 2, (vx_reference)(output1)), NULL);
    ERROR_CHECK_STATUS(vxSetParameterByIndex(node, 3, (vx_reference)(virt)), NULL);
    vx_node node2 = dumbCopyNode(graph, virt, output2);
    ERROR_CHECK_STATUS(vxProcessGraph(graph), NULL);
    ERROR_EXPECT_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(virt), exemplar), VX_ERROR_OPTIMIZED_AWAY, "Correctly failed to set supplementary data on virtual object outside graph after graph processing");
    vxReleaseKernel(&kernel);
    vxReleaseImage(&subimage);
    vxReleaseImage(&image);
    vxReleaseImage(&virt);
    vxReleaseImage(&output1);
    vxReleaseImage(&output2);
    vxReleaseUserDataObject(&exemplar);
    vxReleaseUserDataObject(&childSupp);
    vxReleaseNode(&node);
    vxReleaseNode(&node2);
    vxReleaseGraph(&graph);
}
#endif

TESTCASE_TESTS(supplementary_data,
                testRefCount,
                testAllGetSet,
                testInvalidTypes)