/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <VX/vx.h>
#include <bmp_rd_wr.h>
#include <utility.h>

#define IN_FILE_NAME       "colors.bmp"
#define OUT_FILE_NAME      "vx_tutorial_image_crop_roi.bmp"

vx_image  load_image_from_handle_from_file(
            vx_context context,
            char *filename,
            vx_bool convert_to_gray_scale,
            void **bmp_file_context);

void vx_tutorial_image_crop_roi()
{
    vx_context context;
    vx_image image, roi_image;
    vx_uint32 width, height;
    vx_rectangle_t rect;
    void *bmp_file_context;

    printf(" vx_tutorial_image_crop_roi: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    image = load_image_from_handle_from_file(context, IN_FILE_NAME, vx_false_e, &bmp_file_context);

    vxSetReferenceName((vx_reference)image, "ORIGINAL");
    show_image_attributes(image);

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    rect.start_x = width/4;
    rect.start_y = height/4;
    rect.end_x   = rect.start_x + width/2;
    rect.end_y   = rect.start_y + height/2;

    roi_image = vxCreateImageFromROI(image, &rect);

    vxSetReferenceName((vx_reference)roi_image, "CROP_ROI");
    show_image_attributes(roi_image);

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    save_image_to_file(OUT_FILE_NAME, roi_image);

    vxReleaseImage(&roi_image);
    vxReleaseImage(&image);

    /* now it is safe to release the memory passed to the image
     * NOTE: "roi_image" also holds a reference to the memory since
     *       its created from the "image"
     *       Hence memory passed to "image" should be freed after "roi_image"
     *       is released
     */
    bmp_file_read_release(bmp_file_context);

    vxReleaseContext(&context);

    printf(" vx_tutorial_image_crop_roi: Tutorial Done !!! \n");
    printf(" \n");
}

vx_image  load_image_from_handle_from_file(
            vx_context context,
            char *filename,
            vx_bool convert_to_gray_scale,
            void **bmp_file_context)
{
    vx_image image = NULL;
    uint32_t width, height, stride;
    vx_df_image df;
    vx_status status;
    void *data_ptr;

    status = bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                bmp_file_context);

    if(status==VX_SUCCESS)
    {
        vx_imagepatch_addressing_t image_addr[1];
        void *ptrs[1];
        uint32_t bpp;

        if( df == VX_DF_IMAGE_U8 )
            bpp = 1;
        else
        if( df == VX_DF_IMAGE_RGB )
            bpp = 3;
        else
        if( df == VX_DF_IMAGE_RGBX )
            bpp = 4;
        else
            bpp = 1; /* it should not reach here for BMP files */

        image_addr[0].dim_x = width;
        image_addr[0].dim_y = height;
        image_addr[0].stride_x = bpp;
        image_addr[0].stride_y = stride;
        image_addr[0].scale_x = VX_SCALE_UNITY;
        image_addr[0].scale_y = VX_SCALE_UNITY;
        image_addr[0].step_x = 1;
        image_addr[0].step_y = 1;

        ptrs[0] = data_ptr;

        image = vxCreateImageFromHandle(context, df, image_addr, ptrs, VX_MEMORY_TYPE_HOST);

        /* MUST not free memory that is passed to vxCreateImageFromHandle, until
         * that memory ptr is swapped out from the image
         * or the image handle is released
         */
        /* bmp_file_read_release(bmp_file_context); */
    }
    return image;
}

