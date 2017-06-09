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



#include <bmp_rd_wr.h>
#include <test_engine/test.h>
#include <test_engine/test_image.h>

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
