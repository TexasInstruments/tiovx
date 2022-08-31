/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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
#ifdef BUILD_VPAC_LDC

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <math.h>
#include <float.h>
#include "tivx_utils_checksum.h"
#include "test_hwa_common.h"
#include <utils/iss/include/app_iss.h>

#ifndef M_PI
#define M_PIF   3.14159265358979323846f
#else
#define M_PIF   (vx_float32)M_PI
#endif

TESTCASE(tivxHwaVpacLdc, CT_VXContext, ct_setup_vx_context, 0)

#define TEST_NUM_NODE_INSTANCE 2

/* #define TEST_VISS_PERFORMANCE_LOGGING */

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string, *target_string_2;
} SetTarget_Arg;

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_VPAC2_LDC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC2_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_VPAC_LDC1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_LDC1/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC2_LDC1, TIVX_TARGET_VPAC2_LDC1))
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_VPAC_LDC1))
#endif

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC2_LDC1, NULL))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_LDC1", __VA_ARGS__, TIVX_TARGET_VPAC_LDC1, NULL))
#endif

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacLdc, testNodeCreation, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_matrix matrix = 0;
    vx_user_data_object param_obj;
    vx_user_data_object region_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_enum matrix_type = VX_TYPE_INT16;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    const vx_size matrix_data_size = 2 * matrix_rows * matrix_cols;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);

        {
            vx_enum ch_matrix_type;
            vx_size ch_matrix_rows, ch_matrix_cols, ch_data_size;

            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_TYPE, &ch_matrix_type, sizeof(ch_matrix_type)));
            if (matrix_type != ch_matrix_type)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_TYPE failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_ROWS, &ch_matrix_rows, sizeof(ch_matrix_rows)));
            if (matrix_rows != ch_matrix_rows)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_ROWS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &ch_matrix_cols, sizeof(ch_matrix_cols)));
            if (matrix_cols != ch_matrix_cols)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_COLUMNS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_SIZE, &ch_data_size, sizeof(ch_data_size)));
            if (matrix_data_size > ch_data_size)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_SIZE failed\n");
            }
        }

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                matrix,
                                region_obj,
                                NULL,
                                NULL,
                                NULL,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static vx_status readNV12Input(char* file_name, vx_image in_img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)in_img);

    if(status == VX_SUCCESS)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_size arr_len;
        vx_int32 i, j;

        if(fp == NULL)
        {
            printf("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            uint8_t *data_ptr_8;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32 img_format;
            vx_uint32  num_bytes = 0;

            vxQueryImage(in_img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(in_img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_uint32));

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = img_width;
            rect.end_y = img_height;
            status = vxMapImagePatch(in_img,
                                    &rect,
                                    0,
                                    &map_id,
                                    &image_addr,
                                    &data_ptr,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X);

            /* Copy Luma */
            data_ptr_8 = (uint8_t *)data_ptr;
            for (j = 0; j < img_height; j++)
            {
                num_bytes += fread(data_ptr_8, 1, img_width, fp);
                data_ptr_8 += image_addr.stride_y;
            }

            if(num_bytes != (img_width*img_height)) {
                printf("Luma bytes read = %d, expected = %d\n", num_bytes, img_width*img_height);
                return (VX_FAILURE);
            }

            vxUnmapImagePatch(in_img, map_id);

            if(img_format == VX_DF_IMAGE_NV12)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height / 2;
                status = vxMapImagePatch(in_img,
                                        &rect,
                                        1,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);


                /* Copy CbCr */
                num_bytes = 0;
                data_ptr_8 = (uint8_t *)data_ptr;
                for (j = 0; j < img_height/2; j++)
                {
                    num_bytes += fread(data_ptr_8, 1, img_width, fp);
                    data_ptr_8 += image_addr.stride_y;
                }

                if(num_bytes != (img_width*img_height/2)) {
                    printf("CbCr bytes read = %d, expected = %d\n", num_bytes, img_width*img_height/2);
                    return (VX_FAILURE);
                }

                vxUnmapImagePatch(in_img, map_id);
            }
        }

        fclose(fp);
    }

    return(status);
}

static vx_status save_image_from_ldc(vx_image y8, char *filename_prefix)
{
    char filename[MAXPATHLENGTH];
    vx_status status;

    snprintf(filename, MAXPATHLENGTH, "%s/%s_y8.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, y8);

    return status;
}

TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingDcc, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    tivx_vpac_ldc_params_t params;

    vx_user_data_object dcc_param_ldc = NULL;
    const vx_char dcc_ldc_user_data_object_name[] = "dcc_ldc";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_ldc_buf_map_id;
    uint8_t * dcc_ldc_buf;
    int32_t dcc_status;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    char file[MAXPATHLENGTH];
    uint32_t checksum_expected[2] = {0x00f3ac34, 0xc8295b0e};
    uint32_t checksum_actual;
    size_t sz;
    vx_rectangle_t rect;

    sensor_name = SENSOR_SONY_IMX390_UB953_D3;
    sensor_dcc_mode = 0;
    sensor_dcc_id = 390;
    file_name = "psdkra/app_single_cam/IMX390_001/0_output1.yuv";

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 1920, 1080, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), file_name);
        ASSERT_(return, (sz < MAXPATHLENGTH));

        VX_CALL(readNV12Input(file, input));

        tivx_vpac_ldc_params_init(&params);
        params.luma_interpolation_type = 1;
        params.dcc_camera_id = sensor_dcc_id;

        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeLDC(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_ldc = vxCreateUserDataObject( context, (const vx_char*)&dcc_ldc_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_ldc,
            0,
            dcc_buff_size,
            &dcc_ldc_buf_map_id,
            (void **)&dcc_ldc_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_ldc_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffLDC(sensor_name, sensor_dcc_mode, dcc_ldc_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_ldc, dcc_ldc_buf_map_id));
        /* Done w/ DCC */

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                dcc_param_ldc,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxVerifyGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 1920;
        rect.end_y = 1080;

        checksum_actual = tivx_utils_simple_image_checksum(output, 0, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(checksum_expected[0] == checksum_actual);
        rect.end_y /= 2;
        checksum_actual = tivx_utils_simple_image_checksum(output, 1, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(checksum_expected[1] == checksum_actual);

        //save_image_from_ldc(input, "output/ldcin_y8");
        //save_image_from_ldc(output, "output/ldcout_y8");

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_ldc));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(dcc_param_ldc == 0);

        tivxHwaUnLoadKernels(context);
    }
}


enum CT_AffineMatrixType {
    VX_MATRIX_IDENT = 0,
    VX_MATRIX_ROTATE_90,
    VX_MATRIX_SCALE,
    VX_MATRIX_SCALE_ROTATE,
    VX_MATRIX_RANDOM,
    VX_MATRIX_MIRROR
};

#define VX_NN_AREA_SIZE         1.5
#define VX_BILINEAR_TOLERANCE   1

static CT_Image warp_affine_read_image_8u(const char* fileName, int width, int height)
{
    CT_Image image = NULL;

    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);

    return image;
}

static CT_Image warp_affine_generate_random(const char* fileName, int width, int height)
{
    CT_Image image;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256));

    return image;
}

static vx_float32 ldc_clip(vx_float32 value, vx_float32 clip_value)
{
    vx_float32 output = value;

    if(value > clip_value)
    {
        output = clip_value;
    }
    else if(value < -clip_value)
    {
        output = -clip_value;
    }
    return output;
}

#define RND_FLT(low, high)      (vx_float32)CT_RNG_NEXT_REAL(CT()->seed_, low, high);
static void warp_affine_generate_matrix(vx_float32* m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][2];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_ROTATE_90 == type)
    {
        mat[0][0] = 0.f;
        mat[0][1] = 1.f;

        mat[1][0] = -1.f;
        mat[1][1] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_MIRROR == type)
    {
        mat[0][0] = -1.f;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = ldc_clip(src_width / (vx_float32)dst_width, 7.9999f);
        scale_y = ldc_clip(src_height / (vx_float32)dst_height, 7.9999f);

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
    }
    else// if (VX_MATRIX_RANDOM == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
        mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_affine_create_matrix(vx_context context, vx_float32 *m, int32_t natc)
{
    vx_matrix matrix;
    if(natc)
    {
        matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
    else
    {
        matrix = vxCreateMatrix(context, VX_TYPE_INT16, 2, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            vx_int16 mfixed[6];

            mfixed[0] = (vx_int16)(ldc_clip(m[0]*(4096.0f),32767));
            mfixed[1] = (vx_int16)(ldc_clip(m[1]*(4096.0f),32767));
            mfixed[2] = (vx_int16)(ldc_clip(m[2]*(4096.0f),32767));
            mfixed[3] = (vx_int16)(ldc_clip(m[3]*(4096.0f),32767));
            mfixed[4] = (vx_int16)(ldc_clip(m[4]*(8.0f),32767));
            mfixed[5] = (vx_int16)(ldc_clip(m[5]*(8.0f),32767));

            if (VX_SUCCESS != vxCopyMatrix(matrix, mfixed, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
}


static int warp_affine_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_float64 x0, y0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    x0 = (vx_float64)m[2 * 0 + 0] * (vx_float64)x + (vx_float64)m[2 * 1 + 0] * (vx_float64)y + (vx_float64)m[2 * 2 + 0];
    y0 = (vx_float64)m[2 * 0 + 1] * (vx_float64)x + (vx_float64)m[2 * 1 + 1] * (vx_float64)y + (vx_float64)m[2 * 2 + 1];

    if (VX_INTERPOLATION_NEAREST_NEIGHBOR == interp_type)
    {
        for (yi = (vx_int32)ceil(y0 - VX_NN_AREA_SIZE); (vx_float64)yi <= y0 + VX_NN_AREA_SIZE; yi++)
        {
            for (xi = (vx_int32)ceil(x0 - VX_NN_AREA_SIZE); (vx_float64)xi <= x0 + VX_NN_AREA_SIZE; xi++)
            {
                if (0 < xi && 0 < yi && xi < (vx_int32)input->width-1 && yi < (vx_int32)input->height-1)
                {
                    candidate = *CT_IMAGE_DATA_PTR_8U(input, xi, yi);
                }
                else if (VX_BORDER_CONSTANT == border.mode)
                {
                    candidate = border.constant_value.U8;
                }
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    else if (VX_INTERPOLATION_BILINEAR == interp_type)
    {
        xlower = floor(x0);
        ylower = floor(y0);

        s = x0 - xlower;
        t = y0 - ylower;

        xi = (vx_int32)xlower;
        yi = (vx_int32)ylower;

        candidate = -1;
        if (VX_BORDER_UNDEFINED == border.mode)
        {
            if (1 <= xi && 1 <= yi && xi < (vx_int32)input->width - 2 && yi < (vx_int32)input->height - 2)
            {
                candidate = (int)((1. - s) * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi    ) +
                                        s  * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi    ) +
                                  (1. - s) *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi + 1) +
                                        s  *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi + 1));
            }
        }
        else if (VX_BORDER_CONSTANT == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        else if (VX_BORDER_REPLICATE == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi    ) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi    ) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi + 1) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi + 1));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    CT_FAIL_(return 1, "Interpolation type undefined");
}

static void warp_affine_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += warp_affine_check_pixel(input, output, x, y, interp_type, border, m));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static void warp_affine_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    ASSERT(input && output);
    ASSERT( (interp_type == 0) ||
            (interp_type == 1) ||
            (interp_type == VX_MATRIX_MIRROR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT) ||
            (border.mode == VX_BORDER_REPLICATE));

    warp_affine_validate(input, output, VX_INTERPOLATION_BILINEAR, border, m);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
        printf("Matrix:\n%g %g %g\n%g %g %g\n",
                m[0], m[2], m[4],
                m[1], m[3], m[5]);
    }
}

static vx_image mesh_create(vx_context context, int width, int height, int m)
{
    CT_Image image;
    int factor = 1<<m;
    int table_width = ((width+(factor-1))>>m) + 1;
    int table_height = ((height+(factor-1))>>m) + 1;
    vx_image mesh = 0;

    ASSERT_NO_FAILURE_(return 0,
            image = ct_allocate_image(table_width, table_height, VX_DF_IMAGE_U32));
    /* Simple mesh where every output pixel maps to the input which is:
     *   X:  8 pixels to the right (8 << 3 = 64)
     *   Y:  16 pixels down (16 << 3 = 128)
     */
    CT_FILL_IMAGE_32U(return 0, image,
        {
            *dst_data = ((64 << 16) | 128);
        });

    ASSERT_VX_OBJECT_(return 0,
            mesh = ct_image_to_vx_image(image, context), VX_TYPE_IMAGE);

    return mesh;
}

static vx_user_data_object lut_create(
    vx_context context, void* data, vx_size count)
{
    tivx_vpac_ldc_bit_depth_conv_lut_params_t remap_lut;
    vx_user_data_object lut_obj;
    vx_size size = sizeof(vx_uint16);
    void* ptr = NULL;

    lut_obj = vxCreateUserDataObject(context,
        "tivx_vpac_ldc_bit_depth_conv_lut_params_t",
        sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t), NULL);

    ASSERT_VX_OBJECT_(return 0, lut_obj,
        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

    remap_lut.input_bits = 12;
    remap_lut.output_bits = 8;

    memcpy(remap_lut.lut, data, size *
        TIVX_VPAC_LDC_BIT_DEPTH_CONV_LUT_SIZE);

    if (VX_SUCCESS != vxCopyUserDataObject(lut_obj, 0,
            sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t), &remap_lut,
            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
    {
        VX_CALL_(return 0, vxReleaseUserDataObject(&lut_obj));
    }

    return lut_obj;
}

static void lut_data_fill_identity(void* data, vx_size count)
{
    int i;

    vx_uint16* data16 = (vx_uint16*)data;
    for (i = 0; i < count; ++i)
        data16[i] = i;
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int src_width, src_height;
    int out_width, out_height;
    vx_border_t border;
    vx_enum interp_type;
    int matrix_type;
    int input_mode;
    int output_mode;
    int mesh_mode;
    char *target_string, *target_string_2;
} Arg;

typedef struct {
    const char* testName;
    int input_data_format;
    int output_data_format;
    char *target_string, *target_string_2;
} ArgFormats;

typedef struct {
    const char* testName;
    int negative_test;
    int condition;
    char *target_string, *target_string_2;
} ArgNegative;

#define ADD_VX_BORDERS_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} }))

#define ADD_VX_INTERPOLATION_TYPE_BILINEAR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/INTERPOLATION_BILINEAR", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/INTERPOLATION_BICUBIC", __VA_ARGS__, 0))

#define ADD_VX_MATRIX_PARAM_WARP_AFFINE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_IDENT", __VA_ARGS__,        VX_MATRIX_IDENT)), \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_ROTATE_90", __VA_ARGS__,    VX_MATRIX_ROTATE_90)), \
    CT_EXPAND(nextmacro(testArgName "/VX_MATRIX_SCALE", __VA_ARGS__,        VX_MATRIX_SCALE))

#define ADD_VX_INPUT_MODES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LUMA_ONLY", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/LUMA_CHROMA_420", __VA_ARGS__, 1))

#define ADD_VX_INPUT_MODES_LENA(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LUMA_ONLY", __VA_ARGS__, 0))

#define ADD_VX_OUTPUT_MODES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/DUAL_OUT_OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/DUAL_OUT_ON", __VA_ARGS__, 1))

#define ADD_VX_MESH_MODES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/MESH_OFF", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/MESH_M0", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/MESH_M1", __VA_ARGS__, 2))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_SMALL_SET, ADD_VX_BORDERS_WARP_AFFINE, ADD_VX_INTERPOLATION_TYPE_BILINEAR, ADD_VX_MATRIX_PARAM_WARP_AFFINE, ADD_VX_INPUT_MODES_LENA, ADD_VX_OUTPUT_MODES, ADD_VX_MESH_MODES, ADD_SET_TARGET_PARAMETERS, ARG, warp_affine_read_image_8u, "lena.bmp", 0, 0)


#define PARAMETERS_MULTI_INST \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_SMALL_SET, ADD_VX_BORDERS_WARP_AFFINE, ADD_VX_INTERPOLATION_TYPE_BILINEAR, ADD_VX_MATRIX_PARAM_WARP_AFFINE, ADD_VX_INPUT_MODES_LENA, ADD_VX_OUTPUT_MODES, ADD_VX_MESH_MODES, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG, warp_affine_read_image_8u, "lena.bmp", 0, 0)

#define ADD_VX_IN_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_NV12", __VA_ARGS__, VX_DF_IMAGE_NV12)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_NV12_P12", __VA_ARGS__, TIVX_DF_IMAGE_NV12_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_UYVY", __VA_ARGS__, VX_DF_IMAGE_UYVY)), \

#define ADD_VX_OUT_FORMATS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U8", __VA_ARGS__, VX_DF_IMAGE_U8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_U16", __VA_ARGS__, VX_DF_IMAGE_U16)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_P12", __VA_ARGS__, TIVX_DF_IMAGE_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_NV12", __VA_ARGS__, VX_DF_IMAGE_NV12)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_DF_IMAGE_NV12_P12", __VA_ARGS__, TIVX_DF_IMAGE_NV12_P12)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_UYVY", __VA_ARGS__, VX_DF_IMAGE_UYVY)), \
    CT_EXPAND(nextmacro(testArgName "/VX_DF_IMAGE_YUYV", __VA_ARGS__, VX_DF_IMAGE_YUYV))

#define PARAMETERS_FORMATS \
    CT_GENERATE_PARAMETERS("testFormats", ADD_VX_IN_FORMATS, ADD_VX_OUT_FORMATS, ADD_SET_TARGET_PARAMETERS, ARG)


#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=input_align_12bit", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=luma_interpolation_type", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=init_x", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=init_y", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=yc_mode", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=enable", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=out_block_width", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=out_block_height", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=pixel_pad", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=subsample_factor", __VA_ARGS__, 9))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))

#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ADD_SET_TARGET_PARAMETERS, ARG)

static uint32_t ldc_checksums_ref[3*2*3*2*3] = {
    0x362f22f2, 0xd6dcdec7, 0xd6dcdec7, 0x362f22f2,
    0xd6dcdec7, 0xd6dcdec7, 0x463605cc, 0xdbcbcbf0,
    0xdbcbcbf0, 0x463605cc, 0xdbcbcbf0, 0xdbcbcbf0,
    0x72ae30cb, 0xcfcfdcd7, 0xcfcfdcd7, 0x72ae30cb,
    0xcfcfdcd7, 0xcfcfdcd7, 0x2b2516c4, 0xd6dcdec7,
    0xd6dcdec7, 0x2b2516c4, 0xd6dcdec7, 0xd6dcdec7,
    0x4639062e, 0xdbcbcbf0, 0xdbcbcbf0, 0x4639062e,
    0xdbcbcbf0, 0xdbcbcbf0, 0x74ab2ebd, 0xcfcfdcd7,
    0xcfcfdcd7, 0x74ab2ebd, 0xcfcfdcd7, 0xcfcfdcd7,
    0xc2876133, 0xb9237446, 0xb9237446, 0xc2876133,
    0xb9237446, 0xb9237446, 0xc85d43e7, 0xf6012400,
    0xf6012400, 0xc85d43e7, 0xf6012400, 0xf6012400,
    0x26210e60, 0x5459597a, 0x5459597a, 0x26210e60,
    0x5459597a, 0x5459597a, 0xaa634fd0, 0xb9237446,
    0xb9237446, 0xaa634fd0, 0xb9237446, 0xb9237446,
    0xc85d46d0, 0xfafb2600, 0xfafb2600, 0xc85d46d0,
    0xfafb2600, 0xfafb2600, 0x24f65b78, 0x349b9bb2,
    0x349b9bb2, 0x24f65b78, 0x349b9bb2, 0x349b9bb2,
    0xde2e8367, 0x45626142, 0x45626142, 0xde2e8367,
    0x45626142, 0x45626142, 0x4b8f8729, 0xe68094ac,
    0xe68094ac, 0x4b8f8729, 0xe68094ac, 0xe68094ac,
    0xde2e8367, 0x45626142, 0x45626142, 0xde2e8367,
    0x45626142, 0x45626142, 0xfd389db2, 0x7e2f3ed8,
    0x7e2f3ed8, 0xfd389db2, 0x7e2f3ed8, 0x7e2f3ed8,
    0x579899f6, 0xe26696a4, 0xe26696a4, 0x579899f6,
    0xe26696a4, 0xe26696a4, 0xfd389db2, 0x7e2f3ed8,
    0x7e2f3ed8, 0xfd389db2, 0x7e2f3ed8, 0x7e2f3ed8
};

static uint32_t get_checksum(int out_width, vx_enum interp, int matrix, int output, int mesh)
{
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
    uint16_t e;

    if (16 == out_width)
    {
        a = 0U;
    }
    else if (256 == out_width)
    {
        a = 1U;
    }
    else
    {
        a = 2U;
    }

    if (1 == interp)
    {
        b = 0U;
    }
    else
    {
        b = 1U;
    }

    if (VX_MATRIX_IDENT == matrix)
    {
        c = 0U;
    }
    else if (VX_MATRIX_ROTATE_90 == matrix)
    {
        c = 1U;
    }
    else
    {
        c = 2U;
    }

    d = (uint16_t) output;
    e = (uint16_t) mesh;

    return ldc_checksums_ref[(2*3*2*3*a)+(3*2*3*b)+(2*3*c)+(3*d)+e];
}

TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessing, Arg,
    PARAMETERS_MULTI_INST
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj[TEST_NUM_NODE_INSTANCE] = {0};
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj[TEST_NUM_NODE_INSTANCE] = {0};
    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image input_image[TEST_NUM_NODE_INSTANCE] = {0}, output_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image dual_out_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_image mesh_image[TEST_NUM_NODE_INSTANCE] = {0};
    vx_user_data_object luma_lut[TEST_NUM_NODE_INSTANCE] = {NULL}, chroma_lut[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_matrix matrix[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_reference ref[TEST_NUM_NODE_INSTANCE][2];
    vx_float32 m[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;
    uint32_t i;

    CT_Image input = NULL, output[TEST_NUM_NODE_INSTANCE] = {NULL}, dual_out = NULL;

    vx_border_t border = arg_->border;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->out_width;
        rect.end_y = arg_->out_height;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);
        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_NO_FAILURE(output[i] = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
                ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, arg_->out_width, arg_->out_height, arg_->matrix_type));
                ASSERT_VX_OBJECT(matrix[i] = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
                ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    ASSERT_VX_OBJECT(input_image[i] = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
                    ASSERT_VX_OBJECT(output_image[i] = ct_image_to_vx_image(output[i], context), VX_TYPE_IMAGE);
                }

                if(arg_->output_mode == 1)
                {
                    ASSERT_NO_FAILURE(dual_out = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
                    ASSERT_VX_OBJECT(dual_out_image[i] = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
                    ASSERT_VX_OBJECT(luma_lut[i] = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if (arg_->input_mode == 1)
                    {
                        ASSERT_VX_OBJECT(chroma_lut[i] = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                    }
                }

                tivx_vpac_ldc_params_init(&params);
                ASSERT_VX_OBJECT(param_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                                    sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                params.luma_interpolation_type = arg_->interp_type;

                VX_CALL(vxCopyUserDataObject(param_obj[i], 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                tivx_vpac_ldc_region_params_init(&region);
                ASSERT_VX_OBJECT(region_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                                     sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                region.out_block_width = 16;
                region.out_block_height = 16;
                region.pixel_pad = 0;

                ASSERT_VX_OBJECT(mesh_params_obj[i] = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                                    sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                tivx_vpac_ldc_mesh_params_init(&mesh_params);

                if(arg_->mesh_mode > 0)
                {
                    mesh_image[i] = mesh_create(context, arg_->out_width, arg_->out_height, arg_->mesh_mode-1);
                    mesh_params.mesh_frame_width = arg_->out_width;
                    mesh_params.mesh_frame_height = arg_->out_height;
                    mesh_params.subsample_factor = arg_->mesh_mode-1;
                }

                VX_CALL(vxCopyUserDataObject(region_obj[i], 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(vxCopyUserDataObject(mesh_params_obj[i], 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

                ASSERT_VX_OBJECT(node[i] = tivxVpacLdcNode(graph,
                                param_obj[i],
                                matrix[i],
                                region_obj[i],
                                mesh_params_obj[i],
                                mesh_image[i],
                                NULL,
                                input_image[i],
                                output_image[i],
                                dual_out_image[i]),
                                VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                VX_CALL(vxSetNodeAttribute(node[i], VX_NODE_BORDER, &border, sizeof(border)));
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) )
            {
                if(arg_->output_mode == 1)
                {
                    ref[i][0] = (vx_reference) luma_lut[i];
                    ref[i][1] = (vx_reference) chroma_lut[i];
                    VX_CALL(tivxNodeSendCommand(node[i], 0,
                        TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS, ref[i], 2));
                }
            }
        }

        VX_CALL(vxProcessGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    ASSERT_NO_FAILURE(output[i] = ct_image_from_vx_image(output_image[i]));
                    checksum_expected = get_checksum(arg_->out_width, arg_->interp_type,
                        arg_->matrix_type, arg_->output_mode, arg_->mesh_mode);
                    checksum_actual = tivx_utils_simple_image_checksum(output_image[i], 0, rect);
                    ASSERT(checksum_expected == checksum_actual);
                }
            }
        }

        /* Note: Only running if larger than a certain threshold so that
         * framework overhead doesn't become a factor */
        if (arg_->out_width > 256)
        {
            if ((NULL != arg_->target_string) &&
                (NULL != arg_->target_string_2) )
            {
                vx_perf_t perf_node[TEST_NUM_NODE_INSTANCE], perf_graph;

                for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
                {
                    vxQueryNode(node[i], VX_NODE_PERFORMANCE, &perf_node[i], sizeof(perf_node[i]));
                }
                vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

                if (strncmp(arg_->target_string, arg_->target_string_2, TIVX_TARGET_MAX_NAME) == 0)
                {
                    #if defined(TEST_VISS_PERFORMANCE_LOGGING)
                    printf("targets are same\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg >= (perf_node[0].avg + perf_node[1].avg));
                }
                else
                {
                    #if defined(TEST_VISS_PERFORMANCE_LOGGING)
                    printf("targets are different\n");
                    printf("Graph performance = %4.6f ms\n", perf_graph.avg/1000000.0);
                    printf("First node performance = %4.6f ms\n", perf_node[0].avg/1000000.0);
                    printf("Second node performance = %4.6f ms\n", perf_node[1].avg/1000000.0);
                    #endif
                    ASSERT(perf_graph.avg < (perf_node[0].avg + perf_node[1].avg));
                }
            }
        }

        //save_image_from_ldc(output_image, "output/ldc_y8");

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseMatrix(&matrix[i]));
                VX_CALL(vxReleaseUserDataObject(&param_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&region_obj[i]));
                VX_CALL(vxReleaseUserDataObject(&mesh_params_obj[i]));
                if(arg_->input_mode == 0 || arg_->input_mode == 1)
                {
                    VX_CALL(vxReleaseImage(&output_image[i]));
                    VX_CALL(vxReleaseImage(&input_image[i]));
                }
            }
        }
        VX_CALL(vxReleaseGraph(&graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if(arg_->output_mode == 1)
                {
                    VX_CALL(vxReleaseImage(&dual_out_image[i]));
                    VX_CALL(vxReleaseUserDataObject(&luma_lut[i]));

                    if(arg_->input_mode == 1)
                    {
                        VX_CALL(vxReleaseUserDataObject(&chroma_lut[i]));
                    }
                }
                if(arg_->mesh_mode > 0)
                {
                    VX_CALL(vxReleaseImage(&mesh_image[i]));
                }
            }
        }

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(matrix[i] == 0);
                ASSERT(output_image[i] == 0);
                ASSERT(input_image[i] == 0);
                ASSERT(param_obj[i] == 0);
                ASSERT(region_obj[i] == 0);
                ASSERT(mesh_params_obj[i] == 0);
                ASSERT(dual_out_image[i] == 0);
                ASSERT(luma_lut[i] == 0);
                ASSERT(chroma_lut[i] == 0);
                ASSERT(mesh_image[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                if((arg_->input_mode == 0 || arg_->input_mode == 1) && (arg_->mesh_mode == 0) &&
                   (arg_->matrix_type != VX_MATRIX_SCALE))
                {
                    ASSERT_NO_FAILURE(warp_affine_check(input, output[i], arg_->interp_type, arg_->border, m));
                }
            }
        }
    }
}

static void warp_perspective_generate_matrix(vx_float32 *m, int src_width, int src_height, int dst_width, int dst_height, int type)
{
    vx_float32 mat[3][3];
    vx_float32 angle, scale_x, scale_y, cos_a, sin_a;
    if (VX_MATRIX_IDENT == type)
    {
        mat[0][0] = 1.f;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_ROTATE_90 == type)
    {
        mat[0][0] = 0.f;
        mat[0][1] = 1.f;
        mat[0][2] = 0.f;

        mat[1][0] = -1.f;
        mat[1][1] = 0.f;
        mat[1][2] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_MIRROR == type)
    {
        mat[0][0] = -1.f;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = 1.f;
        mat[1][2] = 0.f;

        mat[2][0] = (vx_float32)src_width;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE == type)
    {
        scale_x = ldc_clip(src_width / (vx_float32)dst_width, 7.9999f);
        scale_y = ldc_clip(src_height / (vx_float32)dst_height, 7.9999f);

        mat[0][0] = scale_x;
        mat[0][1] = 0.f;
        mat[0][2] = 0.f;

        mat[1][0] = 0.f;
        mat[1][1] = scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else if (VX_MATRIX_SCALE_ROTATE == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * scale_x;
        mat[0][1] = sin_a * scale_y;
        mat[0][2] = 0.f;

        mat[1][0] = -sin_a * scale_x;
        mat[1][1] = cos_a  * scale_y;
        mat[1][2] = 0.f;

        mat[2][0] = 0.f;
        mat[2][1] = 0.f;
        mat[2][2] = 1.f;
    }
    else// if (VX_MATRIX_RANDOM == type)
    {
        angle = M_PIF / RND_FLT(3.f, 6.f);
        scale_x = src_width / (vx_float32)dst_width;
        scale_y = src_height / (vx_float32)dst_height;
        cos_a = cosf(angle);
        sin_a = sinf(angle);

        mat[0][0] = cos_a * RND_FLT(scale_x / 2.f, scale_x);
        mat[0][1] = sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[0][2] = RND_FLT(0.f, 0.1f);

        mat[1][0] = -sin_a * RND_FLT(scale_y / 2.f, scale_y);
        mat[1][1] = cos_a  * RND_FLT(scale_x / 2.f, scale_x);
        mat[1][2] = RND_FLT(0.f, 0.1f);

        mat[2][0] = src_width  / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][1] = src_height / 5.f * RND_FLT(-1.f, 1.f);
        mat[2][2] = 1.f;
    }
    memcpy(m, mat, sizeof(mat));
}

static vx_matrix warp_perspective_create_matrix(vx_context context, vx_float32 *m, int32_t natc)
{
    vx_matrix matrix;
    if(natc)
    {
        matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            if (VX_SUCCESS != vxCopyMatrix(matrix, m, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
    else
    {
        matrix = vxCreateMatrix(context, VX_TYPE_INT16, 3, 3);
        if (vxGetStatus((vx_reference)matrix) == VX_SUCCESS)
        {
            vx_int16 mfixed[9];

            mfixed[0] = (vx_int16)(ldc_clip((m[0]/m[8])*(4096.0f),32767));
            mfixed[1] = (vx_int16)(ldc_clip((m[1]/m[8])*(4096.0f),32767));
            mfixed[2] = (vx_int16)(ldc_clip((m[2]/m[8])*(8388608.0f),32767));
            mfixed[3] = (vx_int16)(ldc_clip((m[3]/m[8])*(4096.0f),32767));
            mfixed[4] = (vx_int16)(ldc_clip((m[4]/m[8])*(4096.0f),32767));
            mfixed[5] = (vx_int16)(ldc_clip((m[5]/m[8])*(8388608.0f),32767));
            mfixed[6] = (vx_int16)(ldc_clip((m[6]/m[8])*(8.0f),32767));
            mfixed[7] = (vx_int16)(ldc_clip((m[7]/m[8])*(8.0f),32767));

            if (VX_SUCCESS != vxCopyMatrix(matrix, mfixed, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST))
            {
                VX_CALL_(return 0, vxReleaseMatrix(&matrix));
            }
        }
        return matrix;
    }
}

static int warp_perspective_check_pixel(CT_Image input, CT_Image output, int x, int y, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_float64 x0, y0, z0, xlower, ylower, s, t;
    vx_int32 xi, yi;
    int candidate;
    vx_uint8 res = *CT_IMAGE_DATA_PTR_8U(output, x, y);

    x0 = (vx_float64)m[3 * 0 + 0] * (vx_float64)x + (vx_float64)m[3 * 1 + 0] * (vx_float64)y + (vx_float64)m[3 * 2 + 0];
    y0 = (vx_float64)m[3 * 0 + 1] * (vx_float64)x + (vx_float64)m[3 * 1 + 1] * (vx_float64)y + (vx_float64)m[3 * 2 + 1];
    z0 = (vx_float64)m[3 * 0 + 2] * (vx_float64)x + (vx_float64)m[3 * 1 + 2] * (vx_float64)y + (vx_float64)m[3 * 2 + 2];
    if (fabs(z0) < DBL_MIN)
        return 0;

    x0 = x0 / z0;
    y0 = y0 / z0;
    if (VX_INTERPOLATION_NEAREST_NEIGHBOR == interp_type)
    {
        for (yi = (vx_int32)ceil(y0 - VX_NN_AREA_SIZE); (vx_float64)yi <= y0 + VX_NN_AREA_SIZE; yi++)
        {
            for (xi = (vx_int32)ceil(x0 - VX_NN_AREA_SIZE); (vx_float64)xi <= x0 + VX_NN_AREA_SIZE; xi++)
            {
                if (0 <= xi && 0 <= yi && xi < (vx_int32)input->width-1 && yi < (vx_int32)input->height-1)
                {
                    candidate = *CT_IMAGE_DATA_PTR_8U(input, xi, yi);
                }
                else if (VX_BORDER_CONSTANT == border.mode)
                {
                    candidate = border.constant_value.U8;
                }
                else
                {
                    candidate = -1;
                }
                if (candidate == -1 || candidate == res)
                    return 0;
            }
        }
        CT_FAIL_(return 1, "Check failed for pixel (%d, %d): %d", x, y, (int)res);
    }
    else if (VX_INTERPOLATION_BILINEAR == interp_type)
    {
        xlower = floor(x0);
        ylower = floor(y0);

        s = x0 - xlower;
        t = y0 - ylower;

        xi = (vx_int32)xlower;
        yi = (vx_int32)ylower;

        candidate = -1;
        if (VX_BORDER_UNDEFINED == border.mode)
        {
            if (xi >= 1 && yi >= 1 && xi < (vx_int32)input->width - 2 && yi < (vx_int32)input->height - 2)
            {
                candidate = (int)((1. - s) * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi    ) +
                                        s  * (1. - t) * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi    ) +
                                  (1. - s) *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi    , yi + 1) +
                                        s  *       t  * (vx_float64) *CT_IMAGE_DATA_PTR_8U(input, xi + 1, yi + 1));
            }
        }
        else if (VX_BORDER_CONSTANT == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi    , border.constant_value.U8) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi    , border.constant_value.U8) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi    , yi + 1, border.constant_value.U8) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_CONSTANT_8U(input, xi + 1, yi + 1, border.constant_value.U8));
        }
        else if (VX_BORDER_REPLICATE == border.mode)
        {
            candidate = (int)((1. - s) * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi    ) +
                                    s  * (1. - t) * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi    ) +
                              (1. - s) *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi    , yi + 1) +
                                    s  *       t  * (vx_float32)CT_IMAGE_DATA_REPLICATE_8U(input, xi + 1, yi + 1));
        }
        if (candidate == -1 || (abs(candidate - res) <= VX_BILINEAR_TOLERANCE))
            return 0;
        return 1;
    }
    CT_FAIL_(return 1, "Interpolation type undefined");
}

static void warp_perspective_validate(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32 *m)
{
    vx_uint32 err_count = 0;

    CT_FILL_IMAGE_8U(, output,
            {
                ASSERT_NO_FAILURE(err_count += warp_perspective_check_pixel(input, output, x, y, interp_type, border, m));
            });
    if (10 * err_count > output->width * output->height)
        CT_FAIL_(return, "Check failed for %d pixels", err_count);
}

static void warp_perspective_check(CT_Image input, CT_Image output, vx_enum interp_type, vx_border_t border, vx_float32* m)
{
    ASSERT(input && output);
    ASSERT( (interp_type == 0) ||
            (interp_type == 1) ||
            (interp_type == VX_MATRIX_MIRROR));

    ASSERT( (border.mode == VX_BORDER_UNDEFINED) ||
            (border.mode == VX_BORDER_CONSTANT) ||
            (border.mode == VX_BORDER_REPLICATE));

    warp_perspective_validate(input, output, VX_INTERPOLATION_BILINEAR, border, m);
    if (CT_HasFailure())
    {
        printf("=== INPUT ===\n");
        ct_dump_image_info(input);
        printf("=== OUTPUT ===\n");
        ct_dump_image_info(output);
        printf("Matrix:\n%g %g %g\n%g %g %g\n%g %g %g\n",
                m[0], m[3], m[6],
                m[1], m[4], m[7],
                m[2], m[5], m[8]);
    }
}


TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingPerspective, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_image dual_out_image = 0;
    vx_image mesh_image = 0;
    vx_user_data_object luma_lut = NULL, chroma_lut = NULL;
    vx_matrix matrix = 0;
    vx_reference ref[2];
    vx_float32 m[9];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_rectangle_t rect;

    CT_Image input = NULL, output = NULL, dual_out = NULL;

    vx_border_t border = arg_->border;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = arg_->out_width;
        rect.end_y = arg_->out_height;
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->src_width, arg_->src_height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));

        ASSERT_NO_FAILURE(warp_perspective_generate_matrix(m, input->width, input->height, arg_->out_width, arg_->out_height, arg_->matrix_type));
        ASSERT_VX_OBJECT(matrix = warp_perspective_create_matrix(context, m, 1), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);
        }

        if(arg_->output_mode == 1)
        {
            ASSERT_NO_FAILURE(dual_out = ct_allocate_image(arg_->out_width, arg_->out_height, VX_DF_IMAGE_U8));
            ASSERT_VX_OBJECT(dual_out_image = ct_image_to_vx_image(dual_out, context), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(luma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

            if (arg_->input_mode == 1)
            {
                ASSERT_VX_OBJECT(chroma_lut = lut_create(context, lut_data, 513), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            }
        }

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = arg_->interp_type;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        if(arg_->mesh_mode > 0)
        {
            mesh_image = mesh_create(context, arg_->out_width, arg_->out_height, arg_->mesh_mode-1);
            mesh_params.mesh_frame_width = arg_->out_width;
            mesh_params.mesh_frame_height = arg_->out_height;
            mesh_params.subsample_factor = arg_->mesh_mode-1;
        }

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));

        if(arg_->output_mode == 1)
        {
            ref[0] = (vx_reference) luma_lut;
            ref[1] = (vx_reference) chroma_lut;
            VX_CALL(tivxNodeSendCommand(node, 0,
                TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS, ref, 2));
        }

        VX_CALL(vxProcessGraph(graph));

        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            ASSERT_NO_FAILURE(output = ct_image_from_vx_image(output_image));
            checksum_expected = get_checksum(arg_->out_width, arg_->interp_type,
                arg_->matrix_type, arg_->output_mode, arg_->mesh_mode);
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect);
            ASSERT(checksum_expected == checksum_actual);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        if(arg_->input_mode == 0 || arg_->input_mode == 1)
        {
            VX_CALL(vxReleaseImage(&output_image));
            VX_CALL(vxReleaseImage(&input_image));
        }

        if(arg_->output_mode == 1)
        {
            VX_CALL(vxReleaseImage(&dual_out_image));
            VX_CALL(vxReleaseUserDataObject(&luma_lut));

            if(arg_->input_mode == 1)
            {
                VX_CALL(vxReleaseUserDataObject(&chroma_lut));
            }
        }

        if(arg_->mesh_mode > 0)
        {
            VX_CALL(vxReleaseImage(&mesh_image));
        }

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(luma_lut == 0);
        ASSERT(chroma_lut == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);

        if((arg_->input_mode == 0 || arg_->input_mode == 1) && (arg_->mesh_mode == 0) &&
           (arg_->matrix_type != VX_MATRIX_SCALE))
        {
            ASSERT_NO_FAILURE(warp_perspective_check(input, output, arg_->interp_type, arg_->border, m));
        }
    }
}



typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    const char* fileName;
    int width, height;
    char *target_string, *target_string_2;
} ArgCmd;

#define PARAMETERS_CMD \
    CT_GENERATE_PARAMETERS("lena", ADD_SIZE_NONE, ADD_SET_TARGET_PARAMETERS, ARG, warp_affine_read_image_8u, "lena.bmp")

TEST_WITH_ARG(tivxHwaVpacLdc, testGraphProcessingCommand, ArgCmd, PARAMETERS_CMD)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    tivx_vpac_ldc_region_params_t region;
    vx_user_data_object region_obj;
    tivx_vpac_ldc_mesh_params_t   mesh_params;
    vx_user_data_object mesh_params_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_image dual_out_image = 0;
    vx_image mesh_image = 0;
    vx_matrix matrix = 0, matrix2 = 0;
    vx_reference ref[1];
    vx_float32 m[6], m2[6];
    vx_uint32 error_status;
    vx_uint16 lut_data[513];
    uint32_t checksum_expected;
    uint32_t checksum_actual;
    vx_border_t border = {VX_BORDER_UNDEFINED, {{ 0 }}};

    CT_Image input = NULL, output = NULL, output2 = NULL, dual_out = NULL;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);

        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_NO_FAILURE(input = arg_->generator(arg_->fileName, arg_->width, arg_->height));
        ASSERT_NO_FAILURE(output = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(output2 = ct_allocate_image(input->width, input->height, VX_DF_IMAGE_U8));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m, input->width, input->height, 0, 0, VX_MATRIX_IDENT));
        ASSERT_NO_FAILURE(warp_affine_generate_matrix(m2, input->width, input->height, 0, 0, VX_MATRIX_MIRROR));
        ASSERT_VX_OBJECT(matrix = warp_affine_create_matrix(context, m, 0), VX_TYPE_MATRIX);
        ASSERT_VX_OBJECT(matrix2 = warp_affine_create_matrix(context, m2, 0), VX_TYPE_MATRIX);
        ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, 513));

        ASSERT_VX_OBJECT(input_image = ct_image_to_vx_image(input, context), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = ct_image_to_vx_image(output, context), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.luma_interpolation_type = TIVX_VPAC_LDC_INTERPOLATION_BILINEAR;

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        tivx_vpac_ldc_region_params_init(&region);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t",
                                                             sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        region.out_block_width = 16;
        region.out_block_height = 16;
        region.pixel_pad = 0;

        ASSERT_VX_OBJECT(mesh_params_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t",
                                                            sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);

        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_params_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        matrix,
                        region_obj,
                        mesh_params_obj,
                        mesh_image,
                        NULL,
                        input_image,
                        output_image,
                        dual_out_image),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        //VX_CALL(vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(output = ct_image_from_vx_image(output_image));
        ASSERT_NO_FAILURE(warp_affine_check(input, output, VX_MATRIX_IDENT, border, m));
        //ct_write_image("output/ident.bmp", output);

        ref[0] = (vx_reference)matrix2;
        VX_CALL(tivxNodeSendCommand(node, 0, TIVX_VPAC_LDC_CMD_SET_LDC_PARAMS, ref, 1));
        VX_CALL(vxProcessGraph(graph));

        ASSERT_NO_FAILURE(output2 = ct_image_from_vx_image(output_image));
        ASSERT_NO_FAILURE(warp_affine_check(input, output2, VX_MATRIX_MIRROR, border, m2));
        //ct_write_image("output/mirror.bmp", output2);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseMatrix(&matrix2));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_params_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(matrix2 == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_params_obj == 0);
        ASSERT(dual_out_image == 0);
        ASSERT(mesh_image == 0);

        tivxHwaUnLoadKernels(context);

    }
}


static uint8_t isFormatComboValid(int in, int out, int out_num)
{
    uint8_t valid = 1;

    if ( (in == VX_DF_IMAGE_U8) &&
         !((out == VX_DF_IMAGE_U8) || (out == TIVX_DF_IMAGE_P12)))
    {
        valid = 0;
    }

    if ( (in == VX_DF_IMAGE_U16) &&
         !((out == VX_DF_IMAGE_U16) || ((out_num == 1) && (out == VX_DF_IMAGE_U8))))
    {
        valid = 0;
    }

    if ( (in == TIVX_DF_IMAGE_P12) &&
         !((out == TIVX_DF_IMAGE_P12) || (out == VX_DF_IMAGE_U8)))
    {
        valid = 0;
    }

    if ( (in == VX_DF_IMAGE_NV12) &&
         !((out == VX_DF_IMAGE_NV12) || (out == TIVX_DF_IMAGE_NV12_P12)))
    {
        valid = 0;
    }

    if ( (in == TIVX_DF_IMAGE_NV12_P12) &&
         !((out == TIVX_DF_IMAGE_NV12_P12) || (out == VX_DF_IMAGE_NV12)))
    {
        valid = 0;
    }

    if ( (in == VX_DF_IMAGE_UYVY) &&
         !((out == VX_DF_IMAGE_UYVY) || (out == VX_DF_IMAGE_YUYV) ||
           (out == VX_DF_IMAGE_NV12) || (out == TIVX_DF_IMAGE_NV12_P12) ))
    {
        valid = 0;
    }

    return valid;
}

static uint32_t ldc_formats_checksums_ref[] = {
    0x2d2cdc00, 0xffffa600, 0xbfa9b800, 0x2d2cf000,
    0x49e48800, 0xe529b800, 0xffff8800
};

static uint32_t get_formats_checksum(int in, int out)
{
    uint32_t index = 0;

    if ( ((in == VX_DF_IMAGE_U8) && (out == VX_DF_IMAGE_U8)) ||
         ((in == VX_DF_IMAGE_NV12) && (out == VX_DF_IMAGE_NV12)) ||
         ((in == VX_DF_IMAGE_UYVY) && (out == VX_DF_IMAGE_NV12)))
    {
        index = 0;
    }
    else if ( ((in == VX_DF_IMAGE_U8) && (out == TIVX_DF_IMAGE_P12)) ||
              ((in == VX_DF_IMAGE_NV12) && (out == TIVX_DF_IMAGE_NV12_P12)) ||
              ((in == VX_DF_IMAGE_UYVY) && (out == TIVX_DF_IMAGE_NV12_P12)))
    {
        index = 1U;
    }
    else if ( (in == VX_DF_IMAGE_U16) && (out == VX_DF_IMAGE_U16))
    {
        index = 2U;
    }
    else if ( ((in == TIVX_DF_IMAGE_P12) && (out == VX_DF_IMAGE_U8)) ||
              ((in == TIVX_DF_IMAGE_NV12_P12) && (out == VX_DF_IMAGE_NV12)))
    {
        index = 3U;
    }
    else if ( (in == VX_DF_IMAGE_UYVY) && (out == VX_DF_IMAGE_UYVY))
    {
        index = 4U;
    }
    else if ( (in == VX_DF_IMAGE_UYVY) && (out == VX_DF_IMAGE_YUYV))
    {
        index = 5U;
    }
    else if ( ((in == TIVX_DF_IMAGE_P12) && (out == TIVX_DF_IMAGE_P12)) ||
              ((in == TIVX_DF_IMAGE_NV12_P12) && (out == TIVX_DF_IMAGE_NV12_P12)))
    {
        index = 6U;
    }
    return ldc_formats_checksums_ref[index];
}


TEST_WITH_ARG(tivxHwaVpacLdc, testFormats, ArgFormats,
    PARAMETERS_FORMATS
)
{
    vx_context context = context_->vx_context_;
    tivx_vpac_ldc_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_image input_image = 0, output_image = 0;
    vx_pixel_value_t pixel;
    uint32_t checksum_actual;
    uint32_t checksum_expected;
    vx_rectangle_t rect;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        pixel.U32 = 0x0a7f1345;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 640;
        rect.end_y = 480;

        ASSERT_VX_OBJECT(input_image = vxCreateUniformImage(context, 640, 480, arg_->input_data_format, &pixel), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, 640, 480, arg_->output_data_format), VX_TYPE_IMAGE);

        tivx_vpac_ldc_params_init(&params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t",
                                                            sizeof(tivx_vpac_ldc_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                        param_obj,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        input_image,
                        output_image,
                        NULL),
                        VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if (0 == isFormatComboValid(arg_->input_data_format, arg_->output_data_format, 0))
        {
            EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }
        else
        {
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
            checksum_expected = get_formats_checksum(arg_->input_data_format, arg_->output_data_format);
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect);
            ASSERT(checksum_expected == checksum_actual);
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseImage(&input_image));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(input_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);

    }
}

TEST_WITH_ARG(tivxHwaVpacLdc, testNegativeGraph, ArgNegative, PARAMETERS_NEGATIVE)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_matrix matrix = 0;
    tivx_vpac_ldc_params_t params;
    tivx_vpac_ldc_region_params_t region_params;
    tivx_vpac_ldc_mesh_params_t mesh_params;
    vx_user_data_object param_obj;
    vx_user_data_object region_obj;
    vx_user_data_object mesh_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_enum matrix_type = VX_TYPE_INT16;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    const vx_size matrix_data_size = 2 * matrix_rows * matrix_cols;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);

        {
            vx_enum ch_matrix_type;
            vx_size ch_matrix_rows, ch_matrix_cols, ch_data_size;

            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_TYPE, &ch_matrix_type, sizeof(ch_matrix_type)));
            if (matrix_type != ch_matrix_type)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_TYPE failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_ROWS, &ch_matrix_rows, sizeof(ch_matrix_rows)));
            if (matrix_rows != ch_matrix_rows)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_ROWS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_COLUMNS, &ch_matrix_cols, sizeof(ch_matrix_cols)));
            if (matrix_cols != ch_matrix_cols)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_COLUMNS failed\n");
            }
            VX_CALL(vxQueryMatrix(matrix, VX_MATRIX_SIZE, &ch_data_size, sizeof(ch_data_size)));
            if (matrix_data_size > ch_data_size)
            {
                CT_FAIL("check for Matrix attribute VX_MATRIX_SIZE failed\n");
            }
        }

        tivx_vpac_ldc_params_init(&params);
        tivx_vpac_ldc_mesh_params_init(&mesh_params);
        tivx_vpac_ldc_region_params_init(&region_params);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(mesh_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t", sizeof(tivx_vpac_ldc_mesh_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.input_align_12bit = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.input_align_12bit = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.input_align_12bit = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.input_align_12bit = 2;
                }
                else
                {
                    params.input_align_12bit = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.luma_interpolation_type = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.luma_interpolation_type = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.luma_interpolation_type = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.luma_interpolation_type = 2;
                }
                else
                {
                    params.luma_interpolation_type = 2;
                }
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.init_x = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.init_x = 8184;
                }
                else if (2U == arg_->condition)
                {
                    params.init_x = 8192;
                }
                else if (3U == arg_->condition)
                {
                    params.init_x = 8192;
                }
                else
                {
                    params.init_x = 4004;
                }
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.init_y = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.init_y = 8184;
                }
                else if (2U == arg_->condition)
                {
                    params.init_y = 8192;
                }
                else if (3U == arg_->condition)
                {
                    params.init_y = 8192;
                }
                else
                {
                    params.init_y = 4001;
                }
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.yc_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.yc_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.yc_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.yc_mode = 2;
                }
                else
                {
                    params.yc_mode = 2;
                }
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    region_params.enable = 0;
                }
                else if (1U == arg_->condition)
                {
                    region_params.enable = 1;
                }
                else if (2U == arg_->condition)
                {
                    region_params.enable = 2;
                }
                else if (3U == arg_->condition)
                {
                    region_params.enable = 2;
                }
                else
                {
                    region_params.enable = 2;
                }
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    region_params.out_block_width = 8;
                }
                else if (1U == arg_->condition)
                {
                    region_params.out_block_width = 248;
                }
                else if (2U == arg_->condition)
                {
                    region_params.out_block_width = 0;
                }
                else if (3U == arg_->condition)
                {
                    region_params.out_block_width = 256;
                }
                else
                {
                    region_params.out_block_width = 124;
                }
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    region_params.out_block_height = 2;
                }
                else if (1U == arg_->condition)
                {
                    region_params.out_block_height = 254;
                }
                else if (2U == arg_->condition)
                {
                    region_params.out_block_height = 0;
                }
                else if (3U == arg_->condition)
                {
                    region_params.out_block_height = 256;
                }
                else
                {
                    region_params.out_block_height = 127;
                }
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    region_params.pixel_pad = 0;
                }
                else if (1U == arg_->condition)
                {
                    region_params.pixel_pad = 15;
                }
                else if (2U == arg_->condition)
                {
                    region_params.pixel_pad = 16;
                }
                else if (3U == arg_->condition)
                {
                    region_params.pixel_pad = 16;
                }
                else
                {
                    region_params.pixel_pad = 16;
                }
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    mesh_params.subsample_factor = 0;
                }
                else if (1U == arg_->condition)
                {
                    mesh_params.subsample_factor = 7;
                }
                else if (2U == arg_->condition)
                {
                    mesh_params.subsample_factor = 8;
                }
                else if (3U == arg_->condition)
                {
                    mesh_params.subsample_factor = 8;
                }
                else
                {
                    mesh_params.subsample_factor = 8;
                }
            }
        }

        VX_CALL(vxCopyUserDataObject(param_obj, 0, sizeof(tivx_vpac_ldc_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(region_obj, 0, sizeof(tivx_vpac_ldc_region_params_t), &region_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(mesh_obj, 0, sizeof(tivx_vpac_ldc_mesh_params_t), &mesh_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                matrix,
                                region_obj,
                                mesh_obj,
                                NULL,
                                NULL,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if(2 > arg_->condition)
        {
            ASSERT_NO_FAILURE(vxVerifyGraph(graph));
        }
        else
        {
            ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));
        VX_CALL(vxReleaseUserDataObject(&mesh_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);
        ASSERT(region_obj == 0);
        ASSERT(mesh_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacLdc, testNegativeMatrixType, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_image input = 0, output = 0;
    vx_matrix matrix = 0;
    vx_user_data_object param_obj;
    vx_user_data_object region_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    const vx_enum matrix_type = VX_TYPE_UINT8;
    const vx_size matrix_rows = 3;
    const vx_size matrix_cols = 2;
    const vx_size matrix_data_size = 2 * matrix_rows * matrix_cols;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(input = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(matrix = vxCreateMatrix(context, matrix_type, matrix_cols, matrix_rows), VX_TYPE_MATRIX);
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(region_obj = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacLdcNode(graph,
                                param_obj,
                                matrix,
                                region_obj,
                                NULL,
                                NULL,
                                NULL,
                                input,
                                output,
                                NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        ASSERT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseMatrix(&matrix));
        VX_CALL(vxReleaseImage(&output));
        VX_CALL(vxReleaseImage(&input));
        VX_CALL(vxReleaseUserDataObject(&param_obj));
        VX_CALL(vxReleaseUserDataObject(&region_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(matrix == 0);
        ASSERT(output == 0);
        ASSERT(input == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TESTCASE_TESTS(tivxHwaVpacLdc, testNodeCreation, testGraphProcessingDcc, testGraphProcessing, testGraphProcessingPerspective, testFormats, testNegativeGraph, testNegativeMatrixType, testGraphProcessingCommand)

#endif /* BUILD_VPAC_LDC */