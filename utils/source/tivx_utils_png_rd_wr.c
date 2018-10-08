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

#include <TI/tivx_mem.h>
#include <TI/tivx_debug.h>
#include <tivx_utils_file_rd_wr.h>
#include "png.h"

typedef struct
{
    png_bytep *row_pointers;
    png_byte *data_ptr;
    uint32_t row_pointers_size;
    uint32_t data_ptr_size;
} png_context_t;

/* we link to libpng v1.2 present by default on ubuntu,
 * but opencv used for DOF C model is compiled against
 * libpng v1.6, below function is not defined
 * in v1.6, hence to make the link succeed we define a empty
 * function as below.
 * NOTE: DOF C model API does not natively read/write PNG files
 *  so this is ok to do
 */
void png_set_longjmp_fn(png_structp *ptr, int val)
{

}

vx_status tivx_utils_png_file_read(
            char *filename,
            vx_bool convert_to_gray_scale,
            uint32_t *width,
            uint32_t *height,
            uint32_t *stride,
            vx_df_image *df,
            void **data_ptr,
            void **png_file_context)
{
    FILE *fp;
    vx_status status = VX_SUCCESS;

    *width = 0;
    *height = 0;
    *stride = 0;
    *df = VX_DF_IMAGE_U8;
    *png_file_context = NULL;

    fp = fopen(filename, "rb");
    if(fp==NULL)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, " PNG: unable to open file for reading [%s]\n", filename);
    }
    if(status==VX_SUCCESS)
    {
        uint8_t header[8];
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL;
        png_uint_32  png_width, png_height;
        int  bit_depth, color_type;
        int nbytes;

        nbytes = fread(header, 1, 8, fp);
        if ( nbytes < 8 || png_sig_cmp(header, 0, 8) != 0 )
        {
            status = VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, " PNG: Invalid PNG header [%s]\n", filename);
        }
        if(status==VX_SUCCESS)
        {
            png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (png_ptr==NULL)
            {
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR," PNG: Unable to alloc memory for read_struct [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr==NULL)
            {
                png_destroy_read_struct(&png_ptr, NULL, NULL);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: Unable to alloc memory for info_struct [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: setjmp failure [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {

            png_init_io(png_ptr, fp);
            png_set_sig_bytes(png_ptr, 8);
            png_read_info(png_ptr, info_ptr);

            png_width = png_get_image_width(png_ptr, info_ptr);
            png_height = png_get_image_height(png_ptr, info_ptr);
            color_type = png_get_color_type(png_ptr, info_ptr);
            bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        }
        if(status==VX_SUCCESS)
        {
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                status = VX_SUCCESS;
                VX_PRINT(VX_ZONE_ERROR, " PNG: setjmp failure [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            uint32_t y;
            png_context_t *png_context = tivxMemAlloc(sizeof(png_context_t), TIVX_MEM_EXTERNAL);

            if(png_context)
            {
                png_context->row_pointers_size = sizeof(png_bytep) * png_height;
                png_context->data_ptr_size = 0;

                png_context->row_pointers = (png_bytep*) tivxMemAlloc(png_context->row_pointers_size, TIVX_MEM_EXTERNAL);
                if(*data_ptr==NULL)
                {
                    /* allocate memory */
                    png_context->data_ptr_size = png_get_rowbytes(png_ptr,info_ptr) * png_height;
                    png_context->data_ptr = tivxMemAlloc(png_context->data_ptr_size, TIVX_MEM_EXTERNAL);
                }
                else
                {
                    /* use user provided memory */
                    png_context->data_ptr = *data_ptr;
                }
            }
            if(png_context == NULL || png_context->row_pointers == NULL || png_context->data_ptr == NULL)
            {
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: Unable to alloc memory for data_ptr, row_pointers [%s]\n", filename);
            }
            if(status==VX_SUCCESS)
            {
                for (y=0; y<png_height; y++)
                {
                    png_context->row_pointers[y] = (png_byte*) &png_context->data_ptr[y*png_get_rowbytes(png_ptr,info_ptr)];
                }
                png_read_image(png_ptr, png_context->row_pointers);

                *width = png_width;
                *height = png_height;
                *stride = png_get_rowbytes(png_ptr,info_ptr);
                *data_ptr = png_context->data_ptr;
                if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth == 8)
                {
                    *df = VX_DF_IMAGE_U8;
                }
                if(color_type == PNG_COLOR_TYPE_RGB)
                {
                    *df = VX_DF_IMAGE_RGB;
                }
                *png_file_context = png_context;

                png_read_end(png_ptr, NULL);
                png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            }
        }
        fclose(fp);
    }
    return status;
}

void tivx_utils_png_file_read_release(void *png_file_context)
{
    png_context_t *png_context = (png_context_t*)png_file_context;

    if(png_context)
    {
        if(png_context->row_pointers)
            tivxMemFree(png_context->row_pointers, png_context->row_pointers_size, TIVX_MEM_EXTERNAL);
        if(png_context->data_ptr && png_context->data_ptr_size > 0)
            tivxMemFree(png_context->data_ptr, png_context->data_ptr_size, TIVX_MEM_EXTERNAL);
        tivxMemFree(png_context, sizeof(png_context_t), TIVX_MEM_EXTERNAL);
    }
}

int32_t tivx_utils_png_file_write(
            char *filename,
            uint32_t width,
            uint32_t height,
            uint32_t stride,
            vx_df_image df,
            void *data_ptr)
{
    FILE *fp;
    vx_status status = VX_SUCCESS;

    fp = fopen(filename, "wb");
    if(fp==NULL)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, " PNG: unable to open file for writing [%s]\n", filename);
    }
    if(status==VX_SUCCESS)
    {
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL;
        int  bit_depth, color_type;
        int y;
        png_context_t *png_context = NULL;

        if(status==VX_SUCCESS)
        {
            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (png_ptr==NULL)
            {
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: Unable to alloc memory for write_struct [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr==NULL)
            {
                png_destroy_write_struct(&png_ptr, NULL);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: Unable to alloc memory for info_struct [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                png_destroy_write_struct(&png_ptr, &info_ptr);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: setjmp failure [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            png_init_io(png_ptr, fp);

            if (setjmp(png_jmpbuf(png_ptr)))
            {
                png_destroy_write_struct(&png_ptr, &info_ptr);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: setjmp failure [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            if(df==VX_DF_IMAGE_U8)
            {
                bit_depth = 8;
                color_type = PNG_COLOR_TYPE_GRAY;
            }
            else
            if(df==VX_DF_IMAGE_RGB)
            {
                bit_depth = 8;
                color_type = PNG_COLOR_TYPE_RGB;
            }
            else
            {
                bit_depth = 8;
                color_type = PNG_COLOR_TYPE_GRAY;
            }

            png_set_IHDR(png_ptr, info_ptr, width, height,
                     bit_depth, color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

            png_write_info(png_ptr, info_ptr);
        }
        if(status==VX_SUCCESS)
        {
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                png_destroy_write_struct(&png_ptr, &info_ptr);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: setjmp failure [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            png_context = tivxMemAlloc(sizeof(png_context_t), TIVX_MEM_EXTERNAL);

            if(png_context)
            {
                png_context->row_pointers_size = sizeof(png_bytep) * height;
                png_context->data_ptr_size = 0;
                png_context->row_pointers = (png_bytep*) tivxMemAlloc(png_context->row_pointers_size, TIVX_MEM_EXTERNAL);
                png_context->data_ptr = data_ptr;
            }
            if(png_context == NULL || png_context->row_pointers == NULL || png_context->data_ptr == NULL)
            {
                png_destroy_write_struct(&png_ptr, &info_ptr);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: Unable to alloc memory for data_ptr, row_pointers [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            for (y=0; y<height; y++)
            {
                png_context->row_pointers[y] = (png_byte*) &png_context->data_ptr[y*stride];
            }
            png_write_image(png_ptr, png_context->row_pointers);
        }
        if(status==VX_SUCCESS)
        {
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                png_destroy_write_struct(&png_ptr, &info_ptr);
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " PNG: setjmp failure [%s]\n", filename);
            }
        }
        if(status==VX_SUCCESS)
        {
            png_write_end(png_ptr, NULL);
            png_destroy_write_struct(&png_ptr, &info_ptr);
        }
        tivx_utils_png_file_read_release(png_context);
        fclose(fp);
    }
    return 0;
}

vx_image  tivx_utils_create_vximage_from_pngfile(vx_context context, char *filename, vx_bool convert_to_gray_scale)
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
    status = tivx_utils_png_file_read(
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
        tivx_utils_png_file_read_release(bmp_file_context);
        /** \endcode */
    }
    return image;
}

vx_status tivx_utils_save_vximage_to_pngfile(char *filename, vx_image image)
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
            tivx_utils_png_file_write(filename, width, height, image_addr.stride_y, df, data_ptr);
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

vx_status tivx_utils_load_vximage_from_pngfile(vx_image image, char *filename, vx_bool convert_to_gray_scale)
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

    status = tivx_utils_png_file_read(
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

        if(img_width>width)
            copy_width = width;
        else
            copy_width = img_width;

        if(img_height>height)
            copy_height = height;
        else
            copy_height = img_height;

        src_start_x = (width - copy_width)/2;
        src_start_y = (height - copy_height)/2;

        dst_start_x = (img_width - copy_width)/2;
        dst_start_y = (img_height - copy_height)/2;

        enable_rgb2gray = vx_false_e;
        enable_gray2rgb = vx_false_e;

        if(df!=img_df)
        {
            if(df==VX_DF_IMAGE_RGB && img_df==VX_DF_IMAGE_U8 && convert_to_gray_scale)
            {
                enable_rgb2gray = vx_true_e;
            }
            else
            if(df==VX_DF_IMAGE_U8 && img_df==VX_DF_IMAGE_RGB)
            {
                enable_gray2rgb = vx_true_e;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, " PNG: Image data format mismatch [%s]\n", filename);
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }

        #if 0
        printf(" PNG: src_start=(%d, %d), dst_start=(%d,%d), copy=(%dx%d), r2g=%d, g2r=%d\n",
            src_start_x, src_start_y, dst_start_x, dst_start_y, copy_width, copy_height, enable_rgb2gray, enable_gray2rgb);
        #endif

        if(status == VX_SUCCESS)
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

            rect.start_x = dst_start_x;
            rect.start_y = dst_start_y;
            rect.end_x = dst_start_x + copy_width;
            rect.end_y = dst_start_y + copy_height;

            data_ptr = (void*)((uint8_t*)data_ptr + (stride*src_start_y) + src_start_x*bpp);

            vxMapImagePatch(image,
                &rect,
                0,
                &map_id,
                &image_addr,
                &dst_data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                VX_NOGAP_X
                );

            if(!enable_rgb2gray && !enable_gray2rgb)
            {
                uint32_t y;

                for(y=0; y<copy_height; y++)
                {
                    memcpy(dst_data_ptr, data_ptr, copy_width*bpp);
                    data_ptr = (void*)((uint8_t*)data_ptr + stride);
                    dst_data_ptr = (void*)((uint8_t*)dst_data_ptr + image_addr.stride_y);
                }
            }
            else
            if(enable_rgb2gray)
            {
                uint32_t x, y, r, g, b;

                for(y=0; y<copy_height; y++)
                {
                    for(x=0; x<copy_width; x++)
                    {
                        b = ((uint8_t*)data_ptr)[3*x + 0];
                        g = ((uint8_t*)data_ptr)[3*x + 1];
                        r = ((uint8_t*)data_ptr)[3*x + 2];

                        ((uint8_t*)dst_data_ptr)[x] = (r+b+g)/3;
                    }
                    data_ptr = (void*)((uint8_t*)data_ptr + stride);
                    dst_data_ptr = (void*)((uint8_t*)dst_data_ptr + image_addr.stride_y);
                }
            }
            else
            if(enable_gray2rgb)
            {
                uint32_t x, y, g;

                for(y=0; y<copy_height; y++)
                {
                    for(x=0; x<copy_width; x++)
                    {
                        g = ((uint8_t*)data_ptr)[x];

                        ((uint8_t*)dst_data_ptr)[3*x+0] = g;
                        ((uint8_t*)dst_data_ptr)[3*x+1] = g;
                        ((uint8_t*)dst_data_ptr)[3*x+2] = g;
                    }
                    data_ptr = (void*)((uint8_t*)data_ptr + stride);
                    dst_data_ptr = (void*)((uint8_t*)dst_data_ptr + image_addr.stride_y);
                }
            }

            vxUnmapImagePatch(image, map_id);
        }
        tivx_utils_png_file_read_release(bmp_file_context);
    }
    return status;
}
