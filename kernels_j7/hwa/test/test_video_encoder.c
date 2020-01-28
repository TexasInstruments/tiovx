/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

TESTCASE(tivxHwaVideoEncoder, CT_VXContext, ct_setup_vx_context, 0)

#define MAX_ABS_FILENAME   (1024u)
#define NUM_ITERATIONS     (100u)

#define NUM_FRAMES_IN_IPFILE (5u)

/*
* Utility API used to add a graph parameter from a node, node parameter index
*/
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

/*
* Utility API to set pipeline depth for a graph
*/
static vx_status set_graph_pipeline_depth(vx_graph graph, vx_uint32 pipeline_depth)
{
    return tivxSetGraphPipelineDepth(graph, pipeline_depth);
}

TEST(tivxHwaVideoEncoder, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    tivx_video_encoder_params_t params;
    vx_user_data_object configuration_obj;
    vx_image input_image = NULL;
    vx_user_data_object bitstream_obj;
    uint32_t width = 256;
    uint32_t height = 128;
    vx_graph graph = 0;
    vx_node node_encode = 0;
    uint32_t max_bitstream_size;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VENC1))
    {
        tivxHwaLoadKernels(context);

        tivx_video_encoder_params_init(&params);
        ASSERT_VX_OBJECT(configuration_obj = vxCreateUserDataObject(context, "tivx_video_encoder_params_t", sizeof(tivx_video_encoder_params_t), NULL),
                                                                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        max_bitstream_size = ((uint32_t)(width / 16)
                            * (uint32_t)(height / 16) * WORST_QP_SIZE)
                            + ((height >> 4) * CODED_BUFFER_INFO_SECTION_SIZE);

        ASSERT_VX_OBJECT(bitstream_obj = vxCreateUserDataObject(context, "video_bitstream", sizeof(uint8_t) * max_bitstream_size, NULL),
                                                                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        VX_CALL(vxCopyUserDataObject(configuration_obj, 0, sizeof(tivx_video_encoder_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node_encode = tivxVideoEncoderNode(graph,
                                                            configuration_obj,
                                                            input_image,
                                                            bitstream_obj), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node_encode, VX_TARGET_STRING, TIVX_TARGET_VENC1));

        VX_CALL(vxReleaseNode(&node_encode));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj));

        ASSERT(node_encode == 0);
        ASSERT(graph == 0);
        ASSERT(bitstream_obj == 0);
        ASSERT(input_image == 0);
        ASSERT(configuration_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVideoEncoder, testSingleStreamProcessing)
{
    vx_context context = context_->vx_context_;
    tivx_video_encoder_params_t params;
    vx_user_data_object configuration_obj;
    vx_image input_image = NULL;
    uint8_t *bitstream;
    vx_map_id map_id;
    vx_user_data_object bitstream_obj;
    uint32_t width = 1280;
    uint32_t height = 720;
    vx_graph graph = 0;
    vx_node node_encode = 0;
    vx_status status = VX_SUCCESS;
    char input_file[MAX_ABS_FILENAME];
    char output_file[MAX_ABS_FILENAME];
    FILE* out_fp = NULL;
    int seek_status;
    size_t num_read;
    vx_size bitstream_size;
    uint32_t i;
    uint32_t                   j;
    vx_rectangle_t             rect_y;
    vx_rectangle_t             rect_uv;
    int                        index = 0;
    vx_map_id                  map_id_image_y;
    vx_imagepatch_addressing_t image_addr_y;
    vx_map_id                  map_id_image_uv;
    vx_imagepatch_addressing_t image_addr_uv;
    uint8_t                  *data_ptr_y;
    uint8_t                  *data_ptr_uv;
    FILE* in_fp = NULL;
    uint32_t max_bitstream_size;
    vx_size seek[NUM_ITERATIONS];

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VENC1))
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

        seek[0] = 0;
        for(i = 1; i < NUM_FRAMES_IN_IPFILE; i++)
        {
            seek[i] = seek[i - 1] + ((width * height * 3) / 2 );
        }

        snprintf(output_file, MAX_ABS_FILENAME, "%s/output/encode_output.h264", ct_get_test_file_path());

        remove(output_file);

        tivx_video_encoder_params_init(&params);
        ASSERT_VX_OBJECT(configuration_obj = vxCreateUserDataObject(context, "tivx_video_encoder_params_t", sizeof(tivx_video_encoder_params_t), NULL),
                                                                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;

        VX_CALL(vxCopyUserDataObject(configuration_obj, 0, sizeof(tivx_video_encoder_params_t), &params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_image = vxCreateImage(context, width, height, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);

        max_bitstream_size = ((uint32_t)(width / 16)
                            * (uint32_t)(height / 16) * WORST_QP_SIZE)
                            + ((height >> 4) * CODED_BUFFER_INFO_SECTION_SIZE);


        ASSERT_VX_OBJECT(bitstream_obj = vxCreateUserDataObject(context, "tivx_video_bitstream_t", sizeof(uint8_t) * max_bitstream_size, NULL),
                                                                (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node_encode = tivxVideoEncoderNode(graph,
                                                            configuration_obj,
                                                            input_image,
                                                            bitstream_obj), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_encode, VX_TARGET_STRING, TIVX_TARGET_VENC1));
        VX_CALL(vxVerifyGraph(graph));

        for (i = 0; i < NUM_ITERATIONS; i++)
        {
            snprintf(input_file, MAX_ABS_FILENAME, "%s/tivx/video_encoder/720p_nv12_images_5num.yuv", ct_get_test_file_path());

            VX_CALL(vxMapImagePatch(input_image,
                                    &rect_y,
                                    0,
                                    &map_id_image_y,
                                    &image_addr_y,
                                    (void**) &data_ptr_y,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(input_image,
                                    &rect_uv,
                                    1,
                                    &map_id_image_uv,
                                    &image_addr_uv,
                                    (void**) &data_ptr_uv,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            in_fp = fopen(input_file, "r");
            num_read = 0;
            if (NULL != in_fp)
            {
                seek_status = fseek(in_fp, seek[i % NUM_FRAMES_IN_IPFILE], SEEK_SET);

                if (0 == seek_status)
                {
                    for(j = 0; j < (height ); j++)
                    {
                        num_read  += fread(data_ptr_y + (j * image_addr_y.stride_y), sizeof(uint8_t), width, in_fp);
                    }

                    for(j = 0; j < (height / 2); j++)
                    {
                        num_read  += fread(data_ptr_uv + (j * image_addr_uv.stride_y), sizeof(uint8_t), width, in_fp);
                    }

                }
                fclose(in_fp);
                in_fp = NULL;
                if (((width * height * 3) / 2)!= num_read)
                {
                    VX_PRINT(VX_ZONE_INFO, "%s: Read less than expected!!!\n", input_file);
                }
            }

            VX_CALL(vxUnmapImagePatch(input_image, map_id_image_y));
            VX_CALL(vxUnmapImagePatch(input_image, map_id_image_uv));

            VX_CALL(vxProcessGraph(graph));

            VX_CALL(vxQueryUserDataObject(bitstream_obj, TIVX_USER_DATA_OBJECT_VALID_SIZE, &bitstream_size, sizeof(vx_size)));

            VX_CALL(vxMapUserDataObject(bitstream_obj, 0, bitstream_size, &map_id, (void*) &bitstream, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

            out_fp = fopen(output_file, "ab");
            if (NULL != out_fp)
            {
                num_read = fwrite(bitstream, sizeof(uint8_t), bitstream_size, out_fp);
                fclose(out_fp);
                out_fp = NULL;
                if (bitstream_size != num_read)
                {
                    VX_PRINT(VX_ZONE_ERROR, "%s: Wrote less than expected (%d < %d)!!!\n", output_file, (uint32_t)num_read, (uint32_t)bitstream_size);
                    ASSERT(bitstream_size == num_read);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "%s: Output file not found!!!\n", output_file);
                ASSERT(NULL != out_fp);
            }

            VX_CALL(vxUnmapUserDataObject(bitstream_obj, map_id));
        }

        VX_CALL(vxReleaseNode(&node_encode));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj));
        VX_CALL(vxReleaseImage(&input_image));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj));

        ASSERT(node_encode == 0);
        ASSERT(graph == 0);
        ASSERT(bitstream_obj == 0);
        ASSERT(input_image == 0);
        ASSERT(configuration_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaVideoEncoder, testMultiStreamProcessing)
{
    vx_context context = context_->vx_context_;
    tivx_video_encoder_params_t params_s;
    tivx_video_encoder_params_t params_l;
    vx_user_data_object configuration_obj_s;
    vx_user_data_object configuration_obj_l;
    vx_image input_image_s = NULL;
    vx_image input_image_l = NULL;
    vx_image file_io_image_s = NULL;
    vx_image file_io_image_l = NULL;
    uint8_t *bitstream_s;
    uint8_t *bitstream_l;
    vx_map_id map_id_s;
    vx_map_id map_id_l;
    vx_user_data_object bitstream_obj_s;
    vx_user_data_object bitstream_obj_l;
    uint32_t width_s = 1280;
    uint32_t width_l = 1920;
    uint32_t height_s = 720;
    uint32_t height_l = 1080;
    vx_graph graph = 0;
    vx_node node_encode_s = 0;
    vx_node node_encode_l = 0;
    vx_status status = VX_SUCCESS;
    char input_file_s[MAX_ABS_FILENAME];
    char input_file_l[MAX_ABS_FILENAME];
    char output_file_s[MAX_ABS_FILENAME];
    char output_file_l[MAX_ABS_FILENAME];
    FILE* out_fp_s = NULL;
    FILE* out_fp_l = NULL;
    size_t num_read_s;
    size_t num_read_l;
    vx_size bitstream_size_s;
    vx_size bitstream_size_l;
    uint32_t i;

    uint32_t                   j;
    vx_rectangle_t             rect_s_y;
    vx_rectangle_t             rect_s_uv;
    vx_rectangle_t             rect_l_y;
    vx_rectangle_t             rect_l_uv;
    int                        index_s = 0;
    int                        index_l = 0;
    vx_map_id                  map_id_image_s_y;
    vx_map_id                  map_id_image_s_uv;
    vx_map_id                  map_id_image_l_y;
    vx_map_id                  map_id_image_l_uv;

    vx_imagepatch_addressing_t image_addr_s_y;
    vx_imagepatch_addressing_t image_addr_s_uv;
    vx_imagepatch_addressing_t image_addr_l_y;
    vx_imagepatch_addressing_t image_addr_l_uv;

    uint8_t                  *data_ptr_s_y;
    uint8_t                  *data_ptr_s_uv;
    uint8_t                  *data_ptr_l_y;
    uint8_t                  *data_ptr_l_uv;

    FILE* in_fp_s = NULL;
    FILE* in_fp_l = NULL;
    int seek_status;
    size_t num_read;
    uint32_t max_bitstream_size_s;
    uint32_t max_bitstream_size_l;
    vx_size seek_s[NUM_ITERATIONS];
    vx_size seek_l[NUM_ITERATIONS];

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_VENC1))
    {
        rect_s_y.start_x = 0;
        rect_s_y.start_y = 0;
        rect_s_y.end_x = width_s;
        rect_s_y.end_y = height_s;

        rect_s_uv.start_x = 0;
        rect_s_uv.start_y = 0;
        rect_s_uv.end_x = width_s;
        rect_s_uv.end_y = height_s/2;

        rect_l_y.start_x = 0;
        rect_l_y.start_y = 0;
        rect_l_y.end_x = width_l;
        rect_l_y.end_y = height_l;

        rect_l_uv.start_x = 0;
        rect_l_uv.start_y = 0;
        rect_l_uv.end_x = width_l;
        rect_l_uv.end_y = height_l/2;

        tivxHwaLoadKernels(context);


        seek_s[0] = 0;
        for(i = 1; i < NUM_FRAMES_IN_IPFILE; i++)
        {
            seek_s[i] = seek_s[i - 1] + ((width_s * height_s * 3) / 2 );
        }

        seek_l[0] = 0;
        for(i = 1; i < NUM_FRAMES_IN_IPFILE; i++)
        {
            seek_l[i] = seek_l[i - 1] + ((width_l * height_l * 3) / 2 );
        }

        snprintf(output_file_s, MAX_ABS_FILENAME, "%s/output/encode_output_s.h264", ct_get_test_file_path());
        snprintf(output_file_l, MAX_ABS_FILENAME, "%s/output/encode_output_l.h264", ct_get_test_file_path());

        remove(output_file_s);
        remove(output_file_l);

        tivx_video_encoder_params_init(&params_s);
        tivx_video_encoder_params_init(&params_l);
        ASSERT_VX_OBJECT(configuration_obj_s = vxCreateUserDataObject(context, "tivx_video_encoder_params_t", sizeof(tivx_video_encoder_params_t), NULL),
                                                                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(configuration_obj_l = vxCreateUserDataObject(context, "tivx_video_encoder_params_t", sizeof(tivx_video_encoder_params_t), NULL),
                                                                        (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params_s.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;
        params_l.bitstream_format = TIVX_BITSTREAM_FORMAT_H264;

        VX_CALL(vxCopyUserDataObject(configuration_obj_s, 0, sizeof(tivx_video_encoder_params_t), &params_s, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        VX_CALL(vxCopyUserDataObject(configuration_obj_l, 0, sizeof(tivx_video_encoder_params_t), &params_l, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

        ASSERT_VX_OBJECT(input_image_s = vxCreateImage(context, width_s, height_s, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(input_image_l = vxCreateImage(context, width_l, height_l, VX_DF_IMAGE_NV12), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(file_io_image_s = vxCreateImage(context, width_s, height_s, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(file_io_image_l = vxCreateImage(context, width_l, height_l, VX_DF_IMAGE_RGB), VX_TYPE_IMAGE);

        max_bitstream_size_s = ((uint32_t)(width_s / 16)
                                    * (uint32_t)(height_s / 16) * WORST_QP_SIZE)
                                    + ((height_s >> 4) * CODED_BUFFER_INFO_SECTION_SIZE);

        max_bitstream_size_l = ((uint32_t)(width_l / 16)
                                    * (uint32_t)(height_l / 16) * WORST_QP_SIZE)
                                    + ((height_l >> 4) * CODED_BUFFER_INFO_SECTION_SIZE);

        ASSERT_VX_OBJECT(bitstream_obj_s = vxCreateUserDataObject(context, "tivx_video_bitstream_t", sizeof(uint8_t) * max_bitstream_size_s, NULL),
                                                                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);
        ASSERT_VX_OBJECT(bitstream_obj_l = vxCreateUserDataObject(context, "tivx_video_bitstream_t", sizeof(uint8_t) * max_bitstream_size_l, NULL),
                                                                    (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(node_encode_s = tivxVideoEncoderNode(graph,
                                                                configuration_obj_s,
                                                                input_image_s,
                                                                bitstream_obj_s), VX_TYPE_NODE);
                                                                ASSERT_VX_OBJECT(node_encode_l = tivxVideoEncoderNode(graph,
                                                                configuration_obj_l,
                                                                input_image_l,
                                                                bitstream_obj_l), VX_TYPE_NODE);
        VX_CALL(vxSetNodeTarget(node_encode_s, VX_TARGET_STRING, TIVX_TARGET_VENC1));
        VX_CALL(vxSetNodeTarget(node_encode_l, VX_TARGET_STRING, TIVX_TARGET_VENC1));
        VX_CALL(vxVerifyGraph(graph));

        for (i = 0; i < NUM_ITERATIONS; i++)
        {
            snprintf(input_file_s, MAX_ABS_FILENAME, "%s/tivx/video_encoder/720p_nv12_images_5num.yuv", ct_get_test_file_path());
            snprintf(input_file_l, MAX_ABS_FILENAME, "%s/tivx/video_encoder/1080p_nv12_images_5num.yuv", ct_get_test_file_path());

            VX_CALL(vxMapImagePatch(input_image_s,
                                    &rect_s_y,
                                    0,
                                    &map_id_image_s_y,
                                    &image_addr_s_y,
                                    (void**) &data_ptr_s_y,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(input_image_s,
                                    &rect_s_uv,
                                    1,
                                    &map_id_image_s_uv,
                                    &image_addr_s_uv,
                                    (void**) &data_ptr_s_uv,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(input_image_l,
                                    &rect_l_y,
                                    0,
                                    &map_id_image_l_y,
                                    &image_addr_l_y,
                                    (void**) &data_ptr_l_y,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            VX_CALL(vxMapImagePatch(input_image_l,
                                    &rect_l_uv,
                                    1,
                                    &map_id_image_l_uv,
                                    &image_addr_l_uv,
                                    (void**) &data_ptr_l_uv,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    VX_NOGAP_X
                                    ));

            in_fp_s = fopen(input_file_s, "r");
            num_read = 0;
            if (NULL != in_fp_s)
            {
                seek_status = fseek(in_fp_s, seek_s[i % NUM_FRAMES_IN_IPFILE], SEEK_SET);

                if (0 == seek_status)
                {
                    for(j = 0; j < height_s; j++)
                    {
                        num_read  += fread(data_ptr_s_y + (j * image_addr_s_y.stride_y), sizeof(uint8_t), width_s, in_fp_s);
                    }
                    for(j = 0; j < height_s / 2; j++)
                    {
                    num_read  += fread(data_ptr_s_uv + (j * image_addr_s_uv.stride_y), sizeof(uint8_t), width_s, in_fp_s);
                    }
                }

                fclose(in_fp_s);
                in_fp_s = NULL;
                if (((width_s * height_s * 3) / 2)!= num_read)
                {
                    VX_PRINT(VX_ZONE_INFO, "%s: Read less than expected!!!\n", input_file_s);
                }
            }

            in_fp_l = fopen(input_file_l, "r");
            num_read = 0;
            if (NULL != in_fp_l)
            {
                seek_status = fseek(in_fp_l, seek_l[i % NUM_FRAMES_IN_IPFILE], SEEK_SET);

                if (0 == seek_status)
                {
                    for(j = 0; j < height_l; j++)
                    {
                        num_read  += fread(data_ptr_l_y + (j * image_addr_l_y.stride_y), sizeof(uint8_t), width_l, in_fp_l);
                    }

                    for(j = 0; j < (height_l / 2); j++)
                    {
                        num_read  += fread(data_ptr_l_uv + (j * image_addr_l_uv.stride_y), sizeof(uint8_t), width_l, in_fp_l);
                    }
                }

                fclose(in_fp_l);
                in_fp_l = NULL;
                if (((width_l * height_l * 3) / 2)!= num_read)
                {
                    VX_PRINT(VX_ZONE_INFO, "%s: Read less than expected!!!\n", input_file_l);
                }
            }

            VX_CALL(vxUnmapImagePatch(input_image_s, map_id_image_s_y));
            VX_CALL(vxUnmapImagePatch(input_image_s, map_id_image_s_uv));
            VX_CALL(vxUnmapImagePatch(input_image_l, map_id_image_l_y));
            VX_CALL(vxUnmapImagePatch(input_image_l, map_id_image_l_uv));

            VX_CALL(vxProcessGraph(graph));

            VX_CALL(vxQueryUserDataObject(bitstream_obj_s, TIVX_USER_DATA_OBJECT_VALID_SIZE, &bitstream_size_s, sizeof(vx_size)));
            VX_CALL(vxQueryUserDataObject(bitstream_obj_l, TIVX_USER_DATA_OBJECT_VALID_SIZE, &bitstream_size_l, sizeof(vx_size)));

            VX_CALL(vxMapUserDataObject(bitstream_obj_s, 0, bitstream_size_s, &map_id_s, (void*) &bitstream_s, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
            VX_CALL(vxMapUserDataObject(bitstream_obj_l, 0, bitstream_size_l, &map_id_l, (void*) &bitstream_l, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

            out_fp_s = fopen(output_file_s, "ab");
            if (NULL != out_fp_s)
            {
                num_read_s = fwrite(bitstream_s, sizeof(uint8_t), bitstream_size_s, out_fp_s);
                fclose(out_fp_s);
                out_fp_s = NULL;
                if (bitstream_size_s != num_read_s)
                {
                    VX_PRINT(VX_ZONE_ERROR, "%s: Wrote less than expected (%d < %d)!!!\n", output_file_s, (uint32_t)num_read_s, (uint32_t)bitstream_size_s);
                    ASSERT(bitstream_size_s == num_read_s);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "%s: Output file not found!!!\n", output_file_s);
                ASSERT(NULL != out_fp_s);
            }

            out_fp_l = fopen(output_file_l, "ab");
            if (NULL != out_fp_l)
            {
                num_read_l = fwrite(bitstream_l, sizeof(uint8_t), bitstream_size_l, out_fp_l);
                fclose(out_fp_l);
                out_fp_l = NULL;
                if (bitstream_size_l != num_read_l)
                {
                    VX_PRINT(VX_ZONE_ERROR, "%s: Wrote less than expected (%d < %d)!!!\n", output_file_l, (uint32_t)num_read_l, (uint32_t)bitstream_size_l);
                    ASSERT(bitstream_size_l == num_read_l);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "%s: Output file not found!!!\n", output_file_l);
                ASSERT(NULL != out_fp_l);
            }

            VX_CALL(vxUnmapUserDataObject(bitstream_obj_s, map_id_s));
            VX_CALL(vxUnmapUserDataObject(bitstream_obj_l, map_id_l));
        }

        VX_CALL(vxReleaseNode(&node_encode_l));
        VX_CALL(vxReleaseNode(&node_encode_s));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj_l));
        VX_CALL(vxReleaseUserDataObject(&bitstream_obj_s));
        VX_CALL(vxReleaseImage(&file_io_image_l));
        VX_CALL(vxReleaseImage(&file_io_image_s));
        VX_CALL(vxReleaseImage(&input_image_l));
        VX_CALL(vxReleaseImage(&input_image_s));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj_l));
        VX_CALL(vxReleaseUserDataObject(&configuration_obj_s));

        ASSERT(node_encode_l == 0);
        ASSERT(node_encode_s == 0);
        ASSERT(graph == 0);
        ASSERT(bitstream_obj_l == 0);
        ASSERT(bitstream_obj_s == 0);
        ASSERT(file_io_image_l == 0);
        ASSERT(file_io_image_s == 0);
        ASSERT(input_image_l == 0);
        ASSERT(input_image_s == 0);
        ASSERT(configuration_obj_l == 0);
        ASSERT(configuration_obj_s == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaVideoEncoder, testNodeCreation, testSingleStreamProcessing, testMultiStreamProcessing)

