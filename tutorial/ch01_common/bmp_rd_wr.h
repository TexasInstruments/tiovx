/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef BMP_RD_WR_H
#define BMP_RD_WR_H

#include <VX/vx.h>

/**
 * \brief Read data from BMP file
 *
 * \param filename [in] BMP file name. MUST have .bmp or .BMP extension
 * \param convert_to_gray_scale [in] TRUE: convert to gray scale after reading, FALSE: RGB format
 * \param df [out] Data format of BMP file, MUST be U8 or RGB or RGBX
 * \param width [out] Width of image in pixels
 * \param height [out] Height of image in lines
 * \param stride [out] Stride of image in units of bytes
 * \param data_ptr [out] Data buffer into which the BMP dara is read
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
 *    MUST be called after bmp_file_read using the bmp_file_context returned by the
 *    function bmp_file_read
 *
 *    data_ptr MUST not be after bmp_file_read_release is called.
 *
 * \param data_ptr [in] Data buffer into which the BMP data was previously read
 *
 */
void bmp_file_read_release(void *bmp_file_context);

/**
 * \brief Write data into BMP file
 *
 * \param filename [in] BMP file name. MUST have .bmp or .BMP extension
 * \param df [in] Data format of data written into BMP file, MUST be U8 or RGB or RGBX
 * \param width [in] Width of image in pixels
 * \param height [in] Height of image in lines
 * \param stride [in] Stride of image in units of bytes
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
