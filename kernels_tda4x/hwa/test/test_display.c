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
#include <TI/tivx.h>
#include <TI/tda4x.h>
#include "test_engine/test.h"
#include <string.h>

#define DISPLAY_NUM_RUN_COUNT 100

extern const uint32_t gDispArray1[];
extern const uint32_t gDispArray2[];

TESTCASE(tivxHwaDisplay, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxHwaDisplay, testBufferCopyMode)
{
    vx_context context = context_->vx_context_;
    vx_image disp_image = 0;
    vx_imagepatch_addressing_t image_addr;
    tivx_display_params_t params;
    vx_user_data_object param_obj;
    vx_graph graph = 0;
    vx_node node = 0;
    uint32_t loop_count = 0;

    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY))
    {
        tivxHwaLoadKernels(context);

        ASSERT_VX_OBJECT(disp_image = vxCreateImage(context, 480, 360, VX_DF_IMAGE_RGBX), VX_TYPE_IMAGE);
        
        image_addr.dim_x = 480;
        image_addr.dim_y = 360;
        image_addr.stride_x = 4;
        image_addr.stride_y = 480*4;
        image_addr.scale_x = VX_SCALE_UNITY;
        image_addr.scale_y = VX_SCALE_UNITY;
        image_addr.step_x = 1;
        image_addr.step_y = 1;
        vx_rectangle_t rect;
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = 480;
        rect.end_y = 360;

        vxCopyImagePatch(disp_image,
                &rect,
                0,
                &image_addr,
                gDispArray1,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST
                );

        memset(&params, 0, sizeof(tivx_display_params_t));
        
        params.opMode=TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE;
        params.pipeId=2; /* TODO: Change to DSS_DISP_INST_VID2; */
        params.outWidth=480;
        params.outHeight=360;
        params.posX=800;
        params.posY=440;
        
        ASSERT_VX_OBJECT(param_obj = vxCreateUserDataObject(context, "tivx_display_params_t", sizeof(tivx_display_params_t), &params), VX_TYPE_USER_DATA_OBJECT);

        ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

        ASSERT_VX_OBJECT(node = tivxDisplayNode(graph, param_obj, disp_image), VX_TYPE_NODE);

        VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_DISPLAY));
        VX_CALL(vxVerifyGraph(graph));

        while(loop_count++<DISPLAY_NUM_RUN_COUNT)
        {
            VX_CALL(vxProcessGraph(graph));
            if((loop_count%2) == 1)
            {
                vxCopyImagePatch(disp_image,
                    &rect,
                    0,
                    &image_addr,
                    gDispArray2,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST
                    );
            }
            else
            {
                vxCopyImagePatch(disp_image,
                    &rect,
                    0,
                    &image_addr,
                    gDispArray1,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST
                    );
            }
        }

        VX_CALL(vxReleaseNode(&node));
        VX_CALL(vxReleaseGraph(&graph));
        VX_CALL(vxReleaseImage(&disp_image));
        VX_CALL(vxReleaseUserDataObject(&param_obj));

        ASSERT(node == 0);
        ASSERT(graph == 0);
        ASSERT(disp_image == 0);
        ASSERT(param_obj == 0);

        tivxHwaUnLoadKernels(context);
    }
}

TEST(tivxHwaDisplay, testZeroBufferCopyMode)
{
    vx_context context = context_->vx_context_;
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DISPLAY))
    {
        tivxHwaLoadKernels(context);

        /* Dummy Test */;

        tivxHwaUnLoadKernels(context);
    }
}

TESTCASE_TESTS(tivxHwaDisplay,
    testBufferCopyMode,
    testZeroBufferCopyMode
    )
