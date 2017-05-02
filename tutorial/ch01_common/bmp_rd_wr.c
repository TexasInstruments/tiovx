/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <bmp_rd_wr.h>
#include <test_engine/test.h>
#include <test_engine/test_image.h>


/* dummy definition to allow to link to CT test_engine */
CT_RegisterTestCaseFN g_testcase_register_fns[1] = {0};

vx_status bmp_file_read(
            char *filename,
            vx_bool convert_to_gray_scale,
            uint32_t *width,
            uint32_t *height,
            uint32_t *stride,
            vx_df_image *df,
            void **data_ptr,
            void **bmp_file_context)
{
    CT_Image image = NULL;
    int dcn = convert_to_gray_scale ? 1 : -1;
    uint32_t bpp;
    vx_status status;

    /* workaround to enable CT context */
    CT_SetHasRunningTest();

    image = ct_read_image(filename, dcn);

    if(image != NULL)
    {
        if( image->format == VX_DF_IMAGE_U8 )
            bpp = 1;
        else
        if( image->format == VX_DF_IMAGE_RGB )
            bpp = 3;
        else
            bpp = 4; /* RGBX */

        *width = image->width;
        *height = image->height;
        *stride = image->stride * bpp;
        *df = image->format;
        *data_ptr = image->data.y;

        *bmp_file_context = image;

        status = VX_SUCCESS;
    }
    else
    {
        *width = 0;
        *height = 0;
        *stride = 0;
        *data_ptr = NULL;

        *bmp_file_context = NULL;
        status = VX_FAILURE;
    }
    return status;
}

void bmp_file_read_release(void *bmp_file_context)
{
    if(bmp_file_context)
    {
        CT_FreeObject(bmp_file_context);
    }
}

int32_t bmp_file_write(
            char *filename,
            uint32_t width,
            uint32_t height,
            uint32_t stride,
            vx_df_image df,
            void *data_ptr)
{
    CT_Image image = NULL;
    uint32_t bpp;
    vx_status status;

    if( df == VX_DF_IMAGE_U8 )
        bpp = 1;
    else
    if( df == VX_DF_IMAGE_RGB )
        bpp = 3;
    else
    if( df == VX_DF_IMAGE_RGBX )
        bpp = 4;
    else
        bpp = 0;

    status = VX_FAILURE;
    if( bpp > 0)
    {
        image = ct_allocate_image_hdr(width, height, stride/bpp, df, data_ptr);

        ct_write_image(filename, image);

        CT_FreeObject(image);

        status = VX_SUCCESS;
    }
    return status;
}
