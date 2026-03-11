/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#if defined(LINUX) || defined(QNX)

#include "TI/tivx.h"
#include "TI/tivx_test_kernels.h"
#include "tivx_test_kernels_kernels.h"
#include "tivx_kernel_multi_dsp_not_not.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include <stdio.h>

static tivx_target_kernel vx_mc_not_not_target_kernel[3] = {NULL};

typedef struct {

    vx_graph graph;
    vx_node node1[C7X_COUNT];
    vx_node node2[C7X_COUNT];
    vx_image image[3];
    vx_uint32 start[C7X_COUNT];
    vx_uint32 end[C7X_COUNT];
    vx_image striped_images[3][C7X_COUNT];

    void     *addr[2][TIVX_IMAGE_MAX_PLANES];
    uint32_t size[2][TIVX_IMAGE_MAX_PLANES];
    uint32_t num_entries[2];

} tivxNotNotKernelMCParams;

static const uint32_t max_entries = TIVX_IMAGE_MAX_PLANES;

static vx_status VX_CALLBACK tivxMultiDSPNotNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxMultiDSPNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxMultiDSPNotNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status copyImageHandles(vx_image src, vx_image dst);

static vx_status copyImageHandles(vx_image src, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    void           *addr[TIVX_IMAGE_MAX_PLANES] = {NULL};
    uint32_t        size[TIVX_IMAGE_MAX_PLANES];
    uint32_t        num_entries;

    status = tivxReferenceExportHandle((vx_reference)src,
                                         addr,
                                         size,
                                         max_entries,
                                         &num_entries);

    if (status==VX_SUCCESS)
    {
        status = tivxReferenceImportHandle((vx_reference)dst,
                                           (const void **)addr,
                                           (const uint32_t *)size,
                                           num_entries);
    }

    return status;
}

static vx_status VX_CALLBACK tivxMultiDSPNotNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    tivxNotNotKernelMCParams *prms = NULL;

    if ( (num_params != TIVX_KERNEL_MULTI_DSP_NOT_NOT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_MULTI_DSP_NOT_NOT_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_MULTI_DSP_NOT_NOT_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if (status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            sizeof(tivxNotNotKernelMCParams) != size)
        {
            status = VX_FAILURE;
        }
    }

    if (status==VX_SUCCESS)
    {
        status = copyImageHandles((vx_image)obj_desc[TIVX_KERNEL_MULTI_DSP_NOT_NOT_INPUT_IDX]->host_ref,
                                   prms->image[0]);
    }

    if (status==VX_SUCCESS)
    {
        status = copyImageHandles((vx_image)obj_desc[TIVX_KERNEL_MULTI_DSP_NOT_NOT_OUTPUT_IDX]->host_ref,
                                   prms->image[1]);
    }

    if (status==VX_SUCCESS)
    {
        status = vxProcessGraph(prms->graph);
    }

    return status;
}
static vx_status VX_CALLBACK tivxMultiDSPNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxNotNotKernelMCParams * prms = NULL;
        vx_context context;

        prms = (tivxNotNotKernelMCParams *)tivxMemAlloc(sizeof(tivxNotNotKernelMCParams), TIVX_MEM_EXTERNAL);

        if (prms == NULL)
        {
            status = VX_FAILURE;
        }

        if (status == VX_SUCCESS)
        {
            context = vxCreateContext();
            prms->graph = vxCreateGraph(context);
            status = vxGetStatus((vx_reference)prms->graph);
            if(status != VX_SUCCESS)
            {
                vxReleaseContext(&context);
            }
        }

        if (status == VX_SUCCESS)
        {
            prms->image[0] = (vx_image)(obj_desc[TIVX_KERNEL_MULTI_DSP_NOT_NOT_INPUT_IDX]->host_ref);
            prms->image[1] = (vx_image)(obj_desc[TIVX_KERNEL_MULTI_DSP_NOT_NOT_OUTPUT_IDX]->host_ref);

            if (status == VX_SUCCESS)
            {
                for (i = 0; i < 2; i++)
                {
                    prms->image[i] = (vx_image)tivxCreateReferenceFromExemplar(context, (vx_reference)prms->image[i]);
                }
            }
            /*
             * Note: The 3rd image is the intermediate image
             */
            prms->image[2] = (vx_image)tivxCreateReferenceFromExemplar(context, (vx_reference)prms->image[0]);
            vxReleaseContext(&context);


            // Stripe Division here
            vx_int32 j;
            vx_rectangle_t roi;
            vx_uint32 width, height;

            vxQueryImage(prms->image[0], VX_IMAGE_WIDTH, &width, sizeof(width));
            vxQueryImage(prms->image[0], VX_IMAGE_HEIGHT, &height, sizeof(height));


            roi.start_x = 0;
            roi.end_x = width;

            // Calculate the stripe height dimension
            for (i = 0; i < C7X_COUNT; i++)
            {
                prms->end[i] = ((i + 1) * height) / C7X_COUNT;
                prms->start[i] = (i == 0) ? 0 : prms->end[i-1];
            }

            /* For each image, create an ROI image of size stripe. */
            for (i = 0; (VX_SUCCESS == status) && (i < C7X_COUNT); i++)
            {
                roi.start_y = prms->start[i];
                roi.end_y = prms->end[i];

                /* For each img */
                for (j = 0; j < 3; j++)
                {
                    prms->striped_images[j][i] = vxCreateImageFromROI(prms->image[j], &roi);
                }
                // Assign node params keeping in mind the intermediate image is out of order
                prms->node1[i] = tivxTestNotNode(
                                    prms->graph,
                                    prms->striped_images[0][i],
                                    prms->striped_images[2][i]
                                    );

                status = vxGetStatus((vx_reference)prms->node1[i]);

                prms->node2[i] = tivxTestNotNode(
                                    prms->graph,
                                    prms->striped_images[2][i],
                                    prms->striped_images[1][i]
                                    );

                status |= vxGetStatus((vx_reference)prms->node2[i]);
            }
        }

        if (status == VX_SUCCESS)
        {
            char target_name[C7X_COUNT][TIVX_TARGET_MAX_NAME] = {0};
            /* Generate the target string name
             * Currently hardcoded to match DSP_C7-X, or the resolution of the macro TIVX_TARGET_DSP_C7_X
             * Note: Only supports single digit X
             */
            for (i = 0; i < C7X_COUNT; i++)
            {
#if defined(SOC_J721S2)
                if (i == 1) {
                    snprintf(target_name[i], TIVX_TARGET_MAX_NAME, "%s", TIVX_TARGET_DSP1);
                    vxSetNodeTarget(prms->node1[i], VX_TARGET_STRING, target_name[i]);
                    vxSetNodeTarget(prms->node2[i], VX_TARGET_STRING, target_name[i]);
                    continue;
                }
#endif
                snprintf(target_name[i], sizeof(target_name[i]), "DSP_C7-%d", i+1);
                vxSetNodeTarget(prms->node1[i], VX_TARGET_STRING, target_name[i]);
                vxSetNodeTarget(prms->node2[i], VX_TARGET_STRING, target_name[i]);
            }
        }

        if (status == VX_SUCCESS)
        {
            status = vxVerifyGraph(prms->graph);
        }

        if (status == VX_SUCCESS)
        {
            for (i = 0; i < 2; i++)
            {
                status = tivxReferenceExportHandle((vx_reference)prms->image[i],
                                                     prms->addr[i],
                                                     prms->size[i],
                                                     max_entries,
                                                     &prms->num_entries[i]);
            }
        }

        if (status == VX_SUCCESS)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms, sizeof(tivxNotNotKernelMCParams));
            VX_PRINT_KERNEL(VX_ZONE_INFO, kernel, "Multi DSP Not Not Kernel created\n");
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxMultiDSPNotNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;

        tivxNotNotKernelMCParams * prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (status==VX_SUCCESS)
        {
            for (i=0; i<2; i++)
            {
                status = tivxReferenceImportHandle((vx_reference)prms->image[i],
                                                   (const void **)prms->addr[i],
                                                   (const uint32_t *) prms->size[i],
                                                   prms->num_entries[i]);
            }
        }

        if (status == VX_SUCCESS)
        {
            for (int i = 0; i < 3; i++)
            {
                status |= vxReleaseImage(&prms->image[i]);
                for (int j = 0; j < C7X_COUNT; j++)
                {
                    status |= vxReleaseImage(&prms->striped_images[i][j]);
                }
            }
            status |= vxReleaseGraph(&prms->graph);
        }

        tivxMemFree(prms, sizeof(tivxNotNotKernelMCParams), TIVX_MEM_EXTERNAL);

    }

    return status;
}


void tivxAddTargetKernelMultiDSPNotNot_arm(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if (tivxGetSelfCpuId() == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);

        vx_mc_not_not_target_kernel[0] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_MULTI_DSP_NOT_NOT_NAME,
                            target_name,
                            tivxMultiDSPNotNotProcess,
                            tivxMultiDSPNotNotCreate,
                            tivxMultiDSPNotNotDelete,
                            NULL,
                            NULL);

        strncpy(target_name, TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);

        vx_mc_not_not_target_kernel[1] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_MULTI_DSP_NOT_NOT_NAME,
                            target_name,
                            tivxMultiDSPNotNotProcess,
                            tivxMultiDSPNotNotCreate,
                            tivxMultiDSPNotNotDelete,
                            NULL,
                            NULL);

        strncpy(target_name, TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);

        vx_mc_not_not_target_kernel[2] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_MULTI_DSP_NOT_NOT_NAME,
                            target_name,
                            tivxMultiDSPNotNotProcess,
                            tivxMultiDSPNotNotCreate,
                            tivxMultiDSPNotNotDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelMultiDSPNotNot_arm(void)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    for (i = 0; i < 3; i++)
    {
        status = tivxRemoveTargetKernel(vx_mc_not_not_target_kernel[i]);
        if (status == VX_SUCCESS)
        {
            vx_mc_not_not_target_kernel[i] = NULL;
        }
    }
}

#endif
