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
#define OUT_FILE_NAME      "vx_tutorial_image_load_save_out.bmp"

void vx_tutorial_image_load_save()
{
    vx_context context;
    vx_image image;
    vx_uint32 width, height;

    printf(" vx_tutorial_image_load_save: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    printf(" Loaded image of dimensions %d x %d\n", width, height);

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    save_image_to_file(OUT_FILE_NAME, image);

    vxReleaseImage(&image);
    vxReleaseContext(&context);

    printf(" vx_tutorial_image_load_save: Tutorial Done !!! \n");
    printf(" \n");
}

vx_image  create_image_from_file(vx_context context, char *filename, vx_bool convert_to_gray_scale)
{
    vx_image image = NULL;
    uint32_t width, height, stride;
    vx_df_image df;
    vx_status status;
    void *data_ptr, *bmp_file_context;

    status = bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);

    if(status==VX_SUCCESS)
    {
        image = vxCreateImage(context, width, height, df);
        if(vxGetStatus((vx_reference)image)==VX_SUCCESS)
        {
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
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

            image_addr.dim_x = width;
            image_addr.dim_y = height;
            image_addr.stride_x = bpp;
            image_addr.stride_y = stride;
            image_addr.scale_x = VX_SCALE_UNITY;
            image_addr.scale_y = VX_SCALE_UNITY;
            image_addr.step_x = 1;
            image_addr.step_y = 1;

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;

            vxCopyImagePatch(image,
                &rect,
                0,
                &image_addr,
                data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST
                );
        }
        bmp_file_read_release(bmp_file_context);
    }
    return image;
}

vx_status save_image_to_file(char *filename, vx_image image)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    vx_df_image df;
    void *data_ptr;
    vx_status status;

    status = vxGetStatus((vx_reference)image);
    if(status==VX_SUCCESS)
    {
        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        status = vxMapImagePatch(image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_READ_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        if(status==VX_SUCCESS)
        {
            bmp_file_write(filename, width, height, image_addr.stride_y, df, data_ptr);

            vxUnmapImagePatch(image, map_id);
        }
    }
    return status;
}

vx_status load_image_from_file(vx_image image, char *filename, vx_bool convert_to_gray_scale)
{
    uint32_t width, height, stride;
    vx_df_image df;
    uint32_t img_width, img_height;
    vx_df_image img_df;
    vx_status status;
    void *data_ptr, *bmp_file_context;

    status = bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);

    if(status==VX_SUCCESS)
    {
        img_width = img_height = 0;

        vxQueryImage(image, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &img_df, sizeof(vx_df_image));

        if(width==img_width && height==img_height && df==img_df)
        {
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
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

            image_addr.dim_x = width;
            image_addr.dim_y = height;
            image_addr.stride_x = bpp;
            image_addr.stride_y = stride;
            image_addr.scale_x = VX_SCALE_UNITY;
            image_addr.scale_y = VX_SCALE_UNITY;
            image_addr.step_x = 1;
            image_addr.step_y = 1;

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;

            vxCopyImagePatch(image,
                &rect,
                0,
                &image_addr,
                data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST
                );
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        bmp_file_read_release(bmp_file_context);
    }
    return status;
}
