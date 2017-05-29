/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 * \file bmp_rd_wr.h Utility APIs to read and write .BMP files
 */

#ifndef BMP_RD_WR_H
#define BMP_RD_WR_H

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
 * \param data_ptr [out] Data buffer into which the BMP dara is read. Allocated by this API internally
 * \param bmp_file_context [out] BMP file context for this file
 *
 * \return VX_SUCCESS if file could be opened, parsed and read successfully
 */
vx_status bmp_file_read(
            char *filename,
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
 */
void bmp_file_read_release(void *bmp_file_context);

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
 */
int32_t bmp_file_write(
            char *filename,
            uint32_t width,
            uint32_t height,
            uint32_t stride,
            vx_df_image df,
            void *data_ptr);

#endif
