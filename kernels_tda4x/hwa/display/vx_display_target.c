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

#include "TI/tivx.h"
#include "TI/tda4x.h"
#include "VX/vx.h"
#include "TI/tivx_event.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_display.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include <TI/tivx_queue.h>
#include <ti/drv/dss/dss.h>

#define DISPLAY_MAX_VALID_PLANES                      2U
#define DISPLAY_MAX_COPY_BUFFERS                      2U

typedef struct
{
    uint32_t opMode;
    /**< Operation Mode of display kernel. Refer \ref Display_opMode for values */
    Fvid2_Frame copyFrame[DISPLAY_MAX_COPY_BUFFERS];
    /**< Copy frames for TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE */
    void *copyImagePtr[DISPLAY_MAX_COPY_BUFFERS][DISPLAY_MAX_VALID_PLANES];
    /**< Buffer addresses for copied frame */
    uint32_t copyImageSize[DISPLAY_MAX_VALID_PLANES];
    /**< Buffer addresses for copied frame */
    uint32_t currIdx;
    /**< Buffer addresses for copied frame */
    uint32_t firstFrameDisplay;
    /**< Option to tell whether display is processing first frame */
    SemaphoreP_Handle waitSem;
    /**< Semaphore for ISR */
    Dss_DispCreateParams createParams;
    /**< Create time parameters */
    Dss_DispCreateStatus createStatus;
    /**< Create status returned by driver during Fvid2_create() */
    Dss_DispParams dispParams;
    /**< DSS display parameters */
    Fvid2_Handle drvHandle;
    /**< FVID2 display driver handle */
    Fvid2_CbParams cbParams;
    /**< Callback parameters */
    tivx_queue fvid2FrameQ;
    /**< Internal FVID2 queue */
    uintptr_t fvid2FrameQMem[TIVX_DISPLAY_MAX_NUM_BUFS];
    /**< Queue memory */
    Fvid2_Frame fvid2Frames[TIVX_DISPLAY_MAX_NUM_BUFS];
    /**< FVID2 Frames that will be used for display */
} tivxDisplayParams;

static tivx_target_kernel vx_display_target_kernel1 = NULL;
static tivx_target_kernel vx_display_target_kernel2 = NULL;

static vx_status VX_CALLBACK tivxDisplayCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status tivxDisplayExtractFvid2Format(tivx_obj_desc_image_t *obj_desc_img,
                                               Fvid2_Format *format);
static int32_t tivxDisplayCallback(Fvid2_Handle handle, void *appData);
static uint32_t tivxDisplayGetPipeType(uint32_t drvId);
static int32_t tivxDisplayGetImageSize(tivx_obj_desc_image_t *obj_desc_image,
                                       uint32_t *copySize0,
                                       uint32_t *copySize1);

static vx_status tivxDisplayExtractFvid2Format(tivx_obj_desc_image_t *obj_desc_img,
                                               Fvid2_Format *format)
{
    vx_status status = VX_SUCCESS;
    Fvid2Format_init(format);
    format->width = obj_desc_img->imagepatch_addr[0].dim_x;
    format->height = obj_desc_img->imagepatch_addr[0].dim_y;
    format->ccsFormat = FVID2_CCSF_BITS8_PACKED;
    format->scanFormat = FVID2_SF_PROGRESSIVE;

    switch (obj_desc_img->format)
    {
        case VX_DF_IMAGE_RGB:
            format->dataFormat = FVID2_DF_RGB24_888;
            format->pitch[FVID2_RGB_ADDR_IDX] = obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case VX_DF_IMAGE_RGBX:
            format->dataFormat = FVID2_DF_RGBA32_8888;
            format->pitch[FVID2_RGB_ADDR_IDX] = obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case VX_DF_IMAGE_UYVY:
            format->dataFormat = FVID2_DF_YUV422I_UYVY;
            format->pitch[FVID2_YUV_INT_ADDR_IDX] = obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case VX_DF_IMAGE_NV12:
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = obj_desc_img->imagepatch_addr[1].stride_y;
            break;
        default:
            status = VX_FAILURE;
            break;
    }

    return status;
}

static int32_t tivxDisplayCallback(Fvid2_Handle handle, void *appData)
{
    tivxDisplayParams *displayParams = (tivxDisplayParams *)(appData);
    SemaphoreP_post(displayParams->waitSem);
    return VX_SUCCESS;
}

static uint32_t tivxDisplayGetPipeType(uint32_t drvId)
{
    uint32_t pipeType = CSL_DSS_VID_PIPE_TYPE_VID;
    if((DSS_DISP_INST_VIDL1 == drvId) || (DSS_DISP_INST_VIDL2 == drvId))
    {
        pipeType = CSL_DSS_VID_PIPE_TYPE_VIDL;
    }
    return pipeType;
}

static int32_t tivxDisplayGetImageSize(tivx_obj_desc_image_t *obj_desc_image,
                                       uint32_t *copySize0,
                                       uint32_t *copySize1)
{
    vx_status status = VX_SUCCESS;
    switch (obj_desc_image->format)
    {
        case VX_DF_IMAGE_RGB:
            *copySize0 = obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case VX_DF_IMAGE_RGBX:
            *copySize0 = obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case VX_DF_IMAGE_UYVY:
            *copySize0 = obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case VX_DF_IMAGE_NV12:
            *copySize0 = obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            *copySize1 = obj_desc_image->imagepatch_addr[1].stride_y * obj_desc_image->height;
            break;
        default:
            *copySize0 = 0;
            *copySize1 = 0;
            status = VX_ERROR_INVALID_PARAMETERS;
            break;
    }
    return status;
}

static vx_status VX_CALLBACK tivxDisplayCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *obj_desc_configuration;
    tivx_obj_desc_image_t *obj_desc_image;
    tivxDisplayParams *displayParams = NULL;
    tivx_display_params_t *params;
    void *display_config_target_ptr;
    uint32_t drvId;
    SemaphoreP_Params semParams;

    if((num_params != TIVX_KERNEL_DISPLAY_MAX_PARAMS) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX]))
    {
        status = VX_FAILURE;
    }
    else
    {
        obj_desc_configuration = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX];
        obj_desc_image = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];

        displayParams = tivxMemAlloc(sizeof(tivxDisplayParams), TIVX_MEM_EXTERNAL);
        if(NULL != displayParams)
        {
            memset(displayParams, 0, sizeof(tivxDisplayParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Couldn't allocate memory!\r\n");
            status = VX_ERROR_NO_MEMORY;
        }

        /* TODO: Add check for name and size for tivx_display_params_t */
        if(VX_SUCCESS == status)
        {
            display_config_target_ptr = tivxMemShared2TargetPtr(obj_desc_configuration->mem_ptr.shared_ptr,
                                                                obj_desc_configuration->mem_ptr.mem_heap_region);

            tivxMemBufferMap(display_config_target_ptr,
                             obj_desc_configuration->mem_size,
                             VX_MEMORY_TYPE_HOST,
                             VX_READ_ONLY);

            params = (tivx_display_params_t *)display_config_target_ptr;

            drvId = params->pipeId;
            if(drvId >= DSS_DISP_INST_MAX)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Invalid pipe id used by display!\r\n");
            }
        }
        if(VX_SUCCESS == status)
        {
            Dss_dispCreateParamsInit(&displayParams->createParams);
            Fvid2CbParams_init(&displayParams->cbParams);
            displayParams->opMode = params->opMode;
            displayParams->firstFrameDisplay = TRUE;
            displayParams->cbParams.cbFxn = (Fvid2_CbFxn) (&tivxDisplayCallback);
            displayParams->cbParams.appData = displayParams;
            SemaphoreP_Params_init(&semParams);
            semParams.mode = SemaphoreP_Mode_BINARY;
            displayParams->waitSem = SemaphoreP_create(0U, &semParams);
            if(NULL == displayParams->waitSem)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Semaphore Create Failed!\r\n");
            }
            else
            {
                displayParams->drvHandle = Fvid2_create(DSS_DISP_DRV_ID,
                                                        drvId,
                                                        &displayParams->createParams,
                                                        &displayParams->createStatus,
                                                        &displayParams->cbParams);
                if((NULL == displayParams->drvHandle) ||
                   (displayParams->createStatus.retVal != FVID2_SOK))
                {
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Display Create Failed!\r\n");
                    status = displayParams->createStatus.retVal;
                }
            }
        }
        if(VX_SUCCESS == status)
        {
            Dss_dispParamsInit(&displayParams->dispParams);
            displayParams->dispParams.pipeCfg.pipeType = tivxDisplayGetPipeType(drvId);
            displayParams->dispParams.pipeCfg.outWidth = params->outWidth;
            displayParams->dispParams.pipeCfg.outHeight = params->outHeight;
            displayParams->dispParams.layerPos.startX = params->posX;
            displayParams->dispParams.layerPos.startY = params->posY;
            status = tivxDisplayExtractFvid2Format(obj_desc_image,
                                                   &displayParams->dispParams.pipeCfg.inFmt);
        }
        if(VX_SUCCESS == status)
        {
            if(TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE == params->opMode)
            {
                status = tivxDisplayGetImageSize(obj_desc_image,
                                                 &displayParams->copyImageSize[0],
                                                 &displayParams->copyImageSize[1]);

                if(VX_SUCCESS == status)
                {
                    if(displayParams->copyImageSize[0] != 0)
                    {
                        displayParams->copyImagePtr[0][0] = tivxMemAlloc(displayParams->copyImageSize[0], TIVX_MEM_EXTERNAL);
                        displayParams->copyImagePtr[1][0] = tivxMemAlloc(displayParams->copyImageSize[0], TIVX_MEM_EXTERNAL);
                    }
                    if((displayParams->copyImageSize[1] != 0) && (VX_DF_IMAGE_NV12 == obj_desc_image->format))
                    {
                        displayParams->copyImagePtr[0][1] = tivxMemAlloc(displayParams->copyImageSize[1], TIVX_MEM_EXTERNAL);
                        displayParams->copyImagePtr[1][1] = tivxMemAlloc(displayParams->copyImageSize[1], TIVX_MEM_EXTERNAL);
                    }
                }
                displayParams->currIdx = 0;
            }
        }
        if(VX_SUCCESS == status)
        {
            status = Fvid2_control(displayParams->drvHandle,
                                   IOCTL_DSS_DISP_SET_DSS_PARAMS,
                                   &displayParams->dispParams,
                                   NULL);
            if(status != FVID2_SOK)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Display Set Parameters Failed!\r\n");
                status = VX_FAILURE;
            }
        }
        /* Creating FVID2 frame Q */
        if((VX_SUCCESS == status) && (TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE == displayParams->opMode))
        {
            uint32_t bufId;
            status = tivxQueueCreate(&displayParams->fvid2FrameQ, TIVX_DISPLAY_MAX_NUM_BUFS, displayParams->fvid2FrameQMem, 0);
            if(VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Fvid2 queue create failed!\r\n");
            }

            for(bufId = 0; bufId <TIVX_DISPLAY_MAX_NUM_BUFS; bufId++)
            {
                tivxQueuePut(&displayParams->fvid2FrameQ, (uintptr_t)&displayParams->fvid2Frames[bufId], TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
        }

        if(VX_SUCCESS == status)
        {
            tivxMemBufferUnmap(display_config_target_ptr,
                               obj_desc_configuration->mem_size,
                               VX_MEMORY_TYPE_HOST,
                               VX_READ_ONLY);
            tivxSetTargetKernelInstanceContext(kernel,
                                               displayParams,
                                               sizeof(tivxDisplayParams));
        }
        else
        {
            if(NULL != displayParams)
            {
                tivxMemFree(displayParams, sizeof(tivxDisplayParams), TIVX_MEM_EXTERNAL);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxDisplayDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxDisplayParams *displayParams = NULL;
    Fvid2_FrameList frmList;
    uint32_t size;
    tivx_obj_desc_image_t *obj_desc_image;

    if((num_params != TIVX_KERNEL_DISPLAY_MAX_PARAMS) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX]))
    {
        status = VX_FAILURE;
    }
    else
    {
        obj_desc_image = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **) &displayParams,
                                                    &size);

        if(VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Could not obtain display kernel instance context!\r\n");
        }

        /* Stop Display */
        if(VX_SUCCESS == status)
        {
            status = Fvid2_stop(displayParams->drvHandle, NULL);

            if(VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: FVID2 Stop Failed!\r\n");
            }
        }

        /* Dequeue all the request from the driver */
        if(VX_SUCCESS == status)
        {
            do
            {
                status = Fvid2_dequeue(displayParams->drvHandle,
                                       &frmList,
                                       0,
                                       FVID2_TIMEOUT_NONE);
            } while(VX_SUCCESS == status);

            /* Delete FVID2 handle */
            status = Fvid2_delete(displayParams->drvHandle, NULL);

            if(VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: FVID2 Delete Failed!\r\n");
            }
        }

        /* Deleting FVID2 frame Q */
        if((VX_SUCCESS == status) && (TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE == displayParams->opMode))
        {
            tivxQueueDelete(&displayParams->fvid2FrameQ);
        }

        /* Delete the wait semaphore */
        if(NULL != displayParams->waitSem)
        {
            SemaphoreP_delete(displayParams->waitSem);
            displayParams->waitSem = NULL;
        }

        /* Delete kernel instance params object */
        if(VX_SUCCESS == status)
        {
            displayParams->drvHandle = NULL;

            if(TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE == displayParams->opMode)
            {
                if(displayParams->copyImageSize[0] != 0)
                {
                    tivxMemFree(displayParams->copyImagePtr[0][0], displayParams->copyImageSize[0], TIVX_MEM_EXTERNAL);
                    tivxMemFree(displayParams->copyImagePtr[1][0], displayParams->copyImageSize[0], TIVX_MEM_EXTERNAL);
                }
                if((displayParams->copyImageSize[1] != 0) && (VX_DF_IMAGE_NV12 == obj_desc_image->format))
                {
                    tivxMemFree(displayParams->copyImagePtr[0][1], displayParams->copyImageSize[1], TIVX_MEM_EXTERNAL);
                    tivxMemFree(displayParams->copyImagePtr[1][1], displayParams->copyImageSize[1], TIVX_MEM_EXTERNAL);
                }
            }

            if((NULL != displayParams) && (sizeof(tivxDisplayParams) == size))
            {
                tivxMemFree(displayParams, sizeof(tivxDisplayParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxDisplayParams *displayParams = NULL;
    tivx_obj_desc_image_t *obj_desc_image;
    void *image_target_ptr1, *image_target_ptr2;
    uint32_t size;
    Fvid2_FrameList frmList;
    Fvid2_Frame *frm;

    if((num_params != TIVX_KERNEL_DISPLAY_MAX_PARAMS) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX]))
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void *)&displayParams,
                                                    &size);

        if((VX_SUCCESS != status) ||
           (NULL == displayParams) ||
           (sizeof(tivxDisplayParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        obj_desc_image = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];
        image_target_ptr1 = tivxMemShared2TargetPtr(obj_desc_image->mem_ptr[0].shared_ptr,
                                                    obj_desc_image->mem_ptr[0].mem_heap_region);
        if(VX_DF_IMAGE_NV12 == obj_desc_image->format)
        {
            image_target_ptr2 = tivxMemShared2TargetPtr(obj_desc_image->mem_ptr[1].shared_ptr,
                                                        obj_desc_image->mem_ptr[1].mem_heap_region);

        }

        if(TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE == displayParams->opMode)
        {
            /* Get frame from queue */
            tivxQueueGet(&displayParams->fvid2FrameQ, (uintptr_t*)&frm, TIVX_EVENT_TIMEOUT_NO_WAIT);
            if(NULL != frm)
            {
                /* Assign buffer addresses */
                frm->addr[0U] = (uint64_t)image_target_ptr1;
                if(VX_DF_IMAGE_NV12 == obj_desc_image->format)
                {
                    frm->addr[1U] = (uint64_t)image_target_ptr2;
                }
                frm->fid = FVID2_FID_FRAME;
                frm->appData = obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];

                /* Create frame list */
                frmList.numFrames  = 1U;
                frmList.frames[0U] = frm;
                /* Call Fvid2 Queue */
                status = Fvid2_queue(displayParams->drvHandle, &frmList, 0U);
                if(VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Unable to queue frame!\r\n");
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Could not get frame from queue!\r\n");
            }

            if(TRUE == displayParams->firstFrameDisplay)
            {
                status = Fvid2_start(displayParams->drvHandle, NULL);
                if(VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Could not start display!\r\n");
                }
                else
                {
                    displayParams->firstFrameDisplay = FALSE;
                    /* Can't return frame as this is first display */
                    obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX] = NULL;
                }
            }
            else
            {
                do
                {
                    SemaphoreP_pend(displayParams->waitSem, SemaphoreP_WAIT_FOREVER);
                    status = Fvid2_dequeue(displayParams->drvHandle,
                                        &frmList,
                                        0U,
                                        FVID2_TIMEOUT_NONE);
                } while(FVID2_EAGAIN == status);
                if((1U == frmList.numFrames) && (VX_SUCCESS == status))
                {
                    frm = frmList.frames[0U];
                    /* Return frame */
                    obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX] = frm->appData;
                    tivxQueuePut(&displayParams->fvid2FrameQ, (uintptr_t)frm, TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
                else /* (1U != frmList.numFrames) || ((VX_SUCCESS != status) && (FVID2_EAGAIN != status))*/
                {
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Dequeue operation failed!\r\n");
                }
            }
        }
        else if(TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE == displayParams->opMode)
        {
            /* Copy  and assign buffers */
            memcpy(displayParams->copyImagePtr[displayParams->currIdx][0], image_target_ptr1, displayParams->copyImageSize[0]);
            displayParams->copyFrame[displayParams->currIdx].addr[0] = (uint64_t)displayParams->copyImagePtr[displayParams->currIdx][0];
            if(VX_DF_IMAGE_NV12 == obj_desc_image->format)
            {
                memcpy(displayParams->copyImagePtr[displayParams->currIdx][1], image_target_ptr2, displayParams->copyImageSize[1]);
                displayParams->copyFrame[displayParams->currIdx].addr[1] = (uint64_t)displayParams->copyImagePtr[displayParams->currIdx][1];
            }
            displayParams->copyFrame[displayParams->currIdx].fid = FVID2_FID_FRAME;
            displayParams->copyFrame[displayParams->currIdx].appData = NULL;

            /* Create frame list */
            frmList.numFrames  = 1U;
            frmList.frames[0U] = &displayParams->copyFrame[displayParams->currIdx];
            /* Call Fvid2 Queue */
            status = Fvid2_queue(displayParams->drvHandle, &frmList, 0U);
            if(VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Unable to queue copy frame!\r\n");
            }
            if(TRUE == displayParams->firstFrameDisplay)
            {
                status = Fvid2_start(displayParams->drvHandle, NULL);
                if(VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Could not start display!\r\n");
                }
                displayParams->firstFrameDisplay = FALSE;
                displayParams->currIdx = 1;
            }
            else
            {
                do
                {
                    SemaphoreP_pend(displayParams->waitSem, SemaphoreP_WAIT_FOREVER);
                    status = Fvid2_dequeue(displayParams->drvHandle,
                                           &frmList,
                                           0U,
                                           FVID2_TIMEOUT_NONE);
                } while(FVID2_EAGAIN == status);
                if((1U == frmList.numFrames) && (VX_SUCCESS == status))
                {
                    /* Change Curr Index */
                    if(displayParams->currIdx == 0)
                    {
                        displayParams->currIdx = 1;
                    }
                    else
                    {
                        displayParams->currIdx = 0;
                    }
                }
                else /* (1U != frmList.numFrames) || ((VX_SUCCESS != status) && (FVID2_EAGAIN != status))*/
                {
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Dequeue operation failed!\r\n");
                }
            }
        }
        else
        {
            status = VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Wrong Operation Mode Selected!\r\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelDisplay()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_DISPLAY1,
            TIVX_TARGET_MAX_NAME);

        vx_display_target_kernel1 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_NAME,
                            target_name,
                            tivxDisplayProcess,
                            tivxDisplayCreate,
                            tivxDisplayDelete,
                            tivxDisplayControl,
                            NULL);
                            
        strncpy(target_name, TIVX_TARGET_DISPLAY2,
            TIVX_TARGET_MAX_NAME);

        vx_display_target_kernel2 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_NAME,
                            target_name,
                            tivxDisplayProcess,
                            tivxDisplayCreate,
                            tivxDisplayDelete,
                            tivxDisplayControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelDisplay()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_display_target_kernel1);
    if(status == VX_SUCCESS)
    {
        vx_display_target_kernel1 = NULL;
    }
    status = tivxRemoveTargetKernel(vx_display_target_kernel2);
    if(status == VX_SUCCESS)
    {
        vx_display_target_kernel2 = NULL;
    }
}
