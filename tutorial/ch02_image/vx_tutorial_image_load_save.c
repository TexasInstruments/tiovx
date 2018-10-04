/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
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
 *              <b>NOTE:</b> This function though listed in this tutorial is used in later tutorials
 *           </TD>
 *      </TR>
 *  </TABLE>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <VX/vx.h>
#include <bmp_rd_wr.h>
#include <utility.h>

/* in some systems file IO is not present, so skip it using below flag */
/* #define SKIP_FILEIO */

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
    #ifdef SKIP_FILEIO
    status = 0;
    width = 640;
    height = 480;
    stride = 640;
    if(convert_to_gray_scale)        
        df = VX_DF_IMAGE_U8;
    else
        df = VX_DF_IMAGE_RGB;
    data_ptr = NULL;
    bmp_file_context = NULL;
    #else
    status = bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);
    /** \endcode */
    #endif

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

            if(data_ptr!=NULL)
            {
                vxCopyImagePatch(image,
                    &rect,
                    0,
                    &image_addr,
                    data_ptr,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST
                    );
            }
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
            #ifdef SKIP_FILEIO
            
            #else
            bmp_file_write(filename, width, height, image_addr.stride_y, df, data_ptr);
            #endif
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
    vx_status status = 0;
    void *data_ptr, *bmp_file_context;

    #ifdef SKIP_FILEIO
    status = 0;
    width = 640;
    height = 480;
    stride = 640;
    if(convert_to_gray_scale)        
        df = VX_DF_IMAGE_U8;
    else
        df = VX_DF_IMAGE_RGB;
    data_ptr = NULL;
    bmp_file_context = NULL;
    #else
    status = bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);
    #endif

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

            if(data_ptr!=NULL)
            {
                vxCopyImagePatch(image,
                    &rect,
                    0,
                    &image_addr,
                    data_ptr,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST
                    );
            }
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        bmp_file_read_release(bmp_file_context);
    }
    return status;
}


void ascii_file_read(char *filename, int num_elements, void* buffer, vx_enum data_type){
    FILE* ptr_file;

    // Initialize buffer for reading each line of file
    int nbytes = sizeof(float);
    char* buff = malloc(nbytes);

    // Initialize variables for tokenizing each line based on delimiter values
    char* token;
    char* delim = " \n\t,";
    float val;

    // Initialize different data type arrays
    vx_uint8* u8;
    vx_int32* i32;
    vx_float32* f32;
    uint8_t val_u8;
    int32_t val_int32;

    // Try reading in file
    ptr_file = fopen(filename, "r");
    if(!ptr_file){
        printf("FAILED READING FILE");
    }

    // If file read is successful, try to populate matrix
    int i = 0;
    while(fgets(buff, nbytes, ptr_file) > 0){
        // Split string into tokens at whitespaces, commas, newlines, or tabs
        for(token = strtok(buff, delim); token != NULL; token = strtok(NULL, delim)){
            val = strtod(token, NULL);

            switch(data_type){
                case VX_TYPE_FLOAT32:
                    f32 = buffer;
                    f32[i] = val;
                    break;

                case VX_TYPE_UINT8:
                    u8 = buffer;
                    val_u8 = val;
                    u8[i] = val_u8;
                    break;

                case VX_TYPE_INT32:
                    i32 = buffer;
                    val_int32 = val;
                    i32[i] = val_int32;
                    break;

                default:
                    break;
            }

            i++;
            if (i >= num_elements){
                break;
            }
        }
        // Need to break out of both loops when this condition is met
        if (i >= num_elements){
            break;
        }
    }
    fclose(ptr_file);
    //printf("val of first element: %d\n", u8[0]);
}


vx_matrix create_matrix_from_file(vx_context context, vx_enum data_type, int cols, int rows, char *filename){
    // Create vx_float32 buffer object
    vx_float32 mat[rows*cols];

    // Call generic function that will populate data object
    ascii_file_read(filename, rows*cols, mat, data_type);

    // Create vx_matrix object
    vx_matrix matrix = vxCreateMatrix(context, data_type, cols, rows);

    // Set pointer reference between vx_float32 data object and
    vxCopyMatrix(matrix, mat, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    return matrix;
}

vx_status save_pyramid_to_file(char *filename, vx_pyramid pyr, vx_size levels){
    vx_status status = VX_SUCCESS;
    vx_uint32 index;
    vx_image image;

    printf("Saving pyramid object...\n");
    for (index=0; index<levels; index++){
        // Create buffer for filename
        char img_filename[1000] = "";
        strcat(img_filename, filename);

        // Create new filename for each level
        char img_index[10] = "";
        sprintf(img_index, "%d.bmp", index);
        strcat(img_filename, img_index);
        printf("New filename: %s", img_filename);

        // Save each level
        image = vxGetPyramidLevel(pyr, index);
        status = save_image_to_file(img_filename, image);
        status = vxReleaseImage(&image);
    }

    return status;
}
