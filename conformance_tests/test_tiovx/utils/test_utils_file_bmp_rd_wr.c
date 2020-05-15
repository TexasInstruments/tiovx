/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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

#include <TI/tivx_mem.h>
#include <TI/tivx_debug.h>
#include <test_engine/test.h>
#include <test_engine/test_image.h>
#include <test_engine/test_bmp.h>
#include <test_utils_file_bmp_rd_wr.h>

static CT_Image test_utils_load_ct_image_from_bmpfile(const char* fileName, int32_t dcn);
static CT_Image test_utils_load_ct_image_from_bmpfile_memory(const char* buf, int32_t bufsize, int32_t dcn);
static void test_utils_save_ct_image_to_bmpfile(const char* fileName, CT_Image image);

static CT_Image test_utils_load_ct_image_from_bmpfile(const char* fileName, int32_t dcn)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    CT_Image image = 0, returnVal = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        returnVal = 0;
    }
    else
    {
        sz = (uint32_t)snprintf(file, MAXPATHLENGTH, "%s", fileName);
        ASSERT_(return 0, (sz < MAXPATHLENGTH));

        f = fopen(file, "rb");
        if (!f)
        {
            CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
            returnVal = 0;
        }
        else
        {
            fseek(f, 0, SEEK_END);
            sz = (size_t)ftell(f);
            if( sz > 0U )
            {
                buf = (char*)ct_alloc_mem(sz);
                fseek(f, 0, SEEK_SET);
                if (NULL != buf)
                {
                    if( fread(buf, 1, sz, f) == sz )
                    {
                        image = ct_read_bmp((uint8_t*)buf, (int32_t)sz, dcn);
                        returnVal = image;
                    }
                }
                else
                {
                    fclose(f);
                    returnVal = NULL;
                }
            }

            if (returnVal != NULL)
            {
                ct_free_mem(buf);
                fclose(f);

                if(!image)
                {
                    CT_ADD_FAILURE("Can not read image from \"%s\"", fileName);
                }
            }
        }
    }

    return returnVal;
}

static CT_Image test_utils_load_ct_image_from_bmpfile_memory(const char* buf, int32_t bufsize, int32_t dcn)
{
    CT_Image image = 0;

    image = ct_read_bmp((const uint8_t*)buf, bufsize, dcn);

    return image;
}

static void test_utils_save_ct_image_to_bmpfile(const char* fileName, CT_Image image)
{
    char* dotpos;
    int32_t result = -1;
    size_t size;
    char file[MAXPATHLENGTH];

    if (fileName != NULL)
    {
        size = (size_t)snprintf(file, MAXPATHLENGTH, "%s", fileName);
        ASSERT(size < MAXPATHLENGTH);

        dotpos = strrchr(file, (int32_t)'.');
        if((dotpos != NULL) &&
           ((strcmp(dotpos, ".bmp") == 0) ||
            (strcmp(dotpos, ".BMP") == 0)))
        {
            result = ct_write_bmp(file, image);
        }
        if( result < 0 )
        {
            CT_ADD_FAILURE("Can not write image to \"%s\"", fileName);
        }
    }
    else
    {
        CT_ADD_FAILURE("Image name is not specified (NULL)");
    }
}

vx_status test_utils_bmp_file_read(
            const char *filename,
            vx_bool convert_to_gray_scale,
            uint32_t *width,
            uint32_t *height,
            uint32_t *stride,
            vx_df_image *df,
            void **data_ptr,
            void **bmp_file_context)
{
    CT_Image image = NULL;
    int32_t dcn = (convert_to_gray_scale != (vx_bool)(vx_bool)vx_false_e) ? 1 : -1;
    uint32_t bpp;
    vx_status status;

    /* workaround to enable CT context */
    CT_SetHasRunningTest();

    image = test_utils_load_ct_image_from_bmpfile(filename, dcn);

    if(image != NULL)
    {
        if( image->format == (vx_df_image)VX_DF_IMAGE_U8 )
        {
            bpp = 1;
        }
        else
        if( image->format == (vx_df_image)VX_DF_IMAGE_RGB )
        {
            bpp = 3;
        }
        else
        {
            bpp = 4; /* RGBX */
        }

        *width = image->width;
        *height = image->height;
        *stride = image->stride * bpp;
        *df = image->format;
        *data_ptr = image->data.y;

        *bmp_file_context = image;

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        *width = 0;
        *height = 0;
        *stride = 0;
        *data_ptr = NULL;

        *bmp_file_context = NULL;
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

vx_status test_utils_bmp_file_read_from_memory(
            const void *buf,
            uint32_t buf_size,
            vx_bool convert_to_gray_scale,
            uint32_t *width,
            uint32_t *height,
            uint32_t *stride,
            vx_df_image *df,
            void **data_ptr,
            void **bmp_file_context)
{
    CT_Image image = NULL;
    int32_t dcn = (convert_to_gray_scale != (vx_bool)(vx_bool)vx_false_e) ? 1 : -1;
    uint32_t bpp;
    vx_status status;

    /* workaround to enable CT context */
    CT_SetHasRunningTest();

    image = test_utils_load_ct_image_from_bmpfile_memory(buf, (int32_t)buf_size, dcn);

    if(image != NULL)
    {
        if( image->format == (vx_df_image)VX_DF_IMAGE_U8 )
        {
            bpp = 1;
        }
        else
        if( image->format == (vx_df_image)VX_DF_IMAGE_RGB )
        {
            bpp = 3;
        }
        else
        {
            bpp = 4; /* RGBX */
        }

        *width = image->width;
        *height = image->height;
        *stride = image->stride * bpp;
        *df = image->format;
        *data_ptr = image->data.y;

        *bmp_file_context = image;

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        *width = 0;
        *height = 0;
        *stride = 0;
        *data_ptr = NULL;

        *bmp_file_context = NULL;
        status = (vx_status)VX_FAILURE;
    }
    return status;
}

void test_utils_bmp_file_read_release(void *bmp_file_context)
{
    if(bmp_file_context != NULL)
    {
        CT_FreeObject(bmp_file_context);
    }
}

int32_t test_utils_bmp_file_write(
            const char *filename,
            uint32_t width,
            uint32_t height,
            uint32_t stride,
            vx_df_image df,
            void *data_ptr)
{

    /* workaround to enable CT context */
    CT_SetHasRunningTest();

    CT_Image image = NULL;
    uint32_t bpp;
    vx_status status;

    if( df == (vx_df_image)VX_DF_IMAGE_U8 )
    {
        bpp = 1;
    }
    else
    if( df == (vx_df_image)VX_DF_IMAGE_RGB )
    {
        bpp = 3;
    }
    else
    if( df == (vx_df_image)VX_DF_IMAGE_RGBX )
    {
        bpp = 4;
    }
    else
    {
        bpp = 0;
    }

    status = (vx_status)VX_FAILURE;
    if( bpp > 0U)
    {
        image = ct_allocate_image_hdr(width, height, stride/bpp, df, data_ptr);

        test_utils_save_ct_image_to_bmpfile(filename, image);

        CT_FreeObject(image);

        status = (vx_status)VX_SUCCESS;
    }
    return status;
}

vx_image  test_utils_create_vximage_from_bmpfile(vx_context context, const char *filename, vx_bool convert_to_gray_scale)
{
    vx_image image = NULL;
    uint32_t width, height, stride;
    vx_df_image df;
    vx_status status;
    void *data_ptr = NULL, *bmp_file_context;

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
    status = test_utils_bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);
    /** \endcode */

    if(status==(vx_status)VX_SUCCESS)
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
        if(status==(vx_status)VX_SUCCESS)
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

            if( df == (vx_df_image)VX_DF_IMAGE_U8 )
            {
                bpp = 1;
            }
            else
            if( df == (vx_df_image)VX_DF_IMAGE_RGB )
            {
                bpp = 3;
            }
            else
            if( df == (vx_df_image)VX_DF_IMAGE_RGBX )
            {
                bpp = 4;
            }
            else
            {
                bpp = 1; /* it should not reach here for BMP files */
            }

            image_addr.dim_x = width;
            image_addr.dim_y = height;
            image_addr.stride_x = (int32_t)bpp;
            image_addr.stride_y = (int32_t)stride;
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
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST
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
        test_utils_bmp_file_read_release(bmp_file_context);
        /** \endcode */
    }
    return image;
}

vx_status test_utils_save_vximage_to_bmpfile(const char *filename, vx_image image)
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
    if(status==(vx_status)VX_SUCCESS)
    {
        /** - Query image attributes.
         *
         *  These will be used to select ROI of data to be copied and
         *  and to set attributes of the BMP file
         * \code
         */

        vxQueryImage(image, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, (vx_enum)VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
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
            (vx_enum)VX_READ_ONLY,
            (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_NOGAP_X
            );
        /** \endcode */

        if(status==(vx_status)VX_SUCCESS)
        {
            /** - Write to BMP file using utility API
             *
             * 'image_addr.stride_y' is used to specify the offset in bytes between
             * two consecutive lines in memory. This is returned by vxMapImagePatch()
             * above
             * \code
             */
            test_utils_bmp_file_write(filename, width, height, (uint32_t)image_addr.stride_y, df, data_ptr);
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

vx_status test_utils_load_vximage_from_bmpfile(vx_image image, char *filename, vx_bool convert_to_gray_scale)
{
    uint32_t width, height, stride;
    vx_df_image df;
    uint32_t img_width, img_height;
    vx_df_image img_df;
    vx_status status;
    void *data_ptr = NULL, *bmp_file_context;
    void *dst_data_ptr = NULL;
    uint32_t copy_width, copy_height, src_start_x, src_start_y, dst_start_x, dst_start_y;
    vx_bool enable_rgb2gray, enable_gray2rgb;
    vx_map_id map_id;

    status = test_utils_bmp_file_read(
                filename,
                convert_to_gray_scale,
                &width, &height, &stride, &df, &data_ptr,
                &bmp_file_context);

    if(status==(vx_status)VX_SUCCESS)
    {
        img_height = 0;
        img_width = 0;

        vxQueryImage(image, (vx_enum)VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
        vxQueryImage(image, (vx_enum)VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
        vxQueryImage(image, (vx_enum)VX_IMAGE_FORMAT, &img_df, sizeof(vx_df_image));

        if(img_width>width)
        {
            copy_width = width;
        }
        else
        {
            copy_width = img_width;
        }

        if(img_height>height)
        {
            copy_height = height;
        }
        else
        {
            copy_height = img_height;
        }

        src_start_x = (width - copy_width)/2U;
        src_start_y = (height - copy_height)/2U;

        dst_start_x = (img_width - copy_width)/2U;
        dst_start_y = (img_height - copy_height)/2U;

        enable_rgb2gray = (vx_bool)vx_false_e;
        enable_gray2rgb = (vx_bool)vx_false_e;

        if(df!=img_df)
        {
            if((df==(vx_df_image)VX_DF_IMAGE_RGB) && (img_df==(vx_df_image)VX_DF_IMAGE_U8) && (convert_to_gray_scale != (vx_bool)vx_false_e))
            {
                enable_rgb2gray = (vx_bool)vx_true_e;
            }
            else
            if((df==(vx_df_image)VX_DF_IMAGE_U8) && (img_df==(vx_df_image)VX_DF_IMAGE_RGB))
            {
                enable_gray2rgb = (vx_bool)vx_true_e;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, " BMP: Image data format mismatch [%s]\n", filename);
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }

        #if 0
        printf(" BMP: src_start=(%d, %d), dst_start=(%d,%d), copy=(%dx%d), r2g=%d, g2r=%d\n",
            src_start_x, src_start_y, dst_start_x, dst_start_y, copy_width, copy_height, enable_rgb2gray, enable_gray2rgb);
        #endif

        if(status == (vx_status)VX_SUCCESS)
        {
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
            uint32_t bpp;

            if( df == (vx_df_image)VX_DF_IMAGE_U8 )
            {
                bpp = 1;
            }
            else
            if( df == (vx_df_image)VX_DF_IMAGE_RGB )
            {
                bpp = 3;
            }
            else
            if( df == (vx_df_image)VX_DF_IMAGE_RGBX )
            {
                bpp = 4;
            }
            else
            {
                bpp = 1; /* it should not reach here for BMP files */
            }

            rect.start_x = dst_start_x;
            rect.start_y = dst_start_y;
            rect.end_x = dst_start_x + copy_width;
            rect.end_y = dst_start_y + copy_height;

            data_ptr = (void*)((uint8_t*)data_ptr + (stride*src_start_y) + (src_start_x*bpp));

            vxMapImagePatch(image,
                &rect,
                0,
                &map_id,
                &image_addr,
                &dst_data_ptr,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_NOGAP_X
                );

            if((enable_rgb2gray == (vx_bool)(vx_bool)vx_false_e) && (enable_gray2rgb == (vx_bool)(vx_bool)vx_false_e))
            {
                uint32_t y;

                for(y=0; y<copy_height; y++)
                {
                    memcpy(dst_data_ptr, data_ptr, (size_t)copy_width * (size_t)bpp);
                    data_ptr = (void*)((uint8_t*)data_ptr + stride);
                    dst_data_ptr = (void*)((uint8_t*)dst_data_ptr + image_addr.stride_y);
                }
            }
            else
            if(enable_rgb2gray != 0)
            {
                uint32_t x, y, r, g, b;

                for(y=0; y<copy_height; y++)
                {
                    for(x=0; x<copy_width; x++)
                    {
                        b = ((uint8_t*)data_ptr)[(3U*x) + 0U];
                        g = ((uint8_t*)data_ptr)[(3U*x) + 1U];
                        r = ((uint8_t*)data_ptr)[(3U*x) + 2U];

                        ((uint8_t*)dst_data_ptr)[x] = (uint8_t)((r+b+g)/3U);
                    }
                    data_ptr = (void*)((uint8_t*)data_ptr + stride);
                    dst_data_ptr = (void*)((uint8_t*)dst_data_ptr + image_addr.stride_y);
                }
            }
            else
            if(enable_gray2rgb != 0)
            {
                uint32_t x, y, g;

                for(y=0; y<copy_height; y++)
                {
                    for(x=0; x<copy_width; x++)
                    {
                        g = ((uint8_t*)data_ptr)[x];

                        ((uint8_t*)dst_data_ptr)[(3U*x) + 0U] = (uint8_t)g;
                        ((uint8_t*)dst_data_ptr)[(3U*x) + 1U] = (uint8_t)g;
                        ((uint8_t*)dst_data_ptr)[(3U*x) + 2U] = (uint8_t)g;
                    }
                    data_ptr = (void*)((uint8_t*)data_ptr + stride);
                    dst_data_ptr = (void*)((uint8_t*)dst_data_ptr + image_addr.stride_y);
                }
            }
            else
            {
                /* do nothing */
            }

            vxUnmapImagePatch(image, map_id);
        }
        test_utils_bmp_file_read_release(bmp_file_context);
    }
    return status;
}

