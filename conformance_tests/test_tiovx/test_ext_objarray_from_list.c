/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
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

TESTCASE(tivxObjArrayFromList, CT_VXContext, ct_setup_vx_context, 0)

/* Negative test for object array from wrong type of list */
TEST(tivxObjArrayFromList, negativeTestCreateObjectArray)
{
    vx_context context = context_->vx_context_;

    vx_object_array objectArrayFromList = NULL, objArrayForTest = NULL;
    vx_object_array ArrayOfObjArr[] = {0};
    vx_convolution cnvl = NULL;
    vx_convolution cnvlArray[] = {0};
    vx_reference ref = NULL, item = NULL;
    vx_size count = 5, columns = 5, rows = 5, i = 0;
    vx_reference mismatched[5] = {0};

    // Case: NULL context
    ASSERT(NULL == (objectArrayFromList = tivxCreateObjectArrayFromList(NULL, (vx_reference*)ref, count)));

    // Case: Convolution
    for(i=0; i < count; i++){
        ASSERT_VX_OBJECT(cnvl = vxCreateConvolution(context, columns, rows), VX_TYPE_CONVOLUTION);
        cnvlArray[i] = cnvl;
    }
    ASSERT(NULL == (objectArrayFromList = tivxCreateObjectArrayFromList(context, (vx_reference*)cnvlArray, count)));
    for(i=0; i < count; i++){
        VX_CALL(vxReleaseConvolution(&cnvlArray[i]));
    }

    // Case: Object Array
    ASSERT_VX_OBJECT(ref = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    for(i=0; i < count; i++){
        ASSERT_VX_OBJECT(objArrayForTest = vxCreateObjectArray(context, (vx_reference)ref, count + i), VX_TYPE_OBJECT_ARRAY);
        ArrayOfObjArr[i] = objArrayForTest;
    }
    ASSERT(NULL == (objectArrayFromList = tivxCreateObjectArrayFromList(context, (vx_reference*)ArrayOfObjArr, count)));
    for(i=0; i < count; i++){
        VX_CALL(vxReleaseObjectArray(&ArrayOfObjArr[i]));
    }
    VX_CALL(vxReleaseReference(&ref));

    // Case: Mismatched array
    ASSERT_VX_OBJECT(item = (vx_reference)vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    mismatched[0] = item;
    ASSERT_VX_OBJECT(item = (vx_reference)vxCreateArray(context, VX_TYPE_IMAGE, 16), VX_TYPE_ARRAY);
    mismatched[1] = item;
    objectArrayFromList = tivxCreateObjectArrayFromList(context, mismatched, 2);
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)objectArrayFromList));
    VX_CALL(vxReleaseReference(&mismatched[0]));
    VX_CALL(vxReleaseReference(&mismatched[1]));
}

TEST(tivxObjArrayFromList, negativeTestQueryObjectArray)
{
    #define VX_OBJECT_ARRAY_DEFAULT 0
    #define size_const 5

    vx_context context = context_->vx_context_;

    vx_object_array vxoa = NULL;
    vx_image img = NULL, imgArr[size_const] = {0};
    vx_enum attribute = VX_OBJECT_ARRAY_DEFAULT;
    vx_uint32 udata = 0;
    vx_size size = 0, i = 0;

    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_REFERENCE, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    for (i = 0; i < size_const; i++){
        ASSERT_VX_OBJECT(imgArr[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(vxoa = tivxCreateObjectArrayFromList(context, (vx_reference*)(imgArr), size_const), VX_TYPE_OBJECT_ARRAY);
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_ITEMTYPE, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, VX_OBJECT_ARRAY_NUMITEMS, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_INVALID_PARAMETERS, vxQueryObjectArray(vxoa, TIVX_OBJECT_ARRAY_IS_FROM_LIST, &udata, size));
    ASSERT_EQ_VX_STATUS(VX_ERROR_NOT_SUPPORTED, vxQueryObjectArray(vxoa, attribute, &udata, size));
    VX_CALL(vxReleaseObjectArray(&vxoa));
    for (i = 0; i < size_const; i++){
        VX_CALL(vxReleaseImage(&imgArr[i]));
    }
}

TEST(tivxObjArrayFromList, negativeTestCreateVirtualObjectArray)
{
    vx_context context = context_->vx_context_;
    vx_graph graph = NULL;
    
    // Create references to hold our objects
    vx_object_array objArrayFromList = NULL;
    vx_object_array virtualObjArrayFromList = NULL;
    
    // Arrays to hold our images
    vx_image images[2] = {0};
    vx_image virtualImages[2] = {0};
    vx_image mixedImages[2] = {0};
    vx_size i = 0, count = 2;
    
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    
    // Create image arrays
    for (i = 0; i < count; i++) {
        ASSERT_VX_OBJECT(images[i] = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    for (i = 0; i < count; i++) {
        ASSERT_VX_OBJECT(virtualImages[i] = vxCreateVirtualImage(graph, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    mixedImages[0] = images[0];
    mixedImages[1] = virtualImages[0];
    
    // Case: Virtual from non virtual
    virtualObjArrayFromList = tivxCreateVirtualObjectArrayFromList(graph, (vx_reference*)images, count);
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)virtualObjArrayFromList));

    // Case: Non virtual from virtual
    objArrayFromList = tivxCreateObjectArrayFromList(context, (vx_reference*)virtualImages, count);  
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)objArrayFromList));

    // Case: Either from mismatched array fail
    virtualObjArrayFromList = tivxCreateVirtualObjectArrayFromList(graph, (vx_reference*)mixedImages, count);
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)virtualObjArrayFromList));
    objArrayFromList = tivxCreateObjectArrayFromList(context, (vx_reference*)mixedImages, count);  
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)objArrayFromList));

    // Clean up
    for (i = 0; i < count; i++) {
        VX_CALL(vxReleaseImage(&images[i]));
        VX_CALL(vxReleaseImage(&virtualImages[i]));
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

TEST(tivxObjArrayFromList, ReplicateObjectArrayFromList)
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

    /* Init vars */
    for(vx_size i=0; i<NUM_IMAGES; i++)
    {
        ASSERT_VX_OBJECT(inArray[i] = vxCreateImage(context, imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(intArray[i] = vxCreateImage(context, imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(outArray[i] = vxCreateImage(context, imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        /* fill input img with sample ref data*/
        ASSERT_NO_FAILURE({
            CT_input_array[i] = ct_allocate_image(imagedims[i][0], imagedims[i][1], VX_DF_IMAGE_U8);
            fillSequence(CT_input_array[i], (uint32_t)(1+i*10));
        });

        ASSERT_NO_FAILURE(ct_image_copyto_vx_image(inArray[i], CT_input_array[i]));
    }

    /* make object arrays from lists */
    ASSERT_VX_OBJECT(inOAFL = tivxCreateObjectArrayFromList(context, (vx_reference*)(inArray), NUM_IMAGES), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(intermediateOAFL = tivxCreateObjectArrayFromList(context, (vx_reference*)(intArray), NUM_IMAGES), VX_TYPE_OBJECT_ARRAY);
    ASSERT_VX_OBJECT(outOAFL = tivxCreateObjectArrayFromList(context, (vx_reference*)(outArray), NUM_IMAGES), VX_TYPE_OBJECT_ARRAY);

    /* create graph & nodes */
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
    for (vx_size i = 0; i < NUM_IMAGES; i++){
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


TESTCASE_TESTS(
    tivxObjArrayFromList,
    negativeTestCreateObjectArray,
    negativeTestQueryObjectArray,
    negativeTestCreateVirtualObjectArray,
    ReplicateObjectArrayFromList
)
