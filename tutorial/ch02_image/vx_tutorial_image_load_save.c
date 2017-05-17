/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 * \file vx_tutorial_image_load_save.c Load and save data from OpenVX image objects
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context and OpenVX image data object
 * - How to read a BMP file and load the pixel values into the image data object
 * - How to query the image data object for attributes like width, height
 * - How to read pixel values from an image data object and save it as a BMP file
 * - How to cleanup all created resources and exit the OpenVX application
 *
 * vx_tutorial_image_load_save() is the entry point to the tutorial.
 *
 * We create few utility functions as listed below as part of this tutorial.
 * These functions will be used in subsequent tutorials to load and save images.
 * The utility functions are
 * - create_image_from_file()
 * - save_image_to_file()
 * - load_image_from_file(): this function though listed in this tutorial is
 *       used in a later tutorial
 */

#include <stdio.h>
#include <VX/vx.h>
#include <bmp_rd_wr.h>
#include <utility.h>

/** \brief Input file name */
#define IN_FILE_NAME       "colors.bmp"

/** \brief Output file name */
#define OUT_FILE_NAME      "vx_tutorial_image_load_save_out.bmp"

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_image_load_save()
{
    vx_context context;
    vx_image image;
    vx_uint32 width, height;

    printf(" vx_tutorial_image_load_save: Tutorial Started !!! \n");

    /**
     * Step 1: Create OpenVX context, this MUST be done first before any OpenVX call
     * the context that is returned is used as input for most OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    /**
     * Step 2: Create image object, load it with data from file \ref IN_FILE_NAME
     * \code
     */
    image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);
    /** \endcode */

    /**
     * Step 3: Query image object to print image dimensions for informational purposes
     * \code
     */
    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    /** \endcode */

    printf(" Loaded image of dimensions %d x %d\n", width, height);

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    /**
     * Step 3: Save image object to bitmap file \ref OUT_FILE_NAME
     * \code
     */
    save_image_to_file(OUT_FILE_NAME, image);
    /** \endcode */

    /**
     * Step 4: Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&image);
    /** \endcode */

    /**
     * Step 5: Since we are done using OpenVX context, release it
     * \code
     */
    vxReleaseContext(&context);
    /** \endcode */

    printf(" vx_tutorial_image_load_save: Tutorial Done !!! \n");
    printf(" \n");
}

/**
 * \brief Create a image data object given BMP filename as input
 *
 * \param context [in] OpenVX context within which the image object will get created
 * \param filename [in] BMP filename, MUST have extension of .bmp
 * \param convert_to_gray_scale [in] Converts RGB values in BMP file to 8b grayscale value
 *
 * \return Image data object. \n
 *         Image data format is 24b RGB when \ref convert_to_gray_scale is vx_false_e \n
 *         Image data format is 8b Grayscale when \ref convert_to_gray_scale is vx_true_e
 */
vx_image  create_image_from_file(vx_context context, char *filename, vx_bool convert_to_gray_scale)
{
    vx_image image = NULL;
    uint32_t width, height, stride;
    vx_df_image df;
    vx_status status;
    void *data_ptr, *bmp_file_context;

    /**
     * Step 1: Read BMP file, the BMP file pixel values are stored at location
     * data_ptr. The BMP file attributes are returned in variables 'width', 'hegith',
     * 'df' (data format).
     * 'bmp_file_context' holds the context of the BMP file which MUST be free'ed
     * once the usage of this file is done.
     * \code
     */
    status = bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);
    /** \endcode */

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

