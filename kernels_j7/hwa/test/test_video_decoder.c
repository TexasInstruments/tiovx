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

static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node,vx_uint32 node_parameter_index);
static void app_find_user_object_array_index(vx_user_data_object object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);
static void app_find_image_array_index(vx_image image_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx);

static void printPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName)
{
    printf("[ %c%c ] Execution time for %9d pixels (avg = %4.6f ms, min = %4.6f ms, max = %4.6f ms)\n",
        testName[0], testName[1],
        numPixels,
        performance.avg/1000000.0,
        performance.min/1000000.0,
        performance.max/1000000.0
        );
}

TESTCASE(tivxHwaVideoDecoder, CT_VXContext, ct_setup_vx_context, 0)

#define MAX_ABS_FILENAME   (1024u)
#define MAX_ITERATIONS     (100u)
#define MAX_NUM_BUF        (2u)

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

        ASSERT_VX_OBJECT(bitstream_obj = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * TIVX_VDEC_ALIGN(width) * TIVX_VDEC_ALIGN(height) * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

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

typedef struct stream_info {
    uint32_t width;
    uint32_t height;
    uint32_t num_iterations;
    char input_file[MAX_ABS_FILENAME - 32];
    char output_file[MAX_ABS_FILENAME - 16];
    vx_size bitstream_sizes[MAX_ITERATIONS];
} stream_info;

static char *read_config_file(char *cfg_file)
{
    FILE *fd;
    char *fcontent = NULL;
    int filesize = 0, ret = 0;

    fd = fopen(cfg_file, "r");
    if (NULL == fd)
    {
        VX_PRINT(VX_ZONE_ERROR, "%s: file not found!!!\n", cfg_file);
        return NULL;
    }

    ret = fseek(fd, 0, SEEK_END);
    if (0 != ret)
    {
        VX_PRINT(VX_ZONE_ERROR, "fseek failed to find EOF\n");
        goto error;
    }

    filesize = ftell(fd);
    if (0 > filesize)
    {
        VX_PRINT(VX_ZONE_ERROR, "ftell failed to get file size\n");
        goto error;
    }

    ret = fseek(fd, 0, SEEK_SET);
    if (0 != ret)
    {
        VX_PRINT(VX_ZONE_ERROR, "fseek failed to find start of file\n");
        goto error;
    }

    fcontent = (char*)ct_alloc_mem(filesize + 1);
    if (NULL == fcontent)
    {
        VX_PRINT(VX_ZONE_ERROR, "ct_alloc_mem failed to allocate buffer\n");
        goto error;
    }

    ret = fread(fcontent, sizeof(uint8_t), filesize, fd);

    if (filesize != ret)
    {
        VX_PRINT(VX_ZONE_ERROR, "fread failed to read config file. read=%d != %d",
                ret, filesize);
        ct_free_mem(fcontent);
        goto error;
    }

    fclose(fd);
    fd = NULL;

    fcontent[filesize] = '\0';

    return fcontent;
error:
    fclose(fd);
    return NULL;
}

static uint8_t parse_config_file(char *cfg_file, struct stream_info *info)
{
    char *fcontent = NULL, *substr = NULL, *str = NULL, *tok = NULL;
    int i, ret = 0;

    fcontent = read_config_file(cfg_file);
    if (NULL == fcontent)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to read config file 0\n");
        return 1;
    }

    substr = "width=";
    str = strstr(fcontent, substr);
    if (str)
    {
        info->width = strtol(str + strlen(substr), NULL, 10);
        if (0 == info->width)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
            goto error;
        }
    } else {
        VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
        goto error;
    }

    substr = "height=";
    str = strstr(fcontent, substr);
    if (str)
    {
        info->height = strtol(str + strlen(substr), NULL, 10);
        if (0 == info->height)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
            goto error;
        }
    } else {
        VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
        goto error;
    }

    substr = "input_file=";
    str = strstr(fcontent, substr);
    if (str)
    {
        ret = sscanf(str + strlen(substr), "%s", info->input_file);
        if (0 == ret || EOF == ret)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
            goto error;
        }
    } else {
        VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
        goto error;
    }

    substr = "output_file=";
    str = strstr(fcontent, substr);
    if (str)
    {
        ret = sscanf(str + strlen(substr), "%s", info->output_file);
        if (0 == ret || EOF == ret)
        {
            VX_PRINT(VX_ZONE_INFO, "Failed to find %s, using default.\n", substr);
            info->output_file[0] = '\0';
        }
    } else {
        VX_PRINT(VX_ZONE_INFO, "Failed to find %s, using default.\n", substr);
        info->output_file[0] = '\0';
    }

    substr = "num_iterations=";
    str = strstr(fcontent, substr);
    if (str)
    {
        info->num_iterations = strtol(str + strlen(substr), NULL, 10);
        if (0 == info->num_iterations)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
            goto error;
        }
        if (info->num_iterations > MAX_ITERATIONS)
        {
            VX_PRINT(VX_ZONE_ERROR, "num_iterations > MAX_ITERATIONS. Please increase MAX_ITERATIONS and rebuild.\n");
            goto error;
        }
    } else {
        VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
        goto error;
    }

    substr = "bitstream_sizes=";
    str = strstr(fcontent, substr);
    if (str)
    {
        /* Get substring between square brackets */
        strtok(str, "[");
        substr = strtok(NULL, "]");

        tok = strtok(substr, ", \n");
        for (i = 0; i < info->num_iterations; i++)
        {
            info->bitstream_sizes[i] = strtol(tok, NULL, 10);
            tok = strtok(NULL, ", \n");
            if (tok == NULL)
            {
                i++;
                break;
            }
        }
        if (i < info->num_iterations)
        {
            printf("Error: bitstream_sizes entries (%d) fewer than num_iterations=%d.\n", i, info->num_iterations);
            goto error;
        }
    } else {
        VX_PRINT(VX_ZONE_ERROR, "Failed to find %s\n", substr);
        goto error;
    }

    ct_free_mem(fcontent);
    return 0;
error:
    ct_free_mem(fcontent);
    return 1;
}

TEST(tivxHwaVideoDecoder, testSingleStreamProcessing)
{
    vx_context context = context_->vx_context_;
    tivx_video_decoder_params_t params;
    vx_user_data_object configuration_obj;
    uint8_t *bitstream;
    vx_map_id map_id;
    vx_user_data_object bitstream_obj[MAX_NUM_BUF] = {NULL};

    char cfg_file[MAX_ABS_FILENAME];
    char input_file[MAX_ABS_FILENAME];
    char output_file[MAX_ABS_FILENAME];
    struct stream_info info;

    vx_size seek[MAX_ITERATIONS];
    vx_image output_image[MAX_NUM_BUF] = {NULL};
    vx_graph graph = 0;
    vx_node node_decode = 0;
    vx_status status = VX_SUCCESS;
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
    uint32_t                 num_buf, pipeline_depth, buf_id;
    vx_int32                 array_idx = -1, img_array_idx = -1;
    vx_perf_t perf_ref;

#ifndef DUMP_DECODED_VIDEO_TO_FILE
    const uint32_t checksum_expected[MAX_ITERATIONS] =
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
        snprintf(cfg_file, MAX_ABS_FILENAME, "%s/tivx/video_decoder/dec_single_channel.cfg", ct_get_test_file_path());

        if (parse_config_file(cfg_file, &info))
        {
            VX_PRINT(VX_ZONE_ERROR, "%s: Failed to parse config file\n", cfg_file);
            ASSERT(0);
        }

        snprintf(input_file, MAX_ABS_FILENAME, "%s/tivx/video_decoder/%s", ct_get_test_file_path(), info.input_file);

        if (0 != strlen(info.output_file))
            snprintf(output_file, MAX_ABS_FILENAME, "%s/output/%s", ct_get_test_file_path(), info.output_file);
        else
            snprintf(output_file, MAX_ABS_FILENAME, "%s/output/decoder_output.yuv", ct_get_test_file_path());

        rect_y.start_x = 0;
        rect_y.start_y = 0;
        rect_y.end_x = info.width;
        rect_y.end_y = info.height;

        rect_uv.start_x = 0;
        rect_uv.start_y = 0;
        rect_uv.end_x = info.width;
        rect_uv.end_y = info.height / 2;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        seek[0] = 0;
        for(i = 1; i < 100; i++)
        {
            seek[i] = seek[i - 1] + info.bitstream_sizes[i - 1];
        }

        tivx_video_decoder_params_init(&params);
        ASSERT_VX_OBJECT(configuration_obj = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;

        VX_CALL(vxCopyUserDataObject(configuration_obj, 0, sizeof(tivx_video_decoder_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        num_buf = MAX_NUM_BUF;
        pipeline_depth = MAX_NUM_BUF;
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(bitstream_obj[buf_id] = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * TIVX_VDEC_ALIGN(info.width) * TIVX_VDEC_ALIGN(info.height) * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(output_image[buf_id]  = vxCreateImage(context, TIVX_VDEC_ALIGN(info.width), TIVX_VDEC_ALIGN(info.height), VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_decode = tivxVideoDecoderNode(graph,
                                           configuration_obj,
                                           bitstream_obj[0],
                                           output_image[0]), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_decode, VX_TARGET_STRING, TIVX_TARGET_VDEC1));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];

        int graph_parameter_num = 0;
        vx_int32 input_bitstream_graph_parameter_index;
        vx_int32 output_image_graph_parameter_index;


        add_graph_parameter_by_node_index(graph, node_decode, 1);
        input_bitstream_graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = num_buf;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&bitstream_obj[0];
        graph_parameter_num++;

        add_graph_parameter_by_node_index(graph, node_decode, 2);
        output_image_graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].graph_parameter_index = graph_parameter_num;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list_size = num_buf;
        graph_parameters_queue_params_list[graph_parameter_num].refs_list = (vx_reference*)&output_image[0];
        graph_parameter_num++;

        VX_CALL(vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            graph_parameter_num,
            graph_parameters_queue_params_list));

        tivxSetGraphPipelineDepth(graph, pipeline_depth);

        VX_CALL(vxVerifyGraph(graph));

        int32_t pipeline = -num_buf;
        int32_t enqueueCnt = 0;
        for (i = 0; i < info.num_iterations; i++)
        {
            if(pipeline < 0)
            {
                /* Enqueue output */
                vxGraphParameterEnqueueReadyRef(graph, output_image_graph_parameter_index, (vx_reference*)&output_image[enqueueCnt], 1);
                /* Fill the input buffer. */
                VX_CALL(vxMapUserDataObject(bitstream_obj[enqueueCnt], 0, info.bitstream_sizes[i], &map_id, (void*) &bitstream, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

                in_fp = fopen(input_file, "r");
                if (NULL != in_fp)
                {
                    seek_status = fseek(in_fp, seek[i], SEEK_SET);
                    if (0 == seek_status)
                    {
                        num_read = fread(bitstream, sizeof(uint8_t), info.bitstream_sizes[i], in_fp);
                        fclose(in_fp);
                        in_fp = NULL;
                        if (info.bitstream_sizes[i] != num_read)
                        {
                            VX_PRINT(VX_ZONE_INFO,"%s: Read less than expected!!!\n", input_file);
                            ASSERT(info.bitstream_sizes[i] == num_read);
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

                VX_CALL(vxUnmapUserDataObject(bitstream_obj[enqueueCnt], map_id));
                VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj[enqueueCnt],  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&(info.bitstream_sizes[i]), sizeof(vx_size)));

                /* Enqueue input - start execution */
                vxGraphParameterEnqueueReadyRef(graph, input_bitstream_graph_parameter_index, (vx_reference*)&bitstream_obj[enqueueCnt], 1);

                enqueueCnt++;
                enqueueCnt   = (enqueueCnt  >= num_buf)? 0 : enqueueCnt;
                pipeline++;
            }
            else if(pipeline >= 0)
            {
                vx_image out_image;
                vx_user_data_object in_bitstream;
                uint32_t num_refs;
                /* Dequeue & Save output */
                vxGraphParameterDequeueDoneRef(graph, output_image_graph_parameter_index, (vx_reference*)&out_image, 1, &num_refs);
                app_find_image_array_index(output_image,(vx_reference)out_image, num_buf, &img_array_idx);
                if(img_array_idx != -1)
                {
#ifndef DUMP_DECODED_VIDEO_TO_FILE
                    checksum_actual = tivx_utils_simple_image_checksum(output_image[img_array_idx], 0, rect_y);
                    ASSERT(checksum_expected[i] == checksum_actual);

                    rect_uv = rect_uv; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
                    VX_CALL(vxMapImagePatch(output_image[img_array_idx],
                                            &rect_y,
                                            0,
                                            &map_id_image_y,
                                            &image_addr_y,
                                            (void**) &data_ptr_y,
                                            VX_READ_ONLY,
                                            VX_MEMORY_TYPE_HOST,
                                            VX_NOGAP_X
                                            ));

                    VX_CALL(vxMapImagePatch(output_image[img_array_idx],
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
                            for(j = 0; j < info.height; j++)
                            {
                                num_read = fwrite(data_ptr_y + (j * image_addr_y.stride_y), sizeof(uint8_t), info.width, out_fp);
                            }
                            for(j = 0; j < (info.height / 2); j++)
                            {
                                num_read += fwrite(data_ptr_uv + (j * image_addr_uv.stride_y), sizeof(uint8_t), info.width, out_fp);
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

                    VX_CALL(vxUnmapImagePatch(output_image[img_array_idx], map_id_image_y));
                    VX_CALL(vxUnmapImagePatch(output_image[img_array_idx], map_id_image_uv));
#endif
                }
                /* Dequeue input */
                vxGraphParameterDequeueDoneRef(graph, input_bitstream_graph_parameter_index, (vx_reference*)&in_bitstream, 1, &num_refs);

                /* Enqueue output */
                vxGraphParameterEnqueueReadyRef(graph, output_image_graph_parameter_index, (vx_reference*)&out_image, 1);
                app_find_user_object_array_index(bitstream_obj, (vx_reference)in_bitstream, num_buf, &array_idx);
                if(array_idx != -1)
                {
                    /* Fill the input buffer. */
                    VX_CALL(vxMapUserDataObject(bitstream_obj[array_idx], 0, info.bitstream_sizes[i], &map_id, (void*) &bitstream, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
                    in_fp = fopen(input_file, "r");
                    if (NULL != in_fp)
                    {
                        seek_status = fseek(in_fp, seek[i], SEEK_SET);
                        if (0 == seek_status)
                        {
                            num_read = fread(bitstream, sizeof(uint8_t), info.bitstream_sizes[i], in_fp);
                            fclose(in_fp);
                            in_fp = NULL;
                            if (info.bitstream_sizes[i] != num_read)
                            {
                                VX_PRINT(VX_ZONE_INFO,"%s: Read less than expected!!!\n", input_file);
                                ASSERT(info.bitstream_sizes[i] == num_read);
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
                    VX_CALL(vxUnmapUserDataObject(bitstream_obj[array_idx], map_id));
                    VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj[array_idx],  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&(info.bitstream_sizes[i]), sizeof(vx_size)));
                }
                /* Enqueue input - start execution */
                vxGraphParameterEnqueueReadyRef(graph, input_bitstream_graph_parameter_index, (vx_reference*)&in_bitstream, 1);
            }
        }
        vxWaitGraph(graph);

        /* Pipe-Down */
        for(i = 0; i < num_buf; i++)
        {
            vx_image out_image;
            uint32_t num_refs;
            /* Dequeue & Save output */
            vxGraphParameterDequeueDoneRef(graph, output_image_graph_parameter_index, (vx_reference*)&out_image, 1, &num_refs);
            app_find_image_array_index(output_image,(vx_reference)out_image, num_buf, &img_array_idx);
            if(img_array_idx != -1)
            {
#ifndef DUMP_DECODED_VIDEO_TO_FILE
                checksum_actual = tivx_utils_simple_image_checksum(output_image[img_array_idx], 0, rect_y);
                ASSERT(checksum_expected[i] == checksum_actual);

                rect_uv = rect_uv; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
                VX_CALL(vxMapImagePatch(output_image[img_array_idx],
                                        &rect_y,
                                        0,
                                        &map_id_image_y,
                                        &image_addr_y,
                                        (void**) &data_ptr_y,
                                        VX_READ_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X
                                        ));

                VX_CALL(vxMapImagePatch(output_image[img_array_idx],
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
                        for(j = 0; j < info.height; j++)
                        {
                            num_read = fwrite(data_ptr_y + (j * image_addr_y.stride_y), sizeof(uint8_t), info.width, out_fp);
                        }
                        for(j = 0; j < (info.height / 2); j++)
                        {
                            num_read += fwrite(data_ptr_uv + (j * image_addr_uv.stride_y), sizeof(uint8_t), info.width, out_fp);
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

                VX_CALL(vxUnmapImagePatch(output_image[img_array_idx], map_id_image_y));
                VX_CALL(vxUnmapImagePatch(output_image[img_array_idx], map_id_image_uv));
#endif
            }
        }

        VX_CALL(vxQueryNode(node_decode, VX_NODE_PERFORMANCE, &perf_ref, sizeof(perf_ref)));
        printPerformance(perf_ref, TIVX_VDEC_ALIGN(info.width)*TIVX_VDEC_ALIGN(info.height), "Decoder Node");

        VX_CALL(vxReleaseNode(&node_decode));
        VX_CALL(vxReleaseGraph(&graph));
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            VX_CALL(vxReleaseImage(&output_image[buf_id]));
            VX_CALL(vxReleaseUserDataObject(&bitstream_obj[buf_id]));
        }
        VX_CALL(vxReleaseUserDataObject(&configuration_obj));

        ASSERT(node_decode == 0);
        ASSERT(graph == 0);
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            ASSERT(output_image[buf_id] == 0);
            ASSERT(bitstream_obj[buf_id] == 0);
        }
        ASSERT(configuration_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

typedef struct {
    const char* name;
    int mode;
} Arg_MultiStream;

#define DECODER_PARAMETERS \
    CT_GENERATE_PARAMETERS("serial", ARG, 0), \
    CT_GENERATE_PARAMETERS("parallel", ARG, 1)

TEST_WITH_ARG(tivxHwaVideoDecoder, testMultiStreamProcessing, Arg_MultiStream, DECODER_PARAMETERS)
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
    vx_user_data_object bitstream_obj_s[MAX_NUM_BUF];
    vx_user_data_object bitstream_obj_l[MAX_NUM_BUF];
    vx_size seek_s[MAX_ITERATIONS];
    vx_size seek_l[MAX_ITERATIONS];
    vx_image output_image_s[MAX_NUM_BUF] = {NULL};
    vx_image output_image_l[MAX_NUM_BUF] = {NULL};
    vx_graph graph = 0;
    vx_node node_decode_s = 0;
    vx_node node_decode_l = 0;
    vx_status status = VX_SUCCESS;
    char cfg_file[MAX_ABS_FILENAME];
    struct stream_info info_s;
    struct stream_info info_l;
    int iterations;
    char input_file_s[MAX_ABS_FILENAME];
    char input_file_l[MAX_ABS_FILENAME];
    FILE* in_fp = NULL;
    char output_file_s[MAX_ABS_FILENAME];
    char output_file_l[MAX_ABS_FILENAME];
    FILE* out_fp_s = NULL;
    FILE* out_fp_l = NULL;
    size_t num_read;
    int seek_status;
    vx_perf_t perf_ref;

#ifndef DUMP_DECODED_VIDEO_TO_FILE
    const uint32_t checksum_expected_s[MAX_ITERATIONS] =
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
    const uint32_t checksum_expected_l[MAX_ITERATIONS] =
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
    uint32_t num_buf;
    uint32_t pipeline_depth;
    uint32_t buf_id;
    vx_int32 array_idx_s = -1;
    vx_int32 array_idx_l = -1;
    vx_int32 img_array_idx_s = -1;
    vx_int32 img_array_idx_l = -1;

    uint32_t exe_time[MAX_ITERATIONS] = {0};
    uint64_t timestamp = 0;
    char *second_target = TIVX_TARGET_VDEC1;
    //uint32_t expected_time_median = 29000;

    if (arg_->mode == 1)
    {
        /* Run both VENC instances in parallel */
        second_target = TIVX_TARGET_VDEC2;
        //expected_time_median = 23000;
    }

    if ((vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VDEC1)) && (vx_true_e == tivxIsTargetEnabled(second_target)))
    {
        snprintf(cfg_file, MAX_ABS_FILENAME, "%s/tivx/video_decoder/dec_multi_channel_0.cfg", ct_get_test_file_path());

        if (parse_config_file(cfg_file, &info_s))
        {
            VX_PRINT(VX_ZONE_ERROR, "%s: Failed to parse config file 0\n", cfg_file);
            ASSERT(0);
        }

        snprintf(cfg_file, MAX_ABS_FILENAME, "%s/tivx/video_decoder/dec_multi_channel_1.cfg", ct_get_test_file_path());

        if (parse_config_file(cfg_file, &info_l))
        {
            VX_PRINT(VX_ZONE_ERROR, "%s: Failed to parse config file 1\n", cfg_file);
            ASSERT(0);
        }

        snprintf(input_file_s, MAX_ABS_FILENAME, "%s/tivx/video_decoder/%s", ct_get_test_file_path(), info_s.input_file);
        snprintf(input_file_l, MAX_ABS_FILENAME, "%s/tivx/video_decoder/%s", ct_get_test_file_path(), info_l.input_file);

        if (0 != strlen(info_s.output_file))
            snprintf(output_file_s, MAX_ABS_FILENAME, "%s/output/%s", ct_get_test_file_path(), info_s.output_file);
        else
            snprintf(output_file_s, MAX_ABS_FILENAME, "%s/output/decoder_output_0.yuv", ct_get_test_file_path());

        if (0 != strlen(info_l.output_file))
            snprintf(output_file_l, MAX_ABS_FILENAME, "%s/output/%s", ct_get_test_file_path(), info_l.output_file);
        else
            snprintf(output_file_l, MAX_ABS_FILENAME, "%s/output/decoder_output_1.yuv", ct_get_test_file_path());

        iterations = (info_s.num_iterations < info_l.num_iterations) ? info_s.num_iterations : info_l.num_iterations;

        rect_y_s.start_x = 0;
        rect_y_s.start_y = 0;
        rect_y_s.end_x = info_s.width;
        rect_y_s.end_y = info_s.height;

        rect_uv_s.start_x = 0;
        rect_uv_s.start_y = 0;
        rect_uv_s.end_x = info_s.width;
        rect_uv_s.end_y = info_s.height;

        rect_y_l.start_x = 0;
        rect_y_l.start_y = 0;
        rect_y_l.end_x = info_l.width;
        rect_y_l.end_y = info_l.height;

        rect_uv_l.start_x = 0;
        rect_uv_l.start_y = 0;
        rect_uv_l.end_x = info_l.width;
        rect_uv_l.end_y = info_l.height;

        tivxHwaLoadKernels(context);
        CT_RegisterForGarbageCollection(context, ct_teardown_hwa_kernels, CT_GC_OBJECT);

        seek_s[0] = 0;
        seek_l[0] = 0;
        for(i = 1; i < 100; i++)
        {
            seek_s[i] = seek_s[i - 1] + info_s.bitstream_sizes[i - 1];
            seek_l[i] = seek_l[i - 1] + info_l.bitstream_sizes[i - 1];
        }

        tivx_video_decoder_params_init(&params_s);
        tivx_video_decoder_params_init(&params_l);
        ASSERT_VX_OBJECT(configuration_obj_s = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(configuration_obj_l = vxCreateUserDataObject(context, "tivx_video_decoder_params_t", sizeof(tivx_video_decoder_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params_s.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;
        params_l.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;

        VX_CALL(vxCopyUserDataObject(configuration_obj_s, 0, sizeof(tivx_video_decoder_params_t), &params_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(configuration_obj_l, 0, sizeof(tivx_video_decoder_params_t), &params_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        num_buf = MAX_NUM_BUF;
        pipeline_depth = MAX_NUM_BUF;
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            ASSERT_VX_OBJECT(bitstream_obj_s[buf_id] = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * TIVX_VDEC_ALIGN(info_s.width) * TIVX_VDEC_ALIGN(info_s.height) * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(bitstream_obj_l[buf_id] = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * TIVX_VDEC_ALIGN(info_l.width) * TIVX_VDEC_ALIGN(info_l.height) * 3 / 2, NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
            ASSERT_VX_OBJECT(output_image_s[buf_id] = vxCreateImage(context, TIVX_VDEC_ALIGN(info_s.width), TIVX_VDEC_ALIGN(info_s.height), VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
            ASSERT_VX_OBJECT(output_image_l[buf_id] = vxCreateImage(context, TIVX_VDEC_ALIGN(info_l.width), TIVX_VDEC_ALIGN(info_l.height), VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        }
        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_decode_s = tivxVideoDecoderNode(graph,
                                           configuration_obj_s,
                                           bitstream_obj_s[0],
                                           output_image_s[0]), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node_decode_l = tivxVideoDecoderNode(graph,
                                           configuration_obj_l,
                                           bitstream_obj_l[0],
                                           output_image_l[0]), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_decode_s, VX_TARGET_STRING, TIVX_TARGET_VDEC1));
        VX_CALL(vxSetNodeTarget(node_decode_l, VX_TARGET_STRING, second_target));

        vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[4];

        int graph_parameter = 0;
        vx_int32 input_bitstream_graph_parameter_index_s;
        vx_int32 input_bitstream_graph_parameter_index_l;
        vx_int32 output_image_graph_parameter_index_s;
        vx_int32 output_image_graph_parameter_index_l;


        add_graph_parameter_by_node_index(graph, node_decode_s, 1);
        input_bitstream_graph_parameter_index_s = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].graph_parameter_index = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].refs_list_size = num_buf;
        graph_parameters_queue_params_list[graph_parameter].refs_list = (vx_reference*)&bitstream_obj_s[0];
        graph_parameter++;

        add_graph_parameter_by_node_index(graph, node_decode_l, 1);
        input_bitstream_graph_parameter_index_l = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].graph_parameter_index = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].refs_list_size = num_buf;
        graph_parameters_queue_params_list[graph_parameter].refs_list = (vx_reference*)&bitstream_obj_l[0];
        graph_parameter++;

        add_graph_parameter_by_node_index(graph, node_decode_s, 2);
        output_image_graph_parameter_index_s = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].graph_parameter_index = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].refs_list_size = num_buf;
        graph_parameters_queue_params_list[graph_parameter].refs_list = (vx_reference*)&output_image_s[0];
        graph_parameter++;

        add_graph_parameter_by_node_index(graph, node_decode_l, 2);
        output_image_graph_parameter_index_l = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].graph_parameter_index = graph_parameter;
        graph_parameters_queue_params_list[graph_parameter].refs_list_size = num_buf;
        graph_parameters_queue_params_list[graph_parameter].refs_list = (vx_reference*)&output_image_l[0];
        graph_parameter++;

        VX_CALL(vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            graph_parameter,
            graph_parameters_queue_params_list));

        tivxSetGraphPipelineDepth(graph, pipeline_depth);

        VX_CALL(vxVerifyGraph(graph));


        int32_t pipeline = -num_buf;
        int32_t enqueueCnt = 0;
        for (i = 0; i < iterations; i++)
        {
            if(pipeline < 0)
            {
                /* Enqueue outputs */
                vxGraphParameterEnqueueReadyRef(graph, output_image_graph_parameter_index_s, (vx_reference*)&output_image_s[enqueueCnt], 1);
                vxGraphParameterEnqueueReadyRef(graph, output_image_graph_parameter_index_l, (vx_reference*)&output_image_l[enqueueCnt], 1);
                /* Fill the input buffers. */
                VX_CALL(vxMapUserDataObject(bitstream_obj_s[enqueueCnt], 0, info_s.bitstream_sizes[i], &map_id_s, (void*) &bitstream_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
                VX_CALL(vxMapUserDataObject(bitstream_obj_l[enqueueCnt], 0, info_l.bitstream_sizes[i], &map_id_l, (void*) &bitstream_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

                in_fp = fopen(input_file_s, "r");
                if (NULL != in_fp)
                {
                    seek_status = fseek(in_fp, seek_s[i], SEEK_SET);
                    if (0 == seek_status)
                    {
                        num_read = fread(bitstream_s, sizeof(uint8_t), info_s.bitstream_sizes[i], in_fp);
                        fclose(in_fp);
                        in_fp = NULL;
                        if (info_s.bitstream_sizes[i] != num_read)
                        {
                            VX_PRINT(VX_ZONE_INFO,"%s: Read less than expected!!!\n", input_file_s);
                            ASSERT(info_s.bitstream_sizes[i] == num_read);
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
                        num_read = fread(bitstream_l, sizeof(uint8_t), info_l.bitstream_sizes[i], in_fp);
                        fclose(in_fp);
                        in_fp = NULL;
                        if (info_l.bitstream_sizes[i] != num_read)
                        {
                            VX_PRINT(VX_ZONE_INFO," %s: Read less than expected!!!\n", input_file_l);
                            ASSERT(info_l.bitstream_sizes[i] == num_read);
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

                VX_CALL(vxUnmapUserDataObject(bitstream_obj_s[enqueueCnt], map_id_s));
                VX_CALL(vxUnmapUserDataObject(bitstream_obj_l[enqueueCnt], map_id_l));
                VX_CALL(vxCopyUserDataObject(bitstream_obj_s[enqueueCnt], 0, sizeof(uint8_t) * info_s.bitstream_sizes[i], bitstream_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(vxCopyUserDataObject(bitstream_obj_l[enqueueCnt], 0, sizeof(uint8_t) * info_l.bitstream_sizes[i], bitstream_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj_s[enqueueCnt],  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&(info_s.bitstream_sizes[i]), sizeof(vx_size)));
                VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj_l[enqueueCnt],  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&(info_l.bitstream_sizes[i]), sizeof(vx_size)));
                /* Enqueue inputs - start execution */
                vxGraphParameterEnqueueReadyRef(graph, input_bitstream_graph_parameter_index_s, (vx_reference*)&bitstream_obj_s[enqueueCnt], 1);
                vxGraphParameterEnqueueReadyRef(graph, input_bitstream_graph_parameter_index_l, (vx_reference*)&bitstream_obj_l[enqueueCnt], 1);

                enqueueCnt++;
                enqueueCnt   = (enqueueCnt  >= num_buf)? 0 : enqueueCnt;
                pipeline++;
            }
            else if(pipeline >= 0)
            {
                vx_image out_image_s, out_image_l;
                vx_user_data_object in_bitstream_s, in_bitstream_l;
                uint32_t num_refs;
                /* Dequeue & Save outputs */
                vxGraphParameterDequeueDoneRef(graph, output_image_graph_parameter_index_s, (vx_reference*)&out_image_s, 1, &num_refs);
                vxGraphParameterDequeueDoneRef(graph, output_image_graph_parameter_index_l, (vx_reference*)&out_image_l, 1, &num_refs);
                app_find_image_array_index(output_image_s,(vx_reference)out_image_s, num_buf, &img_array_idx_s);
                app_find_image_array_index(output_image_l,(vx_reference)out_image_l, num_buf, &img_array_idx_l);

                if(img_array_idx_s != -1)
                {
#ifndef DUMP_DECODED_VIDEO_TO_FILE
                    checksum_actual_s = tivx_utils_simple_image_checksum(output_image_s, 0, rect_y_s);
                    ASSERT(checksum_expected_s[i] == checksum_actual_s);
                    rect_uv_s = rect_uv_s; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
                    VX_CALL(vxMapImagePatch(output_image_s[img_array_idx_s],
                                            &rect_y_s,
                                            0,
                                            &map_id_image_y_s,
                                            &image_addr_y_s,
                                            (void**) &data_ptr_y_s,
                                            VX_READ_ONLY,
                                            VX_MEMORY_TYPE_HOST,
                                            VX_NOGAP_X
                                            ));

                    VX_CALL(vxMapImagePatch(output_image_s[img_array_idx_s],
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
                            for(j = 0; j < info_s.height; j++)
                            {
                                num_read = fwrite(data_ptr_y_s + (j * image_addr_y_s.stride_y), sizeof(uint8_t), info_s.width, out_fp_s);
                            }
                            for(j = 0; j < (info_s.height / 2); j++)
                            {
                                num_read += fwrite(data_ptr_uv_s + (j * image_addr_uv_s.stride_y), sizeof(uint8_t), info_s.width, out_fp_s);
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

                    VX_CALL(vxUnmapImagePatch(output_image_s[img_array_idx_s], map_id_image_y_s));
                    VX_CALL(vxUnmapImagePatch(output_image_s[img_array_idx_s], map_id_image_uv_s));
#endif
                }
                if(img_array_idx_l != -1)
                {
#ifndef DUMP_DECODED_VIDEO_TO_FILE
                    checksum_actual_l = tivx_utils_simple_image_checksum(output_image_l, 0, rect_y_l);
                    ASSERT(checksum_expected_l[i] == checksum_actual_l);
                    rect_uv_l = rect_uv_l; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
                    VX_CALL(vxMapImagePatch(output_image_l[img_array_idx_l],
                                            &rect_y_l,
                                            0,
                                            &map_id_image_y_l,
                                            &image_addr_y_l,
                                            (void**) &data_ptr_y_l,
                                            VX_READ_ONLY,
                                            VX_MEMORY_TYPE_HOST,
                                            VX_NOGAP_X
                                            ));

                    VX_CALL(vxMapImagePatch(output_image_l[img_array_idx_l],
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
                            for(j = 0; j < info_l.height; j++)
                            {
                                num_read = fwrite(data_ptr_y_l + (j * image_addr_y_l.stride_y), sizeof(uint8_t), info_l.width, out_fp_l);
                            }
                            for(j = 0; j < (info_l.height / 2); j++)
                            {
                                num_read += fwrite(data_ptr_uv_l + (j * image_addr_uv_l.stride_y), sizeof(uint8_t), info_l.width, out_fp_l);
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

                    VX_CALL(vxUnmapImagePatch(output_image_l[img_array_idx_l], map_id_image_y_l));
                    VX_CALL(vxUnmapImagePatch(output_image_l[img_array_idx_l], map_id_image_uv_l));
#endif
                }
                /* Dequeue inputs. */
                vxGraphParameterDequeueDoneRef(graph, input_bitstream_graph_parameter_index_s, (vx_reference*)&in_bitstream_s, 1, &num_refs);
                vxGraphParameterDequeueDoneRef(graph, input_bitstream_graph_parameter_index_l, (vx_reference*)&in_bitstream_l, 1, &num_refs);

                /* Enqueue outputs. */
                vxGraphParameterEnqueueReadyRef(graph, output_image_graph_parameter_index_s, (vx_reference*)&out_image_s, 1);
                vxGraphParameterEnqueueReadyRef(graph, output_image_graph_parameter_index_l, (vx_reference*)&out_image_l, 1);

                app_find_user_object_array_index(bitstream_obj_s, (vx_reference)in_bitstream_s, num_buf, &array_idx_s);
                app_find_user_object_array_index(bitstream_obj_l, (vx_reference)in_bitstream_l, num_buf, &array_idx_l);
                if(array_idx_s != -1)
                {
                    /* Fill the input buffer. */
                    VX_CALL(vxMapUserDataObject(bitstream_obj_s[enqueueCnt], 0, info_s.bitstream_sizes[i], &map_id_s, (void*) &bitstream_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

                    in_fp = fopen(input_file_s, "r");
                    if (NULL != in_fp)
                    {
                        seek_status = fseek(in_fp, seek_s[i], SEEK_SET);
                        if (0 == seek_status)
                        {
                            num_read = fread(bitstream_s, sizeof(uint8_t), info_s.bitstream_sizes[i], in_fp);
                            fclose(in_fp);
                            in_fp = NULL;
                            if (info_s.bitstream_sizes[i] != num_read)
                            {
                                VX_PRINT(VX_ZONE_INFO,"%s: Read less than expected!!!\n", input_file_s);
                                ASSERT(info_s.bitstream_sizes[i] == num_read);
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
                    VX_CALL(vxUnmapUserDataObject(bitstream_obj_s[enqueueCnt], map_id_s));
                    VX_CALL(vxCopyUserDataObject(bitstream_obj_s[enqueueCnt], 0, sizeof(uint8_t) * info_s.bitstream_sizes[i], bitstream_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                    VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj_s[enqueueCnt],  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&(info_s.bitstream_sizes[i]), sizeof(vx_size)));
                }
                if(array_idx_l != -1)
                {
                    /* Fill the input buffer. */
                    VX_CALL(vxMapUserDataObject(bitstream_obj_l[enqueueCnt], 0, info_l.bitstream_sizes[i], &map_id_l, (void*) &bitstream_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

                    in_fp = fopen(input_file_l, "r");
                    if (NULL != in_fp)
                    {
                        seek_status = fseek(in_fp, seek_l[i], SEEK_SET);
                        if (0 == seek_status)
                        {
                            num_read = fread(bitstream_l, sizeof(uint8_t), info_l.bitstream_sizes[i], in_fp);
                            fclose(in_fp);
                            in_fp = NULL;
                            if (info_l.bitstream_sizes[i] != num_read)
                            {
                                VX_PRINT(VX_ZONE_INFO," %s: Read less than expected!!!\n", input_file_l);
                                ASSERT(info_l.bitstream_sizes[i] == num_read);
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
                    VX_CALL(vxUnmapUserDataObject(bitstream_obj_l[enqueueCnt], map_id_l));
                    VX_CALL(vxCopyUserDataObject(bitstream_obj_l[enqueueCnt], 0, sizeof(uint8_t) * info_l.bitstream_sizes[i], bitstream_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
                    VX_CALL(tivxSetUserDataObjectAttribute(bitstream_obj_l[enqueueCnt],  TIVX_USER_DATA_OBJECT_VALID_SIZE, (void*)&(info_l.bitstream_sizes[i]), sizeof(vx_size)));
                }
                /* Enqueue inputs - start execution. */
                vxGraphParameterEnqueueReadyRef(graph, input_bitstream_graph_parameter_index_s, (vx_reference*)&in_bitstream_s, 1);
                vxGraphParameterEnqueueReadyRef(graph, input_bitstream_graph_parameter_index_l, (vx_reference*)&in_bitstream_l, 1);
            }
        }
        vxWaitGraph(graph);

        /* Pipe-Down */
        for(i = 0; i < num_buf; i++)
        {
            vx_image out_image_s, out_image_l;
            uint32_t num_refs;
            /* Dequeue & Save outputs */
            vxGraphParameterDequeueDoneRef(graph, output_image_graph_parameter_index_s, (vx_reference*)&out_image_s, 1, &num_refs);
            vxGraphParameterDequeueDoneRef(graph, output_image_graph_parameter_index_l, (vx_reference*)&out_image_l, 1, &num_refs);
            app_find_image_array_index(output_image_s,(vx_reference)out_image_s, num_buf, &img_array_idx_s);
            app_find_image_array_index(output_image_l,(vx_reference)out_image_l, num_buf, &img_array_idx_l);

            if(img_array_idx_s != -1)
            {
#ifndef DUMP_DECODED_VIDEO_TO_FILE
                checksum_actual_s = tivx_utils_simple_image_checksum(output_image_s, 0, rect_y_s);
                ASSERT(checksum_expected_s[i] == checksum_actual_s);
                rect_uv_s = rect_uv_s; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
                VX_CALL(vxMapImagePatch(output_image_s[img_array_idx_s],
                                        &rect_y_s,
                                        0,
                                        &map_id_image_y_s,
                                        &image_addr_y_s,
                                        (void**) &data_ptr_y_s,
                                        VX_READ_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X
                                        ));

                VX_CALL(vxMapImagePatch(output_image_s[img_array_idx_s],
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
                        for(j = 0; j < info_s.height; j++)
                        {
                            num_read = fwrite(data_ptr_y_s + (j * image_addr_y_s.stride_y), sizeof(uint8_t), info_s.width, out_fp_s);
                        }
                        for(j = 0; j < (info_s.height / 2); j++)
                        {
                            num_read += fwrite(data_ptr_uv_s + (j * image_addr_uv_s.stride_y), sizeof(uint8_t), info_s.width, out_fp_s);
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

                VX_CALL(vxUnmapImagePatch(output_image_s[img_array_idx_s], map_id_image_y_s));
                VX_CALL(vxUnmapImagePatch(output_image_s[img_array_idx_s], map_id_image_uv_s));
#endif
            }
            if(img_array_idx_l != -1)
            {
#ifndef DUMP_DECODED_VIDEO_TO_FILE
                checksum_actual_l = tivx_utils_simple_image_checksum(output_image_l, 0, rect_y_l);
                ASSERT(checksum_expected_l[i] == checksum_actual_l);
                rect_uv_l = rect_uv_l; /* dummy instruction to avoid compiler error. will be fixed with updated checksum API which can check for UV plane as well */
#else
                VX_CALL(vxMapImagePatch(output_image_l[img_array_idx_l],
                                        &rect_y_l,
                                        0,
                                        &map_id_image_y_l,
                                        &image_addr_y_l,
                                        (void**) &data_ptr_y_l,
                                        VX_READ_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X
                                        ));

                VX_CALL(vxMapImagePatch(output_image_l[img_array_idx_l],
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
                        for(j = 0; j < info_l.height; j++)
                        {
                            num_read = fwrite(data_ptr_y_l + (j * image_addr_y_l.stride_y), sizeof(uint8_t), info_l.width, out_fp_l);
                        }
                        for(j = 0; j < (info_l.height / 2); j++)
                        {
                            num_read += fwrite(data_ptr_uv_l + (j * image_addr_uv_l.stride_y), sizeof(uint8_t), info_l.width, out_fp_l);
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

                VX_CALL(vxUnmapImagePatch(output_image_l[img_array_idx_l], map_id_image_y_l));
                VX_CALL(vxUnmapImagePatch(output_image_l[img_array_idx_l], map_id_image_uv_l));
#endif
            }
        }

#if 1
        VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_ref, sizeof(perf_ref)));
        printPerformance(perf_ref, TIVX_VDEC_ALIGN(info_s.width) * TIVX_VDEC_ALIGN(info_s.height) + TIVX_VDEC_ALIGN(info_l.width) * TIVX_VDEC_ALIGN(info_l.height), "Decoder Node");

#endif
#if 0
        ASSERT(exe_time[iterations-1] < (expected_time_median + 3000));
        ASSERT(exe_time[iterations-1] > (expected_time_median - 3000));
#endif
        VX_CALL(vxReleaseNode(&node_decode_l));
        VX_CALL(vxReleaseNode(&node_decode_s));
        VX_CALL(vxReleaseGraph(&graph));
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            VX_CALL(vxReleaseImage(&output_image_l[buf_id]));
            VX_CALL(vxReleaseImage(&output_image_s[buf_id]));
            VX_CALL(vxReleaseUserDataObject(&bitstream_obj_l[buf_id]));
            VX_CALL(vxReleaseUserDataObject(&bitstream_obj_s[buf_id]));
        }
        VX_CALL(vxReleaseUserDataObject(&configuration_obj_l));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj_s));

        ASSERT(node_decode_l == 0);
        ASSERT(node_decode_s == 0);
        ASSERT(graph == 0);
        for(buf_id = 0; buf_id < num_buf; buf_id++)
        {
            ASSERT(output_image_l[buf_id] == 0);
            ASSERT(output_image_s[buf_id] == 0);
            ASSERT(bitstream_obj_l[buf_id] == 0);
            ASSERT(bitstream_obj_s[buf_id] == 0);
        }
        ASSERT(configuration_obj_l == 0);
        ASSERT(configuration_obj_s == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVideoDecoder, testNodeCreation, testSingleStreamProcessing, testMultiStreamProcessing)

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node,
    vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static void app_find_user_object_array_index(vx_user_data_object object_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx)
{
  vx_int32 i;

  *array_idx = -1;
  for(i = 0; i < array_size; i++)
  {
    if(ref == (vx_reference)object_array[i])
    {
      *array_idx = i;
      break;
    }
  }
}

static void app_find_image_array_index(vx_image image_array[], vx_reference ref, vx_int32 array_size, vx_int32 *array_idx)
{
  vx_int32 i;

  *array_idx = -1;
  for(i = 0; i < array_size; i++)
  {
    if(ref == (vx_reference)image_array[i])
    {
      *array_idx = i;
      break;
    }
  }
}
