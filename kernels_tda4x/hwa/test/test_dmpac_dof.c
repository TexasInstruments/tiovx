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
#include <TI/tda4x.h>
#include "test_engine/test.h"
#include <string.h>
#include "tivx_utils_file_rd_wr.h"

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


TEST(tivxHwaDmpacDof, testGraphProcessing)
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

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DMPAC_DOF))
    {
        uint32_t width = 256, height = 128;
        uint32_t levels = 2, i;
        vx_enum format = VX_DF_IMAGE_U8;

        tivxHwaLoadKernels(context);

        memset(&params, 0, sizeof(tivx_dmpac_dof_params_t));
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL), (enum vx_type_e)VX_TYPE_USER_DATA_OBJECT);

        params.vertical_search_range[0] = 48;
        params.vertical_search_range[1] = 48;
        params.horizontal_search_range = 191;
        params.median_filter_enable = 1;
        params.motion_smoothness_factor = 24;
        params.motion_direction = 1; /* 1: forward direction */

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

        status = save_image_from_dof(flow_vector_out_img, confidence_img, "output/tivx_test_ofTestCase1");
        ASSERT(status==VX_SUCCESS);

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


TESTCASE_TESTS(tivxHwaDmpacDof, testGraphProcessing)

