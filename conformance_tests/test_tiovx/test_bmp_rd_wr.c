#include "test_engine/test.h"
#include "test_tiovx.h"

#include <tivx_utils_file_rd_wr.h>
#include <test_utils_file_bmp_rd_wr.h>

#define TIVX_TEST_FAIL_CLEANUP(x) {(x) = 1; goto cleanup;}
#define TIVX_TEST_UPDATE_STATUS(x) {if ((x)) CT_RecordFailure();}

/* Test error codes. */
#define TIVX_TEST_SUCCESS                                (0)
#define TIVX_TEST_ERROR_EXPAND_FILE_PATH_FAILED          (-1)
#define TIVX_TEST_ERROR_VXIMAGE_FROM_BMP_FAILED          (-2)
#define TIVX_TEST_ERROR_VXIMAGE_META_FORMAT_CHECK_FAILED (-3)
#define TIVX_TEST_ERROR_VXIMAGE_GET_DATA_FAILED          (-4)
#define TIVX_TEST_ERROR_VXIMAGE_ADDR_CHECK_FAILED        (-5)
#define TIVX_TEST_ERROR_VXIMAGE_DATA_CHECK_FAILED        (-6)
#define TIVX_TEST_ERROR_VXIMAGE_WRITE_TO_BMP_FAILED      (-7)

TESTCASE(tivxBmpRdWr, CT_VXContext, ct_setup_vx_context, 0)

const char *inFiles[TIOVX_UTILS_MAXPATHLENGTH/4] =
{
    "${VX_TEST_DATA_PATH}/baboon.bmp",
    "${VX_TEST_DATA_PATH}/blurred_lena_gray.bmp",
    "${VX_TEST_DATA_PATH}/colors.bmp",
};

typedef struct
{
    vx_context  vxContext;
    vx_image    tuImg;
    vx_image    ctImg;
    vx_bool     convToGrayScale;
    const char *outFileName;
    char        inFileName[TIOVX_UTILS_MAXPATHLENGTH];

} TestObjContext;

typedef struct
{
    const char *name;
    uint32_t    index;
    vx_bool     convToGrayScale;
} TestArg;

#define TEST_PARAMS \
    CT_GENERATE_PARAMETERS("bmptest_output_baboon_col",        ARG, 0, 0), \
    CT_GENERATE_PARAMETERS("bmptest_output_baboon_gray",       ARG, 0, 1), \
    CT_GENERATE_PARAMETERS("bmptest_output_blurred_lena_gray", ARG, 1, 1), \
    CT_GENERATE_PARAMETERS("bmptest_output_colors_col",        ARG, 2, 0), \
    CT_GENERATE_PARAMETERS("bmptest_output_colors_gray",       ARG, 2, 1), \

static int32_t ReadInputImages(TestObjContext  *objCntxt,
                               const char      *inFileName)
{
    vx_status   vxStatus;
    int32_t     status;

    status          = TIVX_TEST_SUCCESS;
    objCntxt->tuImg = NULL;
    objCntxt->ctImg = NULL;

    /* Expand the file path. */
    vxStatus = tivx_utils_expand_file_path(inFileName, objCntxt->inFileName);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivx_utils_expand_file_path() failed.\n");
        status = TIVX_TEST_ERROR_EXPAND_FILE_PATH_FAILED;
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        objCntxt->tuImg =
            tivx_utils_create_vximage_from_bmpfile(objCntxt->vxContext,
                                                   objCntxt->inFileName,
                                                   objCntxt->convToGrayScale);

        if (objCntxt->tuImg == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "tivx_utils_create_vximage_from_bmpfile() failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_FROM_BMP_FAILED;
        }
        else
        {
            vxSetReferenceName((vx_reference)objCntxt->tuImg,
                               "TIVX_UTILS_READ_IMAGE");
        }
    }

    /* Read the same input image using CT API. */
    if (status == TIVX_TEST_SUCCESS)
    {
        objCntxt->ctImg =
            test_utils_create_vximage_from_bmpfile(objCntxt->vxContext,
                                                   objCntxt->inFileName,
                                                   objCntxt->convToGrayScale);

        if (objCntxt->ctImg == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "test_utils_create_vximage_from_bmpfile() failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_FROM_BMP_FAILED;
        }
        else
        {
            vxSetReferenceName((vx_reference)objCntxt->tuImg,
                               "CT_READ_IMAGE");
        }
    }

    return status;
}

static int32_t WriteOutputImages(TestObjContext    *objCntxt,
                                 const char        *tuOutFileName,
                                 const char        *ctOutFileName)
{
    char        outFileName[TIOVX_UTILS_MAXPATHLENGTH];
    vx_status   vxStatus;
    int32_t     status;

    status = TIVX_TEST_SUCCESS;

    /* Expand the file path. */
    vxStatus = tivx_utils_expand_file_path(tuOutFileName, outFileName);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivx_utils_expand_file_path() failed.\n");
        status = TIVX_TEST_ERROR_EXPAND_FILE_PATH_FAILED;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        vxStatus = tivx_utils_save_vximage_to_bmpfile(outFileName,
                                                      objCntxt->tuImg);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "tivx_utils_save_vximage_to_bmpfile() failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_WRITE_TO_BMP_FAILED;
        }
    }

    /* Expand the file path. */
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        vxStatus = tivx_utils_expand_file_path(ctOutFileName, outFileName);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivx_utils_expand_file_path() failed.\n");
            status = TIVX_TEST_ERROR_EXPAND_FILE_PATH_FAILED;
        }
    }

    /* Read the same input image using CT API. */
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        vxStatus = test_utils_save_vximage_to_bmpfile(outFileName,
                                                      objCntxt->ctImg);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "test_utils_save_vximage_to_bmpfile() failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_WRITE_TO_BMP_FAILED;
        }
    }

    return status;
}

static int32_t ReleaseImages(TestObjContext   *objCntxt)
{
    int32_t status = TIVX_TEST_SUCCESS;

    if (objCntxt->tuImg != NULL)
    {
        vxReleaseImage(&objCntxt->tuImg);
    }

    if (objCntxt->ctImg != NULL)
    {
        vxReleaseImage(&objCntxt->ctImg);
    }

    return status;
}

static vx_status GetImageDataPtr(vx_image                       image,
                                 vx_imagepatch_addressing_t    *imgAddr,
                                 void                         **dataPtr)
{
    vx_rectangle_t  rect;
    vx_uint32       width;
    vx_uint32       height;
    vx_map_id       map_id;
    vx_status       status;

    /** - Query image attributes.
     *
     *  These will be used to select ROI of data to be copied and
     *  and to set attributes of the BMP file
     */
    vxQueryImage(image, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    /** - Map image data to user accessible memory space
     *
     * 'imgAddr' describes the arrangement of the mapped image data.
     * 'data_ptr' points to the first pixel of the mapped image data.
     * 'map_id' holds the mapped context. This is used to unmapped the data
     *  once application is done with it.
     * 'VX_READ_ONLY' indicates that application will read from the mapped
     *  memory.
     * 'rect' holds the ROI of image object to map. In this example, 'rect'
     *  is set to map the whole image.
     */
    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x   = width;
    rect.end_y   = height;
    *dataPtr     = NULL;

    status = vxMapImagePatch(image,
                             &rect,
                             0,
                             &map_id,
                             imgAddr,
                             dataPtr,
                             (vx_enum)VX_READ_ONLY,
                             (vx_enum)VX_MEMORY_TYPE_HOST,
                             (vx_enum)VX_NOGAP_X);

    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxMapImagePatch() failed.\n");
    }

    /** - Unmap a previously mapped image object
     *
     * Every vxMapImagePatch MUST have a corresponding unmap in OpenVX.
     * The 'map_id' returned by vxMapImagePatch() is used as input by
     * vxUnmapImagePatch()
     */
    vxUnmapImagePatch(image, map_id);

    return status;
}

static int32_t CompareImages(TestObjContext *objCntxt)
{
    uint8_t                    *ctDataPtr;
    uint8_t                    *tuDataPtr;
    vx_imagepatch_addressing_t  ctImgAddr;
    vx_imagepatch_addressing_t  tuImgAddr;
    int32_t                     status;
    vx_bool                     vxStatus;
    vx_bool                     boolStatus;

    status   = TIVX_TEST_SUCCESS;
    vxStatus = (vx_status)VX_SUCCESS;

    boolStatus =
        tivxIsReferenceMetaFormatEqual((vx_reference)objCntxt->tuImg,
                                       (vx_reference)objCntxt->ctImg);

    if (boolStatus == (vx_bool)vx_false_e)
    {
        status = TIVX_TEST_ERROR_VXIMAGE_META_FORMAT_CHECK_FAILED;
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        /* Extract the tu based image data pointers. */
        vxStatus = GetImageDataPtr(objCntxt->tuImg,
                                   &tuImgAddr,
                                   (void**)&tuDataPtr);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "GetImageDataPtr(tuImg) failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_GET_DATA_FAILED;
        }

        VX_PRINT(VX_ZONE_INFO, "\t=======================\n");
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->dim_x    = %d\n", tuImgAddr.dim_x);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->dim_y    = %d\n", tuImgAddr.dim_y);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->stride_x = %d\n", tuImgAddr.stride_x);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->stride_y = %d\n", tuImgAddr.stride_y);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->scale_x  = %d\n", tuImgAddr.scale_x);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->scale_y  = %d\n", tuImgAddr.scale_y);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->step_x   = %d\n", tuImgAddr.step_x);
        VX_PRINT(VX_ZONE_INFO, "\ttuImg->step_y   = %d\n", tuImgAddr.step_y);
        VX_PRINT(VX_ZONE_INFO, "\t=======================\n\n");
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        /* Extract the ct based image data pointers. */
        vxStatus = GetImageDataPtr(objCntxt->ctImg,
                                   &ctImgAddr,
                                   (void**)&ctDataPtr);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "GetImageDataPtr(ctImg) failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_GET_DATA_FAILED;
        }

        VX_PRINT(VX_ZONE_INFO, "\t=======================\n");
        VX_PRINT(VX_ZONE_INFO, "\tctImg->dim_x    = %d\n", ctImgAddr.dim_x);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->dim_y    = %d\n", ctImgAddr.dim_y);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->stride_x = %d\n", ctImgAddr.stride_x);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->stride_y = %d\n", ctImgAddr.stride_y);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->scale_x  = %d\n", ctImgAddr.scale_x);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->scale_y  = %d\n", ctImgAddr.scale_y);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->step_x   = %d\n", ctImgAddr.step_x);
        VX_PRINT(VX_ZONE_INFO, "\tctImg->step_y   = %d\n", ctImgAddr.step_y);
        VX_PRINT(VX_ZONE_INFO, "\t=======================\n\n");
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        if ((tuImgAddr.dim_x    != ctImgAddr.dim_x)    ||
            (tuImgAddr.dim_y    != ctImgAddr.dim_y)    ||
            (tuImgAddr.stride_x != ctImgAddr.stride_x) ||
            (tuImgAddr.stride_y != ctImgAddr.stride_y) ||
            (tuImgAddr.scale_x  != ctImgAddr.scale_x)  ||
            (tuImgAddr.scale_y  != ctImgAddr.scale_y)  ||
            (tuImgAddr.step_x   != ctImgAddr.step_x)   ||
            (tuImgAddr.step_y   != ctImgAddr.step_y))
        {
            VX_PRINT(VX_ZONE_ERROR, "Image address data failed.\n");
            status = TIVX_TEST_ERROR_VXIMAGE_ADDR_CHECK_FAILED;
        }
    }

    /* Compare the pixel data. */
    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        uint32_t    i;
        uint32_t    j;
        uint8_t    *tuPtr;
        uint8_t    *ctPtr;

        tuPtr = (uint8_t *)tuDataPtr;
        ctPtr = (uint8_t *)ctDataPtr;

        for (i = 0; i < tuImgAddr.dim_y; i++)
        {
            for (j = 0; j < tuImgAddr.stride_y; j++)
            {
                if (*tuPtr++ != *ctPtr++)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "Data starts differing at [%d, %d] byte.\n",
                             i, j);
                    status = TIVX_TEST_ERROR_VXIMAGE_DATA_CHECK_FAILED;
                    break;
                }
            }

            if (status == TIVX_TEST_ERROR_VXIMAGE_DATA_CHECK_FAILED)
            {
                break;
            }

        }  /* for (i = 0; i < tuImgAddr.dim_y; i++) */
    }

    return status;
}

TEST_WITH_ARG(tivxBmpRdWr, testBmpRd, TestArg, TEST_PARAMS)
{
    TestObjContext  objCntxt;
    const char     *inFileName;
    char            tuOutFileName[TIOVX_UTILS_MAXPATHLENGTH];
    char            ctOutFileName[TIOVX_UTILS_MAXPATHLENGTH];
    uint32_t        testFail = 0;
    uint32_t        imgIndex;
    int32_t         status;

    objCntxt.vxContext       = context_->vx_context_;
    objCntxt.outFileName     = arg_->name;
    objCntxt.convToGrayScale = arg_->convToGrayScale;
    imgIndex                 = arg_->index;
    inFileName               = inFiles[imgIndex];

    tivx_clr_debug_zone(VX_ZONE_INFO);

    /* Read the images. */
    status = ReadInputImages(&objCntxt, inFileName);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "ReadInputImages() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Compare the input images read by tiovx utils and ct utils
     * independently.
     */
    status = CompareImages(&objCntxt);

    if ((status != TIVX_TEST_SUCCESS) &&
        (status != TIVX_TEST_ERROR_VXIMAGE_DATA_CHECK_FAILED))
    {
        VX_PRINT(VX_ZONE_ERROR, "CompareImages() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Save the images. */
    snprintf(tuOutFileName, TIOVX_UTILS_MAXPATHLENGTH,
             "${VX_TEST_DATA_PATH}/%s_tu.bmp", objCntxt.outFileName);

    snprintf(ctOutFileName, TIOVX_UTILS_MAXPATHLENGTH,
             "${VX_TEST_DATA_PATH}/%s_ct.bmp", objCntxt.outFileName);

    status = WriteOutputImages(&objCntxt, tuOutFileName, ctOutFileName);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "WriteOutputImages() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:
    status = ReleaseImages(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "ReleaseImages() failed.\n");
    }

    tivx_clr_debug_zone(VX_ZONE_INFO);

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TESTCASE_TESTS(tivxBmpRdWr,
               testBmpRd)

