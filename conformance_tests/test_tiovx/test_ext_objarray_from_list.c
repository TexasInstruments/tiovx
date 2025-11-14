/*
 * Copyright (c) 2025 The Khronos Group Inc.
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
/*
 * Copyright (c) 2025 Texas Instruments Incorporated
 */

#include "test_engine/test.h"
#include <tivx.h>
#include <VX/vx_khr_swap_move.h>
#include <TI/tivx_ext_suppl_data.h>

TESTCASE(tivxObjArrayFromList, CT_VXContext, ct_setup_vx_context, 0)

/* Negative test for object array from wrong type of list */
TEST(tivxObjArrayFromList, negativeTestCreateObjectArrayFromList)
{
    vx_context context = context_->vx_context_;

    vx_object_array objectArrayFromList = NULL, objArrayForTest = NULL;
    vx_object_array ArrayOfObjArr[5] = {0};
    vx_convolution cnvl = NULL;
    vx_convolution cnvlArray[5] = {0};
    vx_reference ref = NULL, item = NULL;
    vx_size count = 5, columns = 5, rows = 5, i = 0;
    vx_reference mismatched[3] = {0};
    vx_graph graph = vxCreateGraph(context);

    vx_object_array parent, objArrayFromList;
    vx_image exemplar, child_img;

    // Case: NULL Context
    ASSERT(NULL == (objArrayFromList = tivxCreateObjectArrayFromList(NULL, mismatched, count)));

    // Case: NULL List
    objArrayFromList = tivxCreateObjectArrayFromList(context, NULL, count);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxGetStatus((vx_reference)objArrayFromList));

    // Case: A child from another object array
    ASSERT_VX_OBJECT(exemplar = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT((parent = vxCreateObjectArray(context, (vx_reference)exemplar, 1)), VX_TYPE_OBJECT_ARRAY);
    vxReleaseImage(&exemplar);
    ASSERT_VX_OBJECT(child_img = (vx_image)vxGetObjectArrayItem(parent, 0), VX_TYPE_IMAGE);
    vx_image temp_array_child[1] = {child_img};
    // Attempt creation with child from other objarray
    objArrayFromList = tivxCreateObjectArrayFromList(context, (vx_reference*)temp_array_child, 1);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_SCOPE, vxGetStatus((vx_reference) objArrayFromList));
    VX_CALL(vxReleaseObjectArray(&parent));
    VX_CALL(vxReleaseImage(&child_img));

    // Case: Mismatched array
    ASSERT_VX_OBJECT(mismatched[0] = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mismatched[1] = (vx_reference)vxCreateArray(context, VX_TYPE_IMAGE, 16), VX_TYPE_ARRAY);
    objectArrayFromList = tivxCreateObjectArrayFromList(context, mismatched, 2);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_TYPE, vxGetStatus((vx_reference)objectArrayFromList));

    // Case: List with NULL after first item (GetStatus on NULL returns ERROR_NO_RESOURCES)
    objectArrayFromList = tivxCreateObjectArrayFromList(context, &mismatched[1], 2);
    ASSERT_EQ_VX_STATUS(VX_ERROR_NO_RESOURCES, vxGetStatus((vx_reference)objectArrayFromList));

    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseReference(&mismatched[0]));
    VX_CALL(vxReleaseReference(&mismatched[1]));
}

TEST(tivxObjArrayFromList, negativeTestQueryObjectArrayFromList)
{
    #define VX_OBJECT_ARRAY_DEFAULT 0
    #define size_neg_query 5

    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_image img = NULL, imgArr[size_neg_query] = {0};
    vx_enum attribute = VX_OBJECT_ARRAY_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0, i = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    for (i = 0; i < size_neg_query; i++)
    {
        ASSERT_VX_OBJECT(imgArr[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(vxoa = tivxCreateObjectArrayFromList(context, (vx_reference*)(imgArr), size_neg_query), VX_TYPE_OBJECT_ARRAY);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_NUMITEMS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, TIVX_OBJECT_ARRAY_IS_FROM_LIST, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryObjectArray(vxoa, attribute, &udata, size));
    VX_CALL(vxReleaseObjectArray(&vxoa));
    for (i = 0; i < size_neg_query; i++)
    {
        VX_CALL(vxReleaseImage(&imgArr[i]));
    }
}

TEST(tivxObjArrayFromList, negativeTestCreateVirtualObjectArrayFromList)
{
    #define size_neg_virtual 2
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;

    vx_object_array objArrayFromList = NULL;
    vx_object_array virtualObjArrayFromList = NULL;
    // Lists for create functions
    vx_image images[size_neg_virtual] = {0};
    vx_image virtualImages[size_neg_virtual] = {0};
    vx_image mixedImages[size_neg_virtual] = {0};
    vx_size i = 0;
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    // Create image arrays
    for (i = 0; i < size_neg_virtual; i++)
    {
        ASSERT_VX_OBJECT(images[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    for (i = 0; i < size_neg_virtual; i++)
    {
        ASSERT_VX_OBJECT(virtualImages[i] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(mixedImages[0] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(mixedImages[1] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    // Case: NULL graph
    ASSERT(NULL == (objArrayFromList = tivxCreateVirtualObjectArrayFromList(NULL, (vx_reference*)virtualImages, 2)));

    // Case: Virtual from non virtual
    virtualObjArrayFromList = tivxCreateVirtualObjectArrayFromList(graph, (vx_reference*)images, size_neg_virtual);
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxGetStatus((vx_reference)virtualObjArrayFromList));

    // Case: Virtual from mismatched virtuality array fail
    virtualObjArrayFromList = tivxCreateVirtualObjectArrayFromList(graph, (vx_reference*)mixedImages, size_neg_virtual);
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_COMPATIBLE, vxGetStatus((vx_reference)virtualObjArrayFromList));

    // Positive Case: Virtual from Virtual
    virtualObjArrayFromList = tivxCreateVirtualObjectArrayFromList(graph, (vx_reference*)virtualImages, size_neg_virtual);
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)virtualObjArrayFromList));
    VX_CALL(vxReleaseObjectArray(&virtualObjArrayFromList));

    for (i = 0; i < size_neg_virtual; i++)
    {
        VX_CALL(vxReleaseImage(&images[i]));
        VX_CALL(vxReleaseImage(&virtualImages[i]));
        VX_CALL(vxReleaseImage(&mixedImages[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

static void fillSequence(CT_Image dst, uint32_t seq_init)
{
    uint32_t i, j;
    uint32_t val = seq_init;

    ASSERT(dst);
    ASSERT(dst->format == VX_DF_IMAGE_U8);

    for (i = 0; i < dst->height; ++i)
        for (j = 0; j < dst->width; ++j)
            dst->data.y[i * dst->stride + j] = ++val;
}

TEST(tivxObjArrayFromList, testObjectArrayFromListReplicate)
{
    #define NUM_IMAGES 3

    vx_uint32 *imagedims[3] =
    {
        (vx_uint32[]){256, 128},
        (vx_uint32[]){128, 64},
        (vx_uint32[]){64, 32},
    };

    CT_Image CT_input_array[NUM_IMAGES] = {0}, CT_output_img;

    vx_context context = context_->vx_context_;
    vx_graph graph;
    vx_object_array inOAFL = NULL, intermediateOAFL = NULL, outOAFL;
    vx_image inArray[NUM_IMAGES] = {0}, intArray[NUM_IMAGES] = {0}, outArray[NUM_IMAGES] = {0};
    vx_image temp_output = NULL;
    vx_node node1 = NULL, node2 = NULL;

    // Init vars
    for(vx_size i=0; i<NUM_IMAGES; i++)
    {
        ASSERT_VX_OBJECT(inArray[i] = vxCreateImage(context, imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intArray[i] = vxCreateImage(context, imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(outArray[i] = vxCreateImage(context, imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        // fill input img with sample ref data
        ASSERT_NO_FAILURE({
            CT_input_array[i] = ct_allocate_image(imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8);
            fillSequence(CT_input_array[i], (uint32_t)(1+i*10));
        });

        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(inArray[i], CT_input_array[i]));
    }

    // make object arrays from lists
    ASSERT_VX_OBJECT(inOAFL = tivxCreateObjectArrayFromList(context, (vx_reference*)(inArray), NUM_IMAGES), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(intermediateOAFL = tivxCreateObjectArrayFromList(context, (vx_reference*)(intArray), NUM_IMAGES), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(outOAFL = tivxCreateObjectArrayFromList(context, (vx_reference*)(outArray), NUM_IMAGES), VX_TYPE_OBJECT_ARRAY);

    // create graph & nodes
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    vx_bool replicate[] = { vx_true_e, vx_true_e };
    // 0th element of each array is also first element of its corresponding object array
    ASSERT_VX_OBJECT(node1 = vxNotNode(graph, inArray[0], intArray[0]), VX_TYPE_NODE);
    VX_CALL(vxReplicateNode(graph, node1, replicate, 2));
    ASSERT_VX_OBJECT(node2 = vxNotNode(graph, intArray[0], outArray[0]), VX_TYPE_NODE);
    VX_CALL(vxReplicateNode(graph, node2, replicate, 2));

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    for(vx_size i=0; i<NUM_IMAGES; i++)
    {
        temp_output = (vx_image)vxGetObjectArrayItem((vx_object_array)outOAFL, i);
        ASSERT_NO_FAILURE({
            CT_output_img = ct_image_from_vx_image(temp_output);
        });
        ASSERT_EQ_CTIMAGE(CT_input_array[i], CT_output_img);
        VX_CALL(vxReleaseImage(&temp_output));
    }

    // Clean up
    for (vx_size i = 0; i < NUM_IMAGES; i++)
    {
        VX_CALL(vxReleaseImage(&inArray[i]));
        VX_CALL(vxReleaseImage(&intArray[i]));
        VX_CALL(vxReleaseImage(&outArray[i]));
    }
    VX_CALL(vxReleaseObjectArray(&inOAFL));
    VX_CALL(vxReleaseObjectArray(&intermediateOAFL));
    VX_CALL(vxReleaseObjectArray(&outOAFL));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    return;
}

typedef struct _user_data
{
    vx_uint32 numbers[4];
} user_data_t;

TEST(tivxObjArrayFromList, testObjectArrayFromListSuppData)
{
    vx_context context = context_->vx_context_;
    vx_image imgArr[4] = {0}, oafl_item = NULL;
    vx_object_array vxoafl;
    vx_status status = VX_SUCCESS;
    vx_user_data_object exemplar, supp_oafl, supp_item;

    user_data_t user_data = {.numbers = {1, 2, 3, 4}};
    ASSERT_VX_OBJECT(exemplar = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data), VX_TYPE_USER_DATA_OBJECT);

    for (int i = 0; i < 4; i++)
    {
        ASSERT_VX_OBJECT(imgArr[i] = vxCreateImage(context, 16 + 2*i, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    status = vxSetSupplementaryUserDataObject((vx_reference)imgArr[3], exemplar);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("vxSetSupplementaryUserDataObject failed for imgArr\n");
    }
    ASSERT_VX_OBJECT(vxoafl = tivxCreateObjectArrayFromList(context, (vx_reference*)(imgArr), 4), VX_TYPE_OBJECT_ARRAY);

    // Negative test Get
    supp_oafl = vxGetSupplementaryUserDataObject((vx_reference)vxoafl, NULL, &status);
    if (status == (vx_status)VX_SUCCESS)
    {
        printf("Get Supplementary data return success though no data was Set for vxoafl\n");
    }
    EXPECT_NE_VX_STATUS(VX_SUCCESS, status);

    // Set and Get on object array
    status = vxSetSupplementaryUserDataObject((vx_reference)vxoafl, exemplar);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("vxSetSupplementaryUserDataObject failed for vxoafl\n");
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    ASSERT_VX_OBJECT(supp_oafl = vxGetSupplementaryUserDataObject((vx_reference)vxoafl, NULL, &status), VX_TYPE_USER_DATA_OBJECT);
    if (status != (vx_status)VX_SUCCESS)
    {
        printf("vxGetSupplementaryUserDataObject failed for vxoafl\n");
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);

    // Test Validity
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp_oafl, 0, sizeof(user_data.numbers[0]), &user_data.numbers[3], VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if (user_data.numbers[3] != user_data.numbers[0])
    {
        printf("Incorrect value read from supplementary data of vxoafl\n");
        status = (vx_status)VX_FAILURE;
    }

    // Test Object Array [3] (Set earlier)
    oafl_item = (vx_image)vxGetObjectArrayItem((vx_object_array)vxoafl, 3);
    supp_item = vxGetSupplementaryUserDataObject((vx_reference)oafl_item, NULL, &status);
    if ( status != (vx_status)VX_SUCCESS)
    {
        printf("Get Supplementary data on oafl_item failed\n");
    }
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxCopyUserDataObject(supp_item, 0, sizeof(user_data.numbers[0]), &user_data.numbers[2], VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    if (user_data.numbers[2] != user_data.numbers[0])
    {
        printf("Incorrect value read from supplementary data of oafl_item\n");
        status = (vx_status)VX_FAILURE;
    }

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseUserDataObject(&supp_item));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseUserDataObject(&supp_oafl));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseObjectArray(&vxoafl));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS,vxReleaseImage(&oafl_item));

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseUserDataObject(&exemplar));
    for (int i = 0; i < 4; i++)
    {
        VX_CALL(vxReleaseImage(&imgArr[i]));
    }
}


TEST(tivxObjArrayFromList, testObjectArrayFromListSuppDataCopySwap)
{
    vx_context context = context_->vx_context_;
    vx_status status = VX_SUCCESS;
    vx_user_data_object exemplar1, exemplar2;
    vx_object_array oafl1, oafl2;
    vx_user_data_object supp1, supp2;

    user_data_t user_data1 = {.numbers = {1, 2, 3, 4}};
    ASSERT_VX_OBJECT(exemplar1 = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data1), VX_TYPE_USER_DATA_OBJECT);
    user_data_t user_data2 = {.numbers = {11, 12, 13, 14}};
    ASSERT_VX_OBJECT(exemplar2 = vxCreateUserDataObject(context, "user_data_t", sizeof(user_data_t), &user_data2), VX_TYPE_USER_DATA_OBJECT);

    user_data_t test1 = {0};
    user_data_t test2 = {0};

    // Now check object arrays of pyramids 
    vx_pyramid pyr_ex1[4];
    for (int i = 0; i < 4; ++i)
    {
        ASSERT_VX_OBJECT(pyr_ex1[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 32+2*i, 32+2*i, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID); 
        EXPECT_EQ_VX_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(pyr_ex1[i]), exemplar1), VX_SUCCESS);
        for (int j = 0; j < 4; ++j)
        {
            vx_image img = vxGetPyramidLevel(pyr_ex1[i], j);
            vxSetSupplementaryUserDataObject((vx_reference)(img), exemplar1);
            vxReleaseImage(&img);
        }
    }

    ASSERT_VX_OBJECT(oafl1 = tivxCreateObjectArrayFromList(context, (vx_reference*)(pyr_ex1), 4), VX_TYPE_OBJECT_ARRAY);
    EXPECT_EQ_VX_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(oafl1), exemplar1), VX_SUCCESS);

    vx_pyramid pyr_ex2[4];
    for (int i = 0; i < 4; ++i)
    {
        ASSERT_VX_OBJECT(pyr_ex2[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 32+2*i, 32+2*i, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID); 
    }
    ASSERT_VX_OBJECT(oafl2 = tivxCreateObjectArrayFromList(context, (vx_reference*)(pyr_ex2), 4), VX_TYPE_OBJECT_ARRAY);
    // "negative" testing one of the supplementary is null 
    EXPECT_EQ_VX_STATUS(vxuSwap(context, (vx_reference)(oafl1), (vx_reference)(oafl2)), VX_SUCCESS);
    EXPECT_EQ_VX_STATUS(vxuSwap(context, (vx_reference)(oafl2), (vx_reference)(oafl1)), VX_SUCCESS);
    vxReleaseObjectArray(&oafl2);
    for (int i = 0; i < 4; ++i)
    {
        vxReleasePyramid(&pyr_ex2[i]); 
    }
    // now create suppplementary data 
    for (int i = 0; i < 4; ++i)
    {
        ASSERT_VX_OBJECT(pyr_ex2[i] = vxCreatePyramid(context, 4, VX_SCALE_PYRAMID_HALF, 32+2*i, 32+2*i, VX_DF_IMAGE_U8), VX_TYPE_PYRAMID);
        EXPECT_EQ_VX_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(pyr_ex2[i]), exemplar2), VX_SUCCESS);
        for(int j = 0; j < 4; ++j)
        {
            vx_image img = vxGetPyramidLevel(pyr_ex2[i], j);
            vxSetSupplementaryUserDataObject((vx_reference)(img), exemplar2);
            vxReleaseImage(&img);
        }
    }
    ASSERT_VX_OBJECT(oafl2 = tivxCreateObjectArrayFromList(context, (vx_reference*)(pyr_ex2), 4), VX_TYPE_OBJECT_ARRAY);
    EXPECT_EQ_VX_STATUS(vxSetSupplementaryUserDataObject((vx_reference)(oafl2), exemplar2), VX_SUCCESS);
    supp1 = vxGetSupplementaryUserDataObject((vx_reference)(oafl1), NULL, &status);
    EXPECT_EQ_VX_STATUS(status , VX_SUCCESS);
    supp2 = vxGetSupplementaryUserDataObject((vx_reference)(oafl2), NULL, &status);
    EXPECT_EQ_VX_STATUS(status , VX_SUCCESS);
    // Now try a swap 
    EXPECT_EQ_VX_STATUS(vxuSwap(context, (vx_reference)(oafl1), (vx_reference)(oafl2)), VX_SUCCESS);
    // Now if we look at the supplementary data it should be the other way around 
    EXPECT_EQ_VX_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), VX_SUCCESS);
    EXPECT_EQ_VX_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), VX_SUCCESS);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    EXPECT_EQ_VX_STATUS(status, VX_SUCCESS); 

    // now check the supplementary data of an object in the array 
    vx_reference ref1 = vxGetObjectArrayItem(oafl1, 2);
    vx_reference ref2 = vxGetObjectArrayItem(oafl2, 2);
    EXPECT_EQ_VX_STATUS(vxReleaseUserDataObject(&supp1), VX_SUCCESS);
    EXPECT_EQ_VX_STATUS(vxReleaseUserDataObject(&supp2), VX_SUCCESS);
    status = VX_SUCCESS;
    supp1 = vxGetSupplementaryUserDataObject(ref1, NULL, &status);
    supp2 = vxGetSupplementaryUserDataObject(ref2, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), VX_SUCCESS);
    EXPECT_EQ_VX_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), VX_SUCCESS);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if ((test1.numbers[i] != user_data2.numbers[i]) || (test2.numbers[i] != user_data1.numbers[i]))
        {
            status = VX_FAILURE;
            break;
        }
    }
    EXPECT_EQ_VX_STATUS(status, VX_SUCCESS);

    // now check the supplementary data of a level of a pyramid in the array 
    EXPECT_EQ_VX_STATUS(vxReleaseUserDataObject(&supp1), VX_SUCCESS);
    EXPECT_EQ_VX_STATUS(vxReleaseUserDataObject(&supp2), VX_SUCCESS);
    vx_reference ref3 = (vx_reference)(vxGetPyramidLevel((vx_pyramid)ref1, 2));
    vx_reference ref4 = (vx_reference)(vxGetPyramidLevel((vx_pyramid)ref2, 2));
    status = VX_SUCCESS;
    supp1 = vxGetSupplementaryUserDataObject(ref3, NULL, &status);
    supp2 = vxGetSupplementaryUserDataObject(ref4, NULL, &status);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, status);
    EXPECT_EQ_VX_STATUS(vxCopyUserDataObject(supp1, 0, sizeof(user_data_t), &test1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), VX_SUCCESS);
    EXPECT_EQ_VX_STATUS(vxCopyUserDataObject(supp2, 0, sizeof(user_data_t), &test2, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), VX_SUCCESS);
    status = VX_SUCCESS;
    for (int i = 0; i < 3; ++i)
    {
        if (test1.numbers[i] != user_data2.numbers[i] || test2.numbers[i] != user_data1.numbers[i])
        {
            status = VX_FAILURE;
            break;
        }
    }
    EXPECT_EQ_VX_STATUS(status, VX_SUCCESS);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref2));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref3));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseReference(&ref4));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseUserDataObject(&supp1));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxReleaseUserDataObject(&supp2));
    for (int i = 0; i < 4; ++i)
    {
        VX_CALL(vxReleasePyramid(&pyr_ex1[i]));
        VX_CALL(vxReleasePyramid(&pyr_ex2[i]));
    }
    VX_CALL(vxReleaseObjectArray(&oafl2));
    VX_CALL(vxReleaseObjectArray(&oafl1));
    VX_CALL(vxReleaseUserDataObject(&exemplar1));
    VX_CALL(vxReleaseUserDataObject(&exemplar2));
}

TESTCASE_TESTS(
    tivxObjArrayFromList,
    negativeTestCreateObjectArrayFromList,
    negativeTestQueryObjectArrayFromList,
    negativeTestCreateVirtualObjectArrayFromList,
    testObjectArrayFromListReplicate,
    testObjectArrayFromListSuppData,
    testObjectArrayFromListSuppDataCopySwap
)
