/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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
#include <TI/j7.h>
#include "test_engine/test.h"
#include <string.h>
#include "tivx_utils_file_rd_wr.h"
#include "tivx_utils_checksum.h"

TESTCASE(tivxHwaDmpacDof, CT_VXContext, ct_setup_vx_context, 0)

#define MAX_ABS_FILENAME   (1024u)

static void make_filename(char *abs_filename, char *filename_prefix, uint32_t level)
{
    snprintf(abs_filename, MAX_ABS_FILENAME, "%s/%s%d.bmp",
        ct_get_test_file_path(), filename_prefix, level);
}

static vx_status load_image_into_pyramid_level(vx_pyramid pyr, uint32_t level, char *filename_prefix)
{
    char filename[MAX_ABS_FILENAME];
    vx_image image;
    vx_status status = 0;

    make_filename(filename, filename_prefix, level);
    image = vxGetPyramidLevel(pyr, level);
    status = tivx_utils_load_vximage_from_bmpfile(image, filename, vx_true_e);
    vxReleaseImage(&image);
    return status;
}

static vx_status save_image_from_dof(vx_image flow_vector_img, vx_image confidence_img, char *filename_prefix)
{
    char filename[MAX_ABS_FILENAME];
    vx_status status;

    snprintf(filename, MAX_ABS_FILENAME, "%s/%s_flow_img.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, flow_vector_img);
    if(status == VX_SUCCESS)
    {
        snprintf(filename, MAX_ABS_FILENAME, "%s/%s_confidence_img.bmp",
            ct_get_test_file_path(), filename_prefix);

        status = tivx_utils_save_vximage_to_bmpfile(filename, confidence_img);
    }

    return status;
}

typedef struct {
    const char* testName;
    int median_filter;
    int motion_smoothness;
    int vertical_range;
    int horizontal_range;
    int iir_filter;
    int enable_lk;
} Arg;

static uint32_t dof_checksums_ref[3*3*3*3*2*2] = {
    (uint32_t) 0xaec927b9, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0x2c7120b7, (uint32_t) 0x2c7120b7,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0xcb156759, (uint32_t) 0xcb156759,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x1dbdd65d, (uint32_t) 0x1dbdd65d,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x3921f938, (uint32_t) 0x3921f938,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x2129d3b3, (uint32_t) 0x2129d3b3,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7,
    (uint32_t) 0x868f54c7, (uint32_t) 0x868f54c7
};

static uint32_t get_checksum(uint16_t median, uint16_t motion, uint16_t vert,
    uint16_t horiz, uint16_t iir, uint16_t lk)
{
    uint16_t a, b, c, d, e, f;
    a = median;
    b = motion / 15U;
    c = (vert - 28U) / 14U;
    d = (horiz - 85U) / 40U;
    e = (iir - 1U) / 127U;
    f = lk;
    return dof_checksums_ref[(3U * 3U * 3U * 3U * 2U * a) + (3U * 3U * 3U * 2U * b) + (3U * 3U * 2U * c) +
        (3U * 2U * d) + (2U*e) + f];
}

#define ADD_MEDIAN_FILTER(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/median_filter=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/median_filter=ON", __VA_ARGS__, 1))

#define ADD_MOTION_SMOOTHNESS_FACTOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/motion_smoothness_factor=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/motion_smoothness_factor=16", __VA_ARGS__, 16)), \
    CT_EXPAND(nextmacro(testArgName "/motion_smoothness_factor=31", __VA_ARGS__, 31))

#define ADD_VERTICAL_SEARCH_RANGE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/vertical_search_range=28", __VA_ARGS__, 28)), \
    CT_EXPAND(nextmacro(testArgName "/vertical_search_range=42", __VA_ARGS__, 42)), \
    CT_EXPAND(nextmacro(testArgName "/vertical_search_range=56", __VA_ARGS__, 56))

#define ADD_HORIZONTAL_SEARCH_RANGE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/horizontal_search_range=85", __VA_ARGS__, 85)), \
    CT_EXPAND(nextmacro(testArgName "/horizontal_search_range=130", __VA_ARGS__, 130)), \
    CT_EXPAND(nextmacro(testArgName "/horizontal_search_range=170", __VA_ARGS__, 170))

#define ADD_IIR_FILTER_ALPHA(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/iir_filter_alpha=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/iir_filter_alpha=128", __VA_ARGS__, 128)), \
    CT_EXPAND(nextmacro(testArgName "/iir_filter_alpha=255", __VA_ARGS__, 255))

#define ADD_ENABLE_LK(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/iir_filter_alpha=OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/iir_filter_alpha=ON", __VA_ARGS__, 1))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("dof_real_input", ADD_MEDIAN_FILTER, ADD_MOTION_SMOOTHNESS_FACTOR, ADD_VERTICAL_SEARCH_RANGE, ADD_HORIZONTAL_SEARCH_RANGE, ADD_IIR_FILTER_ALPHA, ADD_ENABLE_LK, ARG)


TEST_WITH_ARG(tivxHwaDmpacDof, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_image flow_vector_out_img = NULL, confidence_img = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_node node_dof_vis = 0;
    vx_status status;
    vx_rectangle_t rect;
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    char output_file[256];

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF))
    {
        uint32_t width = 256, height = 128;
        uint32_t levels = 2, i;
        vx_enum format = VX_DF_IMAGE_U8;
        rect.start_x = 4;
        rect.start_y = 4;
        rect.end_x = width - 4;
        rect.end_y = height - 4;

        tivxHwaLoadKernels(context);

        tivx_dmpac_dof_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.vertical_search_range[0] = arg_->vertical_range;
        params.vertical_search_range[1] = arg_->vertical_range;
        params.horizontal_search_range = arg_->horizontal_range;
        params.median_filter_enable = arg_->median_filter;
        params.motion_smoothness_factor = arg_->motion_smoothness;
        params.motion_direction = 1; /* 1: forward direction */
        params.iir_filter_alpha = arg_->iir_filter;
        params.enable_lk = arg_->enable_lk;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(flow_vector_in = vxCreateImage(context, width, height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, width, height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);

        ASSERT_VX_OBJECT(flow_vector_out_img = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_img = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_dof = tivxDmpacDofNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        input_current,
                        input_reference,
                        flow_vector_in,
                        NULL,
                        flow_vector_out,
                        confidence_histogram), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        ASSERT_VX_OBJECT(node_dof_vis = tivxDofVisualizeNode(graph,
                        flow_vector_out,
                        NULL,
                        flow_vector_out_img,
                        confidence_img), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof_vis, VX_TARGET_STRING, TIVX_TARGET_IPU1_0));

        VX_CALL(vxVerifyGraph(graph));

        for(i=0; i<levels; i++)
        {
            status = load_image_into_pyramid_level(input_current, i, "tivx/dof/tivx_test_ofTestCase1_10_pl");
            ASSERT(status==VX_SUCCESS);
            status = load_image_into_pyramid_level(input_reference, i, "tivx/dof/tivx_test_ofTestCase1_11_pl");
            ASSERT(status==VX_SUCCESS);
        }
        VX_CALL(vxProcessGraph(graph));

        sprintf(output_file, "output/tivx_test_ofTestCase1_%d_%d", arg_->median_filter, arg_->motion_smoothness);
        status = save_image_from_dof(flow_vector_out_img, confidence_img, output_file);
        ASSERT(status==VX_SUCCESS);

        checksum_expected = get_checksum(arg_->median_filter, arg_->motion_smoothness, arg_->vertical_range,
            arg_->horizontal_range, arg_->iir_filter, arg_->enable_lk);
        printf(" Expected checksum: %x\n", checksum_expected);
        checksum_actual = tivx_utils_simple_image_checksum(flow_vector_out_img, rect);
        printf(" Actual checksum: %x\n", checksum_actual);

        VX_CALL(vxReleaseNode(&node_dof));
        VX_CALL(vxReleaseNode(&node_dof_vis));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_in));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseDistribution(&confidence_histogram));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        VX_CALL(vxReleaseImage(&flow_vector_out_img));
        VX_CALL(vxReleaseImage(&confidence_img));

        ASSERT(node_dof == 0);
        ASSERT(node_dof_vis == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int negative_test;
    int condition;
} ArgNegative;

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=vertical-search_range_0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=vertical-search_range_1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=horizontal_search_range", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=max_horizontal_with_vertical-search_range", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=max_vertical_with_horizontal-search_range", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=median_filter_enable", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=motion_smoothness_factor", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=motion_direction", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=iir_filter_alpha", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=enable_lk", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=pyramid_divisibility", __VA_ARGS__, 10))


#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ARG)


TEST_WITH_ARG(tivxHwaDmpacDof, testNegativeGraph, ArgNegative,
    PARAMETERS_NEGATIVE)
{
    vx_context context = context_->vx_context_;
    vx_pyramid input_current = NULL, input_reference = NULL;
    vx_image flow_vector_in = NULL, flow_vector_out = NULL;
    vx_distribution confidence_histogram = NULL;
    tivx_dmpac_dof_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node_dof = 0;
    vx_status status;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF))
    {
        uint32_t width = 256, height = 128;
        uint32_t levels = 5;
        vx_enum format = VX_DF_IMAGE_U8;

        tivxHwaLoadKernels(context);

        tivx_dmpac_dof_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.vertical_search_range[0] = 48;
        params.vertical_search_range[1] = 48;
        params.horizontal_search_range = 191;
        params.median_filter_enable = 1;
        params.motion_smoothness_factor = 24;
        params.motion_direction = 1; /* 1: forward direction */

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 62;
                }
                else if (2U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 63;
                }
                else if (3U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 63;
                }
                else
                {
                    params.vertical_search_range[0U] = 63;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 62;
                }
                else if (2U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 63;
                }
                else if (3U == arg_->condition)
                {
                    params.vertical_search_range[1U] = 63;
                }
                else
                {
                    params.vertical_search_range[1U] = 63;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.horizontal_search_range = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.horizontal_search_range = 191;
                }
                else if (2U == arg_->condition)
                {
                    params.horizontal_search_range = 192;
                }
                else if (3U == arg_->condition)
                {
                    params.horizontal_search_range = 192;
                }
                else
                {
                    params.horizontal_search_range = 192;
                }
                break;
            }
            case 3:
            {
                params.horizontal_search_range = 191;
                if (0U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 0;
                    params.vertical_search_range[1U] = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 56;
                    params.vertical_search_range[1U] = 56;
                }
                else if (2U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 57;
                    params.vertical_search_range[1U] = 57;
                }
                else if (3U == arg_->condition)
                {
                    params.vertical_search_range[0U] = 57;
                    params.vertical_search_range[1U] = 57;
                }
                else
                {
                    params.vertical_search_range[0U] = 57;
                    params.vertical_search_range[1U] = 57;
                }
                break;
            }
            case 4:
            {
                params.vertical_search_range[0U] = 62;
                params.vertical_search_range[1U] = 62;
                if (0U == arg_->condition)
                {
                    params.horizontal_search_range = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.horizontal_search_range = 170;
                }
                else if (2U == arg_->condition)
                {
                    params.horizontal_search_range = 171;
                }
                else if (3U == arg_->condition)
                {
                    params.horizontal_search_range = 171;
                }
                else
                {
                    params.horizontal_search_range = 171;
                }
                break;
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    params.median_filter_enable = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.median_filter_enable = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.median_filter_enable = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.median_filter_enable = 2;
                }
                else
                {
                    params.median_filter_enable = 2;
                }
                break;
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    params.motion_smoothness_factor = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.motion_smoothness_factor = 31;
                }
                else if (2U == arg_->condition)
                {
                    params.motion_smoothness_factor = 32;
                }
                else if (3U == arg_->condition)
                {
                    params.motion_smoothness_factor = 32;
                }
                else
                {
                    params.motion_smoothness_factor = 32;
                }
                break;
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    params.motion_direction = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.motion_direction = 3;
                }
                else if (2U == arg_->condition)
                {
                    params.motion_direction = 4;
                }
                else if (3U == arg_->condition)
                {
                    params.motion_direction = 4;
                }
                else
                {
                    params.motion_direction = 4;
                }
                break;
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    params.iir_filter_alpha = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.iir_filter_alpha = 255;
                }
                else if (2U == arg_->condition)
                {
                    params.iir_filter_alpha = 0;
                }
                else if (3U == arg_->condition)
                {
                    params.iir_filter_alpha = 256;
                }
                else
                {
                    params.iir_filter_alpha = 256;
                }
                break;
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    params.enable_lk = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.enable_lk = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.enable_lk = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.enable_lk = 2;
                }
                else
                {
                    params.enable_lk = 2;
                }
                break;
            }
            case 10:
            {
                if (0U == arg_->condition)
                {
                    height = 64;
                }
                else if (1U == arg_->condition)
                {
                    height = 128;
                }
                else if (2U == arg_->condition)
                {
                    height = 144;
                }
                else if (3U == arg_->condition)
                {
                    height = 144;
                }
                else
                {
                    height = 144;
                }
                break;
            }
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_dmpac_dof_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_current = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(input_reference = vxCreatePyramid(context, levels, VX_SCALE_PYRAMID_HALF, width, height, format), VX_TYPE_PYRAMID);
        ASSERT_VX_OBJECT(flow_vector_in = vxCreateImage(context, width, height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(flow_vector_out = vxCreateImage(context, width, height, VX_DF_IMAGE_U32), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(confidence_histogram = vxCreateDistribution(context, 16, 0, 16), VX_TYPE_DISTRIBUTION);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_dof = tivxDmpacDofNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        input_current,
                        input_reference,
                        flow_vector_in,
                        NULL,
                        flow_vector_out,
                        confidence_histogram), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_dof, VX_TARGET_STRING, TIVX_TARGET_DMPAC_DOF));

        if(2 > arg_->condition)
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }
        VX_CALL(vxReleaseNode(&node_dof));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleasePyramid(&input_current));
        VX_CALL(vxReleasePyramid(&input_reference));
        VX_CALL(vxReleaseImage(&flow_vector_in));
        VX_CALL(vxReleaseImage(&flow_vector_out));
        VX_CALL(vxReleaseDistribution(&confidence_histogram));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node_dof == 0);
        ASSERT(graph == 0);
        ASSERT(input_current == 0);
        ASSERT(input_reference == 0);
        ASSERT(flow_vector_in == 0);
        ASSERT(flow_vector_out == 0);
        ASSERT(confidence_histogram == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TESTCASE_TESTS(tivxHwaDmpacDof, testGraphProcessing, testNegativeGraph)

