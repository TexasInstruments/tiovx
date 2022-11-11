/*

 * Copyright (c) 2012-2020 The Khronos Group Inc.
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
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
 *
 */

#include <TI/tivx_obj_desc.h>
#include <tivx_utils_ipc_ref_xfer.h>
#include "test_tiovx.h"

#define TIVX_TEST_FAIL_CLEANUP(x) {(x) = 1; goto cleanup;}
#define TIVX_TEST_UPDATE_STATUS(x) {if ((x)) CT_RecordFailure();}

#define TIVX_TEST_MAX_NUM_ADDR  (TIVX_PYRAMID_MAX_LEVEL_OBJECTS)

#define TIVX_TEST_SUCCESS                           (0)
#define TIVX_TEST_ERROR_STATUS_CHECK_FAILED         (-1)
#define TIVX_TEST_ERROR_POINTER_CHECK_FAILED        (-2)

#define TIVX_TEST_MAX_PYRAMID_LEVELS                (4)

typedef struct
{
    const char *name;
    vx_enum     type;
    uint32_t    aux;
} TestArg;

#define TIVX_TEST_ENTRY(X, v)   CT_GENERATE_PARAMETERS(#X, ARG, X, (v))

#define TEST_PARAMS \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_RGB), \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_RGBX), \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_NV12), \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_NV21), \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_UYVY), \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_IYUV), \
    TIVX_TEST_ENTRY(VX_TYPE_IMAGE, VX_DF_IMAGE_U8), \
    TIVX_TEST_ENTRY(VX_TYPE_TENSOR, 1), \
    TIVX_TEST_ENTRY(VX_TYPE_TENSOR, 2), \
    TIVX_TEST_ENTRY(VX_TYPE_TENSOR, 3), \
    TIVX_TEST_ENTRY(VX_TYPE_TENSOR, 4), \
    TIVX_TEST_ENTRY(VX_TYPE_USER_DATA_OBJECT, 1024), \
    TIVX_TEST_ENTRY(VX_TYPE_ARRAY, 100), \
    TIVX_TEST_ENTRY(VX_TYPE_CONVOLUTION, 9), \
    TIVX_TEST_ENTRY(VX_TYPE_MATRIX, 8), \
    TIVX_TEST_ENTRY(VX_TYPE_DISTRIBUTION, 100), \
    TIVX_TEST_ENTRY(TIVX_TYPE_RAW_IMAGE, 100), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_RGB), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_RGBX), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_NV12), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_NV21), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_UYVY), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_IYUV), \
    TIVX_TEST_ENTRY(VX_TYPE_PYRAMID, VX_DF_IMAGE_U8), \

TESTCASE(tivxMem, CT_VXContext, ct_setup_vx_context, 0)

static int32_t checkTranslation(const void *ptr, uint32_t size, int32_t expectedStatus)
{
    void       *phyAddr;
    void       *virtAddr;
    uint64_t    dmaBufFd;
    uint32_t    testFail = 0;
    vx_enum     region;
    vx_status   vxStatus;
    int32_t     status;

    status = TIVX_TEST_SUCCESS;

    /* Translate 'ptr' which should hold virtual address, to the associated
     * 'fd' and 'phy' address.
     */
    vxStatus = tivxMemTranslateVirtAddr(ptr, &dmaBufFd, &phyAddr);

    if (vxStatus != expectedStatus)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "tivxMemTranslateVirtAddr() failed. Expecting status [%d] "
                 "but got [%d]\n", expectedStatus, vxStatus);
        status = TIVX_TEST_ERROR_STATUS_CHECK_FAILED;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        /* From the 'dmaBufFd', query the 'virtAddr' and check it against the
         * original 'ptr' we have allocated.
         */
        vxStatus = tivxMemTranslateFd(dmaBufFd, size, &virtAddr, &phyAddr);

        if (vxStatus != expectedStatus)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "tivxMemTranslateFd() failed. Expecting status [%d] "
                     "but got [%d]\n", expectedStatus, vxStatus);
            status = TIVX_TEST_ERROR_STATUS_CHECK_FAILED;
        }
    }

    /* Compate 'ptr' and 'virtAddr'. These should be the same. */
    if ((vxStatus == (vx_status)VX_SUCCESS) && (virtAddr != ptr))
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "The retrieved virtAddr does not match the original "
                 "allocated buffer address.\n");
        status = TIVX_TEST_ERROR_POINTER_CHECK_FAILED;
    }

    return status;
}

static vx_reference testTivxMemAllocObject(vx_context context, vx_enum type, uint32_t  aux)
{
    vx_reference    ref = NULL;

    if (type == (vx_enum)VX_TYPE_IMAGE)
    {
        vx_image    image;

        image = vxCreateImage(context, 64, 48, aux);

        if (image == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateImage() failed.\n");
        }
        else
        {
            vx_imagepatch_addressing_t addr;
            vx_uint8 *pdata = 0;
            vx_rectangle_t rect = {0, 0, 64, 48};
            vx_map_id map_id;

            vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                        VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

            vxUnmapImagePatch(image, map_id);
        }

        ref = (vx_reference)image;
    }
    else if (type == (vx_enum)VX_TYPE_TENSOR)
    {
        vx_tensor   tensor;
        vx_size     dims[TIVX_CONTEXT_MAX_TENSOR_DIMS];
        uint32_t    i;

        for (i = 0; i < aux; i++)
        {
            dims[i] = 100;
        }

        tensor = vxCreateTensor(context, aux, dims, VX_TYPE_UINT8, 0);

        if (tensor == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateTensor() failed.\n");
        }
        else
        {
            vx_size start_map0[TIVX_CONTEXT_MAX_TENSOR_DIMS]= { 0 };
            vx_size end_map0[TIVX_CONTEXT_MAX_TENSOR_DIMS]= { 100, 100, 100, 100 };
            vx_size strides_map8[TIVX_CONTEXT_MAX_TENSOR_DIMS]= { 0 };
            vx_map_id id8;
            vx_uint8 *ptr8 = NULL;

            tivxMapTensorPatch(tensor, aux, start_map0, end_map0, &id8, strides_map8, (void **)&ptr8, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST);

            tivxUnmapTensorPatch(tensor, id8);
        }

        ref = (vx_reference)tensor;
    }
    else if (type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
    {
        vx_user_data_object obj;

        obj = vxCreateUserDataObject(context, NULL, aux, NULL);

        if (obj == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateUserDataObject() failed.\n");
        }
        else
        {
            void *p = NULL;
            vx_map_id map_id;
            vx_int32 i;

            vxMapUserDataObject(obj, 0, aux, &map_id, (void **)&p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

            vxUnmapUserDataObject(obj, map_id);
        }

        ref = (vx_reference)obj;
    }
    else if (type == (vx_enum)VX_TYPE_ARRAY)
    {
        vx_array    array;
        vx_coordinates3d_t localArrayInit[1] = {{1, 1, 1}};

        array = vxCreateArray(context, VX_TYPE_COORDINATES3D, aux);

        if (array == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateArray() failed.\n");
        }
        else
        {
            vxAddArrayItems(array, 1, &localArrayInit[0], sizeof(vx_coordinates3d_t));
        }

        ref = (vx_reference)array;
    }
    else if (type == (vx_enum)VX_TYPE_CONVOLUTION)
    {
        vx_convolution  conv;

        conv = vxCreateConvolution(context, aux, aux);

        if (conv == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateConvolution() failed.\n");
        }
        else
        {
            vx_int16 gx[3][3] = {
                { 3, 0, -3},
                { 10, 0,-10},
                { 3, 0, -3},
            };

            vxCopyConvolutionCoefficients(conv, gx, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }

        ref = (vx_reference)conv;
    }
    else if (type == (vx_enum)VX_TYPE_MATRIX)
    {
        vx_matrix   matrix;

        matrix = vxCreateMatrix(context, VX_TYPE_UINT32, aux, aux);

        if (matrix == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrix() failed.\n");
        }
        else
        {
            uint32_t i;
            vx_uint8* data = ct_alloc_mem(1);

            for (i = 0; i < 1; i++)
            {
                data[i] = 1;
            }

            vxCopyMatrix(matrix, data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }

        ref = (vx_reference)matrix;
    }
    else if (type == (vx_enum)VX_TYPE_DISTRIBUTION)
    {
        vx_distribution  dist;

        dist = vxCreateDistribution(context, aux, 5, 200);

        if (dist == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateDistribution() failed.\n");
        }
        else
        {
            vx_map_id map1;
            int32_t* hptr1 = NULL;

            vxMapDistribution(dist, &map1, (void*)&hptr1, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

            vxUnmapDistribution(dist, map1);
        }

        ref = (vx_reference)dist;
    }
    else if (type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
    {
        tivx_raw_image                  rawImage;
        tivx_raw_image_create_params_t  params;

        params.width                     = 128;
        params.height                    = 128;
        params.num_exposures             = 3;
        params.line_interleaved          = vx_false_e;
        params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        params.format[0].msb             = 12;
        params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
        params.format[1].msb             = 7;
        params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
        params.format[2].msb             = 11;
        params.meta_height_before        = 5;
        params.meta_height_after         = 0;

        rawImage = tivxCreateRawImage(context, &params);

        if (rawImage == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxCreateRawImage() failed.\n");
        }
        else
        {
            vx_map_id map_id;
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t addr;
            uint16_t *ptr = NULL;

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x   = 1;
            rect.end_y   = 1;

            tivxMapRawImagePatch(rawImage, &rect, 0, &map_id, &addr, (void **)&ptr,
                                        VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, TIVX_RAW_IMAGE_PIXEL_BUFFER);

            tivxUnmapRawImagePatch(rawImage, map_id);
        }

        ref = (vx_reference)rawImage;
    }
    else if (type == (vx_enum)VX_TYPE_PYRAMID)
    {
        vx_pyramid  pyramid;
        vx_size     levels = TIVX_TEST_MAX_PYRAMID_LEVELS;
        vx_uint32   width = 640;
        vx_uint32   height = 480;
        vx_float32  scale = VX_SCALE_PYRAMID_HALF;

        pyramid = vxCreatePyramid(context, levels, scale, width, height, aux);

        if (pyramid == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreatePyramid() failed.\n");
        }
        else
        {
            uint32_t i;
            vx_image image;

            for (i = 0; i < TIVX_TEST_MAX_PYRAMID_LEVELS; i++)
            {
                vx_imagepatch_addressing_t addr;
                vx_uint8 *pdata = 0;
                vx_rectangle_t rect = {0, 0, 64, 48};
                vx_map_id map_id;

                image = vxGetPyramidLevel(pyramid, i);

                vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                            VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

                vxUnmapImagePatch(image, map_id);

                vxReleaseImage(&image);
            }
        }

        ref = (vx_reference)pyramid;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", type);
    }

    return ref;
}

static vx_status testTivxMemFreeObject(vx_reference ref, vx_enum type)
{
    vx_status   vxStatus;

    vxStatus = (vx_status)VX_SUCCESS;

    if (ref == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "'ref' NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        if (type == (vx_enum)VX_TYPE_IMAGE)
        {
            vx_image    image = (vx_image)ref;
            vxStatus = vxReleaseImage(&image);
        }
        else if (type == (vx_enum)VX_TYPE_TENSOR)
        {
            vx_tensor   tensor = (vx_tensor)ref;
            vxStatus = vxReleaseTensor(&tensor);
        }
        else if (type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            vx_user_data_object obj = (vx_user_data_object)ref;
            vxStatus = vxReleaseUserDataObject(&obj);
        }
        else if (type == (vx_enum)VX_TYPE_ARRAY)
        {
            vx_array    array = (vx_array)ref;
            vxStatus = vxReleaseArray(&array);
        }
        else if (type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            vx_convolution  conv = (vx_convolution)ref;
            vxStatus = vxReleaseConvolution(&conv);
        }
        else if (type == (vx_enum)VX_TYPE_MATRIX)
        {
            vx_matrix   matrix = (vx_matrix)ref;
            vxStatus = vxReleaseMatrix(&matrix);
        }
        else if (type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            vx_distribution dist = (vx_distribution)ref;
            vxStatus = vxReleaseDistribution(&dist);
        }
        else if (type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_raw_image  rawImage = (tivx_raw_image)ref;
            vxStatus = tivxReleaseRawImage(&rawImage);
        }
        else if (type == (vx_enum)VX_TYPE_PYRAMID)
        {
            vx_pyramid  pyramid = (vx_pyramid)ref;
            vxStatus = vxReleasePyramid(&pyramid);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", type);
            vxStatus = (vx_status)VX_FAILURE;
        }
    }

    return vxStatus;
}

TEST(tivxMem, testTranslateAddrMemAlloc)
{
    void       *ptr;
    void       *phyAddr;
    void       *virtAddr;
    uint64_t    dmaBufFd;
    uint32_t    size;
    uint32_t    testFail = 0;
    vx_enum     region;
    int32_t     status;

    size   = 1024;
    region = TIVX_MEM_EXTERNAL;
    ptr = tivxMemAlloc(size, region);

    if (ptr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxMemAlloc() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Check address translation. */
    status = checkTranslation(ptr, size, VX_SUCCESS);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "checkTranslation() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:
    if (ptr != NULL)
    {
        tivxMemFree(ptr, size, region);
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TEST(tivxMem, testTranslateAddrMalloc)
{
    void       *ptr;
    void       *phyAddr;
    void       *virtAddr;
    uint64_t    dmaBufFd;
    uint32_t    size;
    uint32_t    testFail = 0;
    vx_enum     region;
    int32_t     status;

    /* ALlocate a memory block using malloc(). */
    size   = 1024;
    ptr = malloc(size);

    if (ptr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "malloc() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Check address translation. The translation should fail since the memory
     * has not been allocated using ether tivxMemAlloc() or ion_alloc().
     */
    status = checkTranslation(ptr, size, VX_FAILURE);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "checkTranslation() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:
    if (ptr != NULL)
    {
        free(ptr);
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TEST_WITH_ARG(tivxMem, testReferenceImportExport, TestArg, TEST_PARAMS)
{
    vx_context      context = context_->vx_context_;
    void           *virtAddr1[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    void           *virtAddr2[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    uint32_t        size[TIVX_TEST_MAX_NUM_ADDR];
    vx_reference    ref[2] = {NULL};
    uint32_t        testFail = 0;
    vx_enum         type = (vx_enum)arg_->type;
    uint32_t        maxNumAddr;
    uint32_t        numEntries1;
    uint32_t        numEntries2;
    uint32_t        numPlanes;
    uint32_t        i;
    vx_status       vxStatus;

    /* Allocate objects. Both these objects should not have any
     * internal memory allocated.
     */
    for (i = 0; i < 2; i++)
    {
        ref[i] = testTivxMemAllocObject(context, type, arg_->aux);

        if (ref[i] == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "testTivxMemAllocObject(%d) failed.\n", i);
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

    /* Export the handles from obj[0]. */
    maxNumAddr = TIVX_TEST_MAX_NUM_ADDR;
    vxStatus = tivxReferenceExportHandle(ref[0],
                                         virtAddr1,
                                         size,
                                         maxNumAddr,
                                         &numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Import the handles into obj[1]. */
    vxStatus = tivxReferenceImportHandle(ref[1],
                                         (const void **)virtAddr1,
                                         (const uint32_t *)size,
                                         numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* For images and pyramids, the internal memory allocation across planes
     * is monolithic so address translation needs to be checked only for the
     * first plane.
     *
     * The following logic sets up 'numPlanes' variable to allow skipping
     * the checks for intermediate planes in a given imabe.
     */
    if (type == (vx_enum)VX_TYPE_IMAGE)
    {
        /* For image, just check the first plane so force the loop exit
         * by setting 'numPlanes' to exceed loop count after the first
         * check.
         */
        numPlanes = numEntries1;
    }
    else if (type == (vx_enum)VX_TYPE_PYRAMID)
    {
        /* Translate only the first plane of each level so set up the
         * 'numPlanes' to skip intermediate planes and jump to the next
         * level.
         */
        numPlanes = numEntries1/TIVX_TEST_MAX_PYRAMID_LEVELS;
    }
    else
    {
        /* Default is to check every plane translation. */
        numPlanes = 1;
    }

    /* Check address translation. */
    i = 0;
    while (i < numEntries1)
    {
        int32_t     status;

        status = checkTranslation(virtAddr1[i], size[i], VX_SUCCESS);

        if (status != TIVX_TEST_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "checkTranslation() failed for virtAddr1[%d].\n", i);
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }

        i += numPlanes;
    }

    /* Export the handles from obj[1]. */
    maxNumAddr = TIVX_TEST_MAX_NUM_ADDR;
    vxStatus = tivxReferenceExportHandle(ref[1],
                                         virtAddr2,
                                         size,
                                         maxNumAddr,
                                         &numEntries2);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Check the number of entries. These should match. */
    if (numEntries1 != numEntries2)
    {
        VX_PRINT(VX_ZONE_ERROR, "Number of entries do not match.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Compare the addresses exported from the objects. These should match. */
    for (i = 0; i < numEntries1; i++)
    {
        if (virtAddr1[i] != virtAddr2[i])
        {
            VX_PRINT(VX_ZONE_ERROR, "Address entry [%d] mis-match.\n", i);
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

    /* Two objects have the same handles, which will cause issues when the
     * objects are released. We need to remove the handles from one of the
     * objects. Let us remove them from obj[0]. We do this by importing NULL
     * handles.
     */
    for (i = 0; i < maxNumAddr; i++)
    {
        virtAddr1[i] = NULL;
    }

    /* Import NULL handles into obj[0]. */
    vxStatus = tivxReferenceImportHandle(ref[0],
                                         (const void **)virtAddr1,
                                         (const uint32_t *)size,
                                         numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle(NULL) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:

    /* Free the objects. */
    for (i = 0; i < 2; i++)
    {
        vxStatus = testTivxMemFreeObject(ref[i], type);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "testTivxMemFreeObject(%d) failed.\n", i);
            testFail = 1;
        }
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TEST_WITH_ARG(tivxMem, testReferenceImportExportIpcNullObj, TestArg, TEST_PARAMS)
{
    vx_context      context = context_->vx_context_;
    tivx_utils_ref_ipc_msg_t   ipcMsg;
    void           *virtAddr1[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    void           *virtAddr2[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    uint32_t        size[TIVX_TEST_MAX_NUM_ADDR];
    vx_reference    ref[2] = {NULL};
    uint32_t        testFail = 0;
    vx_enum         type = (vx_enum)arg_->type;
    uint32_t        maxNumAddr;
    uint32_t        numEntries1;
    uint32_t        numEntries2;
    uint32_t        i;
    vx_status       vxStatus;

    /* Allocate objects. */
    ref[0] = testTivxMemAllocObject(context, type, arg_->aux);

    if (ref[0] == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "testTivxMemAllocObject() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Create the IPC message with the object export. */
    vxStatus = tivx_utils_export_ref_for_ipc_xfer(ref[0], &ipcMsg);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivx_utils_export_ref_for_ipc_xfer() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Create a new object with the exported information. ref[1] is
     * NULL, so a new object will be created and handles will be imported.
     */
    vxStatus = tivx_utils_import_ref_from_ipc_xfer(context, &ipcMsg, &ref[1]);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivx_utils_import_ref_from_ipc_xfer() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Export the handles from obj[0]. */
    maxNumAddr = TIVX_TEST_MAX_NUM_ADDR;
    vxStatus = tivxReferenceExportHandle(ref[0],
                                         virtAddr1,
                                         size,
                                         maxNumAddr,
                                         &numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Export the handles from obj[1]. */
    maxNumAddr = TIVX_TEST_MAX_NUM_ADDR;
    vxStatus = tivxReferenceExportHandle(ref[1],
                                         virtAddr2,
                                         size,
                                         maxNumAddr,
                                         &numEntries2);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Check the number of entries. These should match. */
    if (numEntries1 != numEntries2)
    {
        VX_PRINT(VX_ZONE_ERROR, "Number of entries do not match.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Compare the addresses exported from the objects. These should match. */
    for (i = 0; i < numEntries1; i++)
    {
        if (virtAddr1[i] != virtAddr2[i])
        {
            VX_PRINT(VX_ZONE_ERROR, "Address entry [%d] mis-match.\n", i);
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

    /* Two objects have the same handles, which will cause issues when the
     * objects are released. We need to remove the handles from one of the
     * objects. Let us remove them from obj[0]. We do this by importing NULL
     * handles.
     */
    for (i = 0; i < maxNumAddr; i++)
    {
        virtAddr1[i] = NULL;
    }

    /* Import NULL handles into obj[0]. */
    vxStatus = tivxReferenceImportHandle(ref[0],
                                         (const void **)virtAddr1,
                                         (const uint32_t *)size,
                                         numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle(NULL) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:

    /* Free the objects. */
    for (i = 0; i < 2; i++)
    {
        vxStatus = testTivxMemFreeObject(ref[i], type);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "testTivxMemFreeObject(%d) failed.\n", i);
            testFail = 1;
        }
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TEST_WITH_ARG(tivxMem, testReferenceImportExportIpcValidObj, TestArg, TEST_PARAMS)
{
    vx_context      context = context_->vx_context_;
    tivx_utils_ref_ipc_msg_t   ipcMsg;
    void           *virtAddr1[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    void           *virtAddr2[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    uint32_t        size[TIVX_TEST_MAX_NUM_ADDR];
    vx_reference    ref[2] = {NULL};
    uint32_t        testFail = 0;
    vx_enum         type = (vx_enum)arg_->type;
    uint32_t        maxNumAddr;
    uint32_t        numEntries1;
    uint32_t        numEntries2;
    uint32_t        i;
    vx_status       vxStatus;

    /* Allocate objects. Both these objects should not have any
     * internal memory allocated.
     */
    for (i = 0; i < 2; i++)
    {
        ref[i] = testTivxMemAllocObject(context, type, arg_->aux);

        if (ref[i] == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "testTivxMemAllocObject() failed.\n");
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

    /* Create the IPC message with the object export. */
    vxStatus = tivx_utils_export_ref_for_ipc_xfer(ref[0], &ipcMsg);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivx_utils_export_ref_for_ipc_xfer() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Create a new object with the exported information. ref[1] has
     * a valid object so no new object will be created but only
     * the handles will be imported.
     */
    vxStatus = tivx_utils_import_ref_from_ipc_xfer(context, &ipcMsg, &ref[1]);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivx_utils_import_ref_from_ipc_xfer() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Export the handles from obj[0]. */
    maxNumAddr = TIVX_TEST_MAX_NUM_ADDR;
    vxStatus = tivxReferenceExportHandle(ref[0],
                                         virtAddr1,
                                         size,
                                         maxNumAddr,
                                         &numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Export the handles from obj[1]. */
    maxNumAddr = TIVX_TEST_MAX_NUM_ADDR;
    vxStatus = tivxReferenceExportHandle(ref[1],
                                         virtAddr2,
                                         size,
                                         maxNumAddr,
                                         &numEntries2);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Check the number of entries. These should match. */
    if (numEntries1 != numEntries2)
    {
        VX_PRINT(VX_ZONE_ERROR, "Number of entries do not match.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Compare the addresses exported from the objects. These should match. */
    for (i = 0; i < numEntries1; i++)
    {
        if (virtAddr1[i] != virtAddr2[i])
        {
            VX_PRINT(VX_ZONE_ERROR, "Address entry [%d] mis-match.\n", i);
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

    /* Two objects have the same handles, which will cause issues when the
     * objects are released. We need to remove the handles from one of the
     * objects. Let us remove them from obj[0]. We do this by importing NULL
     * handles.
     */
    for (i = 0; i < maxNumAddr; i++)
    {
        virtAddr1[i] = NULL;
    }

    /* Import NULL handles into obj[0]. */
    vxStatus = tivxReferenceImportHandle(ref[0],
                                         (const void **)virtAddr1,
                                         (const uint32_t *)size,
                                         numEntries1);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle(NULL) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:

    /* Free the objects. */
    for (i = 0; i < 2; i++)
    {
        vxStatus = testTivxMemFreeObject(ref[i], type);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "testTivxMemFreeObject(%d) failed.\n", i);
            testFail = 1;
        }
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TEST(tivxMem, testReferenceImportNeg)
{
    vx_context      context = context_->vx_context_;
    void           *virtAddr[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    void           *p;
    uint32_t        size[TIVX_TEST_MAX_NUM_ADDR];
    vx_reference    ref = NULL;
    uint32_t        testFail = 0;
    vx_enum         type = VX_TYPE_IMAGE;
    vx_df_image     format = VX_DF_IMAGE_YUV4;
    uint32_t        numEntries;
    uint32_t        i;
    vx_status       vxStatus;

    /* VX_DF_IMAGE_YUV4 has 3 planes so expects 3 address entries. */
    numEntries = 3;

    /* Import the handles into obj. Since 'ref' is NULL, the call should
     * fail.
     */
    vxStatus = tivxReferenceImportHandle(ref,
                                         (const void **)virtAddr,
                                         (const uint32_t *)size,
                                         numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Allocate object. The object should not have any internal memory
     * allocated.
     */
    ref = testTivxMemAllocObject(context, type, format);

    if (ref == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "testTivxMemAllocObject() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Import the handles into obj. Since 'virtAddr' is NULL, the call should
     * fail.
     */
    vxStatus = tivxReferenceImportHandle(ref, NULL, size, numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Import the handles into obj. Since 'size' is NULL, the call should
     * fail.
     */
    vxStatus = tivxReferenceImportHandle(ref,
                                         (const void **)virtAddr,
                                         NULL,
                                         numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Export handles to get valid size information. */
    vxStatus = tivxReferenceExportHandle(ref,
                                         virtAddr,
                                         size,
                                         TIVX_TEST_MAX_NUM_ADDR,
                                         &numEntries);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Nullify one of the addr[] entries. Import should fail. */
    p = virtAddr[1];
    virtAddr[1] = NULL;

    vxStatus = tivxReferenceImportHandle(ref,
                                         (const void **)virtAddr,
                                         (const uint32_t *)size,
                                         numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Restore the Nullified entry. */
    virtAddr[1] = p;

    /* Run the loop with [0..(numEntries+1)] values. */
    for (i = 0; i < numEntries+2; i++)
    {
        vx_status expected;

        if (i >= numEntries)
        {
            expected = (vx_status)VX_SUCCESS;
        }
        else
        {
            /* Since 'numEntries' is invalid, the call should fail. */
            expected = (vx_status)VX_FAILURE;
        }

        /* Import the handles into obj. */
        vxStatus = tivxReferenceImportHandle(ref,
                                             (const void **)virtAddr,
                                             (const uint32_t *)size,
                                             i);

        if (vxStatus != (vx_status)expected)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxReferenceImportHandle() failed.\n");
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

cleanup:

    /* Free the object. */
    vxStatus = testTivxMemFreeObject(ref, type);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "testTivxMemFreeObject() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TEST(tivxMem, testReferenceExportNeg)
{
    vx_context      context = context_->vx_context_;
    void           *virtAddr[TIVX_TEST_MAX_NUM_ADDR] = {NULL};
    uint32_t        size[TIVX_TEST_MAX_NUM_ADDR];
    vx_reference    ref = NULL;
    uint32_t        testFail = 0;
    vx_enum         type = VX_TYPE_IMAGE;
    vx_df_image     format = VX_DF_IMAGE_YUV4;
    uint32_t        maxNumAddr;
    uint32_t        numEntries;
    uint32_t        i;
    vx_status       vxStatus;

    /* VX_DF_IMAGE_YUV4 has 3 planes so expects 3 address entries. */
    maxNumAddr = 3;

    /* Import the handles into obj. Since 'ref' is NULL, the call should
     * fail.
     */
    vxStatus = tivxReferenceExportHandle(ref,
                                         virtAddr,
                                         size,
                                         maxNumAddr,
                                         &numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Allocate objects. Both these objects should not have any
     * internal memory allocated.
     */
    ref = testTivxMemAllocObject(context, type, format);

    if (ref == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "testTivxMemAllocObject() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Import the handles into obj. Since 'virtAddr' is NULL, the call should
     * fail.
     */
    vxStatus = tivxReferenceExportHandle(ref,
                                         NULL,
                                         size,
                                         maxNumAddr,
                                         &numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Import the handles into obj. Since 'size' is NULL, the call should
     * fail.
     */
    vxStatus = tivxReferenceExportHandle(ref,
                                         virtAddr,
                                         NULL,
                                         maxNumAddr,
                                         &numEntries);

    if (vxStatus != (vx_status)VX_FAILURE)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Run the loop with [0..(numEntries+1)] values. */
    for (i = 0; i < numEntries+2; i++)
    {
        vx_status expected;

        if (i >= maxNumAddr)
        {
            expected = (vx_status)VX_SUCCESS;
        }
        else
        {
            /* Since 'maxNumAddr' is invalid, the call should fail. */
            expected = (vx_status)VX_FAILURE;
        }

        /* Import the handles into obj. */
        vxStatus = tivxReferenceExportHandle(ref,
                                             virtAddr,
                                             size,
                                             i,
                                             &numEntries);

        if (vxStatus != (vx_status)expected)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
            TIVX_TEST_FAIL_CLEANUP(testFail);
        }
    }

cleanup:

    /* Free the object. */
    vxStatus = testTivxMemFreeObject(ref, type);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "testTivxMemFreeObject() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TESTCASE_TESTS(tivxMem,
               testTranslateAddrMemAlloc,
               testTranslateAddrMalloc,
               testReferenceImportExport,
               testReferenceImportExportIpcNullObj,
               testReferenceImportExportIpcValidObj,
               testReferenceImportNeg,
               testReferenceExportNeg
)
