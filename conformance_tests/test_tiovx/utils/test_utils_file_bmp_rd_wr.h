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

/**
 * \file test_utils_file_rd_wr.h Utility APIs to read and write .BMP files
 *       using the low level CT API.
 */

#ifndef TEST_UTILS_FILE_BMP_RD_WR_H
#define TEST_UTILS_FILE_BMP_RD_WR_H

#include <VX/vx.h>

/**
 * \brief Read data from BMP file
 *
 * 'bmp_file_context' holds the BMP file context including internally
 *  allcoated memory/resource information
 *
 *  When data from 'data_ptr' is copied by the user application, the
 *  'bmp_file_context' MUST be released via bmp_file_read_release()
 *  in order to free any memory/resources allocated during bmp_file_read()
 *
 * \param filename [in] BMP file name. MUST have .bmp or .BMP extension
 * \param convert_to_gray_scale [in] TRUE: convert to gray scale after reading, FALSE: keep RGB format after reading
 * \param df [out] Data format of BMP file, will be VX_DF_IMAGE_U8 or VX_DF_IMAGE_RGB
 * \param width [out] Width of image in pixels
 * \param height [out] Height of image in lines
 * \param stride [out] Horizontal stride of image in units of bytes
 * \param data_ptr [in/out] Data buffer into which the BMP dara is read.
 *                       Allocated by this API internally when data_ptr = NULL,
 *                       else memory is allocated by user
 * \param bmp_file_context [out] BMP file context for this file
 *
 * \return VX_SUCCESS if file could be opened, parsed and read successfully
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_bmp_file_read(
            const char *filename,
            vx_bool convert_to_gray_scale,
            uint32_t *width,
            uint32_t *height,
            uint32_t *stride,
            vx_df_image *df,
            void **data_ptr,
            void **bmp_file_context);

/**
 * \brief Free memory allocated during bmp_file_read
 *
 *    MUST be called by application after its done using the pixel data returned bmp_file_read()
 *    The 'bmp_file_context' returned by the function bmp_file_read() must be used as input
 *    to this API.
 *
 *    'data_ptr' returned by bmp_file_read() MUST not be used after bmp_file_read_release() is called.
 *
 * \param bmp_file_context [in] BMP file context returned during bmp_file_read()
 *
 * \ingroup group_tivx_ext_host_utils
 *
 */
void test_utils_bmp_file_read_release(void *bmp_file_context);

/**
 * \brief Write data into BMP file
 *
 * \param filename [in] BMP file name. MUST have .bmp or .BMP extension
 * \param width [in] Width of image in pixels
 * \param height [in] Height of image in lines
 * \param stride [in] Stride of image in units of bytes
 * \param df [in] Data format of data written into BMP file, MUST be VX_DF_IMAGE_U8 or VX_DF_IMAGE_RGB
 * \param data_ptr [in] Data buffer from where the data is to written to BMP file
 *
 * \return VX_SUCCESS if file could be opened, and written successfully
 *
 * \ingroup group_tivx_ext_host_utils
 */
int32_t test_utils_bmp_file_write(
            const char *filename,
            uint32_t width,
            uint32_t height,
            uint32_t stride,
            vx_df_image df,
            void *data_ptr);

/**
 * \brief Create a image data object given BMP filename as input
 *
 * \param context [in] OpenVX context within which the image object will get created
 * \param filename [in] filename, MUST have extension of .bmp
 * \param convert_to_gray_scale [in] vx_true_e: Converts RGB values in BMP file to 8b grayscale value and copies them to image object\n
 *                                   vx_false_e: Retains RGB values from BMP file and copies them to image object\n
 *                                   NOTE: convert_to_gray_scale NOT supported as of NOW
 *
 * \return Image data object. \n
 *         Image data format is VX_DF_IMAGE_RGB when 'convert_to_gray_scale' is vx_false_e \n
 *         Image data format is VX_DF_IMAGE_U8 when 'convert_to_gray_scale' is vx_true_e
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_image  test_utils_create_vximage_from_bmpfile(vx_context context, const char *filename, vx_bool convert_to_gray_scale);


/**
 * \brief Save data from image object to BMP file
 *
 * \param filename [in] filename, MUST have extension of .bmp
 * \param image [in] Image data object. Image data format MUST be VX_DF_IMAGE_RGB or VX_DF_IMAGE_U8
 *
 * \return VX_SUCCESS if BMP could be created and saved with data from image object
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_save_vximage_to_bmpfile(const char *filename, vx_image image);


/**
 * \brief Load data from BMP file into a previously created vx_image object
 *
 * This function is same as create_vximage_from_bmpfile(). Only difference is
 * that the vx_image object is created outside this function.
 *
 * This function queries the vx_image object to make sure its attributes
 * match the attributes of the BMP file from which data needs to be loaded
 * into the image object. In case of mismatch data is not loaded and error is returned.
 *
 * \param image [in] Previouly created image object
 * \param filename [in] filename, MUST have extension of .bmp
 * \param convert_to_gray_scale [in] vx_true_e: Converts RGB values in BMP file to 8b grayscale value and copies them to image object\n
 *                                   vx_false_e: Retains RGB values from BMP file and copies them to image object\n
 *                                   NOTE: convert_to_gray_scale NOT supported as of NOW
 *
 * \return VX_SUCCESS if BMP file data could be loaded into the vx_image object.
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_load_vximage_from_bmpfile(vx_image image, char *filename, vx_bool convert_to_gray_scale);

/**
 * \brief Read BMP data from memory.
 *
 * 'bmp_file_context' holds the BMP file context including internally
 *  allcoated memory/resource information
 *
 *  When data from 'data_ptr' is copied by the user application, the
 *  'bmp_file_context' MUST be released via bmp_file_read_release()
 *  in order to free any memory/resources allocated during bmp_file_read()
 *
 * \param buf [in] Buffer address to read the BMP data from
 * \param convert_to_gray_scale [in] TRUE: convert to gray scale after reading, FALSE: keep RGB format after reading
 * \param df [out] Data format of BMP file, will be VX_DF_IMAGE_U8 or VX_DF_IMAGE_RGB
 * \param width [out] Width of image in pixels
 * \param height [out] Height of image in lines
 * \param stride [out] Horizontal stride of image in units of bytes
 * \param data_ptr [in/out] Data buffer into which the BMP dara is read.
 *                       Allocated by this API internally when data_ptr = NULL,
 *                       else memory is allocated by user
 * \param bmp_file_context [out] BMP file context for this file
 *
 * \return VX_SUCCESS if file could be opened, parsed and read successfully
 *
 * \ingroup group_tivx_ext_host_utils
 */
vx_status test_utils_bmp_file_read_from_memory(
            const void *buf,
            uint32_t buf_size,
            vx_bool convert_to_gray_scale,
            uint32_t *width,
            uint32_t *height,
            uint32_t *stride,
            vx_df_image *df,
            void **data_ptr,
            void **bmp_file_context);

#endif

