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
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/j7.h>
#include "test_engine/test.h"
#include <string.h>
#include <stdio.h>
#include "tivx_utils_file_rd_wr.h"
#include "tivx_utils_checksum.h"
#include "test_hwa_common.h"

TESTCASE(tivxHwaVideoDecoder, CT_VXContext, ct_setup_vx_context, 0)

#define MAX_ABS_FILENAME   (1024u)
#define NUM_ITERATIONS     (100u)

#define DUMP_DECODED_VIDEO_TO_FILE

TEST(tivxHwaVideoDecoder, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    tivx_video_decoder_params_t params;
    vx_user_data_object configuration_obj;
    vx_user_data_object bitstream_obj;
    uint32_t width = 256;
    uint32_t height = 128;
    vx_image output_image = NULL;
    vx_graph graph = 0;
    vx_node node_decode = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VDEC1))
    {
        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        tivx_video_decoder_params_init(&params);
        ASSERT_VX_OBJECT(configuration_obj = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(bitstream_obj = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * width * height * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(configuration_obj, 0, sizeof(tivx_video_decoder_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_decode = tivxVideoDecoderNode(graph,
                                           configuration_obj,
                                           bitstream_obj,
                                           output_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node_decode, VX_TARGET_STRING, TIVX_TARGET_VDEC1));

        VX_CALL(vxReleaseNode(&node_decode));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj));

        ASSERT(node_decode == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(bitstream_obj == 0);
        ASSERT(configuration_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVideoDecoder, testSingleStreamProcessing)
{
    vx_context context = context_->vx_context_;
    tivx_video_decoder_params_t params;
    vx_user_data_object configuration_obj;
    uint8_t *bitstream;
    vx_map_id map_id;
    vx_user_data_object bitstream_obj;

    uint32_t width = 1280;
    uint32_t height = 720;
    vx_size bitstream_sizes[NUM_ITERATIONS] =
    {
        82993, 39524, 45091, 54504, 61170, 63748, 68745, 72731, 75608, 79709,
        83861, 84205, 90563, 89582, 89739, 93068, 91550, 91049, 94106, 92786,
        92651, 95588, 92252, 91914, 95318, 92290, 91719, 93320, 88900, 88733,
        91082, 86712, 86524, 89613, 88475, 88643, 90706, 88526, 88337, 88624,
        87462, 84908, 87462, 86257, 86886, 87548, 86529, 85777, 85991, 83117,
        84542, 87381, 83457, 83582, 85658, 81836, 84291, 88171, 82946, 86689,
        89300, 84837, 82122, 87507, 86454, 84334, 85784, 82419, 81170, 82763,
        81990, 80710, 83374, 82211, 84405, 88921, 84010, 83922, 86596, 86124,
        85306, 85711, 83999, 83530, 82458, 84406, 84058, 85849, 81413, 81070,
        82996, 80906, 80357, 82020, 80679, 80966, 82257, 81149, 81531, 83673
    };

    vx_size seek[NUM_ITERATIONS];
    vx_image output_image = NULL;
    vx_graph graph = 0;
    vx_node node_decode = 0;
    vx_status status = VX_SUCCESS;
    char input_file[MAX_ABS_FILENAME];
    char output_file[MAX_ABS_FILENAME];
    FILE* in_fp = NULL;
    FILE* out_fp = NULL;
    size_t num_read;
    int seek_status;

    vx_rectangle_t             rect_y;
    vx_rectangle_t             rect_uv;
    vx_map_id                  map_id_image_y;
    vx_imagepatch_addressing_t image_addr_y;
    vx_map_id                  map_id_image_uv;
    vx_imagepatch_addressing_t image_addr_uv;
    uint8_t                  *data_ptr_y;
    uint8_t                  *data_ptr_uv;
#ifndef DUMP_DECODED_VIDEO_TO_FILE
    const uint32_t checksum_expected[NUM_ITERATIONS] =
    {
        (uint32_t) 0x00000000, (uint32_t) 0x2bfbdee0, (uint32_t) 0xe3196ed0, (uint32_t) 0x9379a0c7, (uint32_t) 0xe333648e,
        (uint32_t) 0x6ed450c0, (uint32_t) 0x0d7c4a1b, (uint32_t) 0xe0066589, (uint32_t) 0x7e8c561d, (uint32_t) 0x7e650c88,
        (uint32_t) 0x325f31ea, (uint32_t) 0x7e3edc91, (uint32_t) 0x46673e46, (uint32_t) 0xfcf03832, (uint32_t) 0x5b04d299,
        (uint32_t) 0xa1f04851, (uint32_t) 0x08cae540, (uint32_t) 0x45645f97, (uint32_t) 0x71fc3bc7, (uint32_t) 0x3944c01e,
        (uint32_t) 0xa9678a1e, (uint32_t) 0x641d8854, (uint32_t) 0x176aecc9, (uint32_t) 0xa9d0d214, (uint32_t) 0xf94934a9,
        (uint32_t) 0x767b9ff5, (uint32_t) 0xe5516dc4, (uint32_t) 0xf87d1455, (uint32_t) 0x0a15b3d4, (uint32_t) 0x97d2e210,
        (uint32_t) 0xd3a9c07f, (uint32_t) 0x274f5dbc, (uint32_t) 0xb21571bb, (uint32_t) 0xcbe0cd91, (uint32_t) 0x6cf50261,
        (uint32_t) 0xd3b1b4f5, (uint32_t) 0x8c1b0692, (uint32_t) 0xa245d049, (uint32_t) 0xdeb37b6d, (uint32_t) 0x8776a9b9,
        (uint32_t) 0x20e095bc, (uint32_t) 0xb149cd5e, (uint32_t) 0x4a942ae1, (uint32_t) 0xbf49c02b, (uint32_t) 0x97a62841,
        (uint32_t) 0x77def22d, (uint32_t) 0xb674ed59, (uint32_t) 0xdf547cd4, (uint32_t) 0x386ec685, (uint32_t) 0xb20577b4,
        (uint32_t) 0xd4c8c817, (uint32_t) 0xe12bc569, (uint32_t) 0x02f62cb9, (uint32_t) 0xbc7ba9ee, (uint32_t) 0x62199b82,
        (uint32_t) 0x4e5908ff, (uint32_t) 0xe1fce9ae, (uint32_t) 0xe2b0ae67, (uint32_t) 0xeb88e296, (uint32_t) 0x80ac66f9,
        (uint32_t) 0x8b1ac922, (uint32_t) 0x3f18d8cd, (uint32_t) 0x4bf37a1f, (uint32_t) 0x104299a9, (uint32_t) 0x65365dad,
        (uint32_t) 0xc5575e25, (uint32_t) 0x16b73bfa, (uint32_t) 0xcf58ec91, (uint32_t) 0x93e03f66, (uint32_t) 0xcaa143c8,
        (uint32_t) 0xdb749858, (uint32_t) 0x67c796d9, (uint32_t) 0x34311efb, (uint32_t) 0x8d57e16a, (uint32_t) 0x9a4ebe56,
        (uint32_t) 0x0c859c32, (uint32_t) 0x534f7892, (uint32_t) 0x4f06cc87, (uint32_t) 0x0ffc032b, (uint32_t) 0x05df69d1,
        (uint32_t) 0xae218b7c, (uint32_t) 0xb3c6dca1, (uint32_t) 0x3483109f, (uint32_t) 0x8b6eeedf, (uint32_t) 0x7a835aea,
        (uint32_t) 0xf032354b, (uint32_t) 0x9bf54769, (uint32_t) 0x8fffca86, (uint32_t) 0xb1cf3af5, (uint32_t) 0x9c2377f5,
        (uint32_t) 0x81873955, (uint32_t) 0x9388a4d8, (uint32_t) 0x36b5c6fb, (uint32_t) 0x69de4b98, (uint32_t) 0xaced9db7,
        (uint32_t) 0xcb2b4286, (uint32_t) 0xc43a743f, (uint32_t) 0x46afcd48, (uint32_t) 0x6444183a, (uint32_t) 0x4b3f8aff
    };
    uint32_t checksum_actual;
#endif
    uint32_t i, j;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VDEC1))
    {
        rect_y.start_x = 0;
        rect_y.start_y = 0;
        rect_y.end_x = width;
        rect_y.end_y = height;

        rect_uv.start_x = 0;
        rect_uv.start_y = 0;
        rect_uv.end_x = width;
        rect_uv.end_y = (height * 1)/2;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        seek[0] = 0;
        for(i = 1; i < 100; i++)
        {
            seek[i] = seek[i - 1] + bitstream_sizes[i - 1];
        }

        tivx_video_decoder_params_init(&params);
        ASSERT_VX_OBJECT(configuration_obj = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(bitstream_obj = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * width * height * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;

        snprintf(input_file, MAX_ABS_FILENAME, "%s/tivx/video_decoder/1280x720_allIframe_CBR_20mbps_HIGHSPEED_HP_CABAC.264", ct_get_test_file_path());
        snprintf(output_file, MAX_ABS_FILENAME, "%s/output/1280x720_allIframe_CBR_20mbps_HIGHSPEED_HP_CABAC.yuv", ct_get_test_file_path());

        VX_CALL(vxCopyUserDataObject(configuration_obj, 0, sizeof(tivx_video_decoder_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(output_image = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_decode = tivxVideoDecoderNode(graph,
                                           configuration_obj,
                                           bitstream_obj,
                                           output_image), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_decode, VX_TARGET_STRING, TIVX_TARGET_VDEC1));

        VX_CALL(vxVerifyGraph(graph));

        for (i = 0; i < NUM_ITERATIONS; i++)
        {
            VX_CALL(vxMapUserDataObject(bitstream_obj, 0, bitstream_sizes[i], &map_id, (void*) &bitstream, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

            in_fp = fopen(input_file, "r");
            if (NULL != in_fp)
            {
                seek_status = fseek(in_fp, seek[i], SEEK_SET);
                if (0 == seek_status)
                {
                    num_read = fread(bitstream, sizeof(uint8_t), bitstream_sizes[i], in_fp);
                    fclose(in_fp);
                    in_fp = NULL;
                    if (bitstream_sizes[i] != num_read)
                    {
                        VX_PRINT(VX_ZONE_INFO,"%s: Read less than expected!!!\n", input_file);
                        ASSERT(bitstream_sizes[i] == num_read);
                    }
                }
               else
                {
                    fclose(in_fp);
                    in_fp = NULL;
                    VX_PRINT(VX_ZONE_ERROR,"%s: Seek failed!!!\n", input_file);
                    ASSERT(0 == seek_status);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"%s: Input file not found!!!\n", input_file);
                ASSERT(NULL != in_fp);
            }

            VX_CALL(vxUnmapUserDataObject(bitstream_obj, map_id));
            VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj,  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&bitstream_sizes[i], sizeof(vx_size)));

            VX_CALL(vxProcessGraph(graph));

#ifndef DUMP_DECODED_VIDEO_TO_FILE
            checksum_actual = tivx_utils_simple_image_checksum(output_image, 0, rect_y);
            ASSERT(checksum_expected[i] == checksum_actual);

            rect_uv = rect_uv; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
            VX_CALL(vxMapImagePatch(output_image,
                                    &rect_y,
                                    0,
                                    &map_id_image_y,
                                    &image_addr_y,
                                    (void**) &data_ptr_y,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(output_image,
                                    &rect_uv,
                                    1,
                                    &map_id_image_uv,
                                    &image_addr_uv,
                                    (void**) &data_ptr_uv,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            if (i < 5)
            {
                out_fp = fopen(output_file, "ab");
                if (NULL != out_fp)
                {
                    for(j = 0; j < (height ); j++)
                    {
                        num_read = fwrite(data_ptr_y + (j * image_addr_y.stride_y), sizeof(uint8_t), width, out_fp);
                    }
                    for(j = 0; j < (height / 2); j++)
                    {
                        num_read += fwrite(data_ptr_uv + (j * image_addr_uv.stride_y), sizeof(uint8_t), width, out_fp);
                    }

                    fclose(out_fp);
                    out_fp = NULL;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"%s: output file not found!!!\n", output_file);
                    ASSERT(NULL != out_fp);
                }
            }

            VX_CALL(vxUnmapImagePatch(output_image, map_id_image_y));
            VX_CALL(vxUnmapImagePatch(output_image, map_id_image_uv));
#endif

        }

        VX_CALL(vxReleaseNode(&node_decode));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output_image));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj));

        ASSERT(node_decode == 0);
        ASSERT(graph == 0);
        ASSERT(output_image == 0);
        ASSERT(bitstream_obj == 0);
        ASSERT(configuration_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVideoDecoder, testMultiStreamProcessing)
{
    vx_context context = context_->vx_context_;
    tivx_video_decoder_params_t params_s;
    tivx_video_decoder_params_t params_l;
    vx_user_data_object configuration_obj_s;
    vx_user_data_object configuration_obj_l;
    uint8_t *bitstream_s;
    uint8_t *bitstream_l;
    vx_map_id map_id_s;
    vx_map_id map_id_l;
    vx_user_data_object bitstream_obj_s;
    vx_user_data_object bitstream_obj_l;
    uint32_t width_s = 1280;
    uint32_t height_s = 720;
    uint32_t width_l = 1920;
    uint32_t height_l = 1080;
    vx_size bitstream_sizes_s[NUM_ITERATIONS] =
    {
        82993, 39524, 45091, 54504, 61170, 63748, 68745, 72731, 75608, 79709,
        83861, 84205, 90563, 89582, 89739, 93068, 91550, 91049, 94106, 92786,
        92651, 95588, 92252, 91914, 95318, 92290, 91719, 93320, 88900, 88733,
        91082, 86712, 86524, 89613, 88475, 88643, 90706, 88526, 88337, 88624,
        87462, 84908, 87462, 86257, 86886, 87548, 86529, 85777, 85991, 83117,
        84542, 87381, 83457, 83582, 85658, 81836, 84291, 88171, 82946, 86689,
        89300, 84837, 82122, 87507, 86454, 84334, 85784, 82419, 81170, 82763,
        81990, 80710, 83374, 82211, 84405, 88921, 84010, 83922, 86596, 86124,
        85306, 85711, 83999, 83530, 82458, 84406, 84058, 85849, 81413, 81070,
        82996, 80906, 80357, 82020, 80679, 80966, 82257, 81149, 81531, 83673
    };
    vx_size bitstream_sizes_l[NUM_ITERATIONS] =
    {

        82437, 74618, 81778, 71937, 68102, 68683, 68911, 65367, 66464, 65291,
        67193, 68828, 68452, 66693, 61739, 70680, 67804, 63271, 62826, 60666,
        58988, 56842, 54840, 58680, 53905, 59807, 55503, 58511, 57463, 58687,
        58489, 57178, 56055, 55856, 56100, 58440, 58487, 60590, 61904, 63392,
        65509, 62019, 68031, 64219, 64406, 64441, 62162, 63102, 61798, 61111,
        61536, 62029, 59767, 61261, 61034, 60390, 60929, 61243, 59143, 61027,
        59800, 60261, 60562, 61152, 59890, 60642, 61511, 62192, 61695, 61985,
        57433, 63286, 58342, 63865, 65264, 61688, 67141, 62821, 62345, 62591,
        62550, 63561, 62973, 64209, 63780, 62575, 60594, 62560, 63860, 60122,
        62093, 58748, 58381, 57974, 57720, 56947, 57262, 57572, 57868, 59016
    };
    vx_size seek_s[NUM_ITERATIONS];
    vx_size seek_l[NUM_ITERATIONS];
    vx_image output_image_s = NULL;
    vx_image output_image_l = NULL;
    vx_graph graph = 0;
    vx_node node_decode_s = 0;
    vx_node node_decode_l = 0;
    vx_status status = VX_SUCCESS;
    char input_file_s[MAX_ABS_FILENAME];
    char input_file_l[MAX_ABS_FILENAME];
    FILE* in_fp = NULL;
    char output_file_s[MAX_ABS_FILENAME];
    char output_file_l[MAX_ABS_FILENAME];
    FILE* out_fp_s = NULL;
    FILE* out_fp_l = NULL;
    size_t num_read;
    int seek_status;
#ifndef DUMP_DECODED_VIDEO_TO_FILE
    const uint32_t checksum_expected_s[NUM_ITERATIONS] =
    {
        (uint32_t) 0x00000000, (uint32_t) 0x2bfbdee0, (uint32_t) 0xe3196ed0, (uint32_t) 0x9379a0c7, (uint32_t) 0xe333648e,
        (uint32_t) 0x6ed450c0, (uint32_t) 0x0d7c4a1b, (uint32_t) 0xe0066589, (uint32_t) 0x7e8c561d, (uint32_t) 0x7e650c88,
        (uint32_t) 0x325f31ea, (uint32_t) 0x7e3edc91, (uint32_t) 0x46673e46, (uint32_t) 0xfcf03832, (uint32_t) 0x5b04d299,
        (uint32_t) 0xa1f04851, (uint32_t) 0x08cae540, (uint32_t) 0x45645f97, (uint32_t) 0x71fc3bc7, (uint32_t) 0x3944c01e,
        (uint32_t) 0xa9678a1e, (uint32_t) 0x641d8854, (uint32_t) 0x176aecc9, (uint32_t) 0xa9d0d214, (uint32_t) 0xf94934a9,
        (uint32_t) 0x767b9ff5, (uint32_t) 0xe5516dc4, (uint32_t) 0xf87d1455, (uint32_t) 0x0a15b3d4, (uint32_t) 0x97d2e210,
        (uint32_t) 0xd3a9c07f, (uint32_t) 0x274f5dbc, (uint32_t) 0xb21571bb, (uint32_t) 0xcbe0cd91, (uint32_t) 0x6cf50261,
        (uint32_t) 0xd3b1b4f5, (uint32_t) 0x8c1b0692, (uint32_t) 0xa245d049, (uint32_t) 0xdeb37b6d, (uint32_t) 0x8776a9b9,
        (uint32_t) 0x20e095bc, (uint32_t) 0xb149cd5e, (uint32_t) 0x4a942ae1, (uint32_t) 0xbf49c02b, (uint32_t) 0x97a62841,
        (uint32_t) 0x77def22d, (uint32_t) 0xb674ed59, (uint32_t) 0xdf547cd4, (uint32_t) 0x386ec685, (uint32_t) 0xb20577b4,
        (uint32_t) 0xd4c8c817, (uint32_t) 0xe12bc569, (uint32_t) 0x02f62cb9, (uint32_t) 0xbc7ba9ee, (uint32_t) 0x62199b82,
        (uint32_t) 0x4e5908ff, (uint32_t) 0xe1fce9ae, (uint32_t) 0xe2b0ae67, (uint32_t) 0xeb88e296, (uint32_t) 0x80ac66f9,
        (uint32_t) 0x8b1ac922, (uint32_t) 0x3f18d8cd, (uint32_t) 0x4bf37a1f, (uint32_t) 0x104299a9, (uint32_t) 0x65365dad,
        (uint32_t) 0xc5575e25, (uint32_t) 0x16b73bfa, (uint32_t) 0xcf58ec91, (uint32_t) 0x93e03f66, (uint32_t) 0xcaa143c8,
        (uint32_t) 0xdb749858, (uint32_t) 0x67c796d9, (uint32_t) 0x34311efb, (uint32_t) 0x8d57e16a, (uint32_t) 0x9a4ebe56,
        (uint32_t) 0x0c859c32, (uint32_t) 0x534f7892, (uint32_t) 0x4f06cc87, (uint32_t) 0x0ffc032b, (uint32_t) 0x05df69d1,
        (uint32_t) 0xae218b7c, (uint32_t) 0xb3c6dca1, (uint32_t) 0x3483109f, (uint32_t) 0x8b6eeedf, (uint32_t) 0x7a835aea,
        (uint32_t) 0xf032354b, (uint32_t) 0x9bf54769, (uint32_t) 0x8fffca86, (uint32_t) 0xb1cf3af5, (uint32_t) 0x9c2377f5,
        (uint32_t) 0x81873955, (uint32_t) 0x9388a4d8, (uint32_t) 0x36b5c6fb, (uint32_t) 0x69de4b98, (uint32_t) 0xaced9db7,
        (uint32_t) 0xcb2b4286, (uint32_t) 0xc43a743f, (uint32_t) 0x46afcd48, (uint32_t) 0x6444183a, (uint32_t) 0x4b3f8aff
    };
    const uint32_t checksum_expected_l[NUM_ITERATIONS] =
    {
        (uint32_t) 0x00000000, (uint32_t) 0xc69403ee, (uint32_t) 0x48304f53, (uint32_t) 0xa689da58, (uint32_t) 0x142cc7e8,
        (uint32_t) 0x0bb00265, (uint32_t) 0x1ca65968, (uint32_t) 0x14f8e97b, (uint32_t) 0xd8ed5ade, (uint32_t) 0x2968c214,
        (uint32_t) 0x177ebb3b, (uint32_t) 0xbe911160, (uint32_t) 0x6e7a5a5b, (uint32_t) 0x0e870dee, (uint32_t) 0x87723dbe,
        (uint32_t) 0xb247524b, (uint32_t) 0x91ebdfe9, (uint32_t) 0xb71b7b19, (uint32_t) 0x832e4d79, (uint32_t) 0xbb85e856,
        (uint32_t) 0x46ece964, (uint32_t) 0x18489694, (uint32_t) 0x7c31bb1c, (uint32_t) 0xe37f0d6f, (uint32_t) 0xa809f28f,
        (uint32_t) 0xa01ab621, (uint32_t) 0x3219c8ce, (uint32_t) 0x9a71e234, (uint32_t) 0x5e61b531, (uint32_t) 0x6ad0b747,
        (uint32_t) 0xd81df077, (uint32_t) 0x0e57b192, (uint32_t) 0xbefde0ba, (uint32_t) 0xb4f869ff, (uint32_t) 0x7b0e6984,
        (uint32_t) 0x8a9d3d47, (uint32_t) 0x0dfa7be2, (uint32_t) 0xc082ee38, (uint32_t) 0xacbbf9cb, (uint32_t) 0x8070daa6,
        (uint32_t) 0xfe353f34, (uint32_t) 0xbef72049, (uint32_t) 0xb9999d76, (uint32_t) 0x37a4501a, (uint32_t) 0xc5e2d9ab,
        (uint32_t) 0xe7f4eed1, (uint32_t) 0x1e23f43b, (uint32_t) 0x3143aa98, (uint32_t) 0xfcea2606, (uint32_t) 0xa0ab381e,
        (uint32_t) 0x7e6a428d, (uint32_t) 0xbd591530, (uint32_t) 0x9ab8f9c3, (uint32_t) 0x38028b19, (uint32_t) 0x7e5e0576,
        (uint32_t) 0x27cab807, (uint32_t) 0xa0594d72, (uint32_t) 0x7eb74321, (uint32_t) 0xc9fb99a6, (uint32_t) 0xb921e743,
        (uint32_t) 0x21a12254, (uint32_t) 0xf07ca1c5, (uint32_t) 0x77818990, (uint32_t) 0x893b2a37, (uint32_t) 0xe1f9a78f,
        (uint32_t) 0xf835b12a, (uint32_t) 0x5e5efbf0, (uint32_t) 0x5f648969, (uint32_t) 0xc5249f2f, (uint32_t) 0xde700e75,
        (uint32_t) 0x8efe94ef, (uint32_t) 0xaaaefae9, (uint32_t) 0x4099fd8c, (uint32_t) 0x9e82d475, (uint32_t) 0xb389062c,
        (uint32_t) 0x8d8728c3, (uint32_t) 0xcb75799c, (uint32_t) 0x9693fdce, (uint32_t) 0xa1dbd67c, (uint32_t) 0xd9ef6152,
        (uint32_t) 0xeb524a50, (uint32_t) 0xa8ac78ae, (uint32_t) 0xd01a8a01, (uint32_t) 0x1098752d, (uint32_t) 0xf77b2a0c,
        (uint32_t) 0x06290110, (uint32_t) 0x9eebb207, (uint32_t) 0x7b7011ae, (uint32_t) 0xa1224fbf, (uint32_t) 0x13137366,
        (uint32_t) 0xd033f6b4, (uint32_t) 0x8ed8552a, (uint32_t) 0x2f0eb228, (uint32_t) 0x46ec6ae1, (uint32_t) 0x3109240b,
        (uint32_t) 0x56b11b91, (uint32_t) 0xe777fcd6, (uint32_t) 0xe4c4575e, (uint32_t) 0x07cf5691, (uint32_t) 0xf5ab0eb1
    };
    uint32_t checksum_actual_s;
    uint32_t checksum_actual_l;
#endif
    vx_rectangle_t             rect_y_s;
    vx_rectangle_t             rect_uv_s;
    vx_map_id                  map_id_image_y_s;
    vx_imagepatch_addressing_t image_addr_y_s;
    vx_map_id                  map_id_image_uv_s;
    vx_imagepatch_addressing_t image_addr_uv_s;
    uint8_t                  *data_ptr_y_s;
    uint8_t                  *data_ptr_uv_s;
    vx_rectangle_t             rect_y_l;
    vx_rectangle_t             rect_uv_l;
    vx_map_id                  map_id_image_y_l;
    vx_imagepatch_addressing_t image_addr_y_l;
    vx_map_id                  map_id_image_uv_l;
    vx_imagepatch_addressing_t image_addr_uv_l;
    uint8_t                  *data_ptr_y_l;
    uint8_t                  *data_ptr_uv_l;

    uint32_t i, j;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VDEC1))
    {
        rect_y_s.start_x = 0;
        rect_y_s.start_y = 0;
        rect_y_s.end_x = width_s;
        rect_y_s.end_y = height_s;

        rect_uv_s.start_x = 0;
        rect_uv_s.start_y = 0;
        rect_uv_s.end_x = width_s;
        rect_uv_s.end_y = height_s;

        rect_y_l.start_x = 0;
        rect_y_l.start_y = 0;
        rect_y_l.end_x = width_l;
        rect_y_l.end_y = height_l;

        rect_uv_l.start_x = 0;
        rect_uv_l.start_y = 0;
        rect_uv_l.end_x = width_l;
        rect_uv_l.end_y = height_l;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        seek_s[0] = 0;
        seek_l[0] = 0;
        for(i = 1; i < 100; i++)
        {
            seek_s[i] = seek_s[i - 1] + bitstream_sizes_s[i - 1];
            seek_l[i] = seek_l[i - 1] + bitstream_sizes_l[i - 1];
        }

        tivx_video_decoder_params_init(&params_s);
        tivx_video_decoder_params_init(&params_l);
        ASSERT_VX_OBJECT(configuration_obj_s = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(configuration_obj_l = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(bitstream_obj_s = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * width_s * height_s * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(bitstream_obj_l = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * width_l * height_l * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params_s.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;
        params_l.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;

        snprintf(input_file_s, MAX_ABS_FILENAME, "%s/tivx/video_decoder/1280x720_allIframe_CBR_20mbps_HIGHSPEED_HP_CABAC.264", ct_get_test_file_path());
        snprintf(input_file_l, MAX_ABS_FILENAME, "%s/tivx/video_decoder/pedestrian_1920x1080_420sp_375F_25fps.264", ct_get_test_file_path());

        snprintf(output_file_s, MAX_ABS_FILENAME, "%s/output/1280x720_allIframe_CBR_20mbps_HIGHSPEED_HP_CABAC.yuv", ct_get_test_file_path());
        snprintf(output_file_l, MAX_ABS_FILENAME, "%s/output/pedestrian_1920x1080_420sp_375F_25fps.yuv", ct_get_test_file_path());

        VX_CALL(vxCopyUserDataObject(configuration_obj_s, 0, sizeof(tivx_video_decoder_params_t), &params_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(configuration_obj_l, 0, sizeof(tivx_video_decoder_params_t), &params_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(output_image_s = vxCreateImage(context, width_s, height_s, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(output_image_l = vxCreateImage(context, width_l, height_l, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_decode_s = tivxVideoDecoderNode(graph,
                                           configuration_obj_s,
                                           bitstream_obj_s,
                                           output_image_s), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node_decode_l = tivxVideoDecoderNode(graph,
                                           configuration_obj_l,
                                           bitstream_obj_l,
                                           output_image_l), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_decode_s, VX_TARGET_STRING, TIVX_TARGET_VDEC1));
        VX_CALL(vxSetNodeTarget(node_decode_l, VX_TARGET_STRING, TIVX_TARGET_VDEC1));

        VX_CALL(vxVerifyGraph(graph));

        for (i = 0; i < NUM_ITERATIONS; i++)
        {
            VX_CALL(vxMapUserDataObject(bitstream_obj_s, 0, bitstream_sizes_s[i], &map_id_s, (void*) &bitstream_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
            VX_CALL(vxMapUserDataObject(bitstream_obj_l, 0, bitstream_sizes_l[i], &map_id_l, (void*) &bitstream_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

            in_fp = fopen(input_file_s, "r");
            if (NULL != in_fp)
            {
                seek_status = fseek(in_fp, seek_s[i], SEEK_SET);
                if (0 == seek_status)
                {
                    num_read = fread(bitstream_s, sizeof(uint8_t), bitstream_sizes_s[i], in_fp);
                    fclose(in_fp);
                    in_fp = NULL;
                    if (bitstream_sizes_s[i] != num_read)
                    {
                        VX_PRINT(VX_ZONE_INFO,"%s: Read less than expected!!!\n", input_file_s);
                        ASSERT(bitstream_sizes_s[i] == num_read);
                    }
                }
               else
                {
                    fclose(in_fp);
                    in_fp = NULL;
                    VX_PRINT(VX_ZONE_ERROR,"%s: Seek failed!!!\n", input_file_s);
                    ASSERT(0 == seek_status);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"%s: Input file not found!!!\n", input_file_s);
                ASSERT(NULL != in_fp);
            }

            in_fp = fopen(input_file_l, "r");
            if (NULL != in_fp)
            {
                seek_status = fseek(in_fp, seek_l[i], SEEK_SET);
                if (0 == seek_status)
                {
                    num_read = fread(bitstream_l, sizeof(uint8_t), bitstream_sizes_l[i], in_fp);
                    fclose(in_fp);
                    in_fp = NULL;
                    if (bitstream_sizes_l[i] != num_read)
                    {
                        VX_PRINT(VX_ZONE_INFO," %s: Read less than expected!!!\n", input_file_l);
                        ASSERT(bitstream_sizes_l[i] == num_read);
                    }
                }
               else
                {
                    fclose(in_fp);
                    in_fp = NULL;
                    VX_PRINT(VX_ZONE_ERROR,"%s: Seek failed!!!\n", input_file_l);
                    ASSERT(0 == seek_status);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"%s: Input file not found!!!\n", input_file_l);
                ASSERT(NULL != in_fp);
            }

            VX_CALL(vxUnmapUserDataObject(bitstream_obj_s, map_id_s));
            VX_CALL(vxUnmapUserDataObject(bitstream_obj_l, map_id_l));
            VX_CALL(vxCopyUserDataObject(bitstream_obj_s, 0, sizeof(uint8_t) * bitstream_sizes_s[i], bitstream_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(vxCopyUserDataObject(bitstream_obj_l, 0, sizeof(uint8_t) * bitstream_sizes_l[i], bitstream_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
            VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj_s,  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&bitstream_sizes_s[i], sizeof(vx_size)));
            VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj_l,  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&bitstream_sizes_l[i], sizeof(vx_size)));

            VX_CALL(vxProcessGraph(graph));

#ifndef DUMP_DECODED_VIDEO_TO_FILE
            checksum_actual_s = tivx_utils_simple_image_checksum(output_image_s, 0, rect_y_s);
            checksum_actual_l = tivx_utils_simple_image_checksum(output_image_l, 0, rect_y_l);
            ASSERT(checksum_expected_s[i] == checksum_actual_s);
            ASSERT(checksum_expected_l[i] == checksum_actual_l);

            rect_uv_s = rect_uv_s; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
            rect_uv_l = rect_uv_l; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */

#else
            VX_CALL(vxMapImagePatch(output_image_s,
                                    &rect_y_s,
                                    0,
                                    &map_id_image_y_s,
                                    &image_addr_y_s,
                                    (void**) &data_ptr_y_s,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(output_image_s,
                                    &rect_uv_s,
                                    1,
                                    &map_id_image_uv_s,
                                    &image_addr_uv_s,
                                    (void**) &data_ptr_uv_s,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            if (i < 5)
            {
                out_fp_s = fopen(output_file_s, "ab");
                if (NULL != out_fp_s)
                {
                    for(j = 0; j < (height_s ); j++)
                    {
                        num_read = fwrite(data_ptr_y_s + (j * image_addr_y_s.stride_y), sizeof(uint8_t), width_s, out_fp_s);
                    }
                    for(j = 0; j < (height_s / 2); j++)
                    {
                        num_read += fwrite(data_ptr_uv_s + (j * image_addr_uv_s.stride_y), sizeof(uint8_t), width_s, out_fp_s);
                    }

                    fclose(out_fp_s);
                    out_fp_s = NULL;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"%s: output file not found!!!\n", output_file_s);
                    ASSERT(NULL != out_fp_s);
                }
            }

            VX_CALL(vxUnmapImagePatch(output_image_s, map_id_image_y_s));
            VX_CALL(vxUnmapImagePatch(output_image_s, map_id_image_uv_s));

            VX_CALL(vxMapImagePatch(output_image_l,
                                    &rect_y_l,
                                    0,
                                    &map_id_image_y_l,
                                    &image_addr_y_l,
                                    (void**) &data_ptr_y_l,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(output_image_l,
                                    &rect_uv_l,
                                    1,
                                    &map_id_image_uv_l,
                                    &image_addr_uv_l,
                                    (void**) &data_ptr_uv_l,
                                    VX_READ_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            if (i < 5)
            {
                out_fp_l = fopen(output_file_l, "ab");
                if (NULL != out_fp_l)
                {
                    for(j = 0; j < (height_l ); j++)
                    {
                        num_read = fwrite(data_ptr_y_l + (j * image_addr_y_l.stride_y), sizeof(uint8_t), width_l, out_fp_l);
                    }
                    for(j = 0; j < (height_l / 2); j++)
                    {
                        num_read += fwrite(data_ptr_uv_l + (j * image_addr_uv_l.stride_y), sizeof(uint8_t), width_l, out_fp_l);
                    }

                    fclose(out_fp_l);
                    out_fp_l = NULL;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"%s: output file not found!!!\n", output_file_l);
                    ASSERT(NULL != out_fp_l);
                }
            }

            VX_CALL(vxUnmapImagePatch(output_image_l, map_id_image_y_l));
            VX_CALL(vxUnmapImagePatch(output_image_l, map_id_image_uv_l));
#endif
        }

        VX_CALL(vxReleaseNode(&node_decode_l));
        VX_CALL(vxReleaseNode(&node_decode_s));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&output_image_l));
        VX_CALL(vxReleaseImage(&output_image_s));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj_l));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj_s));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj_l));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj_s));

        ASSERT(node_decode_l == 0);
        ASSERT(node_decode_s == 0);
        ASSERT(graph == 0);
        ASSERT(output_image_l == 0);
        ASSERT(output_image_s == 0);
        ASSERT(bitstream_obj_l == 0);
        ASSERT(bitstream_obj_s == 0);
        ASSERT(configuration_obj_l == 0);
        ASSERT(configuration_obj_s == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVideoDecoder, testNodeCreation, testSingleStreamProcessing, testMultiStreamProcessing)

