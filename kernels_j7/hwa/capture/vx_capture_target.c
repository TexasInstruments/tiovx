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

#include <stdio.h>
#include "TI/tivx.h"
#include "VX/vx.h"
#include "TI/tivx_event.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_capture.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_capture_priv.h"

#include <TI/tivx_queue.h>
#include <ti/drv/fvid2/fvid2.h>
#include <ti/drv/csirx/csirx.h>
#include <tivx_obj_desc_priv.h>
#include <vx_reference.h>
#include <vx_internal.h>

#define CAPTURE_FRAME_DROP_LEN                          (4096U*4U)

#define CAPTURE_INST_ID_INVALID                         (0xFFFFU)

#define CAPTURE_IN_CSI_DT_INVALID                       (0xFFFFFFFFU)

#define CAPTURE_TIMEOUT_VALID                           (0U)
#define CAPTURE_TIMEOUT_EXCEEDED                        (1U)

#define CAPTURE_MS_TO_US                                (1000U)

static char target_name[][TIVX_TARGET_MAX_NAME] =
{
    TIVX_TARGET_CAPTURE1,
    TIVX_TARGET_CAPTURE2,
    TIVX_TARGET_CAPTURE3,
    TIVX_TARGET_CAPTURE4,
    TIVX_TARGET_CAPTURE5,
    TIVX_TARGET_CAPTURE6,
    TIVX_TARGET_CAPTURE7,
    TIVX_TARGET_CAPTURE8,
#if defined(SOC_J784S4)
    TIVX_TARGET_CAPTURE9,
    TIVX_TARGET_CAPTURE10,
    TIVX_TARGET_CAPTURE11,
    TIVX_TARGET_CAPTURE12,
#endif
};

#define CAPTURE_NUM_TARGETS                             (sizeof(target_name)/sizeof(target_name[0]))

typedef struct tivxCaptureParams_t tivxCaptureParams;

typedef struct
{
    uint32_t instId;
    /**< Csirx Drv Instance ID. */
    uint8_t numCh;
    /**< Number of channels processed on given CSIRX DRV instance. */
    uint32_t chVcMap[TIVX_CAPTURE_MAX_CH];
    /**< Virtual ID for channels for current capture instance. */
    Fvid2_Handle drvHandle;
    /**< FVID2 capture driver handle. */
    Csirx_CreateParams createPrms;
    /**< Csirx create time parameters */
    Csirx_CreateStatus createStatus;
    /**< Csirx create time status */
    Fvid2_CbParams drvCbPrms;
    /**< Capture callback params */
    uint8_t raw_capture;
    /**< flag indicating raw capture */
    Csirx_InstStatus captStatus;
    /**< CSIRX Capture status. */
    Csirx_DPhyCfg dphyCfg;
    /**< CSIRX DPHY configuration. */
    tivxCaptureParams *captParams;
    /**< Reference to capture node parameters. */
} tivxCaptureInstParams;

struct tivxCaptureParams_t
{
    tivxCaptureInstParams instParams[TIVX_CAPTURE_MAX_INST];
    /**< Capture Instance parameters */
    uint32_t numOfInstUsed;
    /**< Number of CSIRX DRV instances used in current TIOVX Node. */
    uint8_t numCh;
    /**< Number of channels processed on given capture node instance. */
    tivx_obj_desc_t *img_obj_desc[TIVX_CAPTURE_MAX_CH];
    /**< Captured Images */
    uint8_t steady_state_started;
    /**< Flag indicating whether or not steady state has begun. */
    tivx_event  frame_available;
    /**< Following Queues i.e. freeFvid2FrameQ, pendingFrameQ, fvid2_free_q_mem,
     *   fvid2Frames, and pending_frame_free_q_mem are for given instance of the
     *   Node. If Node instance contains more than 1 instances of the CSIRX DRV
     *   instances, then first 'n' channels are for first instance of the driver
     *   then n channels for next driver and so on... */
    /**< Event indicating when a frame is available. */
    tivx_queue freeFvid2FrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal FVID2 queue */
    tivx_queue pendingFrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal pending frame queue */
    tivx_queue pendingObjArrayQ;
    /**< Internal pending obj arr queue */
    tivx_queue pendingFrameTimestampLoQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal queue tracking lower 32 bits of time stamp of pending frames */
    tivx_queue pendingFrameTimestampHiQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal queue tracking upper 32 bits of time stamp of pending frames */
    uintptr_t fvid2_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< FVID2 queue mem */
    Fvid2_Frame fvid2Frames[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< FVID2 frame structs */
    uintptr_t pending_frame_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending frame queue mem */
    uintptr_t pending_obj_arr_q_mem[TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending obj arr queue mem */
    uintptr_t pending_frame_timestamp_lo_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending timestamp lo queue mem */
    uintptr_t pending_frame_timestamp_hi_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending timestamp hi queue mem */
    uint32_t timeout;
    /**< Total timeout to check for dead camera; taken directly from
     *   tivx_capture_params_t input */
    uint32_t timeoutInitial;
    /**< Initial timeout to check for dead camera; taken directly from
     *   tivx_capture_params_t input */
    uint64_t timeoutRemaining;
    /**< Remaining timeout for dead camera */
    uint8_t activeChannelMask;
    /**< Mask for active channels; bit 0 maps to channel 0, bit N maps to bit N;
     *   1 indicates active, 0 indicates inactive */
    tivx_queue errorFrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal error frame queue; contains descriptor ID's of invalid frames */
    uintptr_t error_frame_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< error frame queue mem */
    tivx_obj_desc_t *error_obj_desc[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< Error Image Object Descriptors; allocated in control callback and points to
     *   memory from descriptor sent from application */
    uint8_t enableErrorFrameTimeout;
    /**< Flag indicating if error frame has been sent and can use error timeout
     *   Error timeout is only used if this error frame is sent */

    /* Make frame list instance specific */
    Fvid2_FrameList frmList;
};

static tivx_target_kernel vx_capture_target_kernel[CAPTURE_NUM_TARGETS] = {NULL};

static vx_status captDrvCallback(Fvid2_Handle handle, void *appData, void *reserved);
static uint32_t tivxCaptureExtractInCsiDataType(uint32_t format);
static uint32_t tivxCaptureExtractCcsFormat(uint32_t format);
static uint32_t tivxCaptureExtractDataFormat(uint32_t format);
static vx_status tivxCaptureEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *output_desc,
       tivxCaptureParams *prms);
static vx_status tivxCaptureSetCreateParams(
       tivxCaptureParams *prms,
       const tivx_obj_desc_user_data_object_t *obj_desc);
static vx_status VX_CALLBACK tivxCaptureProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCaptureCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCaptureDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCaptureControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status tivxCaptureGetStatistics(tivxCaptureParams *prms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxCaptureAllocErrorDesc(tivxCaptureParams *prms,
    tivx_obj_desc_t *obj_desc);
static void tivxCaptureCopyStatistics(tivxCaptureParams *prms,
    tivx_capture_statistics_t *capt_status_prms);
static void tivxCaptureGetChannelIndices(const tivxCaptureParams *prms,
                                         uint32_t instId,
                                         uint32_t *startChIdx,
                                         uint32_t *endChIdx);
static uint32_t tivxCaptureGetNodeChannelNum(const tivxCaptureParams *prms,
                                             uint32_t instId,
                                             uint32_t chId);
static uint32_t tivxCaptureGetDrvInstIndex(const tivxCaptureParams *prms,
                                           uint32_t instId);
static uint32_t tivxCaptureMapInstId(uint32_t instId);
static void tivxCapturePrintStatus(tivxCaptureInstParams *prms);
static vx_status tivxCaptureStart(tivxCaptureParams *prms);
static void tivxCaptureSetTimeout(tivxCaptureParams *prms);
static vx_status tivxCaptureTimeout(tivxCaptureParams *prms);
static void tivxCaptureGetObjDesc(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        tivx_obj_desc_object_array_t *output_desc,
        uint64_t *timestamp);
static vx_status tivxCaptureDequeueFrameFromDriver(tivxCaptureParams *prms);
static uint32_t tivxCaptureIsAllChFrameAvailable(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        uint8_t timeoutExceeded);

/**
 *******************************************************************************
 *
 * \brief Callback function from driver to application
 *
 * Callback function gets called from Driver to application on reception of
 * a frame
 *
 * \param  handle       [IN] Driver handle for which callback has come.
 * \param  appData      [IN] Application specific data which is registered
 *                           during the callback registration.
 * \param  reserved     [IN] Reserved.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static vx_status captDrvCallback(Fvid2_Handle handle, void *appData, void *reserved)
{
    tivxCaptureParams *prms = (tivxCaptureParams*)appData;

    tivxEventPost(prms->frame_available);

    return (vx_status)VX_SUCCESS;
}

/* Waiting on capture frame available event based on timeout
 *    Log the wait time and subtract from the remaining time
 *    Use remaining time as the wait time */
static vx_status tivxCaptureTimeout(tivxCaptureParams *prms)
{
    vx_status status;
    uint64_t timestamp = 0;

    timestamp = tivxPlatformGetTimeInUsecs();

    status = tivxEventWait(prms->frame_available, (uint32_t)prms->timeoutRemaining);

    /* Calculate time that the tivxEventWait waited */
    timestamp = tivxPlatformGetTimeInUsecs() - timestamp;

    if (1U == prms->enableErrorFrameTimeout)
    {
        /* Rounding up so that the timeout does not get clipped for each subsequent camera */
        if (timestamp > (CAPTURE_MS_TO_US * prms->timeoutRemaining))
        {
            prms->timeoutRemaining = 0u;
        }
        else
        {
            /* Update timeoutRemaining based on amount of time already waited */
            prms->timeoutRemaining = (((CAPTURE_MS_TO_US * prms->timeoutRemaining + CAPTURE_MS_TO_US) - timestamp) / CAPTURE_MS_TO_US);
        }
    }

    return status;
}

static vx_status tivxCaptureEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *output_desc,
       tivxCaptureParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    void *output_image_target_ptr;
    uint64_t captured_frame;
    uint32_t chId = 0U;
    Fvid2_FrameList *frmList;
    Fvid2_Frame *fvid2Frame;
    uint32_t startChIdx, endChIdx, instIdx;
    tivxCaptureInstParams *instParams;
    uint16_t obj_desc_id;

    frmList = &prms->frmList;
    tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                       prms->numCh);

    tivxQueuePut(&prms->pendingObjArrayQ, (uintptr_t)output_desc, TIVX_EVENT_TIMEOUT_NO_WAIT);

    /* Prepare and queue frame-list for each instance */
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];
        tivxCaptureGetChannelIndices(prms, instIdx, &startChIdx, &endChIdx);
        frmList->numFrames = 0;

        for (chId = startChIdx ; chId < endChIdx ; chId++)
        {
            /* Only enqueue the frame if it is a valid frame */
            if (tivxFlagIsBitSet(prms->img_obj_desc[chId]->flags, TIVX_REF_FLAG_IS_INVALID) == 0U)
            {
                if ((uint32_t)TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[chId]->type)
                {
                    tivx_obj_desc_raw_image_t *raw_image;

                    raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[chId];

                    obj_desc_id = prms->img_obj_desc[chId]->obj_desc_id;

                    /* Question: is the fact that we are just using mem_ptr[0] and not remaining planes correct? */
                    output_image_target_ptr = tivxMemShared2TargetPtr(&raw_image->mem_ptr[0]);

                    captured_frame = ((uintptr_t)output_image_target_ptr +
                        (uint64_t)tivxComputePatchOffset(0, 0, &raw_image->imagepatch_addr[0U]));
                }
                else
                {
                    tivx_obj_desc_image_t *image;
                    image = (tivx_obj_desc_image_t *)prms->img_obj_desc[chId];

                    obj_desc_id = prms->img_obj_desc[chId]->obj_desc_id;

                    /* Question: is the fact that we are just using mem_ptr[0] and not remaining exposures correct? */
                    output_image_target_ptr = tivxMemShared2TargetPtr(&image->mem_ptr[0]);

                    captured_frame = ((uintptr_t)output_image_target_ptr +
                        (uint64_t)tivxComputePatchOffset(0, 0, &image->imagepatch_addr[0U]));
                }

                tivxQueueGet(&prms->freeFvid2FrameQ[chId], (uintptr_t*)&fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);

                if (NULL != fvid2Frame)
                {
                    uint32_t obj_desc_id_u32 = (uint32_t)obj_desc_id;

                    /* Put into frame list as it is for same driver instance */
                    frmList->frames[frmList->numFrames]           = fvid2Frame;
                    frmList->frames[frmList->numFrames]->chNum    = (chId - startChIdx);
                    frmList->frames[frmList->numFrames]->addr[0U] = captured_frame;
                    frmList->frames[frmList->numFrames]->appData  = (void *)obj_desc_id_u32;
                    frmList->numFrames++;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, " CAPTURE: Could not retrieve buffer from buffer queue!!!\n");
                }
            }
            else
            {
                tivxQueuePut(&prms->errorFrameQ[chId], (uintptr_t)output_desc->obj_desc_id[chId], TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
        }

        /* Only call Fvid2_queue if there are valid frames to enqueue */
        if (frmList->numFrames > 0U)
        {
            fvid2_status = Fvid2_queue(instParams->drvHandle, frmList, 0);
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Frame could not be queued for frame %d !!!\n", chId);
                break;
            }
        }

    }

    return status;
}

static uint32_t tivxCaptureExtractInCsiDataType(uint32_t format)
{
    uint32_t inCsiDataType;

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_RGB:
            inCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGBX:
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
            inCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (uint32_t)TIVX_RAW_IMAGE_P12_BIT:
            inCsiDataType = FVID2_CSI2_DF_RAW12;
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            inCsiDataType = FVID2_CSI2_DF_YUV422_8B;
            break;
        default:
            inCsiDataType = CAPTURE_IN_CSI_DT_INVALID;
            break;
    }

    return inCsiDataType;
}

static uint32_t tivxCaptureExtractInCsiDataTypeFromRawImg(tivx_obj_desc_raw_image_t *raw_img)
{
    uint32_t inCsiDataType = CAPTURE_IN_CSI_DT_INVALID;
    tivx_raw_image_create_params_t *params = &raw_img->params;

    if (TIVX_RAW_IMAGE_16_BIT == params->format[0].pixel_container)
    {
        switch (params->format[0].msb)
        {
            case 9u:
                inCsiDataType = FVID2_CSI2_DF_RAW10;
            break;
            case 11u:
                inCsiDataType = FVID2_CSI2_DF_RAW12;
            break;
            case 13u:
                inCsiDataType = FVID2_CSI2_DF_RAW14;
            break;
            case 15u:
                inCsiDataType = FVID2_CSI2_DF_RAW16;
            break;
            default:
                break;
        }
    }
    else if (TIVX_RAW_IMAGE_8_BIT == params->format[0].pixel_container)
    {
        switch (params->format[0].msb)
        {
            case 5u:
                inCsiDataType = FVID2_CSI2_DF_RAW6;
            break;
            case 6u:
                inCsiDataType = FVID2_CSI2_DF_RAW7;
            break;
            case 7u:
                inCsiDataType = FVID2_CSI2_DF_RAW8;
            break;
            default:
                break;
        }
    }
    else if (TIVX_RAW_IMAGE_P12_BIT == params->format[0].pixel_container)
    {
        if (11u == params->format[0].msb)
        {
            inCsiDataType = FVID2_CSI2_DF_RAW12;
        }
    }
    else
    {
        /* Don Nothing */
    }

    return (inCsiDataType);
}

static uint32_t tivxCaptureExtractCcsFormat(uint32_t format)
{
    uint32_t ccsFormat = FVID2_CCSF_BITS12_PACKED;

    switch (format)
    {
        case (uint32_t)TIVX_RAW_IMAGE_P12_BIT:
            ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        case (vx_enum)TIVX_RAW_IMAGE_16_BIT:
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            break;
        default:
            ccsFormat = FVID2_CCSF_MAX;
            break;
    }

    return ccsFormat;
}

static uint32_t tivxCaptureMapInstId(uint32_t instId)
{
    uint32_t drvInstId = 0xFFFF;
    switch (instId)
    {
        case 0:
            drvInstId = CSIRX_INSTANCE_ID_0;
            break;
        case 1:
            drvInstId = CSIRX_INSTANCE_ID_1;
            break;
#if defined(SOC_J784S4)
        case 2:
            drvInstId = CSIRX_INSTANCE_ID_2;
            break;
#endif
        default:
            /* do nothing */
            break;
    }

    return (drvInstId);
}

/* TODO: Complete this case statement */
static uint32_t tivxCaptureExtractDataFormat(uint32_t format)
{
    uint32_t dataFormat = FVID2_DF_BGRX32_8888;

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_UYVY:
            dataFormat = FVID2_DF_YUV422I_UYVY;
        break;
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            dataFormat = FVID2_DF_YUV422I_YUYV;
        break;
        default:
            dataFormat = FVID2_DF_BGRX32_8888;
        break;
    }

    return dataFormat;
}

static vx_status tivxCaptureSetCreateParams(
       tivxCaptureParams *prms,
       const tivx_obj_desc_user_data_object_t *obj_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t loopCnt = 0U, i, format, width, height, planes, stride[TIVX_IMAGE_MAX_PLANES];
    void *capture_config_target_ptr;
    tivx_capture_params_t *params;
    uint32_t chIdx, instId = 0U, instIdx;
    Csirx_CreateParams *createParams;
    tivx_obj_desc_raw_image_t *raw_image = NULL;

    capture_config_target_ptr = tivxMemShared2TargetPtr(&obj_desc->mem_ptr);

    tivxCheckStatus(&status, tivxMemBufferMap(capture_config_target_ptr, obj_desc->mem_size,
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    params = (tivx_capture_params_t *)capture_config_target_ptr;

    /* Scan through all the instances provided in the Node instance and prepare CSIRX DRV instance data/cfg */
    for (instIdx = 0U ; instIdx < params->numInst ; instIdx++)
    {
        /* set instance to be used for capture */
        prms->instParams[instIdx].instId = tivxCaptureMapInstId(params->instId[instIdx]);
        prms->numOfInstUsed++;
    }
    /* Scan through all the channels provided in the Node instance and prepare CSIRX DRV instance data/cfg */
    for (chIdx = 0U ; chIdx < params->numCh ; chIdx++)
    {
        instId = tivxCaptureGetDrvInstIndex(prms, params->chInstMap[chIdx]);
        if (instId >= prms->numOfInstUsed)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Wrong Instance ID provided: %d !!!\n", params->chInstMap[chIdx]);
            break;
        }
        else
        {
            prms->instParams[instId].chVcMap[prms->instParams[instId].numCh] =
                                                        params->chVcNum[chIdx];
            prms->instParams[instId].numCh++;
        }
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        if ((vx_enum)TIVX_OBJ_DESC_RAW_IMAGE == (vx_enum)prms->img_obj_desc[0]->type)
        {
            raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[0];
            format = raw_image->params.format[0].pixel_container; /* TODO: Question: what should be done when this is different per exposure */
            width = raw_image->params.width;
            height = raw_image->params.height + (raw_image->params.meta_height_before + raw_image->params.meta_height_after);
            planes = raw_image->params.num_exposures;
            for (i = 0; i < planes; i++)
            {
                stride[i] = (uint32_t)raw_image->imagepatch_addr[i].stride_y;
            }
            prms->instParams[instId].raw_capture = 1U;
        }
        else
        {
            tivx_obj_desc_image_t *image;
            image = (tivx_obj_desc_image_t *)prms->img_obj_desc[0];
            format = image->format;
            width = image->imagepatch_addr[0].dim_x;
            height = image->imagepatch_addr[0].dim_y;
            planes = image->planes;
            for (i = 0; i < planes; i++)
            {
                stride[i] = (uint32_t)image->imagepatch_addr[i].stride_y;
            }
            prms->instParams[instId].raw_capture = 0U;
        }

        /* Copying timeout values from user to local structure */
        prms->timeout        = params->timeout;
        prms->timeoutInitial = params->timeoutInitial;

        /* Do following for each CSIRX DRV instance in the current Node */
        for (instIdx = 0U ; instIdx < params->numInst ; instIdx++)
        {
            prms->instParams[instIdx].captParams = prms;
            /* set instance configuration parameters */
            createParams = &prms->instParams[instIdx].createPrms;
            Csirx_createParamsInit(createParams);
            /* Set CSIRX D-PHY configuration parameters */
            Csirx_initDPhyCfg(&prms->instParams[instIdx].dphyCfg);
            prms->instParams[instIdx].dphyCfg.inst               = params->instId[instIdx];
            prms->instParams[instIdx].dphyCfg.rightLaneBandSpeed = params->instCfg[instIdx].laneBandSpeed;
            prms->instParams[instIdx].dphyCfg.leftLaneBandSpeed  = params->instCfg[instIdx].laneBandSpeed;

            /* set module configuration parameters */
            createParams->instCfg.enableCsiv2p0Support = params->instCfg[instIdx].enableCsiv2p0Support;
            createParams->instCfg.enableErrbypass      = (uint32_t)FALSE;
            createParams->instCfg.numPixelsStrm0       = params->instCfg[instIdx].numPixels;
            createParams->instCfg.enableStrm[CSIRX_CAPT_STREAM_ID] = 1U;
            createParams->instCfg.numDataLanes = params->instCfg[instIdx].numDataLanes;
            for (loopCnt = 0U ;
                 loopCnt < createParams->instCfg.numDataLanes ;
                 loopCnt++)
            {
                createParams->instCfg.dataLanesMap[loopCnt] = params->instCfg[instIdx].dataLanesMap[loopCnt];
            }

            createParams->numCh = prms->instParams[instIdx].numCh;
            for (loopCnt = 0U ; loopCnt < createParams->numCh ; loopCnt++)
            {
                createParams->chCfg[loopCnt].chId = loopCnt;
                createParams->chCfg[loopCnt].chType = CSIRX_CH_TYPE_CAPT;
                createParams->chCfg[loopCnt].vcNum = prms->instParams[instIdx].chVcMap[loopCnt];

                if ((uint32_t)TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[0]->type)
                {
                    if (NULL != raw_image)
                    {
                        createParams->chCfg[loopCnt].inCsiDataType =
                            tivxCaptureExtractInCsiDataTypeFromRawImg(raw_image);
                    }
                }
                else
                {
                    createParams->chCfg[loopCnt].inCsiDataType =
                        tivxCaptureExtractInCsiDataType(format);
                }
                if (CAPTURE_IN_CSI_DT_INVALID ==
                                    createParams->chCfg[loopCnt].inCsiDataType)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR,
                        " CAPTURE: ERROR: Un-supported Capture Data-type!!!\n");
                    break;
                }

                createParams->chCfg[loopCnt].outFmt.width =
                    width;
                createParams->chCfg[loopCnt].outFmt.height =
                    height;
                for (i = 0; i < planes; i ++)
                {
                    createParams->chCfg[loopCnt].outFmt.pitch[i] =
                        stride[i];
                }

                createParams->chCfg[loopCnt].outFmt.dataFormat =
                    tivxCaptureExtractDataFormat(format);
                createParams->chCfg[loopCnt].outFmt.ccsFormat =
                    tivxCaptureExtractCcsFormat(format);
            }
            /* set frame drop buffer parameters */
            createParams->frameDropBufLen = CAPTURE_FRAME_DROP_LEN;
            createParams->frameDropBuf = (uint64_t)tivxMemAlloc(createParams->frameDropBufLen, (vx_enum)TIVX_MEM_EXTERNAL);

            if (0 == createParams->frameDropBuf)
            {
                status = VX_ERROR_NO_MEMORY;
                VX_PRINT(VX_ZONE_ERROR,
                    " CAPTURE: ERROR: Insufficient memory for frameDropBuf!!!\n");
                break;
            }
        }
    }

    tivxCheckStatus(&status, tivxMemBufferUnmap(capture_config_target_ptr,
       obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
       (vx_enum)VX_READ_ONLY));

    return status;
}

static vx_status tivxCaptureStart(tivxCaptureParams *prms)
{
    vx_status status = VX_SUCCESS;
    uint32_t instIdx;
    int32_t fvid2_status = FVID2_SOK;

    if (0U == prms->steady_state_started)
    {
        /* start all driver instances in the node */
        for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
        {
            fvid2_status = Fvid2_start(prms->instParams[instIdx].drvHandle, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could not start FVID2 !!!\n");
                break;
            }
        }
    }

    return status;
}

/* Setting timeout based on capture params and whether user has provided error frame */
static void tivxCaptureSetTimeout(tivxCaptureParams *prms)
{
    /* Only apply timeout if error frame has been sent; otherwise wait forever */
    if (1U == prms->enableErrorFrameTimeout)
    {
        /* Using timeoutInitial if all channels are active, else using timeout */
        if ( ((1<<(prms->numCh))-1) == prms->activeChannelMask)
        {
            prms->timeoutRemaining = prms->timeoutInitial;
        }
        else
        {
            prms->timeoutRemaining = prms->timeout;
        }
    }
    else
    {
        prms->timeoutRemaining = TIVX_EVENT_TIMEOUT_WAIT_FOREVER;

        if (0U == prms->steady_state_started)
        {
            /* Notifying user with a warning that the error frame was not provided if the timeout is not FOREVER */
            if ( (TIVX_EVENT_TIMEOUT_WAIT_FOREVER != prms->timeoutInitial) ||
                 (TIVX_EVENT_TIMEOUT_WAIT_FOREVER != prms->timeout) )
            {
                VX_PRINT(VX_ZONE_WARNING, " CAPTURE: WARNING: Error frame not provided using tivxCaptureRegisterErrorFrame, defaulting to waiting forever !!!\n");
            }
        }
    }
}

/* Determines if a frame has been received from each active channel */
static uint32_t tivxCaptureIsAllChFrameAvailable(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        uint8_t timeoutExceeded)
{
    uint32_t is_all_ch_frame_available = 1;
    vx_uint32 chId = 0U;

    /* Initial loop through channel to check for inactive and active channels */
    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        tivxQueuePeek(&prms->pendingFrameQ[chId], (uintptr_t*)&recv_obj_desc_id[chId]);

        if (NULL==recv_obj_desc_id[chId])
        {
            /* Handle case that capture node timed out; set associated bit in activeChannelMask to 0 */
            if (CAPTURE_TIMEOUT_EXCEEDED == timeoutExceeded)
            {
                prms->activeChannelMask &= ~(1<<chId);
                VX_PRINT(VX_ZONE_INFO,
                    " Channel %d not received!!!\n", chId);
            }
        }
        else
        {
            /* Handle the case that a camera came back up and set associated bit in activeChannelMask to 1 */
            prms->activeChannelMask |= (1<<chId);
        }
    }

    /* Loop through channels when capture timeout has not been exceeded to
     * check if all frames have been received */
    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        tivxQueuePeek(&prms->pendingFrameQ[chId], (uintptr_t*)&recv_obj_desc_id[chId]);

        /* Handle case that capture node timed out */
        if (CAPTURE_TIMEOUT_VALID == timeoutExceeded)
        {
            /* Marks that a frame is not available only if this is an active channel; i.e., channel has died
             * or if no channel are active */
            if( ( (NULL==recv_obj_desc_id[chId]) &&
                  ((1<<chId) & prms->activeChannelMask) ) ||
                (0U == prms->activeChannelMask) )
            {
                is_all_ch_frame_available = 0;
            }
        }
    }

    return is_all_ch_frame_available;
}

static vx_status tivxCaptureDequeueFrameFromDriver(tivxCaptureParams *prms)
{
    vx_status status = VX_SUCCESS;
    uint32_t instIdx, tmp_obj_desc_id = 0U, tmp_timestamp_lo = 0U, tmp_timestamp_hi = 0U;
    uint64_t tmp_timestamp = 0U;
    tivxCaptureInstParams *instParams;
    Fvid2_Frame *fvid2Frame;
    vx_uint32 frmIdx = 0U, chId = 0U;
    int32_t fvid2_status = FVID2_SOK;
    Fvid2_FrameList *frmList;

    frmList = &prms->frmList;
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];

        frmList->numFrames = 0;
        fvid2_status = Fvid2_dequeue(instParams->drvHandle,
                                     frmList,
                                     0,
                                     FVID2_TIMEOUT_NONE);

        if(FVID2_SOK == fvid2_status)
        {
            for(frmIdx=0; frmIdx < frmList->numFrames; frmIdx++)
            {
                fvid2Frame = frmList->frames[frmIdx];
                chId = tivxCaptureGetNodeChannelNum(
                                    prms,
                                    instIdx,
                                    fvid2Frame->chNum);

                tmp_obj_desc_id = (uint32_t)fvid2Frame->appData;
                tmp_timestamp = fvid2Frame->timeStamp64;

                tivx_uint64_to_uint32(
                    tmp_timestamp,
                    &tmp_timestamp_hi,
                    &tmp_timestamp_lo
                );

                tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);
                tivxQueuePut(&prms->pendingFrameQ[chId], (uintptr_t)tmp_obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
                tivxQueuePut(&prms->pendingFrameTimestampLoQ[chId], tmp_timestamp_lo, TIVX_EVENT_TIMEOUT_NO_WAIT);
                tivxQueuePut(&prms->pendingFrameTimestampHiQ[chId], tmp_timestamp_hi, TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
        }
        else if (fvid2_status == FVID2_ENO_MORE_BUFFERS)
        {
            /* continue: move onto next driver instance
              within node as current driver instance did
              not generate this CB */
        }
        else
        {
            /* TIOVX-687: Note: disabling for now until investigated further */
            if (FVID2_EAGAIN != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR,
                    " CAPTURE: ERROR: FVID2 Dequeue failed !!!\n");
            }
        }
    }

    return status;
}

/* Populating capture output with object descriptors based on if camera is enabled */
static void tivxCaptureGetObjDesc(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        tivx_obj_desc_object_array_t *output_desc,
        uint64_t *timestamp)
{
    uint32_t tmp_timestamp_hi = 0U, tmp_timestamp_lo = 0U;
    uint64_t tmp_timestamp = 0U;
    vx_uint32 chId = 0U;

    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        tivxQueueGet(&prms->pendingFrameQ[chId], (uintptr_t*)&recv_obj_desc_id[chId], TIVX_EVENT_TIMEOUT_NO_WAIT);
    }

    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        uint32_t tmp_desc_id_32;
        uint16_t tmp_obj_desc_16;
        tivx_obj_desc_t *tmp_obj_desc;

        if (NULL!=recv_obj_desc_id[chId])
        {
            tmp_desc_id_32 = (uint32_t)recv_obj_desc_id[chId];
        }
        else
        {
            tivxQueueGet(&prms->errorFrameQ[chId], (uintptr_t*)&tmp_desc_id_32, TIVX_EVENT_TIMEOUT_NO_WAIT);
        }

        tmp_obj_desc_16 = (uint16_t)tmp_desc_id_32;
        tivxGetObjDescList(&tmp_obj_desc_16, &tmp_obj_desc, 1);

        output_desc->obj_desc_id[chId] = (uint16_t)tmp_obj_desc_16;

        if(tmp_obj_desc!=NULL)
        {
            tmp_obj_desc->scope_obj_desc_id = (uint16_t)output_desc->base.obj_desc_id;
            tivxQueueGet(&prms->pendingFrameTimestampLoQ[chId], (uintptr_t*)&tmp_timestamp_lo, TIVX_EVENT_TIMEOUT_NO_WAIT);
            tivxQueueGet(&prms->pendingFrameTimestampHiQ[chId], (uintptr_t*)&tmp_timestamp_hi, TIVX_EVENT_TIMEOUT_NO_WAIT);

            tivx_uint32_to_uint64(
                    &tmp_timestamp,
                    tmp_timestamp_hi,
                    tmp_timestamp_lo
                );

            tmp_obj_desc->timestamp = tmp_timestamp;

            /* Setting the timestamp for object array to largest value of object array elements */
            if (tmp_timestamp > *timestamp)
            {
                *timestamp = tmp_timestamp;
            }
        }
    }
}

static vx_status VX_CALLBACK tivxCaptureProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxCaptureParams *prms = NULL;
    tivx_obj_desc_object_array_t *output_desc;
    vx_uint32 size, chId = 0U;
    vx_enum state;
    uint64_t timestamp = 0U;
    uint8_t timeoutExceeded = CAPTURE_TIMEOUT_VALID;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCaptureParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            status = tivxGetTargetKernelInstanceState(kernel, &state);
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        /* Steady state: receives a buffer and returns a buffer */
        if ((vx_enum)VX_NODE_STATE_STEADY == state)
        {
            /* Providing buffers to capture source */
            status = tivxCaptureEnqueueFrameToDriver(output_desc, prms);

            if ((vx_status)VX_SUCCESS != status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Enqueue Frame to Driver failed !!!\n");
            }

            /* Starts FVID2 on initial frame */
            if ((vx_status)VX_SUCCESS == status)
            {
                status = tivxCaptureStart(prms);
            }

            /* Pends until a frame is available then dequeue frames from capture driver */
            if ((vx_status)VX_SUCCESS == status)
            {
                uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH];
                tivx_obj_desc_object_array_t *recv_obj_arr_desc;

                uint32_t is_all_ch_frame_available = 0;

                for(chId = 0U ; chId < TIVX_CAPTURE_MAX_CH ; chId++)
                {
                    recv_obj_desc_id[chId] = NULL;
                }

                tivxCaptureSetTimeout(prms);

                while(is_all_ch_frame_available == 0U)
                {
                    is_all_ch_frame_available = tivxCaptureIsAllChFrameAvailable(prms, recv_obj_desc_id, timeoutExceeded);

                    if(is_all_ch_frame_available == 0U)
                    {
                        status |= tivxCaptureTimeout(prms);

                        if (status != VX_SUCCESS)
                        {
                            prms->timeoutRemaining = 0;
                            timeoutExceeded = CAPTURE_TIMEOUT_EXCEEDED;
                        }
                        else
                        {
                            status = tivxCaptureDequeueFrameFromDriver(prms);
                        }
                    }
                }

                /* Getting next obj arr obj desc from queue to populate with the latest dequeued frames */
                tivxQueueGet(&prms->pendingObjArrayQ, (uintptr_t*)&recv_obj_arr_desc, TIVX_EVENT_TIMEOUT_NO_WAIT);

                obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX] = (tivx_obj_desc_t *)recv_obj_arr_desc;

                output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

                tivxCaptureGetObjDesc(prms, recv_obj_desc_id, output_desc, &timestamp);

                obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX]->timestamp = timestamp;

                if (0U == prms->steady_state_started)
                {
                    prms->steady_state_started = 1;
                }
            }
        }
        /* Pipe-up state: only receives a buffer; does not return a buffer */
        else
        {
            status = tivxCaptureEnqueueFrameToDriver(output_desc, prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxCaptureCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivx_obj_desc_user_data_object_t *input_obj_desc;
    tivx_obj_desc_object_array_t *output_desc;
    tivxCaptureParams *prms = NULL;
    uint32_t chId, bufId, instIdx;
    tivxCaptureInstParams *instParams = NULL;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        input_obj_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX];
        output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxCaptureParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxCaptureParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could allocate memory !!!\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Initialize steady_state_started to 0 */
            prms->steady_state_started = 0;

            /* Black frame has not been sent */
            prms->enableErrorFrameTimeout = 0U;

            /* Initialize raw capture to 0 */
            for (instIdx = 0U ; instIdx < TIVX_CAPTURE_MAX_INST ; instIdx++)
            {
                prms->instParams[instIdx].raw_capture = 0;
            }

            /* Set number of channels to number of items in object array */
            prms->numCh = (uint8_t)output_desc->num_items;

            prms->activeChannelMask = (1<<(prms->numCh))-1;

            if (prms->numCh > TIVX_CAPTURE_MAX_CH)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Object descriptor number of channels exceeds max value allowed by capture!!!\r\n");
            }
        }

        /* Setting CSIRX capture parameters */
        if ((vx_status)VX_SUCCESS == status)
        {
            tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                           prms->numCh);

            status = tivxCaptureSetCreateParams(prms, input_obj_desc);
        }

        /* Creating frame available event */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxEventCreate(&prms->frame_available);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Event creation failed in capture!!!\r\n");
            }
        }

        /* Creating FVID2 handle */
        if ((vx_status)VX_SUCCESS == status)
        {
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                Fvid2CbParams_init(&instParams->drvCbPrms);

                instParams->drvCbPrms.cbFxn   = (Fvid2_CbFxn) &captDrvCallback;
                instParams->drvCbPrms.appData = prms;

                instParams->drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                                                     instParams->instId,
                                                     &instParams->createPrms,
                                                     &instParams->createStatus,
                                                     &instParams->drvCbPrms);

                if ((NULL == instParams) ||
                    (NULL == instParams->drvHandle) ||
                    (instParams->createStatus.retVal != FVID2_SOK))
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture Create Failed!!!\r\n");
                    status = (vx_status)VX_FAILURE;
                }
                else
                {
                    fvid2_status = Fvid2_control(
                        instParams->drvHandle, IOCTL_CSIRX_SET_DPHY_CONFIG,
                        &instParams->dphyCfg, NULL);
                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, ": Failed to set PHY Parameters!!!\r\n");
                    }
                    else
                    {
                        /* Register Error Events */
                        Csirx_EventPrms eventPrms;
                        Csirx_eventPrmsInit(&eventPrms);
                        fvid2_status = Fvid2_control(instParams->drvHandle,
                                               IOCTL_CSIRX_REGISTER_EVENT,
                                               &eventPrms,
                                               NULL);
                        if (FVID2_SOK != fvid2_status)
                        {
                            status = (vx_status)VX_FAILURE;
                            VX_PRINT(VX_ZONE_ERROR, ": Failed to set Event Parameters!!!\r\n");
                        }
                    }
                }
            }
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != instParams))
        {
            Fvid2_TimeStampParams tsParams;

            tsParams.timeStampFxn = (Fvid2_TimeStampFxn)&tivxPlatformGetTimeInUsecs;
            /* register time stamping function */
            fvid2_status = Fvid2_control(instParams->drvHandle,
                                   FVID2_REGISTER_TIMESTAMP_FXN,
                                   &tsParams,
                                   NULL);

            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, ": Failed to set PHY Parameters!!!\r\n");
            }
        }

        /* Creating FVID2 frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0u ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->freeFvid2FrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->fvid2_free_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture queue create failed!!!\r\n");
                    break;
                }

                for(bufId = 0u ; bufId < (TIVX_CAPTURE_MAX_NUM_BUFS) ; bufId++)
                {
                    tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)&prms->fvid2Frames[chId][bufId], TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
            }
        }

        /* Creating pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->pendingFrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_free_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
            }
        }

        /* Creating pending frame obj arr Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxQueueCreate(&prms->pendingObjArrayQ, TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_obj_arr_q_mem, 0);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
            }
        }

        /* TODO: Should there be a flag to determine whether or not to create this? */
        /* Creating pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->errorFrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->error_frame_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
            }
        }

        /* Creating pending timestamp Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->pendingFrameTimestampLoQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_timestamp_lo_free_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
            }
        }

        /* Creating pending timestamp Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->pendingFrameTimestampHiQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_timestamp_hi_free_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms, sizeof(tivxCaptureParams));
        }
        else if (NULL != prms)
        {
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                if (NULL != instParams->drvHandle)
                {
                    /* Disable Error Events */
                    fvid2_status = Fvid2_control(instParams->drvHandle,
                                           IOCTL_CSIRX_UNREGISTER_EVENT,
                                           (void *)CSIRX_EVENT_GROUP_ERROR,
                                           NULL);
                    if(FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, ": Capture Event Unregister failed!!!\r\n");
                    }
                    Fvid2_delete(instParams->drvHandle, NULL);
                    instParams->drvHandle = NULL;
                }

                /* Freeing memory used for frame drop buf */
                if ((vx_status)VX_SUCCESS == status)
                {
                    Csirx_CreateParams *createParams;

                    createParams = &instParams->createPrms;

                    tivxMemFree((void*)(uint32_t)createParams->frameDropBuf, createParams->frameDropBufLen, (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }

            if (NULL != prms->frame_available)
            {
                tivxEventDelete(&prms->frame_available);
            }

            tivxMemFree(prms, sizeof(tivxCaptureParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
        else
        {
            /* do nothing */
        }
    }

    return status;
}

static void tivxCapturePrintStatus(tivxCaptureInstParams *prms)
{
    int32_t fvid2_status;
    uint32_t cnt;

    if (NULL != prms)
    {
        fvid2_status = Fvid2_control(prms->drvHandle,
                                IOCTL_CSIRX_GET_INST_STATUS,
                                &prms->captStatus,
                                NULL);
        if (FVID2_SOK == fvid2_status)
        {
            printf(   "==========================================================\r\n");
            printf(   " Capture Status: Instance|%d\r\n", prms->instId);
            printf(   "==========================================================\r\n");
            printf(   " overflowCount: %d\r\n", prms->captStatus.overflowCount);
            printf(   " spuriousUdmaIntrCount: %d\r\n", prms->captStatus.spuriousUdmaIntrCount);
            printf(   " frontFIFOOvflCount: %d\r\n", prms->captStatus.frontFIFOOvflCount);
            printf(   " crcCount: %d\r\n", prms->captStatus.crcCount);
            printf(   " eccCount: %d\r\n", prms->captStatus.eccCount);
            printf(   " correctedEccCount: %d\r\n", prms->captStatus.correctedEccCount);
            printf(   " dataIdErrorCount: %d\r\n", prms->captStatus.dataIdErrorCount);
            printf(   " invalidAccessCount: %d\r\n", prms->captStatus.invalidAccessCount);
            printf(   " invalidSpCount: %d\r\n", prms->captStatus.invalidSpCount);
            for(cnt = 0U ; cnt < CSIRX_NUM_STREAM ; cnt ++)
            {
                printf(   " strmFIFOOvflCount[%d]: %d\r\n", cnt, prms->captStatus.strmFIFOOvflCount[cnt]);
            }
            printf(   " Channel Num | Frame Queue Count | Frame De-queue Count | Frame Drop Count | Error Frame Count |\r\n");
            for(cnt = 0U ; cnt < prms->numCh ; cnt ++)
            {
                printf(
                      " %11d | %17d | %20d | %16d | %17d |\r\n",
                      cnt,
                      prms->captStatus.queueCount[cnt],
                      prms->captStatus.dequeueCount[cnt],
                      prms->captStatus.dropCount[cnt],
                      prms->captStatus.errorFrameCount[cnt]);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Control failed !!!\n");
        }
    }
}

static vx_status VX_CALLBACK tivxCaptureDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivxCaptureParams *prms = NULL;
    Fvid2_FrameList *frmList;
    uint32_t size, chId, bufId, instIdx;
    tivxCaptureInstParams *instParams;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could not obtain kernel instance context !!!\n");
        }

        if(NULL == prms)
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel instance context is NULL!!!\n");
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            frmList = &prms->frmList;
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                /* Stopping FVID2 Capture */
                if ((vx_status)VX_SUCCESS == status)
                {
                    fvid2_status = Fvid2_stop(instParams->drvHandle, NULL);

                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Capture not stopped !!!\n");
                    }
                }

                /* Dequeue all the request from the driver */
                if ((vx_status)VX_SUCCESS == status)
                {
                    Fvid2FrameList_init(frmList);
                    do
                    {
                        fvid2_status = Fvid2_dequeue(
                            instParams->drvHandle,
                            frmList,
                            0,
                            FVID2_TIMEOUT_NONE);
                    } while (FVID2_SOK == fvid2_status);

                    if (FVID2_ENO_MORE_BUFFERS != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Capture Dequeue Failed !!!\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    tivxCapturePrintStatus(instParams);
                }

                /* Freeing memory used for frame drop buf */
                if ((vx_status)VX_SUCCESS == status)
                {
                    Csirx_CreateParams *createParams;

                    createParams = &instParams->createPrms;

                    tivxMemFree((void*)(uint32_t)createParams->frameDropBuf, createParams->frameDropBufLen, (vx_enum)TIVX_MEM_EXTERNAL);
                }

                /* Disable Error Events */
                fvid2_status = Fvid2_control(instParams->drvHandle,
                                       IOCTL_CSIRX_UNREGISTER_EVENT,
                                       (void *)CSIRX_EVENT_GROUP_ERROR,
                                       NULL);
                if(FVID2_SOK != fvid2_status)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, ": Capture Event Unregister failed!!!\r\n");
                }
                /* Deleting FVID2 handle */
                if ((vx_status)VX_SUCCESS == status)
                {
                    fvid2_status = Fvid2_delete(instParams->drvHandle, NULL);

                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Delete Failed !!!\n");
                    }
                }

                /* Free-ing kernel instance params */
                if ( (vx_status)VX_SUCCESS == status)
                {
                    instParams->drvHandle = NULL;
                }
            }
        }

        /* Deleting FVID2 frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U; chId < prms->numCh ; chId++)
            {
                tivxQueueDelete(&prms->freeFvid2FrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                tivxQueueDelete(&prms->pendingFrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            tivxQueueDelete(&prms->pendingObjArrayQ);
        }

        /* Freeing error object descriptors if they have been allocated */
        if (((vx_status)VX_SUCCESS == status) &&
            (1U == prms->enableErrorFrameTimeout))
        {
            for (chId = 0U; chId < prms->numCh; chId++)
            {
                for (bufId = 0U; bufId < TIVX_CAPTURE_MAX_NUM_BUFS; bufId++)
                {
                    if(prms->error_obj_desc[chId][bufId]!=NULL)
                    {
                        status = ownObjDescFree((tivx_obj_desc_t**)&prms->error_obj_desc[chId][bufId]);
                    }

                    if ((vx_status)VX_SUCCESS != status)
                    {
                        break;
                    }
                }
            }
        }

        /* Deleting pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                tivxQueueDelete(&prms->errorFrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                tivxQueueDelete(&prms->pendingFrameTimestampLoQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                tivxQueueDelete(&prms->pendingFrameTimestampHiQ[chId]);
            }
        }

        /* Deleting event */
        if ((vx_status)VX_SUCCESS == status)
        {
            tivxEventDelete(&prms->frame_available);
        }

        if (sizeof(tivxCaptureParams) == size)
        {
            tivxMemFree(prms, sizeof(tivxCaptureParams), (vx_status)TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

static void tivxCaptureCopyStatistics(tivxCaptureParams *prms,
    tivx_capture_statistics_t *capt_status_prms)
{
    uint32_t i, instIdx, strmIdx;
    tivxCaptureInstParams *instParams;

    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];
        for (i = 0U ; i < instParams->numCh ; i++)
        {
            capt_status_prms->queueCount[instIdx][i]      = instParams->captStatus.queueCount[i];
            capt_status_prms->dequeueCount[instIdx][i]    = instParams->captStatus.dequeueCount[i];
            capt_status_prms->dropCount[instIdx][i]       = instParams->captStatus.dropCount[i];
            capt_status_prms->errorFrameCount[instIdx][i] = instParams->captStatus.errorFrameCount[i];
        }
        capt_status_prms->overflowCount[instIdx]         = instParams->captStatus.overflowCount;
        capt_status_prms->spuriousUdmaIntrCount[instIdx] = instParams->captStatus.spuriousUdmaIntrCount;
        capt_status_prms->frontFIFOOvflCount[instIdx]    = instParams->captStatus.frontFIFOOvflCount;
        capt_status_prms->crcCount[instIdx]              = instParams->captStatus.crcCount;
        capt_status_prms->eccCount[instIdx]              = instParams->captStatus.eccCount;
        capt_status_prms->correctedEccCount[instIdx]     = instParams->captStatus.correctedEccCount;
        capt_status_prms->dataIdErrorCount[instIdx]      = instParams->captStatus.dataIdErrorCount;
        capt_status_prms->invalidAccessCount[instIdx]    = instParams->captStatus.invalidAccessCount;
        capt_status_prms->invalidSpCount[instIdx]        = instParams->captStatus.invalidSpCount;
        for (strmIdx = 0U ; strmIdx < TIVX_CAPTURE_MAX_STRM ; strmIdx++)
        {
            capt_status_prms->strmFIFOOvflCount[instIdx][strmIdx] =
                            instParams->captStatus.strmFIFOOvflCount[strmIdx];
        }
    }
    capt_status_prms->activeChannelMask  = prms->activeChannelMask;
}

static vx_status tivxCaptureGetStatistics(tivxCaptureParams *prms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                             status = (vx_status)VX_SUCCESS;
    tivx_capture_statistics_t                 *capt_status_prms = NULL;
    void                                  *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        if (sizeof(tivx_capture_statistics_t) ==
                usr_data_obj->mem_size)
        {
            capt_status_prms = (tivx_capture_statistics_t *)target_ptr;

            tivxCaptureCopyStatistics(prms, capt_status_prms);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxCaptureAllocErrorDesc(tivxCaptureParams *prms,
    tivx_obj_desc_t *obj_desc)
{
    vx_status                             status = (vx_status)VX_SUCCESS;
    uint16_t obj_desc_id = 0U;
    vx_reference ref = NULL;
    uint64_t ref64 = 0;
    uint32_t bufId, chId, planeId;

    ref64 = ownReferenceGetHostRefFromObjDescId(obj_desc->obj_desc_id);
    tivxFlagBitSet(&obj_desc->flags, TIVX_REF_FLAG_IS_INVALID);

    /* Allocate object descriptors */
    for (chId = 0U; chId < prms->numCh; chId++)
    {
        for (bufId = 0U; bufId < TIVX_CAPTURE_MAX_NUM_BUFS; bufId++)
        {
            /* Passing a NULL pointer as "ref" then overwriting it the next line w/ the 64 bit value */
            prms->error_obj_desc[chId][bufId] = ownObjDescAlloc((vx_enum)obj_desc->type, ref);

            /* Since vx_reference is a 32 bit address, this needs to use the 64 bit value of the host_ref */
            prms->error_obj_desc[chId][bufId]->host_ref = ref64;

            if (NULL != prms->error_obj_desc[chId][bufId])
            {
                tivxFlagBitSet(&prms->error_obj_desc[chId][bufId]->flags, TIVX_REF_FLAG_IS_INVALID);

                if ((uint32_t)TIVX_OBJ_DESC_RAW_IMAGE == (vx_enum)obj_desc->type)
                {
                    tivx_obj_desc_raw_image_t *tmp_raw_image;
                    tivx_obj_desc_raw_image_t *ref_raw_image;

                    tmp_raw_image = (tivx_obj_desc_raw_image_t *)prms->error_obj_desc[chId][bufId];
                    ref_raw_image = (tivx_obj_desc_raw_image_t *)obj_desc;

                    tmp_raw_image->params.num_exposures      = ref_raw_image->params.num_exposures;
                    tmp_raw_image->params.width              = ref_raw_image->params.width;
                    tmp_raw_image->params.height             = ref_raw_image->params.height;
                    tmp_raw_image->params.line_interleaved   = ref_raw_image->params.line_interleaved;
                    tmp_raw_image->params.meta_height_before = ref_raw_image->params.meta_height_before;
                    tmp_raw_image->params.meta_height_after  = ref_raw_image->params.meta_height_after;

                    tmp_raw_image->create_type       = ref_raw_image->create_type;

                    for (planeId = 0U; planeId < tmp_raw_image->params.num_exposures; planeId++)
                    {
                        tmp_raw_image->mem_ptr[planeId].host_ptr        = ref_raw_image->mem_ptr[planeId].host_ptr;
                        tmp_raw_image->mem_ptr[planeId].mem_heap_region = ref_raw_image->mem_ptr[planeId].mem_heap_region;
                        tmp_raw_image->mem_ptr[planeId].shared_ptr      = ref_raw_image->mem_ptr[planeId].shared_ptr;
                        tmp_raw_image->mem_ptr[planeId].dma_buf_fd      = ref_raw_image->mem_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->img_ptr[planeId].host_ptr        = ref_raw_image->img_ptr[planeId].host_ptr;
                        tmp_raw_image->img_ptr[planeId].mem_heap_region = ref_raw_image->img_ptr[planeId].mem_heap_region;
                        tmp_raw_image->img_ptr[planeId].shared_ptr      = ref_raw_image->img_ptr[planeId].shared_ptr;
                        tmp_raw_image->img_ptr[planeId].dma_buf_fd      = ref_raw_image->img_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->meta_before_ptr[planeId].host_ptr        = ref_raw_image->meta_before_ptr[planeId].host_ptr;
                        tmp_raw_image->meta_before_ptr[planeId].mem_heap_region = ref_raw_image->meta_before_ptr[planeId].mem_heap_region;
                        tmp_raw_image->meta_before_ptr[planeId].shared_ptr      = ref_raw_image->meta_before_ptr[planeId].shared_ptr;
                        tmp_raw_image->meta_before_ptr[planeId].dma_buf_fd      = ref_raw_image->meta_before_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->meta_after_ptr[planeId].host_ptr        = ref_raw_image->meta_after_ptr[planeId].host_ptr;
                        tmp_raw_image->meta_after_ptr[planeId].mem_heap_region = ref_raw_image->meta_after_ptr[planeId].mem_heap_region;
                        tmp_raw_image->meta_after_ptr[planeId].shared_ptr      = ref_raw_image->meta_after_ptr[planeId].shared_ptr;
                        tmp_raw_image->meta_after_ptr[planeId].dma_buf_fd      = ref_raw_image->meta_after_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->params.format[planeId]            = ref_raw_image->params.format[planeId];
                        tmp_raw_image->mem_size[planeId]                 = ref_raw_image->mem_size[planeId];
                        tmp_raw_image->imagepatch_addr[planeId].dim_x    = ref_raw_image->imagepatch_addr[planeId].dim_x;
                        tmp_raw_image->imagepatch_addr[planeId].dim_y    = ref_raw_image->imagepatch_addr[planeId].dim_y;
                        tmp_raw_image->imagepatch_addr[planeId].stride_x = ref_raw_image->imagepatch_addr[planeId].stride_x;
                        tmp_raw_image->imagepatch_addr[planeId].stride_y = ref_raw_image->imagepatch_addr[planeId].stride_y;
                        tmp_raw_image->imagepatch_addr[planeId].scale_x  = ref_raw_image->imagepatch_addr[planeId].scale_x;
                        tmp_raw_image->imagepatch_addr[planeId].scale_y  = ref_raw_image->imagepatch_addr[planeId].scale_y;
                        tmp_raw_image->imagepatch_addr[planeId].step_x   = ref_raw_image->imagepatch_addr[planeId].step_x;
                        tmp_raw_image->imagepatch_addr[planeId].step_y   = ref_raw_image->imagepatch_addr[planeId].step_y;
                    }
                }
                else
                {
                    tivx_obj_desc_image_t *tmp_image;
                    tivx_obj_desc_image_t *ref_image;

                    tmp_image = (tivx_obj_desc_image_t *)prms->error_obj_desc[chId][bufId];
                    ref_image   = (tivx_obj_desc_image_t *)obj_desc;

                    tmp_image->planes        = ref_image->planes;
                    tmp_image->uniform_image_pixel_value = ref_image->uniform_image_pixel_value;
                    tmp_image->width                     = ref_image->width;
                    tmp_image->height                    = ref_image->height;
                    tmp_image->format                    = ref_image->format;
                    tmp_image->color_range               = ref_image->color_range;

                    tmp_image->valid_roi.start_x = ref_image->valid_roi.start_x;
                    tmp_image->valid_roi.start_y = ref_image->valid_roi.start_x;
                    tmp_image->valid_roi.end_x   = ref_image->valid_roi.end_x;
                    tmp_image->valid_roi.end_y   = ref_image->valid_roi.end_y;
                    tmp_image->color_space       = ref_image->color_space;
                    tmp_image->create_type       = ref_image->create_type;

                    for (planeId = 0U; planeId < ref_image->planes; planeId++)
                    {
                        tmp_image->mem_ptr[planeId].host_ptr        = ref_image->mem_ptr[planeId].host_ptr;
                        tmp_image->mem_ptr[planeId].mem_heap_region = ref_image->mem_ptr[planeId].mem_heap_region;
                        tmp_image->mem_ptr[planeId].shared_ptr      = ref_image->mem_ptr[planeId].shared_ptr;
                        tmp_image->mem_ptr[planeId].dma_buf_fd      = ref_image->mem_ptr[planeId].dma_buf_fd;
                        tmp_image->mem_size[planeId]                = ref_image->mem_size[planeId];

                        tmp_image->imagepatch_addr[planeId].dim_x    = ref_image->imagepatch_addr[planeId].dim_x;
                        tmp_image->imagepatch_addr[planeId].dim_y    = ref_image->imagepatch_addr[planeId].dim_y;
                        tmp_image->imagepatch_addr[planeId].stride_x = ref_image->imagepatch_addr[planeId].stride_x;
                        tmp_image->imagepatch_addr[planeId].stride_y = ref_image->imagepatch_addr[planeId].stride_y;
                        tmp_image->imagepatch_addr[planeId].scale_x  = ref_image->imagepatch_addr[planeId].scale_x;
                        tmp_image->imagepatch_addr[planeId].scale_y  = ref_image->imagepatch_addr[planeId].scale_y;
                        tmp_image->imagepatch_addr[planeId].step_x   = ref_image->imagepatch_addr[planeId].step_x;
                        tmp_image->imagepatch_addr[planeId].step_y   = ref_image->imagepatch_addr[planeId].step_y;
                    }
                }

                obj_desc_id = prms->error_obj_desc[chId][bufId]->obj_desc_id;
                tivxQueuePut(&prms->errorFrameQ[chId], (uintptr_t)obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Object descriptor allocation failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        prms->enableErrorFrameTimeout = 1U;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxCaptureControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t             size, instIdx;
    tivxCaptureParams *prms = NULL;
    tivxCaptureInstParams *instParams;

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == prms) ||
        (sizeof(tivxCaptureParams) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing */
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_CAPTURE_PRINT_STATISTICS:
            {
                for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                {
                    instParams = &prms->instParams[instIdx];
                    tivxCapturePrintStatus(instParams);
                }
                break;
            }
            case TIVX_CAPTURE_GET_STATISTICS:
            {
                if (NULL != obj_desc[0])
                {
                    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                    {
                        instParams = &prms->instParams[instIdx];
                        fvid2_status = Fvid2_control(instParams->drvHandle,
                                                IOCTL_CSIRX_GET_INST_STATUS,
                                                &instParams->captStatus,
                                                NULL);
                        if (FVID2_SOK != fvid2_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Get status returned failure\n");
                            status = (vx_status)VX_FAILURE;
                            break;
                        }
                    }
                    if ((vx_status)VX_SUCCESS == status)
                    {
                        status = tivxCaptureGetStatistics(prms,
                            (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                        if ((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Get status failed\n");
                            status = (vx_status)VX_FAILURE;
                        }

                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "User data object was NULL\n");
                    status = (vx_status)VX_FAILURE;
                }
                break;
            }
            case TIVX_CAPTURE_REGISTER_ERROR_FRAME:
            {
                if ( NULL != obj_desc[0] )
                {
                    if (0U == prms->enableErrorFrameTimeout)
                    {
                        status = tivxCaptureAllocErrorDesc(prms, obj_desc[0U]);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Reference frame already provided\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Provided reference was NULL\n");
                    status = (vx_status)VX_FAILURE;
                }
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }
    return status;
}

void tivxAddTargetKernelCapture(void)
{
    vx_enum self_cpu;
    vx_uint32 i = 0;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        for (i = 0; i < CAPTURE_NUM_TARGETS; i++)
        {
            vx_capture_target_kernel[i] = tivxAddTargetKernelByName(
                                TIVX_KERNEL_CAPTURE_NAME,
                                target_name[i],
                                tivxCaptureProcess,
                                tivxCaptureCreate,
                                tivxCaptureDelete,
                                tivxCaptureControl,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelCapture(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i = 0;

    for (i = 0; i < CAPTURE_NUM_TARGETS; i++)
    {
        status = tivxRemoveTargetKernel(vx_capture_target_kernel[i]);
        if(status == (vx_status)VX_SUCCESS)
        {
            vx_capture_target_kernel[i] = NULL;
        }
    }
}

static void tivxCaptureGetChannelIndices(const tivxCaptureParams *prms,
                                         uint32_t instId,
                                         uint32_t *startChIdx,
                                         uint32_t *endChIdx)
{
    uint32_t instIdx;

    *startChIdx = 0U;
    *endChIdx   = 0U;
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        /* get start channel ID here */
        if (instIdx == instId)
        {
            break;
        }
        else
        {
            *startChIdx += prms->instParams[instIdx].numCh;
        }
    }
    /* Get last channel ID here */
    if (instIdx < prms->numOfInstUsed)
    {
        *endChIdx = *startChIdx + prms->instParams[instIdx].numCh;
    }
}

static uint32_t tivxCaptureGetNodeChannelNum(const tivxCaptureParams *prms,
                                             uint32_t instId,
                                             uint32_t chId)
{
    uint32_t instIdx, chIdx = 0U;

    /* Get addition of all the channels processed on all previous driver instances */
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        if (instIdx == instId)
        {
            break;
        }
        else
        {
            chIdx += prms->instParams[instIdx].numCh;
        }
    }
    chIdx += chId;

    return (chIdx);
}

static uint32_t tivxCaptureGetDrvInstIndex(const tivxCaptureParams *prms,
                                           uint32_t instId)
{
    uint32_t instIdx, instVal;

    instVal = tivxCaptureMapInstId(instId);
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        if (prms->instParams[instIdx].instId == instVal)
        {
            /* Found out the index for required instance */
            break;
        }
    }

    return instIdx;
}
