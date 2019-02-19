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

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>
#include "tivx_utils_file_rd_wr.h"

#define MAX_ABS_FILENAME   (1024u)

TESTCASE(tivxPngRdWr, CT_VXContext, ct_setup_vx_context, 0)

static void make_filename(char *abs_filename, char *filename)
{
    snprintf(abs_filename, MAX_ABS_FILENAME, "%s/%s",
        ct_get_test_file_path(), filename);
}

TEST(tivxPngRdWr, testPngFileRdWr)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    uint32_t width, height, stride;
    vx_df_image df_image;
    char filename[MAX_ABS_FILENAME];
    void *data_ptr;
    void *png_context;
    vx_status status;

    make_filename(filename, "tivx/dof/tivx_test_ofTestCase1_10_pl0.png");

    /* when data_ptr = NULL,
     *   png_file_read allocates memory internally,
     * else
     *   memory MUST be allocated externally and MUST be >= expected image size
     */
    data_ptr = NULL;
    status = tivx_utils_png_file_read(filename, vx_false_e,
            &width, &height, &stride, &df_image,
            &data_ptr, &png_context);
    ASSERT(status==VX_SUCCESS);

    printf(" Reading file [%s], %d x %d, %d bytes, type=%08x\n",
        filename,
        width, height, stride, df_image);

    ASSERT(width == 256 && height == 128 && stride == 256 && df_image == VX_DF_IMAGE_U8);

    make_filename(filename, "output/tivx_test_ofTestCase1_10_pl0_out0.png");
    printf(" Writing file [%s], %d x %d, %d bytes, type=%08x\n",
        filename,
        width, height, stride, df_image);
    status = tivx_utils_png_file_write(filename,
            width, height, stride, df_image,
            data_ptr);
    ASSERT(status==VX_SUCCESS);

    /* MUST release context after data_ptr is no longer used */
    tivx_utils_png_file_read_release(png_context);
}

TEST(tivxPngRdWr, testPngVxImageRdWr)
{
    vx_context context = context_->vx_context_;
    vx_image image;
    uint32_t width, height;
    vx_df_image df_image;
    char filename[MAX_ABS_FILENAME];
    vx_status status;

    make_filename(filename, "tivx/dof/tivx_test_ofTestCase1_10_pl0.png");
    image = tivx_utils_create_vximage_from_pngfile(context, filename, vx_false_e);

    ASSERT_VX_OBJECT(image, VX_TYPE_IMAGE);
    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_FORMAT, &df_image, sizeof(vx_df_image));

    printf(" Created vx_image from file [%s], %d x %d, type=%08x\n",
        filename,
        width, height, df_image);

    ASSERT(width == 256 && height == 128 && df_image == VX_DF_IMAGE_U8);

    make_filename(filename, "output/tivx_test_ofTestCase1_10_pl0_out1.png");

    printf(" Saving vx_image to file [%s], %d x %d, type=%08x\n",
        filename,
        width, height, df_image);
    status = tivx_utils_save_vximage_to_pngfile(filename, image);
    ASSERT(status==VX_SUCCESS);


    make_filename(filename, "tivx/dof/tivx_test_ofTestCase1_10_pl0.png");
    status = tivx_utils_load_vximage_from_pngfile(image, filename, vx_false_e);

    ASSERT(status==VX_SUCCESS);

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_FORMAT, &df_image, sizeof(vx_df_image));

    printf(" Loaded into vx_image from file [%s], %d x %d, type=%08x\n",
        filename,
        width, height, df_image);

    vxReleaseImage(&image);
}

TESTCASE_TESTS(tivxPngRdWr, testPngFileRdWr, testPngVxImageRdWr)

