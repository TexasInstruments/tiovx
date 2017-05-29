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
 * To include OpenVX interfaces include below file
 * \code
 * #include <VX/vx.h>
 * \endcode
 *
 * To include utility APIs to read and write BMP file include below file
 * \code
 * #include <bmp_rd_wr.h>
 * \endcode
 *
 * Follow the comments in the function vx_tutorial_image_load_save()
 * to understand this tutorial
 *
 * As part of this tutorial, we create few utility functions as listed below.
 * These functions will be used in subsequent tutorials to load and save images.
 *  <TABLE frame="box" rules="all" cellspacing="0" width="50%" border="1" cellpadding="3">
 *      <TR bgcolor="lightgrey">
 *          <TH> Utility function </TH>
 *          <TH> Description </TH>
 *      </TR>
 *      <TR>
 *          <TD> create_image_from_file() </TD>
 *          <TD> Reads a .bmp file and create a vx_image object with pixel values read from the BMP file </TD>
 *      </TR>
 *      <TR>
 *          <TD> save_image_to_file() </TD>
 *          <TD> Given a vx_image object, creates a BMP file with pixel values from the vx_image object</TD>
 *      </TR>
 *      <TR>
 *          <TD> load_image_from_file() </TD>
 *          <TD> Same as create_image_from_file(), only a created vx_image object is passed to this function.
 *              <b>NOTE:</b> This function though listed in this tutorial is used in a later tutorials
 *           </TD>
 *      </TR>
 *  </TABLE>
 *
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
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image image;
    /** \endcode */
    vx_uint32 width, height;

    printf(" vx_tutorial_image_load_save: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context.
     *
     * This MUST be done first before any OpenVX API call.
     * The context that is returned is used as input for subsequent OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    /**
     * - Create image object.
     *
     * Follow the comments in create_image_from_file() to see
     * how a vx_image object is created and filled with RGB data from BMP file \ref IN_FILE_NAME
     * \code
     */
    image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);
    /** \endcode */

    /**
     * - Query image object.
     *
     * Here we print the image dimensions.
     * \code
     */
    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    /** \endcode */

    printf(" Loaded image of dimensions %d x %d\n", width, height);

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    /**
     * - Save image object to bitmap file \ref OUT_FILE_NAME.
     *
     * Follow the comments in save_image_to_file() to see
     * how data in vx_image object is accessed to store pixel values from the image object to
     * BMP file \ref OUT_FILE_NAME
     * \code
     */
    save_image_to_file(OUT_FILE_NAME, image);
    /** \endcode */

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&image);
    /** \endcode */

    /**
     * - Release context object.
     *
     * Since we are done using OpenVX context, release it.
     * No further OpenVX API calls should be done, until a context is again created using
     * vxCreateContext()
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
 * \param convert_to_gray_scale [in] vx_true_e: Converts RGB values in BMP file to 8b grayscale value and copies them to image object\n
 *                                   vx_false_e: Retains RGB values from BMP file and copies them to image object\n
 *
 * \return Image data object. \n
 *         Image data format is VX_DF_IMAGE_RGB when 'convert_to_gray_scale' is vx_false_e \n
 *         Image data format is VX_DF_IMAGE_U8 when 'convert_to_gray_scale' is vx_true_e
 */
vx_image  create_image_from_file(vx_context context, char *filename, vx_bool convert_to_gray_scale)
{
    vx_image image = NULL;
    uint32_t width, height, stride;
    vx_df_image df;
    vx_status status;
    void *data_ptr, *bmp_file_context;

    /**
     * - Read BMP file.
     *
     * The BMP file pixel values are stored at location
     * 'data_ptr'. The BMP file attributes are returned in variables
     * 'width', 'heigth', 'df' (data format).
     * 'bmp_file_context' holds the context of the BMP file which is free'ed
     * after copying the pixel values from 'data_ptr' into a vx_image object.
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
        /**
         * - Create OpenVX image object.
         *
         * Creates a OpenVX image object of 'width' x 'height' and having
         * data format 'df'.
         *
         * <b>TIP:</b> In OpenVX whenever an object is created use
         * vxGetStatus() to find if the object creation was successful.
         * The object must be typecasted to vx_reference type when calling
         * vxGetStatus() API. If the reference is valid VX_SUCCESS should be
         * returned by vxGetStatus().
         * \code
         */
        image = vxCreateImage(context, width, height, df);
        status = vxGetStatus((vx_reference)image);
        /** \endcode */
        if(status==VX_SUCCESS)
        {

            /**
             * - Copy pixel values from 'data_ptr' into image object.
             *
             * 'image_addr' is used to describe arrangement of data within
             * 'data_ptr'. The attributes 'wdith','height','stride' are used
             * to set 'dim_x', 'dim_y', 'stride_y' fields in 'image_addr'.
             * 'stride_x' is derived from data format 'df'. Other fields are set
             * to unity.
             *
             * 'rect' is used to select ROI within 'data_ptr' from which
             * data is to be copied to image object. Here 'rect' is set to copy
             * all the pixels from 'data_ptr' into the image object.
             *
             * 'VX_WRITE_ONLY' indicates that application wants to write into the
             * vx_image object.
             *
             * 'VX_MEMORY_TYPE_HOST' indicates the type of memory that is pointed to by
             * 'data_ptr'. Use 'VX_MEMORY_TYPE_HOST' for TIVX applications.
             * \code
             */

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

            /** \endcode */
        }
        /** - Release BMP file context.
         *
         *  Since pixel values are copied from 'data_ptr' to vx_image object
         *  Now 'data_ptr' and any other resources allocted by bmp_file_read()
         *  are free'ed by calling bmp_file_read_release()
         *  \code
         */
        bmp_file_read_release(bmp_file_context);
        /** \endcode */
    }
    return image;
}

/**
 * \brief Save data from image object to BMP file
 *
 * \param filename [in] BMP filename, MUST have extension of .bmp
 * \param image [in] Image data object. Image data format MUST be VX_DF_IMAGE_RGB or VX_DF_IMAGE_U8
 *
 * \return VX_SUCCESS if BMP could be created and saved with data from image object
 */
vx_status save_image_to_file(char *filename, vx_image image)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id;
    vx_df_image df;
    void *data_ptr;
    vx_status status;

    /** - Check if image object is valid
     *
     * \code
     */
    status = vxGetStatus((vx_reference)image);
    /** \endcode */
    if(status==VX_SUCCESS)
    {
        /** - Query image attributes.
         *
         *  These will be used to select ROI of data to be copied and
         *  and to set attributes of the BMP file
         * \code
         */

        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
        /** \endcode */

        /** - Map image data to user accessible memory space
         *
         * 'image_addr' describes the arrangement of the mapped image data. \n
         * 'data_ptr' points to the first pixel of the mapped image data. \n
         * 'map_id' holds the mapped context. This is used to unmapped the data once application is done with it. \n
         * 'VX_READ_ONLY' indicates that application will read from the mapped memory. \n
         * 'rect' holds the ROI of image object to map. In this example, 'rect' is set to map the whole image.
         *
         * \code
         */
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
        /** \endcode */

        if(status==VX_SUCCESS)
        {
            /** - Write to BMP file using utility API
             *
             * 'image_addr.stride_y' is used to specify the offset in bytes between
             * two consecutive lines in memory. This is returned by vxMapImagePatch()
             * above
             * \code
             */
            bmp_file_write(filename, width, height, image_addr.stride_y, df, data_ptr);
            /** \endcode */

            /** - Unmapped a previously mapped image object
             *
             * Every vxMapImagePatch MUST have a corresponding unmap in OpenVX.
             * The 'map_id' returned by vxMapImagePatch() is used as input by
             * vxUnmapImagePatch()
             * \code
             */
            vxUnmapImagePatch(image, map_id);
            /** \endcode */
        }
    }
    return status;
}

/**
 * \brief Load data from BMP file into a previously created vx_image object
 *
 * This function is same as create_image_from_file(). Only difference is
 * that the vx_image object is created outside this function. Some tutorials
 * use this function instead of create_image_from_file()
 *
 * This function queries the vx_image object to make sure its attributes
 * match the attributes of the BMP file from which data needs to be loaded
 * into the image object. In case of mismatch data is not loaded and error is returned.
 *
 * \param image [in] Previouly created image object
 * \param filename [in] BMP filename, MUST have extension of .bmp
 * \param convert_to_gray_scale [in] vx_true_e: Converts RGB values in BMP file to 8b grayscale value and copies them to image object\n
 *                                   vx_false_e: Retains RGB values from BMP file and copies them to image object\n
 *
 * \return VX_SUCCESS if BMP file data could be loaded into the vx_image object.
 */
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

