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
 * \file vx_tutorial_image_crop_roi.c Crop a rectangular region from an image
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context and OpenVX image data object
 * - How to read a BMP from handle from file and load the pixel values into the image data object
 * - How to query the image data object for attributes like width, height
 * - How to create an image from a rectangular region of another image
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
 * Follow the comments in the function vx_tutorial_image_crop_roi()
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
 *          <TD> load_image_from_handle_from_file() </TD>
 *          <TD> Loads an image data object from handle from file </TD>
 *      </TR>
 *  </TABLE>
 *
 */

#include <stdio.h>
#include <VX/vx.h>
#include <utility.h>

/** \brief Input file name */
#define IN_FILE_NAME       "${VX_TEST_DATA_PATH}/colors.bmp"

/** \brief Output file name */
#define OUT_FILE_NAME      "${VX_TEST_DATA_PATH}/vx_tutorial_image_crop_roi.bmp"

vx_image  load_image_from_handle_from_file(
            vx_context context,
            char *filename,
            vx_bool convert_to_gray_scale,
            tivx_utils_bmp_image_params_t *imgParams);

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_image_crop_roi()
{
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image image, roi_image;
    vx_uint32 width, height;
    vx_rectangle_t rect;
    tivx_utils_bmp_image_params_t   imgParams;
    /** \endcode */

    printf(" vx_tutorial_image_crop_roi: Tutorial Started !!! \n");

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

    image = load_image_from_handle_from_file(context, IN_FILE_NAME, (vx_bool)vx_false_e, &imgParams);

    vxSetReferenceName((vx_reference)image, "ORIGINAL");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(image);
    /** \endcode */

    vxQueryImage(image, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    rect.start_x = width/4;
    rect.start_y = height/4;
    rect.end_x   = rect.start_x + width/2;
    rect.end_y   = rect.start_y + height/2;

    /**
     * - Create image from region of interest.
     *
     * Creates an image from another image given a rectangle
     * \code
     */
    roi_image = vxCreateImageFromROI(image, &rect);

    vxSetReferenceName((vx_reference)roi_image, "CROP_ROI");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(roi_image);
    /** \endcode */

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    /**
     * - Save image object to bitmap file \ref OUT_FILE_NAME.
     *
     * Follow the comments in tivx_utils_save_vximage_to_bmpfile() to see
     * how data in vx_image object is accessed to store pixel values from the image object to
     * BMP file \ref OUT_FILE_NAME
     * \code
     */
    tivx_utils_save_vximage_to_bmpfile(OUT_FILE_NAME, roi_image);
    /** \endcode */

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&roi_image);
    vxReleaseImage(&image);
    /** \endcode */

    /* now it is safe to release the memory passed to the image
     * NOTE: "roi_image" also holds a reference to the memory since
     *       its created from the "image"
     *       Hence memory passed to "image" should be freed after "roi_image"
     *       is released
     */
    tivx_utils_bmp_read_release(&imgParams);

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

    printf(" vx_tutorial_image_crop_roi: Tutorial Done !!! \n");
    printf(" \n");
}

/**
 * \brief Load image from handle from file
 *
 * \param context [in] Context
 * \param filename [in] BMP filename, MUST have extension of .bmp
 * \param convert_to_gray_scale [in] vx_true_e: Converts RGB values in BMP file to 8b grayscale value and copies them to image object\n
 *                                   vx_false_e: Retains RGB values from BMP file and copies them to image object\n
 * \param bmp_file_context [in] BMP file context
 *
 * \return Image data object from file. \n
 */
vx_image  load_image_from_handle_from_file(
            vx_context context,
            char *filename,
            vx_bool convert_to_gray_scale,
            tivx_utils_bmp_image_params_t *imgParams)
{
    char outFilePath[TIOVX_UTILS_MAXPATHLENGTH];
    vx_image image = NULL;
    uint32_t width, height, stride;
    vx_df_image df;
    vx_status vxStatus;
    int32_t     status;
    void *data_ptr;
    int32_t dcn = (convert_to_gray_scale != (vx_bool)(vx_bool)vx_false_e) ? 1 : -1;

    status = tivx_utils_expand_file_path(filename, outFilePath);

    status = tivx_utils_bmp_read(outFilePath, dcn, imgParams);

    if (status == 0)
    {
        vxStatus = (vx_status)VX_SUCCESS;
        width    = imgParams->width;
        height   = imgParams->height;
        stride   = imgParams->stride_y;
        df       = imgParams->format;
        data_ptr = imgParams->data;
    }
    else
    {
        vxStatus = (vx_status)VX_FAILURE;
    }

    if(vxStatus == (vx_status)VX_SUCCESS)
    {
        vx_imagepatch_addressing_t image_addr[1];
        void *ptrs[1];
        uint32_t bpp;

        if( df == (vx_df_image)VX_DF_IMAGE_U8 )
            bpp = 1;
        else
        if( df == (vx_df_image)VX_DF_IMAGE_RGB )
            bpp = 3;
        else
        if( df == (vx_df_image)VX_DF_IMAGE_RGBX )
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

       /**
         * - Create image from handle.
         *
         * Creates a reference to image object that was externally allocated
         * \code
         */
        image = vxCreateImageFromHandle(context, df, image_addr, ptrs, (vx_enum)VX_MEMORY_TYPE_HOST);
        /** \endcode */

        /* MUST not free memory that is passed to vxCreateImageFromHandle, until
         * that memory ptr is swapped out from the image
         * or the image handle is released
         */
        /* bmp_file_read_release(bmp_file_context); */
    }
    return image;
}

