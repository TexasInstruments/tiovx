/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/j7.h>
#include "test_engine/test.h"
#include "tivx_utils_file_rd_wr.h"
#include <string.h>
#include <utils/iss/include/app_iss.h>
#include "test_hwa_common.h"

#define APP_MAX_FILE_PATH           (512u)

#define ADD_SIZE_64x48(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/sz=64x48", __VA_ARGS__, 64, 48))

static void ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack);

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
        fclose(fp);
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
        fclose(fp);
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

TEST(tivxHwaVpacViss, testNodeCreation)
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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

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
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP1, ADD_LINE_FALSE, ARG), \
    /*CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP2, ADD_LINE_FALSE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP3, ADD_LINE_FALSE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP1, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP2, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_640x480, ADD_EXP3, ADD_LINE_TRUE, ARG), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_2048x1024, ADD_EXP1, ARG)*/

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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.ee_mode = 2;
        params.mux_output0 = 0;
        params.mux_output1 = 0;
        params.mux_output2 = 0;
        params.mux_output3 = 0;
        params.mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.chroma_mode = 0;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

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

TEST(tivxHwaVpacViss, testGraphProcessingFile)
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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.ee_mode = 0;
        params.mux_output0 = 4;
        params.mux_output1 = 0;
        params.mux_output2 = 4;
        params.mux_output3 = 0;
        params.mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.chroma_mode = 0;
        params.bypass_glbce = 1; // Note: default glbce still giving issues when enabled
        params.bypass_nsf4 = 1; // TODO: untested

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/raw_1280x720.raw", 2);

        VX_CALL(vxProcessGraph(graph));
        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_out.yuv");
        write_output_image_nv12_8bit(file, y8_r8_c2);

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

TEST(tivxHwaVpacViss, testGraphProcessingFileDcc)
{
    vx_context context = context_->vx_context_;
    vx_user_data_object configuration = NULL;
    vx_user_data_object ae_awb_result = NULL;
    tivx_raw_image raw = NULL;
    vx_image y12 = NULL, uv12_c1 = NULL, y8_r8_c2 = NULL, uv8_g8_c3 = NULL, s8_b8_c4 = NULL;
    vx_distribution histogram = NULL;
    vx_user_data_object h3a_aew_af = NULL;
    char file[MAXPATHLENGTH];
    /* Dcc objects */
    vx_user_data_object dcc_param_viss;
    const vx_char dcc_viss_user_data_object_name[] = "dcc_viss";
    vx_size dcc_buff_size = 1;
    vx_map_id dcc_viss_buf_map_id;
    uint8_t * dcc_viss_buf;
    int32_t dcc_status;

    tivx_vpac_viss_params_t params;
    tivx_ae_awb_params_t ae_awb_params;

    vx_graph graph = 0;
    vx_node node = 0;

    tivx_raw_image_create_params_t raw_params;

    raw_params.width = 1936;
    raw_params.height = 1096;
    raw_params.num_exposures = 1;
    raw_params.line_interleaved = vx_false_e;
    raw_params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
    raw_params.format[0].msb = 11;
    raw_params.meta_height_before = 0;
    raw_params.meta_height_after = 4;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
    {
        vx_uint32 width, height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        ASSERT_VX_OBJECT(raw = tivxCreateRawImage(context, &raw_params), (enum vx_type_e)TIVX_TYPE_RAW_IMAGE);

        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(width)));
        VX_CALL(tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(height)));

        // Note: image is non-zero but not validated
        ASSERT_VX_OBJECT(y8_r8_c2 = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        /* Create/Configure configuration input structure */
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        /* Creating DCC */
        dcc_buff_size = appIssGetDCCSizeVISS(SENSOR_SONY_IMX390_UB953_D3, 0U);

        dcc_param_viss = vxCreateUserDataObject(
            context,
            (const vx_char*)&dcc_viss_user_data_object_name,
            dcc_buff_size,
            NULL
        );

        vxMapUserDataObject(
            dcc_param_viss,
            0,
            dcc_buff_size,
            &dcc_viss_buf_map_id,
            (void **)&dcc_viss_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );
        memset(dcc_viss_buf, 0xAB, dcc_buff_size);

        dcc_status = appIssGetDCCBuffVISS(SENSOR_SONY_IMX390_UB953_D3, 0U, dcc_viss_buf, dcc_buff_size);
        ASSERT(dcc_status == 0);

        vxUnmapUserDataObject(dcc_param_viss, dcc_viss_buf_map_id);
        /* Done w/ DCC */

        tivx_vpac_viss_params_init(&params);
        params.sensor_dcc_id = 390;
        params.ee_mode = 0;
        params.mux_output0 = 0;
        params.mux_output1 = 0;
        params.mux_output2 = 4;
        params.mux_output3 = 0;
        params.mux_output4 = 3;
        params.h3a_in = 3;
        params.h3a_aewb_af_mode = 0;
        params.chroma_mode = 0;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, dcc_param_viss,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

        ct_read_raw_image(raw, "psdkra/app_single_cam/IMX390_001/input2.raw", 2);

        VX_CALL(vxVerifyGraph(graph));

        VX_CALL(vxProcessGraph(graph));

        snprintf(file, MAXPATHLENGTH, "%s/%s", ct_get_test_file_path(), "output/viss_dcc_out.yuv");
        write_output_image_nv12_8bit(file, y8_r8_c2);

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&y8_r8_c2));
        VX_CALL(tivxReleaseRawImage(&raw));
        VX_CALL(vxReleaseUserDataObject(&ae_awb_result));
        VX_CALL(vxReleaseUserDataObject(&configuration));
        VX_CALL(vxReleaseUserDataObject(&dcc_param_viss));

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
    int width, height, exposures, mux0, mux1, mux2, mux3, mux4;
    vx_bool line_interleaved;
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
    CT_GENERATE_PARAMETERS("randomInput", ADD_SIZE_64x48, ADD_EXP1, ADD_MUX0, ADD_MUX1, ADD_MUX2, ADD_MUX3, ADD_MUX4, ADD_LINE_FALSE, ARG), \

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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.ee_mode = 0;
        params.mux_output0 = arg_->mux0;
        params.mux_output1 = arg_->mux1;
        params.mux_output2 = arg_->mux2;
        params.mux_output3 = arg_->mux3;
        params.mux_output4 = arg_->mux4;
        params.h3a_aewb_af_mode = 0;
        params.chroma_mode = 0;
        params.bypass_glbce = 1;
        params.bypass_nsf4 = 1;
        params.h3a_in = 0;

        if (TIVX_VPAC_VISS_MUX2_YUV422 == arg_->mux2)
        {
            params.chroma_mode = 1;
        }

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        memset(&ae_awb_params, 0, sizeof(tivx_ae_awb_params_t));
        ASSERT_VX_OBJECT(ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t",
                                                            sizeof(tivx_ae_awb_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.ee_mode = 0;
        params.mux_output0 = arg_->mux0;
        params.mux_output1 = arg_->mux1;
        params.mux_output2 = arg_->mux2;
        params.mux_output3 = arg_->mux3;
        params.mux_output4 = arg_->mux4;
        params.h3a_aewb_af_mode = 0;
        params.chroma_mode = 0;
        params.bypass_glbce = 0;
        params.bypass_nsf4 = 0;

        VX_CALL(vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxVpacVissNode(graph, configuration, ae_awb_result, NULL,
                                                raw, y12, uv12_c1, y8_r8_c2, uv8_g8_c3, s8_b8_c4,
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

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

static void ct_read_raw_image(tivx_raw_image image, const char* fileName, uint16_t file_byte_pack)
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

            if(file_byte_pack == num_bytes)
            {
                int i;
                uint8_t *dst = data_ptr;
                uint8_t *src = (uint8_t*)buf;
                for(i = 0; i < height; i++)
                {
                    memcpy((void*)&dst[image_addr.stride_y*i], (void*)&src[width*num_bytes*i], width*num_bytes);
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
            uint32_t size;
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

TEST(tivxHwaVpacViss, testGraphProcessingRaw)
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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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
        memset(&params, 0, sizeof(tivx_vpac_viss_params_t));
        ASSERT_VX_OBJECT(configuration = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t",
                                                            sizeof(tivx_vpac_viss_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.ee_mode = 2;
        params.mux_output0 = 0;
        params.mux_output1 = 0;
        params.mux_output2 = 0;
        params.mux_output3 = 0;
        params.mux_output4 = 3;
        params.h3a_aewb_af_mode = 0;
        params.chroma_mode = 0;
        params.bypass_glbce = 0;
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
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxVerifyGraph(graph));

        ct_read_raw_image(raw, "tivx/bayer_1280x720.raw", 2);

        VX_CALL(vxProcessGraph(graph));

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

        /* For visual verification */
        save_image_from_viss(y8_r8_c2, "output/out_y8");

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
    CT_GENERATE_PARAMETERS("testNegative", ADD_NEGATIVE_TEST, ADD_NEGATIVE_CONDITION, ARG)

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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VPAC_VISS1))
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

        params.mux_output0 = 0U;
        params.mux_output1 = 0U;
        params.mux_output2 = 0U;
        params.mux_output3 = 0U;
        params.mux_output4 = 3U;
        params.bypass_glbce = 0U;
        params.bypass_nsf4 = 0U;
        params.h3a_in = 0U;
        params.h3a_aewb_af_mode = 0U;
        params.ee_mode = 0U;
        params.chroma_mode = 0U;

        switch (arg_->negative_test)
        {
            case 0:
            {
                if (0U == arg_->condition)
                {
                    params.mux_output0 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.mux_output0 = 4;
                }
                else if (2U == arg_->condition)
                {
                    params.mux_output0 = 5;
                }
                else if (3U == arg_->condition)
                {
                    params.mux_output0 = 5;
                }
                else
                {
                    params.mux_output0 = 2;
                }
                break;
            }
            case 1:
            {
                if (0U == arg_->condition)
                {
                    params.mux_output1 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.mux_output1 = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.mux_output1 = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.mux_output1 = 3;
                }
                else
                {
                    params.mux_output1 = 1;
                }
                break;
            }
            case 2:
            {
                if (0U == arg_->condition)
                {
                    params.mux_output2 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.mux_output2 = 5;
                }
                else if (2U == arg_->condition)
                {
                    params.mux_output2 = 6;
                }
                else if (3U == arg_->condition)
                {
                    params.mux_output2 = 6;
                }
                else
                {
                    params.mux_output2 = 6;
                }
                break;
            }
            case 3:
            {
                if (0U == arg_->condition)
                {
                    params.mux_output3 = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.mux_output3 = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.mux_output3 = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.mux_output3 = 3;
                }
                else
                {
                    params.mux_output3 = 3;
                }
                break;
            }
            case 4:
            {
                if (0U == arg_->condition)
                {
                    params.mux_output4 = 1;
                }
                else if (1U == arg_->condition)
                {
                    params.mux_output4 = 3;
                }
                else if (2U == arg_->condition)
                {
                    params.mux_output4 = 0;
                }
                else if (3U == arg_->condition)
                {
                    params.mux_output4 = 4;
                }
                else
                {
                    params.mux_output4 = 4;
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
                    params.h3a_in = 4;
                }
                else if (3U == arg_->condition)
                {
                    params.h3a_in = 4;
                }
                else
                {
                    params.h3a_in = 4;
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
                    params.ee_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.ee_mode = 2;
                }
                else if (2U == arg_->condition)
                {
                    params.ee_mode = 3;
                }
                else if (3U == arg_->condition)
                {
                    params.ee_mode = 3;
                }
                else
                {
                    params.ee_mode = 3;
                }
                break;
            }
            case 10:
            {
                if (0U == arg_->condition)
                {
                    params.chroma_mode = 0;
                }
                else if (1U == arg_->condition)
                {
                    params.chroma_mode = 1;
                }
                else if (2U == arg_->condition)
                {
                    params.chroma_mode = 2;
                }
                else if (3U == arg_->condition)
                {
                    params.chroma_mode = 2;
                }
                else
                {
                    params.chroma_mode = 2;
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
                    ae_awb_params.h3a_source_data = 4;
                }
                else if (3U == arg_->condition)
                {
                    ae_awb_params.h3a_source_data = 4;
                }
                else
                {
                    ae_awb_params.h3a_source_data = 4;
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
                                                h3a_aew_af, histogram), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_VPAC_VISS1));

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

#if defined(BUILD_CT_TIOVX_HWA_NEGATIVE_TESTS)
#define testMuxNegative testMuxNegative
#else
#define testMuxNegative DISABLED_testMuxNegative
#endif

TESTCASE_TESTS(tivxHwaVpacViss,
               testNodeCreation,
               testGraphProcessingFile,
               testGraphProcessingFileDcc,
               testNegativeGraph/*,
               testMuxNegative ,
               testMux,
               testGraphProcessing,
               testGraphProcessingRaw*/)
