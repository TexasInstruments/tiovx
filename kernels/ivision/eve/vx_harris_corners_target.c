/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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



#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_kernel_harris_corners.h>
#include <TI/tivx_target_kernel.h>
#include <common/xdais_types.h> /* Adding to support vx_uint32/vx_int32 data types */
#include <ti/xdais/ires.h>
#include <tivx_alg_ivision_if.h>
#include <apps/harrisCornerDetection32/algo/inc/iHarrisCornerDetection32_ti.h>
#include <tivx_kernels_target_utils.h>

#define HARRIS_CORNER_MAX_BLOCK_SIZE        (32)

typedef struct
{
    IVISION_BufDesc     inBufDesc;
    IVISION_BufDesc    *inBufDescList[1u];
    IVISION_InBufs      inBufs;

    IVISION_BufDesc     outBufDesc;
    IVISION_BufDesc    *outBufDescList[1u];
    IVISION_OutBufs     outBufs;

    HARRIS_CORNER_DETECTION_32_TI_CreateParams  createParams;

    IVISION_InArgs      inArgs;
    HARRIS_CORNER_DETECTION_32_TI_outArgs   outArgs;

    HARRIS_CORNER_DETECTION_32_TI_ControlInParams ctlInParams;
    HARRIS_CORNER_DETECTION_32_TI_ControlOutParams ctlOutParams;

    vx_uint16          *output;
    vx_uint32           output_size;
    Void               *algHandle;
} tivxHarrisCornersParams;

static tivx_target_kernel vx_harris_corners_target_kernel = NULL;

static void tivxHarrisCFreeMem(tivxHarrisCornersParams *prms);

static vx_status VX_CALLBACK tivxKernelHarrisCProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxHarrisCornersParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_array_t *arr;
    uint8_t *src_addr;
    uint32_t size, num_corners;
    tivx_obj_desc_scalar_t *sc_nms_thr, *sc_cnt;
    vx_keypoint_t *kp;
    vx_rectangle_t rect;
    IALG_Cmd iv_cmd;

    if (num_params != TIVX_KERNEL_HARRISC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRISC_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HARRISC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_HARRISC_OUT_ARR_IDX];
        sc_nms_thr = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_NMS_THR_IDX];
        sc_cnt = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_OUT_SC_CNT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHarrisCornersParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *src_desc_target_ptr;
        void *arr_desc_target_ptr;

        src_desc_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_heap_region);
        arr_desc_target_ptr = tivxMemShared2TargetPtr(
            arr->mem_ptr.shared_ptr, arr->mem_ptr.mem_heap_region);

        tivxMemBufferMap(src_desc_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferMap(arr_desc_target_ptr, arr->mem_size,
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;
        src_addr = (uint8_t *)((uintptr_t)src_desc_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        /* Change the corner detection threshold first */
        iv_cmd = HARRIS_CORNER_DETECTION_SET_THRESHOLD;
        prms->ctlInParams.nmsThreshold = sc_nms_thr->data.s32;

        status = tivxAlgiVisionControl(prms->algHandle, iv_cmd,
            (const IALG_Params *)&prms->ctlInParams,
            (IALG_Params *)&prms->ctlOutParams);
        if (0 != status)
        {
            status = VX_FAILURE;
        }

        if (0 == status)
        {
            prms->inBufDesc.bufPlanes[0u].buf = src_addr;
            prms->outBufDesc.bufPlanes[0u].buf = prms->output;

            status = tivxAlgiVisionProcess(
                            prms->algHandle,
                            &prms->inBufs,
                            &prms->outBufs,
                            (IVISION_InArgs*)&prms->inArgs,
                            (IVISION_OutArgs *)&prms->outArgs);
        }
        if (status == VX_SUCCESS)
        {
            num_corners = prms->outArgs.numCorners;

            if (NULL != sc_cnt)
            {
                sc_cnt->data.u32 = num_corners;
            }

            /* Copy array and other parameters */
            if (num_corners > arr->capacity)
            {
                num_corners = arr->capacity;
            }
            arr->num_items = num_corners;

            kp = (vx_keypoint_t *)arr_desc_target_ptr;
            for (i = 0; i < num_corners; i ++)
            {
                kp->x = rect.start_x + (prms->output[2 * i] & 0xFFFFu);
                kp->y = rect.start_y + (prms->output[(2 * i) + 1u] & 0xFFFFu);
                kp->strength = 0;
                kp->scale = 0.0f;
                kp->orientation = 0.0f;
                kp->tracking_status = 1;
                kp->error = 0.0f;

                kp ++;
            }
        }

        tivxMemBufferUnmap(src_desc_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(arr_desc_target_ptr, arr->mem_size,
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i, border;
    tivx_obj_desc_image_t *img;
    tivxHarrisCornersParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_nms_thr, *sc_win_size, *sc_spr_meth;
    tivx_obj_desc_scalar_t *sc_scale_factor, *sc_q_shift, *sc_scr_meth;
    tivx_obj_desc_array_t *arr;

    if (num_params != TIVX_KERNEL_HARRISC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRISC_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[
            TIVX_KERNEL_HARRISC_OUT_ARR_IDX];
        sc_nms_thr = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_NMS_THR_IDX];
        sc_spr_meth = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_SUPPR_METHOD_IDX];
        sc_win_size = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_WIN_SIZE_IDX];
        sc_scale_factor = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_FACTOR_IDX];
        sc_q_shift = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_Q_SHIFT_IDX];
        sc_scr_meth = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_SCORE_METHOD_IDX];

        prms = tivxMemAlloc(sizeof(tivxHarrisCornersParams), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxHarrisCornersParams));

            prms->output_size = 2u * arr->capacity * sizeof(uint16_t);
            prms->output = tivxMemAlloc(prms->output_size, TIVX_MEM_EXTERNAL);

            if (NULL == prms->output)
            {
                status = VX_ERROR_NO_MEMORY;
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            border = GRAD_FILTER_SZ - 1U +
                (sc_win_size->data.u08 - 1U) + (sc_spr_meth->data.u08 - 1U);

            prms->createParams.visionParams.algParams.size =
                sizeof(HARRIS_CORNER_DETECTION_32_TI_CreateParams);
            prms->createParams.visionParams.cacheWriteBack = NULL;
            prms->createParams.imgFrameWidth = img->imagepatch_addr[0].dim_x -
                border;
            prms->createParams.imgFrameHeight = img->imagepatch_addr[0].dim_y -
                border;

            /* Image size must be multiple of 32 */
            prms->createParams.imgFrameWidth =
                (prms->createParams.imgFrameWidth /
                    HARRIS_CORNER_MAX_BLOCK_SIZE) *
                    HARRIS_CORNER_MAX_BLOCK_SIZE;
            prms->createParams.imgFrameHeight =
                (prms->createParams.imgFrameHeight /
                    HARRIS_CORNER_MAX_BLOCK_SIZE) *
                    HARRIS_CORNER_MAX_BLOCK_SIZE;

            prms->createParams.maxNumCorners = arr->capacity;
            prms->createParams.harrisScoreScalingFactor =
                sc_scale_factor->data.u32;
            prms->createParams.nmsThresh = sc_nms_thr->data.s32;
            prms->createParams.qShift = sc_q_shift->data.u08;
            prms->createParams.outputFormat =
                HARRIS_CORNER_DETECTION_32_TI_OUTPUT_FORMAT_LIST;

            if (0 == sc_scr_meth->data.u08)
            {
                prms->createParams.harrisScoreMethod =
                    HARRIS_CORNER_DETECTION_32_TI_HARRIS_SCORE_METHOD_A;
            }
            else
            {
                prms->createParams.harrisScoreMethod =
                    HARRIS_CORNER_DETECTION_32_TI_HARRIS_SCORE_METHOD_B;
            }

            switch (sc_win_size->data.u08)
            {
                default:
                case 3:
                    prms->createParams.harrisWindowSize =
                        HARRIS_CORNER_DETECTION_32_TI_HARRIS_WINDOW_3x3;
                    break;
                case 5:
                    prms->createParams.harrisWindowSize =
                        HARRIS_CORNER_DETECTION_32_TI_HARRIS_WINDOW_5x5;
                    break;
                case 7:
                    prms->createParams.harrisWindowSize =
                        HARRIS_CORNER_DETECTION_32_TI_HARRIS_WINDOW_7x7;
                    break;
            }
            switch (sc_spr_meth->data.u08)
            {
                default:
                case 3:
                    prms->createParams.suppressionMethod =
                        HARRIS_CORNER_DETECTION_32_TI_SUPPRESSION_METHOD_NMS3x3;
                    break;
                case 5:
                    prms->createParams.suppressionMethod =
                        HARRIS_CORNER_DETECTION_32_TI_SUPPRESSION_METHOD_NMS5x5;
                    break;
                case 7:
                    prms->createParams.suppressionMethod =
                        HARRIS_CORNER_DETECTION_32_TI_SUPPRESSION_METHOD_NMS7x7;
                    break;
            }

            /* Initialize Buffer Descriptors */
            prms->inBufs.size       = sizeof(prms->inBufs);
            prms->inBufs.numBufs    = 1u;
            prms->inBufs.bufDesc    = prms->inBufDescList;
            prms->inBufDescList[0u] = &prms->inBufDesc;

            prms->outBufs.size       = sizeof(prms->outBufs);
            prms->outBufs.numBufs    = 1u;
            prms->outBufDescList[0u] = &prms->outBufDesc;
            prms->outBufs.bufDesc    = prms->outBufDescList;

            memset(&prms->inArgs, 0x0, sizeof(prms->inArgs));

            prms->inArgs.subFrameInfo = 0u;
            prms->inArgs.size = sizeof(IVISION_InArgs);

            prms->inBufDesc.numPlanes = 1;
            prms->inBufDesc.bufPlanes[0u].frameROI.topLeft.x =
                border / 2u;
            prms->inBufDesc.bufPlanes[0u].frameROI.topLeft.y =
                border / 2u;
            prms->inBufDesc.bufPlanes[0u].frameROI.width =
                prms->createParams.imgFrameWidth;
            prms->inBufDesc.bufPlanes[0u].frameROI.height =
                prms->createParams.imgFrameHeight;

            prms->inBufDesc.bufPlanes[0u].width =
                img->imagepatch_addr[0u].stride_y;
            prms->inBufDesc.bufPlanes[0u].height =
                img->imagepatch_addr[0u].dim_y;

            prms->outBufDesc.numPlanes = 1u;
            prms->outBufDesc.bufPlanes[0u].frameROI.topLeft.x = 0u;
            prms->outBufDesc.bufPlanes[0u].frameROI.topLeft.y = 0u;
            prms->outBufDesc.bufPlanes[0u].width = 2u *
                arr->capacity * sizeof(uint16_t);
            prms->outBufDesc.bufPlanes[0u].height = 1u;
            prms->outBufDesc.bufPlanes[0u].frameROI.width = 2u *
                arr->capacity * sizeof(uint16_t);
            prms->outBufDesc.bufPlanes[0u].frameROI.height = 1u;

            prms->ctlInParams.algParams.size =
                sizeof(HARRIS_CORNER_DETECTION_32_TI_ControlInParams);
            prms->ctlOutParams.algParams.size =
                sizeof(HARRIS_CORNER_DETECTION_32_TI_ControlOutParams);

            prms->outArgs.iVisionOutArgs.size =
                sizeof(HARRIS_CORNER_DETECTION_32_TI_outArgs);
            prms->outArgs.numCorners = 0u;
            prms->outArgs.activeImgWidth = 0u;
            prms->outArgs.activeImgHeight = 0u;
            prms->outArgs.outputBlockWidth = 0u;
            prms->outArgs.outputBlockHeight = 0u;

            prms->algHandle = tivxAlgiVisionCreate(
                &HARRIS_CORNER_DETECTION_32_TI_VISION_FXNS,
                (IALG_Params *)(&prms->createParams));

            if (NULL == prms->algHandle)
            {
                status = VX_FAILURE;
            }
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHarrisCornersParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxHarrisCFreeMem(prms);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxHarrisCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_HARRISC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRISC_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHarrisCornersParams) == size))
        {
            if (prms->algHandle)
            {
                tivxAlgiVisionDelete(prms->algHandle);
            }
            tivxHarrisCFreeMem(prms);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelHarrisCorners()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_EVE1) || (self_cpu == TIVX_CPU_ID_EVE2) ||
        (self_cpu == TIVX_CPU_ID_EVE3) || (self_cpu == TIVX_CPU_ID_EVE4))
    {
        if (self_cpu == TIVX_CPU_ID_EVE1)
        {
            strncpy(target_name, TIVX_TARGET_EVE1,
                TIVX_TARGET_MAX_NAME);
        }
        else if (self_cpu == TIVX_CPU_ID_EVE2)
        {
            strncpy(target_name, TIVX_TARGET_EVE2,
                TIVX_TARGET_MAX_NAME);
        }
        else if (self_cpu == TIVX_CPU_ID_EVE3)
        {
            strncpy(target_name, TIVX_TARGET_EVE3,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_EVE4,
                TIVX_TARGET_MAX_NAME);
        }

        vx_harris_corners_target_kernel = tivxAddTargetKernel(
            TIVX_KERNEL_IVISION_HARRIS_CORNERS,
            target_name,
            tivxKernelHarrisCProcess,
            tivxKernelHarrisCCreate,
            tivxKernelHarrisCDelete,
            tivxKernelHarrisCControl,
            NULL);
    }
}


void tivxRemoveTargetKernelHarrisCorners()
{
    tivxRemoveTargetKernel(vx_harris_corners_target_kernel);
}

static void tivxHarrisCFreeMem(tivxHarrisCornersParams *prms)
{
    if (NULL != prms)
    {
        if (prms->output)
        {
            tivxMemFree(prms->output, prms->output_size, TIVX_MEM_EXTERNAL);
            prms->output = NULL;
        }

        tivxMemFree(prms, sizeof(tivxHarrisCornersParams), TIVX_MEM_EXTERNAL);
    }
}

