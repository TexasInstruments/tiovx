/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
 *
 */

#if !defined(_TIVX_UTILS_BMP_H_)
#define _TIVX_UTILS_BMP_H_

#include <stdio.h>
#include <stdint.h>

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    /** Image width. */
    uint32_t            width;

    /** Image height. */
    uint32_t            height;

    /** Number of bits per pixel. */
    uint16_t            bpp;

    /** Stride. */
    uint32_t            stride_y;

    /** Image format. */
    vx_df_image         format;

    /** Image data. */
    uint8_t            *data;

    /** Image data size. */
    uint32_t            dataSize;

} tivx_utils_bmp_image_params_t;

int32_t tivx_utils_bmp_read_mem(const uint8_t                  *data,
                                uint32_t                        dataSize,
                                int32_t                         dcn,
                                tivx_utils_bmp_image_params_t  *imgParams);

int32_t tivx_utils_bmp_read(const char                     *filename,
                            int32_t                         dcn,
                            tivx_utils_bmp_image_params_t  *imgParams);

int32_t tivx_utils_bmp_read_release(tivx_utils_bmp_image_params_t  *imgParams);

int32_t tivx_utils_bmp_write(const char    *filename,
                             const uint8_t *data,
                             int32_t        width,
                             int32_t        height,
                             int32_t        stride_y,
                             vx_df_image    df);

#ifdef __cplusplus
}
#endif

#endif /* _TIVX_UTILS_BMP_H_ */

