/*
 *
 * Copyright (c) 2018-2020 Texas Instruments Incorporated
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
#ifdef BUILD_VPAC_VISS


#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <utils/iss/include/app_iss.h>
#include "test_hwa_common.h"
#include "tivx_utils_checksum.h"

#ifdef VPAC3L
#define VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE 72
#define VISS_CHECKSUMS_H3A_REF_SIZE 6
#else
#define VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE 60
#define VISS_CHECKSUMS_H3A_REF_SIZE 5
#endif

#define TEST_NUM_NODE_INSTANCE 2

/* #define TEST_VISS_PERFORMANCE_LOGGING */

/* #define TEST_VISS_CHECKSUM_LOGGING */

#define APP_MAX_FILE_PATH           (512u)

#define ADD_SIZE_64x48(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=64x48", __VA_ARGS__, 64, 48))

static void ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack, uint16_t downshift_bits);
static void ct_write_user_data_object(vx_user_data_object user_data_object, const char* fileName);
static vx_status save_image_from_viss(vx_image y8, char *filename_prefix);

TESTCASE(tivxHwaVpacViss, CT_VXContext, ct_setup_vx_context, 0)

/* Write NV12 output image */
static vx_int32 write_output_image_fp(FILE * fp, vx_image out_image)
{
    vx_uint32 width, height;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;
    vx_map_id map_id1, map_id2;
    void *data_ptr1, *data_ptr2;
    uint8_t *temp_ptr = NULL;
    vx_uint32 num_bytes_per_pixel = 1;
    vx_uint32 num_luma_bytes_written_to_file = 0, num_chroma_bytes_written_to_file = 0, num_bytes_written_to_file = 0;
    vx_int32 i;

    vxQueryImage(out_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out_image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    printf("out width =  %d\n", width);
    printf("out height =  %d\n", height);
    printf("out format =  %d\n", df);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(out_image,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr1;
    for(i=0; i<height; i++)
    {
        num_luma_bytes_written_to_file += fwrite(temp_ptr, 1, width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    vxMapImagePatch(out_image,
        &rect,
        1,
        &map_id2,
        &image_addr,
        &data_ptr2,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr2)
    {
        printf("data_ptr2 is NULL \n");
        return -1;
    }

    temp_ptr = (uint8_t *)data_ptr2;
    for(i=0; i<height/2; i++)
    {
        num_chroma_bytes_written_to_file += fwrite(temp_ptr, 1, width*num_bytes_per_pixel, fp);
        temp_ptr += image_addr.stride_y;
    }

    num_bytes_written_to_file = num_luma_bytes_written_to_file + num_chroma_bytes_written_to_file;

    vxUnmapImagePatch(out_image, map_id1);
    vxUnmapImagePatch(out_image, map_id2);

    return num_bytes_written_to_file;
}

/* Open and write NV12 output image */
static vx_int32 write_output_image_nv12_8bit(char * file_name, vx_image out)
{
    FILE * fp;
    printf("Opening file %s \n", file_name);

    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out);
    printf("Written image \n");
    fclose(fp);
    printf("%d bytes written to %s\n", len1, file_name);
    return len1 ;
}

typedef struct
{
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;

} SetTarget_Arg;

#if defined(SOC_J784S4)
#if defined(x86_64)
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_VPAC2_VISS1))
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC2_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_VPAC2_VISS1))
#endif
#else
#define ADD_SET_TARGET_PARAMETERS_MULTI_INST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/NULL", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, NULL)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_VPAC_VISS1))
#endif

#if defined(SOC_J784S4)
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1)), \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC2_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC2_VISS1))
#else
#define ADD_SET_TARGET_PARAMETERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/TIVX_TARGET_VPAC_VISS1", __VA_ARGS__, TIVX_TARGET_VPAC_VISS1))
#endif

#define SET_NODE_TARGET_PARAMETERS \
    CT_GENERATE_PARAMETERS("target", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacViss, testNodeCreation, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static CT_Image raw_read_image(const char* fileName, int width, int height)
{
    CT_Image image = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image = ct_read_image(fileName, 1);
    ASSERT_(return 0, image);
    ASSERT_(return 0, image->format == VX_DF_IMAGE_U8);
    return image;
}


typedef struct {
    const char* testName;
    int width, height, exposures;
    vx_bool line_interleaved;
    char* target_string;
} Arg;

#define ADD_SIZE_2048x1024(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=2048x1024", __VA_ARGS__, 2048, 1024))

#define ADD_EXP1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/n_exp=1", __VA_ARGS__, 1))

#define ADD_EXP2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/n_exp=2", __VA_ARGS__, 2))

#define ADD_EXP3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/n_exp=3", __VA_ARGS__, 3))

#define ADD_LINE_FALSE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=false", __VA_ARGS__, vx_false_e))

#define ADD_LINE_TRUE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=true", __VA_ARGS__, vx_true_e))

#define ADD_SIZE_640x480(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=640x480", __VA_ARGS__, 640, 480))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP1, ADD_LINE_FALSE, ADD_SET_TARGET_PARAMETERS, ARG), \
    /*CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP2, ADD_LINE_FALSE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP3, ADD_LINE_FALSE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP1, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP2, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP3, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_2048x1024, ADD_EXP1, ARG)*/

/* Test case for TIOVX-1236 */
TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessing, Arg,
    PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_Y8;
        params.fcp[0].mux_output0 = 0;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_YUV422;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_422;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, NULL, NULL,
                                                raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                                h3a_aew_af, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}


TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFile, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 1280; // TODO: Add validate check for min/max size
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);*/
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        /*ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);*/

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
        params.fcp[0].mux_output1 = 0;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params.fcp[0].mux_output3 = 0;
        params.fcp[0].mux_output4 = 3;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_output_image_nv12_8bit(file, y8_r8_c2);
        //save_image_from_viss(y8_r8_c2, "output/out_y8");

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        //VX_CALL(vxReleaseDistribution(&histogram));
        /*VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));*/
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        /*VX_CALL(vxReleaseImage(&uv12_c1));*/
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static vx_int32 get_dcc_file_size(char * file_name)
{
    vx_uint32 num_bytes;
    FILE * fp = fopen(file_name, "rb");

    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    num_bytes = ftell(fp);
    fclose(fp);

    return num_bytes;
}

static vx_int32 read_dcc_file(char * file_name, uint8_t * buf, uint32_t num_bytes)
{
    vx_uint32 num_bytes_read_from_file;
    FILE * fp = fopen(file_name, "rb");

    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }
    num_bytes_read_from_file = fread(buf, sizeof(uint8_t), num_bytes, fp);
    fclose(fp);

    return num_bytes_read_from_file;
}

typedef struct {
    const char* testName;
    int dcc;
    int results_2a;
    int bypass_glbce;
    int bypass_nsf4;
    char *target_string, *target_string_2;
} ArgDcc;

static uint32_t viss_checksums_luma_ref[VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE] = {
    (uint32_t) 0xf47beefd, (uint32_t) 0x03b9eda6, (uint32_t) 0x98c0c9df, (uint32_t) 0xade9ca14,
    (uint32_t) 0xf47beefd, (uint32_t) 0x03b9eda6, (uint32_t) 0x98c0c9df, (uint32_t) 0xade9ca14,
    (uint32_t) 0x781f7630, (uint32_t) 0xe7d95550, (uint32_t) 0xeaed49e2, (uint32_t) 0x60b695a7,
    (uint32_t) 0x080d9c2d, (uint32_t) 0x4bed2b15, (uint32_t) 0xb019be0a, (uint32_t) 0xdca1c62f,
    (uint32_t) 0x080d9c2d, (uint32_t) 0x4bed2b15, (uint32_t) 0xb019be0a, (uint32_t) 0xdca1c62f,
    (uint32_t) 0x908a52ee, (uint32_t) 0x271d8c34, (uint32_t) 0xa9deaf7a, (uint32_t) 0xcfbdbebe,
    (uint32_t) 0xffab8e7d, (uint32_t) 0x51cbd1f6, (uint32_t) 0x65995e4a, (uint32_t) 0x41bad204,
    (uint32_t) 0x627e6edc, (uint32_t) 0xde350c54, (uint32_t) 0xa81d15ee, (uint32_t) 0x120dacb6,
    (uint32_t) 0xfefd6eb9, (uint32_t) 0xa0da7954, (uint32_t) 0x17eb771b, (uint32_t) 0xdbe70fff,
    (uint32_t) 0xa822fcba, (uint32_t) 0xdcb0c966, (uint32_t) 0x3af111cf, (uint32_t) 0xb720e2e6,
    (uint32_t) 0xa822fcba, (uint32_t) 0xdcb0c966, (uint32_t) 0x3af111cf, (uint32_t) 0xb720e2e6,
    (uint32_t) 0xe2263148, (uint32_t) 0x0d47a692, (uint32_t) 0x61c52600, (uint32_t) 0x9f39b173,
    (uint32_t) 0xe5ca9fb9, (uint32_t) 0x5d90b6fb, (uint32_t) 0xfe5f53fc, (uint32_t) 0x84acc97c,
    (uint32_t) 0x0924d246, (uint32_t) 0xbe1f4acb, (uint32_t) 0xc6750795, (uint32_t) 0xcd55e141,
    (uint32_t) 0x8c1cf5bf, (uint32_t) 0x03e50bc8, (uint32_t) 0x122b99ea, (uint32_t) 0x535c63b3
    #ifdef VPAC3L
    ,(uint32_t) 0x0c90fad6, (uint32_t) 0x833c9b64, (uint32_t) 0x0c90fad6, (uint32_t) 0x833c9b64
    ,(uint32_t) 0x24188991, (uint32_t) 0x3e003428, (uint32_t) 0x24188991, (uint32_t) 0x3e003428
    ,(uint32_t) 0x6605773f, (uint32_t) 0x7b42c5cc, (uint32_t) 0x6605773f, (uint32_t) 0x7b42c5cc
    #endif
};
static uint32_t viss_checksums_chroma_ref[VISS_CHECKSUMS_LUMA_CHROMA_REF_SIZE] = {
    (uint32_t) 0x22cdd15d, (uint32_t) 0x485cc747, (uint32_t) 0x190b425e, (uint32_t) 0x9a4dc6c0,
    (uint32_t) 0x22cdd15d, (uint32_t) 0x485cc747, (uint32_t) 0x190b425e, (uint32_t) 0x9a4dc6c0,
    (uint32_t) 0xff5c5d31, (uint32_t) 0xadb0bec7, (uint32_t) 0x661e6851, (uint32_t) 0xda4e097f,
    (uint32_t) 0x3083ca4b, (uint32_t) 0x5fec52b8, (uint32_t) 0x2ce1134c, (uint32_t) 0x05cbea74,
    (uint32_t) 0x3083ca4b, (uint32_t) 0x5fec52b8, (uint32_t) 0x2ce1134c, (uint32_t) 0x05cbea74,
    (uint32_t) 0x2d19a877, (uint32_t) 0x62c6d227, (uint32_t) 0x4207c710, (uint32_t) 0x4f88f026,
    (uint32_t) 0x456ea247, (uint32_t) 0x845f901a, (uint32_t) 0x3a9dafd1, (uint32_t) 0x4b91a897,
    (uint32_t) 0x54c9af0c, (uint32_t) 0x686fe091, (uint32_t) 0x3b149b94, (uint32_t) 0xdd7220a3,
    (uint32_t) 0x4f673a33, (uint32_t) 0xbd582de8, (uint32_t) 0x292394dd, (uint32_t) 0xb9eb728d,
    (uint32_t) 0x987a6c65, (uint32_t) 0x7bbf7d1d, (uint32_t) 0x99a80276, (uint32_t) 0x8f833a56,
    (uint32_t) 0x987a6c65, (uint32_t) 0x7bbf7d1d, (uint32_t) 0x99a80276, (uint32_t) 0x8f833a56,
    (uint32_t) 0x5dc5ce7a, (uint32_t) 0x3f14c7b0, (uint32_t) 0x0ff5279e, (uint32_t) 0x35356d95,
    (uint32_t) 0x227bafb5, (uint32_t) 0xe165e425, (uint32_t) 0x3073d950, (uint32_t) 0xfa2b5cae,
    (uint32_t) 0xb026e875, (uint32_t) 0xd7075a56, (uint32_t) 0x2f9e5202, (uint32_t) 0xef5d8951,
    (uint32_t) 0x1ed6ba03, (uint32_t) 0x8eb710fd, (uint32_t) 0x05da456a, (uint32_t) 0x1b0fc632
    #ifdef VPAC3L
    ,(uint32_t) 0x77188952, (uint32_t) 0x8a2a986c, (uint32_t) 0x77188952, (uint32_t) 0x8a2a986c
    ,(uint32_t) 0xe6e31d5d, (uint32_t) 0xf8fd4571, (uint32_t) 0xe6e31d5d, (uint32_t) 0xf8fd4571
    ,(uint32_t) 0x1826c8e5, (uint32_t) 0xf2d2692d, (uint32_t) 0x1826c8e5, (uint32_t) 0xf2d2692d
    #endif
};

static uint32_t viss_checksums_h3a_ref[VISS_CHECKSUMS_H3A_REF_SIZE] = {
    (uint32_t) 0x00000000, (uint32_t) 0xccf74f25, (uint32_t) 0x2d728f69, (uint32_t) 0x7b0ba698, (uint32_t) 0x824ffb91
    #ifdef VPAC3L
    ,(uint32_t) 0x3d1ff5ac
    #endif
};


static uint32_t get_checksum(uint32_t *table, vx_int32 dcc, vx_int32 results_2a, vx_int32 bypass_glbce, vx_int32 bypass_nsf4)
{
    return table[(2*2*3)*dcc + (2*2)*results_2a + (2)*bypass_glbce + bypass_nsf4];
}


#define ADD_GLBCE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/glbce=on", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/glbce=bypass", __VA_ARGS__, 1))

#define ADD_NSF4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/nsf4=on", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/nsf4=bypass", __VA_ARGS__, 1))

#define ADD_2A(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/2a=NULL", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/2a=invalid", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/2a=valid", __VA_ARGS__, 2))

#ifdef VPAC3L
#define ADD_DCC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dcc=off", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0233", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ov2312", __VA_ARGS__, 5))
#else
#define ADD_DCC(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/dcc=off", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=imx390", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0233", __VA_ARGS__, 2))
#endif

#if 0
    CT_EXPAND(nextmacro(testArgName "/dcc=ub9xx", __VA_ARGS__, 3))
    CT_EXPAND(nextmacro(testArgName "/dcc=ar0820", __VA_ARGS__, 4))
#endif

#define PARAMETERS_DCC \
    CT_GENERATE_PARAMETERS("cksm", ADD_DCC, ADD_2A, ADD_GLBCE, ADD_NSF4, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDcc, ArgDcc, PARAMETERS_DCC)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image y8_r8_c2[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    #ifdef VPAC3L
    if(5 == arg_->dcc)
    {
        raw_params.width = 1600;
        raw_params.height = 1300;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 2312;
        sensor_name = SENSOR_OV2312_UB953_LI;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
        downshift_bits = 0;
    }
    else
    #endif
    if(4 == arg_->dcc)
    {
        raw_params.width = 3840;
        raw_params.height = 2160;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 820;
        sensor_name = SENSOR_ONSEMI_AR0820_UB953_LI;
        sensor_dcc_mode = 1;
        file_name = "psdkra/app_single_cam/AR0820_001/AR0820_12bWDR_3840x2160_GRBG.raw";
        downshift_bits = 0;
    }
    else if(3 == arg_->dcc)
    {
        raw_params.width = 3840;
        raw_params.height = 2160;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 9702;
        sensor_name = UB9XX_RAW_TESTPAT;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/UB960/ub960_test_pattern_3840x2160.raw";
        downshift_bits = 0;
    }
    else if(2 == arg_->dcc)
    {
        raw_params.width = 2048;
        raw_params.height = 1280;
        raw_params.meta_height_after = 0;
        sensor_dcc_id = 233;
        sensor_name = SENSOR_ONSEMI_AR0233_UB953_MARS;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/AR0233_001/input1.raw";
        downshift_bits = 4;
    }
    else
    {
        raw_params.width = 1936;
        raw_params.height = 1096;
        raw_params.meta_height_after = 4;
        sensor_dcc_id = 390;
        sensor_name = SENSOR_SONY_IMX390_UB953_D3;
        sensor_dcc_mode = 0;
        file_name = "psdkra/app_single_cam/IMX390_001/input2.raw";
        downshift_bits = 0;
    }

    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    #ifdef VPAC3L
    if(5 == arg_->dcc)
        raw_params.format[0].msb = 9;
    else
    #endif
    raw_params.format[0].msb = 11;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width, height, i, j;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                // Note: image is non-zero but not validated
                ASSERT_VX_OBJECT(y8_r8_c2[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
                params.fcp[0].mux_output0 = 0;
                params.fcp[0].mux_output1 = 0;
                params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
                #ifdef VPAC3L
                if(5 == arg_->dcc)
                    params.h3a_in = TIVX_VPAC_VISS_H3A_IN_PCID;
                else
                #endif
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
                params.bypass_glbce = arg_->bypass_glbce;
                params.bypass_nsf4 = arg_->bypass_nsf4;
                #ifdef VPAC3L
                if(5 == arg_->dcc)
                    params.bypass_pcid = 0;
                #endif

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                /* Create/Configure ae_awb_params input structure */
                if(0 != arg_->results_2a)
                {
                    tivx_ae_awb_params_init(&ae_awb_params);

                    if(2 == arg_->results_2a)
                    {
                        ae_awb_params.ae_valid = 1;
                        ae_awb_params.exposure_time = 16666;
                        ae_awb_params.analog_gain = 1030;
                        #ifdef VPAC3L
                        if(5 == arg_->dcc)
                            ae_awb_params.awb_valid = 0;
                        else
                        #endif
                        ae_awb_params.awb_valid = 1;
                        ae_awb_params.color_temperature = 3000;
                        for (j=0; j<4; j++)
                        {
                            ae_awb_params.wb_gains[j] = 525;
                            ae_awb_params.wb_offsets[j] = 2;
                        }
                    }

                    ASSERT_VX_OBJECT(ae_awb_result[i] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                                        sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                }

                if(0 != arg_->dcc)
                {
                    /* Creating DCC */
                    dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                    ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                        dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    VX_CALL(vxMapUserDataObject(
                        dcc_param_viss[i],
                        0,
                        dcc_buff_size,
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST,
                        0
                    ));
                    memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                    dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                    ASSERT(dcc_status == 0);

                    VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                    /* Done w/ DCC */

                    /* Creating H3A output */
                    ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                    if(NULL != h3a_aew_af[i])
                    {
                        VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                            0,
                            sizeof(tivx_h3a_data_t),
                            &dcc_viss_buf_map_id,
                            (void **)&dcc_viss_buf,
                            (vx_enum)VX_WRITE_ONLY,
                            (vx_enum)VX_MEMORY_TYPE_HOST,
                            0
                            ));

                        memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                        VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                    }

                }

                ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, y8_r8_c2[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));
        if(arg_->bypass_glbce == 0)
        {
            VX_CALL(vxProcessGraph(graph));
        }

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%s", ct_get_test_file_path(), "output/viss_dcc_out_", i, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_output_image_nv12_8bit(file, y8_r8_c2[i]);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                checksum_expected = get_checksum(viss_checksums_luma_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);

                rect.end_x = width/2;
                rect.end_y = height/2;
                checksum_expected = get_checksum(viss_checksums_chroma_ref, arg_->dcc, arg_->results_2a, arg_->bypass_glbce, arg_->bypass_nsf4);
                checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2[i], 1, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                ASSERT(checksum_expected == checksum_expected);
                #endif

                if( (0 != arg_->dcc) && (NULL != h3a_aew_af[i]))
                {
                    tivx_h3a_data_t *h3a_out;
                    vx_size h3a_valid_size;
                    VX_CALL(vxQueryUserDataObject(h3a_aew_af[i], TIVX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
                    ASSERT(h3a_valid_size > 64);
                    ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    /* TIOVX-1247: Setting the cpu_id and channel_id to 0 indiscriminately in order to re-use the same checksums */
                    h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

                    h3a_out->cpu_id = 0;
                    h3a_out->channel_id = 0;

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));

                    checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af[i], 0, h3a_valid_size);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    printf("%d\n", h3a_valid_size);
                    #endif
                    ASSERT(viss_checksums_h3a_ref[arg_->dcc] == checksum_actual);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    ct_write_user_data_object(h3a_aew_af[i], "output/viss_dcc_h3a_out.bin");
                    #endif
                }
            }
        }

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

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&y8_r8_c2[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
                if(0 != arg_->results_2a)
                {
                    VX_CALL(vxReleaseUserDataObject(&ae_awb_result[i]));
                }
                if(0 != arg_->dcc)
                {
                    VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                    VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
                }
            }
        }

        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(y8_r8_c2[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int width, height, exposures, mux0, mux1, mux2, mux3, mux4;
    vx_bool line_interleaved;
    char* target_string;
} Arg_mux;

static uint8_t isMuxValid(int mux0, int mux1, int mux2, int mux3, int mux4)
{
    uint8_t retVal = 1;

    if ( (mux0 == 1) || (mux0 == 2) || (mux0 == 5) )
    {
        retVal = 0;
    }

    if ( (mux1 == 1) || (mux1 == 3) || (mux1 == 4) || (mux1 == 5) )
    {
        retVal = 0;
    }

    if ( (mux3 == 3) || (mux3 == 4) || (mux3 == 5) )
    {
        retVal = 0;
    }

    if ( (mux4 == 0) || (mux4 == 4) || (mux4 == 5) )
    {
        retVal = 0;
    }

    return retVal;
}

#define ADD_LINE_FALSE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=false", __VA_ARGS__, vx_false_e))

#define ADD_LINE_TRUE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/line_intlv=true", __VA_ARGS__, vx_true_e))

#define ADD_MUX0(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux0=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux0=5", __VA_ARGS__, 5))

#define ADD_MUX1(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux1=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux1=5", __VA_ARGS__, 5))

#define ADD_MUX2(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux2=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux2=5", __VA_ARGS__, 5))

#define ADD_MUX3(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux3=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux3=5", __VA_ARGS__, 5))

#define ADD_MUX4(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/mux4=0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/mux4=5", __VA_ARGS__, 5))

/* TODO: Add DCC as option */
/* TODO: Add h3a_in as option */
/* TODO: Add chroma mode as option */
/* TODO: Alternate P12, U16 */
/* TODO: Negative tests w/ correct mux but incorrect val */
/* TODO: Fix failing scenarios:
   mux0 = 4, mux1 = 0 */

#define MUX_PARAMETERS \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_64x48, ADD_EXP1, ADD_MUX0, ADD_MUX1, ADD_MUX2, ADD_MUX3, ADD_MUX4, ADD_LINE_FALSE, ADD_SET_TARGET_PARAMETERS, ARG), \

TEST_WITH_ARG(tivxHwaVpacViss, testMux, Arg_mux,
    MUX_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        if (TIVX_VPAC_VISS_MUX2_YUV422 != arg_->mux2)
        {
            if (TIVX_VPAC_VISS_MUX0_Y12 == arg_->mux0)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX0_VALUE12 == arg_->mux0)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX0_NV12_P12 == arg_->mux0)
            {
                ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, TIVX_DF_IMAGE_NV12_P12), VX_TYPE_IMAGE);
            }
            else
            {
                y12 = NULL;
            }

            if (TIVX_VPAC_VISS_MUX0_NV12_P12 == arg_->mux0)
            {
                uv12_c1 = NULL;
            }
            else
            {
                if (TIVX_VPAC_VISS_MUX1_UV12 == arg_->mux1)
                {
                    ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
                }
                else if (TIVX_VPAC_VISS_MUX1_C1 == arg_->mux1)
                {
                    ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
                }
                else
                {
                    uv12_c1 = NULL;
                }
            }
        }

        if (TIVX_VPAC_VISS_MUX2_Y8 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_RED == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_C2 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_VALUE8 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_NV12 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2)
        {
            ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_YUYV), VX_TYPE_IMAGE);
        }
        else
        {
            y8_r8_c2 = NULL;
        }

        if ((TIVX_VPAC_VISS_MUX2_NV12 == arg_->mux2) ||
            (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2))
        {
            uv8_g8_c3 = NULL;
        }
        else
        {
            if (TIVX_VPAC_VISS_MUX3_UV8 == arg_->mux3)
            {
                ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX3_GREEN == arg_->mux3)
            {
                ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
            }
            else if (TIVX_VPAC_VISS_MUX3_C3 == arg_->mux3)
            {
                ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
            }
            else
            {
                uv8_g8_c3 = NULL;
            }
        }

        if (TIVX_VPAC_VISS_MUX4_BLUE == arg_->mux4)
        {
            ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX4_C4 == arg_->mux4)
        {
            ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        }
        else if (TIVX_VPAC_VISS_MUX4_SAT == arg_->mux4)
        {
            ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        }
        else
        {
            s8_b8_c4 = NULL;
        }

        //ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = arg_->mux0;
        params.fcp[0].mux_output1 = arg_->mux1;
        params.fcp[0].mux_output2 = arg_->mux2;
        params.fcp[0].mux_output3 = arg_->mux3;
        params.fcp[0].mux_output4 = arg_->mux4;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;
        params.h3a_in = 0;

        if (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2)
        {
            params.fcp[0].chroma_mode = 1;
        }

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if (VX_SUCCESS == vxVerifyGraph(graph))
        {
            VX_CALL(vxProcessGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        //VX_CALL(vxReleaseDistribution(&histogram));

        if (NULL != s8_b8_c4)
        {
            VX_CALL(vxReleaseImage(&s8_b8_c4));
            ASSERT(s8_b8_c4 == 0);
        }

        if ( NULL != uv8_g8_c3 )
        {
            VX_CALL(vxReleaseImage(&uv8_g8_c3));
            ASSERT(uv8_g8_c3 == 0);
        }

        if (NULL != y8_r8_c2)
        {
            VX_CALL(vxReleaseImage(&y8_r8_c2));
            ASSERT(y8_r8_c2 == 0);
        }

        if ( NULL != uv12_c1 )
        {
            VX_CALL(vxReleaseImage(&uv12_c1));
            ASSERT(uv12_c1 == 0);
        }

        if ( NULL != y12 )
        {
            VX_CALL(vxReleaseImage(&y12));
            ASSERT(y12 == 0);
        }

        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);

        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST_WITH_ARG(tivxHwaVpacViss, testMuxNegative, Arg_mux,
    MUX_PARAMETERS
)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = arg_->width;
    raw_params.height = arg_->height;
    raw_params.num_exposures = arg_->exposures;
    raw_params.line_interleaved = arg_->line_interleaved;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].ee_mode = 0;
        params.fcp[0].mux_output0 = arg_->mux0;
        params.fcp[0].mux_output1 = arg_->mux1;
        params.fcp[0].mux_output2 = arg_->mux2;
        params.fcp[0].mux_output3 = arg_->mux3;
        params.fcp[0].mux_output4 = arg_->mux4;
        params.h3a_aewb_af_mode = 0;
        params.fcp[0].chroma_mode = 0;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, arg_->target_string));

        if (0 == isMuxValid(arg_->mux0, arg_->mux1, arg_->mux2, arg_->mux3, arg_->mux4))
        {
            EXPECT_NE_VX_STATUS(VX_SUCCESS, vxVerifyGraph(graph));
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

static void ct_write_image2(vx_image image, const char* fileName)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "wb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }
    else
    {
        vx_uint32 width, height;
        vx_imagepatch_addressing_t image_addr;
        vx_rectangle_t rect;
        vx_map_id map_id;
        vx_df_image df;
        void *data_ptr;
        vx_uint32 num_bytes = 1;

        vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

        if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
        {
            num_bytes = 2;
        }
        else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
        {
            num_bytes = 4;
        }

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        vxMapImagePatch(image,
            &rect,
            0,
            &map_id,
            &image_addr,
            &data_ptr,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            VX_NOGAP_X
            );

        fwrite(data_ptr, 1, width*height*num_bytes, f);

        vxUnmapImagePatch(image, map_id);
    }

    fclose(f);
}

static void ct_read_image2(vx_image image, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_uint32 width, height;
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
            vx_map_id map_id;
            vx_df_image df;
            void *data_ptr;
            vx_uint32 num_bytes = 1;

            vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
            vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
            vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

            if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
            {
                num_bytes = 2;
            }
            else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
            {
                num_bytes = 4;
            }

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;

            vxMapImagePatch(image,
                &rect,
                0,
                &map_id,
                &image_addr,
                &data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                VX_NOGAP_X
                );

            if(file_byte_pack == num_bytes)
            {
                memcpy(data_ptr, buf, width*height*num_bytes);
            }
            else if((file_byte_pack == 2) && (num_bytes == 1))
            {
                int i;
                uint8_t *dst = data_ptr;
                uint16_t *src = (uint16_t*)buf;
                for(i = 0; i < width*height; i++)
                {
                    dst[i] = src[i];
                }
            }
            vxUnmapImagePatch(image, map_id);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

static void ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack, uint16_t downshift_bits)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Image file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open image file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_uint32 width, height;
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
            vx_map_id map_id;
            void *data_ptr;
            vx_uint32 num_bytes = 1;
            tivx_raw_image_format_t format[3];

            tivxQueryRawImage(image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));

            if( format[0].pixel_container == TIVX_RAW_IMAGE_16_BIT )
            {
                num_bytes = 2;
            }
            else if( format[0].pixel_container == TIVX_RAW_IMAGE_8_BIT )
            {
                num_bytes = 1;
            }
            else if( format[0].pixel_container == TIVX_RAW_IMAGE_P12_BIT )
            {
                num_bytes = 0;
            }

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;

            tivxMapRawImagePatch(image,
                &rect,
                0,
                &map_id,
                &image_addr,
                &data_ptr,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                TIVX_RAW_IMAGE_PIXEL_BUFFER
                );

            if((file_byte_pack == num_bytes) && (downshift_bits == 0))
            {
                int i;
                uint8_t *dst = data_ptr;
                uint8_t *src = (uint8_t*)buf;
                for(i = 0; i < height; i++)
                {
                    memcpy((void*)&dst[image_addr.stride_y*i], (void*)&src[width*num_bytes*i], width*num_bytes);
                }
            }
            else if((file_byte_pack == 2) && (num_bytes == 2))
            {
                int i, j;
                uint16_t *dst = data_ptr;
                uint16_t *src = (uint16_t*)buf;
                for(j = 0; j < height; j++)
                {
                    for(i = 0; i < width; i++)
                    {
                        dst[i] = src[i] >> downshift_bits;
                    }
                    dst += image_addr.stride_y/2;
                    src += width;
                }
            }
            else if((file_byte_pack == 2) && (num_bytes == 1))
            {
                int i, j;
                uint8_t *dst = data_ptr;
                uint16_t *src = (uint16_t*)buf;
                for(j = 0; j < height; j++)
                {
                    for(i = 0; i < width; i++)
                    {
                        dst[i] = src[i];
                    }
                    dst += image_addr.stride_y;
                    src += width;
                }
            }
            else
            {
                if(file_byte_pack != num_bytes)
                {
                    printf("ct_read_raw_image: size mismatch!!\n");
                    fclose(f);
                    return;
                }
            }
            tivxUnmapRawImagePatch(image, map_id);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

static void ct_read_hist(vx_distribution hist, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("Hist file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open hist file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_map_id map_id;
            vx_df_image df;
            void *data_ptr;
            vx_uint32 num_bytes = 1;

            vxCopyDistribution (hist, buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

static void ct_read_user_data_object(vx_user_data_object user_data_object, const char* fileName, uint16_t file_byte_pack)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];

    if (!fileName)
    {
        CT_ADD_FAILURE("User data object file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "rb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open arrays file: %s\n", fileName);
        return;
    }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if( sz > 0 )
    {
        buf = (char*)ct_alloc_mem(sz);
        fseek(f, 0, SEEK_SET);
        if( fread(buf, 1, sz, f) == sz )
        {
            vx_size size = 0;
            vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_SIZE, &size, sizeof(size));
            if (sz < size)
            {
                vxCopyUserDataObject(user_data_object, 0, sz, buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            }
            else
            {
                vxCopyUserDataObject(user_data_object, 0, size, buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
            }
        }
    }

    ct_free_mem(buf);
    fclose(f);
}

static void ct_write_user_data_object(vx_user_data_object user_data_object, const char* fileName)
{
    FILE* f = 0;
    size_t sz;
    char* buf = 0;
    char file[MAXPATHLENGTH];
    vx_size size;
    vx_status status;

    if (!fileName)
    {
        CT_ADD_FAILURE("User data object file name not specified\n");
        return;
    }

    sz = snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), fileName);
    ASSERT_(return, (sz < MAXPATHLENGTH));

    f = fopen(file, "wb");
    if (!f)
    {
        CT_ADD_FAILURE("Can't open user data object file: %s\n", fileName);
        return;
    }

    status = vxQueryUserDataObject(user_data_object, VX_USER_DATA_OBJECT_SIZE, &size, sizeof(size));
    if (VX_SUCCESS != status)
    {
        fclose(f);
        return;
    }

    if( size > 0 )
    {
        buf = (char*)ct_alloc_mem(size);
        if (NULL != buf)
        {
            status = vxCopyUserDataObject(user_data_object, 0, size, buf, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            if (VX_SUCCESS != status)
            {
                ct_free_mem(buf);
                fclose(f);
                return;
            }
            fwrite(buf, 1, size, f);
        }
    }

    ct_free_mem(buf);
    fclose(f);
}


static vx_status save_image_from_viss(vx_image y8, char *filename_prefix)
{
    char filename[MAXPATHLENGTH];
    vx_status status;

    snprintf(filename, MAXPATHLENGTH, "%s/%s_y8.bmp",
        ct_get_test_file_path(), filename_prefix);

    status = tivx_utils_save_vximage_to_bmpfile(filename, y8);

    return status;
}

static vx_int32 ct_cmp_image2(vx_image image, vx_image image_ref)
{
    vx_uint32 width, height;
    vx_imagepatch_addressing_t image_addr, ref_addr;
    vx_rectangle_t rect;
    vx_map_id map_id, map_id_ref;
    vx_df_image df;
    void *data_ptr, *ref_ptr;
    vx_uint32 num_bytes = 1;
    vx_int32 i, j;
    vx_int32 error = 0;

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(image,
        &rect,
        0,
        &map_id,
        &image_addr,
        &data_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    vxMapImagePatch(image_ref,
        &rect,
        0,
        &map_id_ref,
        &ref_addr,
        &ref_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if( df == VX_DF_IMAGE_U8)
    {
        for(j=0; j<height; j++)
        {
            vx_uint8 *d_ptr = (vx_uint8 *)((vx_uint8 *)data_ptr + (j * image_addr.stride_y));
            vx_uint8 *r_ptr = (vx_uint8 *)((vx_uint8 *)ref_ptr + (j * ref_addr.stride_y));
            for(i=0; i<width; i++)
            {
                if(d_ptr[i] != r_ptr[i])
                {
                    error++;
                }
            }
        }
    }
    else if( (df == VX_DF_IMAGE_U16) || (df == VX_DF_IMAGE_S16) )
    {
        for(j=0; j<height; j++)
        {
            vx_uint16 *d_ptr = (vx_uint16 *)((vx_uint16 *)data_ptr + (j * image_addr.stride_y));
            vx_uint16 *r_ptr = (vx_uint16 *)((vx_uint16 *)ref_ptr + (j * ref_addr.stride_y));
            for(i=0; i<width; i++)
            {
                if(d_ptr[i] != r_ptr[i])
                {
                    error++;
                }
            }
        }
    }
    else if( (df == VX_DF_IMAGE_U32) || (df == VX_DF_IMAGE_S32) )
    {
        for(j=0; j<height; j++)
        {
            vx_uint32 *d_ptr = (vx_uint32 *)((vx_uint32 *)data_ptr + (j * image_addr.stride_y));
            vx_uint32 *r_ptr = (vx_uint32 *)((vx_uint32 *)ref_ptr + (j * ref_addr.stride_y));
            for(i=0; i<width; i++)
            {
                if(d_ptr[i] != r_ptr[i])
                {
                    error++;
                }
            }
        }
    }

    vxUnmapImagePatch(image, map_id);
    vxUnmapImagePatch(image_ref, map_id_ref);

    return error;
}

static vx_int32 ct_cmp_hist(vx_distribution hist, vx_distribution hist_ref)
{
    vx_map_id map_id, map_id_ref;
    uint32_t *data_ptr, *ref_ptr;
    vx_uint32 num_bytes = 1;
    vx_size histogram_numBins;
    vx_int32 i, j;
    vx_int32 error = 0;

    vxQueryDistribution(hist, VX_DISTRIBUTION_BINS, &histogram_numBins, sizeof(histogram_numBins));

    vxMapDistribution(hist,
        &map_id,
        (void *)&data_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        0);

    vxMapDistribution(hist_ref,
        &map_id_ref,
        (void *)&ref_ptr,
        VX_READ_ONLY,
        VX_MEMORY_TYPE_HOST,
        0);

    for(i=0; i<histogram_numBins; i++)
    {
        if(data_ptr[i] != ref_ptr[i])
        {
            error++;
        }
    }

    vxUnmapDistribution(hist, map_id);
    vxUnmapDistribution(hist_ref, map_id_ref);

    return error;
}

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingRaw, SetTarget_Arg, SET_NODE_TARGET_PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_image y12_ref = NULL, uv12_c1_ref = NULL, y8_r8_c2_ref = NULL, uv8_g8_c3_ref = NULL, s8_b8_c4_ref = NULL;
    vx_distribution histogram = NULL;
    vx_distribution histogram_ref = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    vx_user_data_object h3a_aew_af_ref = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    void *h3a_output;
    uint32_t checksum_actual = 0;
    vx_rectangle_t rect;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 1280;
    raw_params.height = 720;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[1].msb = 11;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 0;

    CT_Image src0 = NULL, src1 = NULL,  src2 = NULL;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_Y8;
        params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        params.fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_Y8;
        params.fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
        params.fcp[0].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
        params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create/Configure ae_awb_result input structure */
        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        /* Create h3a_aew_af output buffer. tivx_h3a_data_t includes memory for H3A payload  */
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/bayer_1280x720.raw", 2, 0);

        VX_CALL(vxProcessGraph(graph));
        //VX_CALL(vxProcessGraph(graph));

        /* Check output */

#if CHECK_OUTPUT

        ASSERT_VX_OBJECT(y12_ref = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1_ref = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2_ref = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3_ref = vxCreateImage(context, width, height/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4_ref = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram_ref = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);
        ASSERT_VX_OBJECT(h3a_aew_af_ref = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ct_read_image2(y12_ref, "tivx/out_y12.raw", 2);
        ct_read_image2(uv12_c1_ref, "tivx/out_uv12.raw", 2);
        ct_read_image2(y8_r8_c2_ref, "tivx/out_y8_ee.raw", 2);
        ct_read_image2(uv8_g8_c3_ref, "tivx/out_uv8.raw", 2);
        ct_read_image2(s8_b8_c4_ref, "tivx/out_s8.raw", 2);
        ct_read_hist(histogram_ref, "tivx/out_hist.raw", 4);
        ct_read_user_data_object(h3a_aew_af_ref, "tivx/out_h3a.raw", 4);

        ASSERT(ct_cmp_image2(y12, y12_ref) == 0);
        ASSERT(ct_cmp_image2(uv12_c1, uv12_c1_ref) == 0);
        ASSERT(ct_cmp_image2(y8_r8_c2, y8_r8_c2_ref) == 0);
        ASSERT(ct_cmp_image2(uv8_g8_c3, uv8_g8_c3_ref) == 0);
        ASSERT(ct_cmp_image2(s8_b8_c4, s8_b8_c4_ref) == 0);
        ASSERT(ct_cmp_hist(histogram, histogram_ref) == 0);

        {
            uint32_t *data_ptr;
            tivx_h3a_data_t *h3a_out;
            uint8_t *ptr, *ref_ptr;
            int32_t error = 0;
            vx_map_id map_id, map_id_ref;
            int32_t i;

            vxMapUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &map_id,
                (void *)&data_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST, 0);
            h3a_out = (tivx_h3a_data_t*)data_ptr;
            ptr = (uint8_t *)&h3a_out->data;

            vxMapUserDataObject(h3a_aew_af_ref, 0, sizeof(tivx_h3a_data_t), &map_id_ref,
                (void *)&ref_ptr,
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST, 0);

            for(i=0; i<h3a_out->size; i++)
            {
                if(ptr[i] != ref_ptr[i])
                {
                    error++;
                }
            }
            vxUnmapUserDataObject(h3a_aew_af, map_id);
            vxUnmapUserDataObject(h3a_aew_af_ref, map_id_ref);
            ASSERT(error == 0);
        }

        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_ref));
        VX_CALL(vxReleaseDistribution(&histogram_ref));
        VX_CALL(vxReleaseImage(&s8_b8_c4_ref));
        VX_CALL(vxReleaseImage(&uv8_g8_c3_ref));
        VX_CALL(vxReleaseImage(&y8_r8_c2_ref));
        VX_CALL(vxReleaseImage(&uv12_c1_ref));
        VX_CALL(vxReleaseImage(&y12_ref));
#endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = raw_params.width;
        rect.end_y = raw_params.height;

        /* For visual verification */
        save_image_from_viss(y8_r8_c2, "output/out_y8");

        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        printf("0x%08x\n", checksum_actual);
        ASSERT(0xd45afbd4 == checksum_actual);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    int negative_test;
    int condition;
    char* target_string;
} ArgNegative;

#define ADD_NEGATIVE_TEST(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output0", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output1", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output2", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output3", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=mux_output4", __VA_ARGS__, 4)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=bypass_glbce", __VA_ARGS__, 5)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=bypass_nsf4", __VA_ARGS__, 6)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=h3a_in", __VA_ARGS__, 7)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=h3a_aewb_af_mode", __VA_ARGS__, 8)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=ee_mode", __VA_ARGS__, 9)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=chroma_mode", __VA_ARGS__, 10)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=h3a_source_data", __VA_ARGS__, 11)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=ae_valid", __VA_ARGS__, 12)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=ae_converged", __VA_ARGS__, 13)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=awb_valid", __VA_ARGS__, 14)), \
    CT_EXPAND(nextmacro(testArgName "/negative_test=awb_converged", __VA_ARGS__, 15))

#define ADD_NEGATIVE_CONDITION(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_positive", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_positive", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/condition=lower_negative", __VA_ARGS__, 2)), \
    CT_EXPAND(nextmacro(testArgName "/condition=upper_negative", __VA_ARGS__, 3)), \
    CT_EXPAND(nextmacro(testArgName "/condition=middle_negative", __VA_ARGS__, 4))
#define PARAMETERS_NEGATIVE \
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ADD_SET_TARGET_PARAMETERS, ARG)

TEST_WITH_ARG(tivxHwaVpacViss, testNegativeGraph, ArgNegative,
    PARAMETERS_NEGATIVE)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;
    tivx_h3a_data_t h3a_data;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;
    raw_params.width = 128;
    raw_params.height = 128;
    raw_params.num_exposures = 3;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.format[1].pixel_container = TIVX_RAW_IMAGE_8_BIT;
    raw_params.format[1].msb = 7;
    raw_params.format[2].pixel_container = TIVX_RAW_IMAGE_P12_BIT;
    raw_params.format[2].msb = 11;
    raw_params.meta_height_before = 5;
    raw_params.meta_height_after = 0;

    ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));

    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);
        ASSERT_VX_OBJECT(y12 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv12_c1 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(uv8_g8_c3 = vxCreateImage(context, 128, 128/2, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(s8_b8_c4 = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(histogram = vxCreateDistribution(context, 256, 0, 256), VX_TYPE_DISTRIBUTION);

        tivx_vpac_viss_params_init(&params);
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        tivx_ae_awb_params_init(&ae_awb_params);
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        tivx_h3a_data_init(&h3a_data);
        ASSERT_VX_OBJECT(h3a_aew_af = vxCreateUserDataObject(context, "tivx_h3a_data_t",
                                                            sizeof(tivx_h3a_data_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.fcp[0].mux_output0 = 0U;
        params.fcp[0].mux_output1 = 0U;
        params.fcp[0].mux_output2 = 0U;
        params.fcp[0].mux_output3 = 0U;
        params.fcp[0].mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.fcp[0].ee_mode = 0U;
        params.fcp[0].chroma_mode = 0U;

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 4;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 5;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output0 = 5;
                }
                else
                {
                    params.fcp[0].mux_output0 = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output1 = 3;
                }
                else
                {
                    params.fcp[0].mux_output1 = 1;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output2 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output2 = 5;
                }
                else if (2U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.fcp[0].mux_output2 = 8;
                    #else
                    params.fcp[0].mux_output2 = 6;
                    #endif
                }
                else if (3U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.fcp[0].mux_output2 = 8;
                    #else
                    params.fcp[0].mux_output2 = 6;
                    #endif
                }
                else
                {
                    #ifdef VPAC3L
                    params.fcp[0].mux_output2 = 8;
                    #else
                    params.fcp[0].mux_output2 = 6;
                    #endif
                }
                break;
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output3 = 3;
                }
                else
                {
                    params.fcp[0].mux_output3 = 3;
                }
                break;
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 3;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 0;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].mux_output4 = 4;
                }
                else
                {
                    params.fcp[0].mux_output4 = 4;
                }
                break;
            }
            case 5:
            {
                if (0U == arg_->condition)
                {
                    params.bypass_glbce = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.bypass_glbce = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.bypass_glbce = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.bypass_glbce = 2;
                }
                else
                {
                    params.bypass_glbce = 2;
                }
                break;
            }
            case 6:
            {
                if (0U == arg_->condition)
                {
                    params.bypass_nsf4 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.bypass_nsf4 = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.bypass_nsf4 = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.bypass_nsf4 = 2;
                }
                else
                {
                    params.bypass_nsf4 = 2;
                }
                break;
            }
            case 7:
            {
                if (0U == arg_->condition)
                {
                    params.h3a_in = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.h3a_in = 3;
                }
                else if (2U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.h3a_in = 5;
                    #else
                    params.h3a_in = 4;
                    #endif
                }
                else if (3U == arg_->condition)
                {
                    #ifdef VPAC3L
                    params.h3a_in = 5;
                    #else
                    params.h3a_in = 4;
                    #endif
                }
                else
                {
                    #ifdef VPAC3L
                    params.h3a_in = 5;
                    #else
                    params.h3a_in = 4;
                    #endif
                }
                break;
            }
            case 8:
            {
                if (0U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.h3a_aewb_af_mode = 2;
                }
                else
                {
                    params.h3a_aewb_af_mode = 2;
                }
                break;
            }
            case 9:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].ee_mode = 3;
                }
                else
                {
                    params.fcp[0].ee_mode = 3;
                }
                break;
            }
            case 10:
            {
                if (0U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.fcp[0].chroma_mode = 2;
                }
                else
                {
                    params.fcp[0].chroma_mode = 2;
                }
                break;
            }
            case 11:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.h3a_source_data = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.h3a_source_data = 3;
                }
                else if (2U == arg_->condition)
                {
                    #ifdef VPAC3L
                    ae_awb_params.h3a_source_data = 5;
                    #else
                    ae_awb_params.h3a_source_data = 4;
                    #endif
                }
                else if (3U == arg_->condition)
                {
                    #ifdef VPAC3L
                    ae_awb_params.h3a_source_data = 5;
                    #else
                    ae_awb_params.h3a_source_data = 4;
                    #endif
                }
                else
                {
                    #ifdef VPAC3L
                    ae_awb_params.h3a_source_data = 5;
                    #else
                    ae_awb_params.h3a_source_data = 4;
                    #endif
                }
                break;
            }
            case 12:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.ae_valid = 2;
                }
                else
                {
                    ae_awb_params.ae_valid = 2;
                }
                break;
            }
            case 13:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.ae_converged = 2;
                }
                else
                {
                    ae_awb_params.ae_converged = 2;
                }
                break;
            }
            case 14:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.awb_valid = 2;
                }
                else
                {
                    ae_awb_params.awb_valid = 2;
                }
                break;
            }
            case 15:
            {
                if (0U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 0;
                }
                else if (1U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 1;
                }
                else if (2U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 2;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.awb_converged = 2;
                }
                else
                {
                    ae_awb_params.awb_converged = 2;
                }
                break;
            }
        }

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(h3a_aew_af, 0, sizeof(tivx_h3a_data_t), &h3a_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram, NULL, NULL), VX_TYPE_NODE);

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
        VX_CALL(vxReleaseDistribution(&histogram));
        VX_CALL(vxReleaseImage(&s8_b8_c4));
        VX_CALL(vxReleaseImage(&uv8_g8_c3));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&uv12_c1));
        VX_CALL(vxReleaseImage(&y12));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(h3a_aew_af == 0);
        ASSERT(histogram == 0);
        ASSERT(s8_b8_c4 == 0);
        ASSERT(uv8_g8_c3 == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(uv12_c1 == 0);
        ASSERT(y12 == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result == 0);
        ASSERT(configuration == 0);

        tivxHwaUnLoadKernels(context);
    }
}

#ifdef VPAC3L
static vx_int32 write_output_ir_image(char * file_name, vx_image out, vx_uint32 ir_format)
{
    FILE * fp;
    vx_uint32 width, height;
    vx_rectangle_t rect;
    void *data_ptr1;
    vx_map_id map_id1;
    vx_df_image df;
    vx_imagepatch_addressing_t image_addr;
    vx_uint32 num_bytes_written_to_file = 0;

    printf("Opening file %s \n", file_name);
    fp = fopen(file_name, "wb");
    if(!fp)
    {
        printf("Unable to open file %s\n", file_name);
        return -1;
    }

    vxQueryImage(out, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(out, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(out, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));

    printf("out width =  %d\n", width);
    printf("out height =  %d\n", height);
    printf("out format =  %d\n", df);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    vxMapImagePatch(out,
        &rect,
        0,
        &map_id1,
        &image_addr,
        &data_ptr1,
        VX_WRITE_ONLY,
        VX_MEMORY_TYPE_HOST,
        VX_NOGAP_X
        );

    if(!data_ptr1)
    {
        printf("data_ptr1 is NULL \n");
        fclose(fp);
        return -1;
    }

    if(0 == ir_format)
        num_bytes_written_to_file = fwrite((uint8_t *)data_ptr1, 1, width*height*1, fp);
    else if(1 == ir_format)
        num_bytes_written_to_file = fwrite((uint8_t *)data_ptr1, 1, width*height*1.5, fp);
    else if(2 == ir_format)
        num_bytes_written_to_file = fwrite((uint8_t *)data_ptr1, 1, width*height*1, fp);

    printf("Written image \n");
    fclose(fp);
    printf("%d bytes written to %s\n", num_bytes_written_to_file, file_name);

    return num_bytes_written_to_file;
}

typedef struct {
    const char* testName;
    int results_2a;
    int ir_format;
    char *target_string, *target_string_2;
} ArgDccIr;

static uint32_t viss_checksums_ref_ir[9] = {
    (uint32_t) 0x8ba222d0, (uint32_t) 0x6aa2313f, (uint32_t) 0xb8cc80d0,
    (uint32_t) 0x8ba222d0, (uint32_t) 0x6aa2313f, (uint32_t) 0xb8cc80d0,
    (uint32_t) 0x1974762c, (uint32_t) 0x8618a728, (uint32_t) 0x63506100
};

static uint32_t viss_checksums_h3a_ref_ir = (uint32_t) 0xa18d1b8e;

#define ADD_IR_FORMAT(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/ir_format=8b", __VA_ARGS__, 0)), \
    CT_EXPAND(nextmacro(testArgName "/ir_format=12b", __VA_ARGS__, 1)), \
    CT_EXPAND(nextmacro(testArgName "/ir_format=16b", __VA_ARGS__, 2))

#define PARAMETERS_DCC_IR \
    CT_GENERATE_PARAMETERS("cksm_ir", ADD_2A, ADD_IR_FORMAT, ADD_SET_TARGET_PARAMETERS_MULTI_INST, ARG)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccIr, ArgDccIr, PARAMETERS_DCC_IR)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object ae_awb_result[TEST_NUM_NODE_INSTANCE] = {NULL};
    tivx_raw_image raw[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_image ir_op[TEST_NUM_NODE_INSTANCE] = {NULL};
    vx_user_data_object h3a_aew_af[TEST_NUM_NODE_INSTANCE] = {NULL};
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss[TEST_NUM_NODE_INSTANCE] = {NULL};
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node[TEST_NUM_NODE_INSTANCE] = {NULL};

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }
    if (NULL != arg_->target_string_2)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string_2));
    }

    raw_params.width = 1600;
    raw_params.height = 1300;
    raw_params.meta_height_after = 0;
    sensor_dcc_id = 2312;
    sensor_name = SENSOR_OV2312_UB953_LI;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
    downshift_bits = 0;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width, height, i, j;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT_VX_OBJECT(raw[i] = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
                VX_CALL(tivxQueryRawImage(raw[i], TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

                // Note: image is non-zero but not validated
                if(0 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
                else if(1 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
                else if(2 == arg_->ir_format)
                    ASSERT_VX_OBJECT(ir_op[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U16), VX_TYPE_IMAGE);

                /* Create/Configure configuration input structure */
                tivx_vpac_viss_params_init(&params);

                params.bypass_pcid = 0;
                params.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
                params.enable_bayer_op = TIVX_VPAC_VISS_BAYER_DISABLE;
                params.sensor_dcc_id = sensor_dcc_id;
                params.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;

                if(0 == arg_->ir_format)
                    params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;
                else if(1 == arg_->ir_format)
                    params.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR12_P12;
                else if(2 == arg_->ir_format)
                    params.fcp[0].mux_output0 = 0;

                params.fcp[0].mux_output1 = 0;

                if(2 == arg_->ir_format)
                    params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_IR12_U16;
                else
                    params.fcp[0].mux_output2 = 0;

                params.fcp[0].mux_output3 = 0;
                params.fcp[0].mux_output4 = 3;
                params.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
                params.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
                params.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

                ASSERT_VX_OBJECT(configuration[i] = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                                    sizeof(tivx_vpac_viss_params_t), &params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                /* Create/Configure ae_awb_params input structure */
                if(0 != arg_->results_2a)
                {
                    tivx_ae_awb_params_init(&ae_awb_params);

                    if(2 == arg_->results_2a)
                    {
                        ae_awb_params.ae_valid = 0;
                        ae_awb_params.exposure_time = 16666;
                        ae_awb_params.analog_gain = 1030;
                        ae_awb_params.awb_valid = 1;
                        ae_awb_params.color_temperature = 3000;
                        for (j=0; j<4; j++)
                        {
                            ae_awb_params.wb_gains[j] = 525;
                            ae_awb_params.wb_offsets[j] = 2;
                        }
                    }

                    ASSERT_VX_OBJECT(ae_awb_result[i] = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                                        sizeof(tivx_ae_awb_params_t), &ae_awb_params), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
                }

                /* Creating DCC */
                dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

                ASSERT_VX_OBJECT(dcc_param_viss[i] = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
                    dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                VX_CALL(vxMapUserDataObject(
                    dcc_param_viss[i],
                    0,
                    dcc_buff_size,
                    &dcc_viss_buf_map_id,
                    (void **)&dcc_viss_buf,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    0
                ));
                memset(dcc_viss_buf, 0xAB, dcc_buff_size);

                dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
                ASSERT(dcc_status == 0);

                VX_CALL(vxUnmapUserDataObject(dcc_param_viss[i], dcc_viss_buf_map_id));
                /* Done w/ DCC */

                /* Creating H3A output */
                ASSERT_VX_OBJECT(h3a_aew_af[i] = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

                if(NULL != h3a_aew_af[i])
                {
                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));
                }

                if((0 == arg_->ir_format) || (1 == arg_->ir_format))
                {
                    ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], ir_op[i], NULL, NULL, NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);
                }
                else if(2 == arg_->ir_format)
                {
                    ASSERT_VX_OBJECT(node[i] = tivxVpacVissNode(graph, configuration[i], ae_awb_result[i], dcc_param_viss[i],
                                                        raw[i], NULL, NULL, ir_op[i], NULL, NULL,
                                                        h3a_aew_af[i], NULL, NULL, NULL), VX_TYPE_NODE);
                }

                if (i==0)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string));
                }
                else if (i==1)
                {
                    VX_CALL(vxSetNodeTarget(node[i], VX_TARGET_STRING, arg_->target_string_2));
                }

                ct_read_raw_image(raw[i], file_name, 2, downshift_bits);
            }
        }

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                snprintf(file, MAXPATHLENGTH, "%s/%s%d%d%s", ct_get_test_file_path(), "output/viss_dcc_ir_out_", i,(int)arg_->ir_format, ".yuv");
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                write_output_ir_image(file, ir_op[i], arg_->ir_format);
                #endif

                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = width;
                rect.end_y = height;

                checksum_expected = viss_checksums_ref_ir[(3)*(arg_->results_2a) + arg_->ir_format];
                checksum_actual = tivx_utils_simple_image_checksum(ir_op[i], 0, rect);
                #if defined(TEST_VISS_CHECKSUM_LOGGING)
                printf("0x%08x\n", checksum_actual);
                #endif
                ASSERT(checksum_expected == checksum_actual);

                if(NULL != h3a_aew_af[i])
                {
                    tivx_h3a_data_t *h3a_out;
                    vx_size h3a_valid_size;
                    VX_CALL(vxQueryUserDataObject(h3a_aew_af[i], TIVX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
                    ASSERT(h3a_valid_size > 64);
                    ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

                    VX_CALL(vxMapUserDataObject(h3a_aew_af[i],
                        0,
                        sizeof(tivx_h3a_data_t),
                        &dcc_viss_buf_map_id,
                        (void **)&dcc_viss_buf,
                        (vx_enum)VX_WRITE_ONLY,
                        (vx_enum)VX_MEMORY_TYPE_HOST,
                        0
                        ));

                    h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

                    h3a_out->cpu_id = 0;
                    h3a_out->channel_id = 0;

                    VX_CALL(vxUnmapUserDataObject(h3a_aew_af[i], dcc_viss_buf_map_id));

                    checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af[i], 0, h3a_valid_size);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    printf("0x%08x\n", checksum_actual);
                    #endif
                    ASSERT(viss_checksums_h3a_ref_ir == checksum_actual);
                    #if defined(TEST_VISS_CHECKSUM_LOGGING)
                    ct_write_user_data_object(h3a_aew_af[i], "output/viss_dcc_h3a_ir_out.bin");
                    #endif
                }
            }
        }

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

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                VX_CALL(vxReleaseNode(&node[i]));
                VX_CALL(vxReleaseImage(&ir_op[i]));
                VX_CALL(tivxReleaseRawImage(&raw[i]));
                VX_CALL(vxReleaseUserDataObject(&configuration[i]));
                if(0 != arg_->results_2a)
                {
                    VX_CALL(vxReleaseUserDataObject(&ae_awb_result[i]));
                }
                VX_CALL(vxReleaseUserDataObject(&h3a_aew_af[i]));
                VX_CALL(vxReleaseUserDataObject(&dcc_param_viss[i]));
            }
        }

        VX_CALL(vxReleaseGraph(&graph));

        ASSERT(graph == 0);

        for (i = 0; i < TEST_NUM_NODE_INSTANCE; i++)
        {
            if ( ((i==0) && (NULL != arg_->target_string)) ||
                 ((i==1) && (NULL != arg_->target_string_2)) )
            {
                ASSERT(node[i] == 0);
                ASSERT(h3a_aew_af[i] == 0);
                ASSERT(ir_op[i] == 0);
                ASSERT(raw[i] == 0);
                ASSERT(ae_awb_result[i] == 0);
                ASSERT(configuration[i] == 0);
                ASSERT(dcc_param_viss[i] == 0);
            }
        }

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* testName;
    CT_Image(*generator)(const char* fileName, int width, int height);
    char *target_string;
} ArgDccRgbIr;

#define PARAMETERS_DCC_RGB_IR \
    CT_GENERATE_PARAMETERS("cksm_rgb_ir", ADD_SET_TARGET_PARAMETERS, ARG, NULL)

TEST_WITH_ARG(tivxHwaVpacViss, testGraphProcessingFileDccRgbIr, ArgDccRgbIr, PARAMETERS_DCC_RGB_IR)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration_rgb = NULL, configuration_ir = NULL;
    vx_user_data_object ae_awb_result_rgb = NULL, ae_awb_result_ir = NULL;
    tivx_raw_image raw = NULL;
    vx_image y8_r8_c2 = NULL;
    vx_image ir_op = NULL;
    vx_user_data_object h3a_aew_af_rgb = NULL, h3a_aew_af_ir = NULL;
    char file_rgb[MAXPATHLENGTH];
    char file_ir[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss = NULL;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;
    uint32_t checksum_actual = 0, checksum_expected = 0;
    vx_rectangle_t rect;
    uint32_t sensor_dcc_id;
    uint32_t sensor_dcc_mode;
    char *sensor_name = NULL;
    char *file_name = NULL;
    uint16_t downshift_bits;

    tivx_vpac_viss_params_t params_rgb, params_ir;
    tivx_ae_awb_params_t ae_awb_params_rgb, ae_awb_params_ir;

    vx_graph graph_rgb = 0;
    vx_graph graph_ir = 0;
    vx_node node_rgb = NULL;
    vx_node node_ir = NULL;

    tivx_raw_image_create_params_t raw_params;

    if (NULL != arg_->target_string)
    {
        ASSERT(vx_true_e == tivxIsTargetEnabled(arg_->target_string));
    }

    raw_params.width = 1600;
    raw_params.height = 1300;
    raw_params.meta_height_after = 0;
    sensor_dcc_id = 2312;
    sensor_name = SENSOR_OV2312_UB953_LI;
    sensor_dcc_mode = 0;
    file_name = "psdkra/app_single_cam/OV2312_001/input1.raw";
    downshift_bits = 0;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 9;
    raw_params.meta_height_before = 0;

    {
        vx_uint32 width, height, j;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(graph_rgb = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph_ir = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(ir_op = vxCreateImage(context, width, height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        tivx_vpac_viss_params_init(&params_rgb);
        tivx_vpac_viss_params_init(&params_ir);

        params_rgb.bypass_pcid = 0;
        params_rgb.sensor_dcc_id = sensor_dcc_id;
        params_rgb.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params_rgb.fcp[0].mux_output0 = 0;
        params_rgb.fcp[0].mux_output1 = 0;
        params_rgb.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
        params_rgb.fcp[0].mux_output3 = 0;
        params_rgb.fcp[0].mux_output4 = 3;
        params_rgb.h3a_in = TIVX_VPAC_VISS_H3A_IN_PCID;
        params_rgb.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params_rgb.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        params_ir.bypass_pcid = 0;
        params_ir.enable_ir_op = TIVX_VPAC_VISS_IR_ENABLE;
        params_ir.enable_bayer_op = TIVX_VPAC_VISS_BAYER_DISABLE;
        params_ir.sensor_dcc_id = sensor_dcc_id;
        params_ir.fcp[0].ee_mode = TIVX_VPAC_VISS_EE_MODE_OFF;
        params_ir.fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_IR8;
        params_ir.fcp[0].mux_output1 = 0;
        params_ir.fcp[0].mux_output2 = 0;
        params_ir.fcp[0].mux_output3 = 0;
        params_ir.fcp[0].mux_output4 = 3;
        params_ir.h3a_in = TIVX_VPAC_VISS_H3A_IN_LSC;
        params_ir.h3a_aewb_af_mode = TIVX_VPAC_VISS_H3A_MODE_AEWB;
        params_ir.fcp[0].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;

        ASSERT_VX_OBJECT(configuration_rgb = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), &params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(configuration_ir = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), &params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Create/Configure ae_awb_params input structure */
        tivx_ae_awb_params_init(&ae_awb_params_rgb);
        tivx_ae_awb_params_init(&ae_awb_params_ir);

        ae_awb_params_rgb.ae_valid = 1;
        ae_awb_params_rgb.exposure_time = 16666;
        ae_awb_params_rgb.analog_gain = 1030;
        ae_awb_params_rgb.awb_valid = 0;
        ae_awb_params_rgb.color_temperature = 3000;
        for (j=0; j<4; j++)
        {
            ae_awb_params_rgb.wb_gains[j] = 525;
            ae_awb_params_rgb.wb_offsets[j] = 2;
        }

        ae_awb_params_ir.ae_valid = 0;
        ae_awb_params_ir.exposure_time = 16666;
        ae_awb_params_ir.analog_gain = 1030;
        ae_awb_params_ir.awb_valid = 1;
        ae_awb_params_ir.color_temperature = 3000;
        for (j=0; j<4; j++)
        {
            ae_awb_params_ir.wb_gains[j] = 525;
            ae_awb_params_ir.wb_offsets[j] = 2;
        }

        ASSERT_VX_OBJECT(ae_awb_result_rgb = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), &ae_awb_params_rgb), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(ae_awb_result_ir = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), &ae_awb_params_ir), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(sensor_name, sensor_dcc_mode);

        ASSERT_VX_OBJECT(dcc_param_viss = vxCreateUserDataObject( context, (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size, NULL),(enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        ));
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(sensor_name, sensor_dcc_mode, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        VX_CALL(vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id));
        /* Done w/ DCC */

        /* Creating H3A output */
        ASSERT_VX_OBJECT(h3a_aew_af_rgb = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(h3a_aew_af_ir = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL),
            (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        if(NULL != h3a_aew_af_rgb)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af_rgb,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_rgb, dcc_viss_buf_map_id));
        }

        if(NULL != h3a_aew_af_ir)
        {
            VX_CALL(vxMapUserDataObject(h3a_aew_af_ir,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            memset(dcc_viss_buf, 0, sizeof(tivx_h3a_data_t));

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_ir, dcc_viss_buf_map_id));
        }

        ASSERT_VX_OBJECT(node_rgb = tivxVpacVissNode(graph_rgb, configuration_rgb, ae_awb_result_rgb, dcc_param_viss,
                                            raw, NULL, NULL, y8_r8_c2, NULL, NULL,
                                            h3a_aew_af_rgb, NULL, NULL, NULL), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node_ir = tivxVpacVissNode(graph_ir, configuration_ir, ae_awb_result_ir, dcc_param_viss,
                                            raw, ir_op, NULL, NULL, NULL, NULL,
                                            h3a_aew_af_ir, NULL, NULL, NULL), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node_rgb, VX_TARGET_STRING, arg_->target_string));
        VX_CALL(vxSetNodeTarget(node_ir, VX_TARGET_STRING, arg_->target_string));

        ct_read_raw_image(raw, file_name, 2, downshift_bits);

        VX_CALL(vxVerifyGraph(graph_rgb));
        VX_CALL(vxVerifyGraph(graph_ir));

        VX_CALL(vxProcessGraph(graph_rgb));
        VX_CALL(vxProcessGraph(graph_ir));
        VX_CALL(vxProcessGraph(graph_rgb));
        VX_CALL(vxProcessGraph(graph_ir));

        snprintf(file_rgb, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_rgbir_rgb_out.yuv");
        snprintf(file_ir, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_rgbir_ir_out.yuv");
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        write_output_image_nv12_8bit(file_rgb, y8_r8_c2);
        write_output_ir_image(file_ir, ir_op, 0);
        #endif

        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

        checksum_expected = viss_checksums_luma_ref[71];
        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        checksum_expected = viss_checksums_ref_ir[6];
        checksum_actual = tivx_utils_simple_image_checksum(ir_op, 0, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        rect.end_x = width/2;
        rect.end_y = height/2;
        checksum_expected = viss_checksums_chroma_ref[71];
        checksum_actual = tivx_utils_simple_image_checksum(y8_r8_c2, 1, rect);
        #if defined(TEST_VISS_CHECKSUM_LOGGING)
        printf("0x%08x\n", checksum_actual);
        #endif
        ASSERT(checksum_expected == checksum_actual);

        if(NULL != h3a_aew_af_rgb)
        {
            tivx_h3a_data_t *h3a_out;
            vx_size h3a_valid_size;
            VX_CALL(vxQueryUserDataObject(h3a_aew_af_rgb, TIVX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
            ASSERT(h3a_valid_size > 64);
            ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

            VX_CALL(vxMapUserDataObject(h3a_aew_af_rgb,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

            h3a_out->cpu_id = 0;
            h3a_out->channel_id = 0;

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_rgb, dcc_viss_buf_map_id));

            checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af_rgb, 0, h3a_valid_size);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            ASSERT(viss_checksums_h3a_ref[5] == checksum_actual);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            ct_write_user_data_object(h3a_aew_af_rgb, "output/viss_dcc_h3a_rgbir_rgb_out.bin");
            #endif
        }

        if(NULL != h3a_aew_af_ir)
        {
            tivx_h3a_data_t *h3a_out;
            vx_size h3a_valid_size;
            VX_CALL(vxQueryUserDataObject(h3a_aew_af_ir, TIVX_USER_DATA_OBJECT_VALID_SIZE, &h3a_valid_size, sizeof(vx_size)));
            ASSERT(h3a_valid_size > 64);
            ASSERT(h3a_valid_size <= sizeof(tivx_h3a_data_t));

            VX_CALL(vxMapUserDataObject(h3a_aew_af_ir,
                0,
                sizeof(tivx_h3a_data_t),
                &dcc_viss_buf_map_id,
                (void **)&dcc_viss_buf,
                (vx_enum)VX_WRITE_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST,
                0
                ));

            h3a_out = (tivx_h3a_data_t*)dcc_viss_buf;

            h3a_out->cpu_id = 0;
            h3a_out->channel_id = 0;

            VX_CALL(vxUnmapUserDataObject(h3a_aew_af_ir, dcc_viss_buf_map_id));

            checksum_actual = tivx_utils_user_data_object_checksum(h3a_aew_af_ir, 0, h3a_valid_size);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            printf("0x%08x\n", checksum_actual);
            #endif
            ASSERT(viss_checksums_h3a_ref_ir == checksum_actual);
            #if defined(TEST_VISS_CHECKSUM_LOGGING)
            ct_write_user_data_object(h3a_aew_af_ir, "output/viss_dcc_h3a_rgbir_ir_out.bin");
            #endif
        }

        VX_CALL(vxReleaseNode(&node_rgb));
        VX_CALL(vxReleaseNode(&node_ir));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(vxReleaseImage(&ir_op));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&configuration_rgb));
        VX_CALL(vxReleaseUserDataObject(&configuration_ir));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result_rgb));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result_ir));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_rgb));
        VX_CALL(vxReleaseUserDataObject(&h3a_aew_af_ir));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

        VX_CALL(vxReleaseGraph(&graph_rgb));
        VX_CALL(vxReleaseGraph(&graph_ir));

        ASSERT(graph_rgb == 0);
        ASSERT(graph_ir == 0);

        ASSERT(node_rgb == 0);
        ASSERT(node_ir == 0);
        ASSERT(h3a_aew_af_rgb == 0);
        ASSERT(h3a_aew_af_ir == 0);
        ASSERT(y8_r8_c2 == 0);
        ASSERT(ir_op == 0);
        ASSERT(raw == 0);
        ASSERT(ae_awb_result_rgb == 0);
        ASSERT(ae_awb_result_ir == 0);
        ASSERT(configuration_rgb == 0);
        ASSERT(configuration_ir == 0);
        ASSERT(dcc_param_viss == 0);

        tivxHwaUnLoadKernels(context);
    }
}
#endif

#if defined(BUILD_CT_TIOVX_HWA_NEGATIVE_TESTS)
#define testMuxNegative testMuxNegative
#else
#define testMuxNegative DISABLED_testMuxNegative
#endif

TESTCASE_TESTS(tivxHwaVpacViss,
               testNodeCreation,
               #ifndef VPAC3L
               testGraphProcessing,
               #endif
               testGraphProcessingFile,
               #ifndef VPAC3L
               testGraphProcessingRaw,
               #endif
               testGraphProcessingFileDcc,
               #ifdef VPAC3L
               testGraphProcessingFileDccIr,
               testGraphProcessingFileDccRgbIr,
               #endif
               testNegativeGraph/*,
               testMuxNegative ,
               testMux,
               testGraphProcessing,
               */)
#endif /* BUILD_VPAC_VISS */
