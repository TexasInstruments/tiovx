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

#include "TI/tivx.h"
#include "TI/j7.h"
#include "VX/vx.h"
#include "TI/tivx_event.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_capture.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

#include <TI/tivx_queue.h>
#include <ti/drv/fvid2/fvid2.h>
#include <ti/drv/csirx/csirx.h>

#define CAPTURE_FRAME_DROP_LEN (4096U*4U)

typedef struct
{
    tivx_obj_desc_t *img_obj_desc[TIVX_CAPTURE_MAX_CH];
    /* Captured Images */
    uint8_t steady_state_started;
    /**< Flag indicating whether or not steady state has begun. */
    tivx_event  frame_available;
    /**< Event indicating when a frame is available. */
    uint32_t instId;
    /**< Csirx Drv Instance ID. */
    uint8_t numCh;
    /**< Number of channels. */
    Fvid2_Handle drvHandle;
    /**< FVID2 capture driver handle. */
    Csirx_CreateParams createPrms;
    /**< Csirx create time parameters */
    Csirx_CreateStatus createStatus;
    /**< Csirx create time status */
    Fvid2_CbParams drvCbPrms;
    /**< Capture callback params */
    tivx_queue freeFvid2FrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal FVID2 queue */
    tivx_queue pendingFrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal pending frame queue */
    uintptr_t fvid2_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< FVID2 queue mem */
    Fvid2_Frame fvid2Frames[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< FVID2 frame structs */
    uintptr_t pending_frame_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending frame queue mem */
    uint8_t raw_capture;
    /**< flag indicating raw capture */
    Csirx_InstStatus captStatus;
    /**< CSIRX Capture status. */

    Csirx_DPhyCfg dphyCfg;
} tivxCaptureParams;

static tivx_target_kernel vx_capture_target_kernel1 = NULL;
static tivx_target_kernel vx_capture_target_kernel2 = NULL;

static vx_status captDrvCallback(Fvid2_Handle handle, void *appData, void *reserved);
static uint32_t tivxCaptureExtractInCsiDataType(uint32_t format);
static uint32_t tivxCaptureExtractCcsFormat(uint32_t format);
static uint32_t tivxCaptureExtractDataFormat(uint32_t format);
static vx_status tivxCaptureEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *output_desc,
       tivxCaptureParams *prms);
static void tivxCaptureSetCreateParams(
       tivxCaptureParams *prms,
       tivx_obj_desc_user_data_object_t *obj_desc);
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

    return VX_SUCCESS;
}

static vx_status tivxCaptureEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *output_desc,
       tivxCaptureParams *prms)
{
    vx_status status = VX_SUCCESS;
    void *output_image_target_ptr;
    uint64_t captured_frame;
    uint32_t chId = 0U;
    static Fvid2_FrameList frmList;
    Fvid2_Frame *fvid2Frame;

    tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                       prms->numCh);

    frmList.numFrames = prms->numCh;

    for (chId = 0; chId < prms->numCh; chId++)
    {
        if (TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[0]->type)
        {
            tivx_obj_desc_raw_image_t *raw_image;

            raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[chId];

            /* Question: is the fact that we are just using mem_ptr[0] and not remaining planes correct? */
            output_image_target_ptr = tivxMemShared2TargetPtr(
                raw_image->mem_ptr[0].shared_ptr,
                raw_image->mem_ptr[0].mem_heap_region);

            captured_frame = ((uintptr_t)output_image_target_ptr +
                tivxComputePatchOffset(0, 0, &raw_image->imagepatch_addr[0U]));
        }
        else
        {
            tivx_obj_desc_image_t *image;
            image = (tivx_obj_desc_image_t *)prms->img_obj_desc[chId];

            /* Question: is the fact that we are just using mem_ptr[0] and not remaining exposures correct? */
            output_image_target_ptr = tivxMemShared2TargetPtr(
                image->mem_ptr[0].shared_ptr,
                image->mem_ptr[0].mem_heap_region);

            captured_frame = ((uintptr_t)output_image_target_ptr +
                tivxComputePatchOffset(0, 0, &image->imagepatch_addr[0U]));
        }

        tivxQueueGet(&prms->freeFvid2FrameQ[chId], (uintptr_t*)&fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);

        if (NULL != fvid2Frame)
        {
            frmList.frames[chId]           = fvid2Frame;
            frmList.frames[chId]->chNum    = chId;
            frmList.frames[chId]->addr[0U] = captured_frame;
            frmList.frames[chId]->appData  = output_desc;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: Could not retrieve buffer from buffer queue!!!\n");
        }
    }

    if (VX_SUCCESS == status)
    {
        status = Fvid2_queue(prms->drvHandle, &frmList, 0);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Frame could not be queued for frame %d !!!\n", chId);
        }
    }

    return status;
}

static uint32_t tivxCaptureExtractInCsiDataType(uint32_t format)
{
    uint32_t inCsiDataType = FVID2_CSI2_DF_RGB888;

    switch (format)
    {
        case VX_DF_IMAGE_RGB:
            inCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case VX_DF_IMAGE_RGBX:
            inCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case VX_DF_IMAGE_U16:
        case TIVX_RAW_IMAGE_P12_BIT:
            inCsiDataType = FVID2_CSI2_DF_RAW12;
            break;
        default:
            break;
    }

    return inCsiDataType;
}

static uint32_t tivxCaptureExtractCcsFormat(uint32_t format)
{
    uint32_t ccsFormat = FVID2_CCSF_BITS12_PACKED;

    switch (format)
    {
        case TIVX_RAW_IMAGE_P12_BIT:
            ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        case TIVX_RAW_IMAGE_16_BIT:
        case VX_DF_IMAGE_U16:
            ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            break;
        default:
            break;
    }

    return ccsFormat;
}

/* TODO: Complete this case statement */
static uint32_t tivxCaptureExtractDataFormat(uint32_t format)
{
    uint32_t dataFormat = FVID2_DF_BGRX32_8888;

    return dataFormat;
}

static void tivxCaptureSetCreateParams(
       tivxCaptureParams *prms,
       tivx_obj_desc_user_data_object_t *obj_desc)
{
    uint32_t loopCnt = 0U, i, format, width, height, planes, stride[TIVX_IMAGE_MAX_PLANES];
    void *capture_config_target_ptr;
    tivx_capture_params_t *params;

    capture_config_target_ptr = tivxMemShared2TargetPtr(
        obj_desc->mem_ptr.shared_ptr, obj_desc->mem_ptr.mem_heap_region);

    tivxMemBufferMap(capture_config_target_ptr, obj_desc->mem_size,
        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    params = (tivx_capture_params_t *)capture_config_target_ptr;

    /* set instance configuration parameters */
    Csirx_createParamsInit(&prms->createPrms);

    if (TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[0]->type)
    {
        tivx_obj_desc_raw_image_t *raw_image;
        raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[0];
        format = raw_image->params.format[0].pixel_container; /* TODO: Question: what should be done when this is different per exposure */
        width = raw_image->params.width;
        height = raw_image->params.height;
        planes = raw_image->params.num_exposures;
        for (i = 0; i < planes; i++)
        {
            stride[i] = raw_image->imagepatch_addr[i].stride_y;
        }
        prms->raw_capture = 1;
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
            stride[i] = image->imagepatch_addr[i].stride_y;
        }
        prms->raw_capture = 0;
    }

    prms->createPrms.numCh = prms->numCh;

    for (loopCnt = 0U ; loopCnt < prms->createPrms.numCh ; loopCnt++)
    {
        prms->createPrms.chCfg[loopCnt].chId = loopCnt;
        prms->createPrms.chCfg[loopCnt].chType = CSIRX_CH_TYPE_CAPT;
        prms->createPrms.chCfg[loopCnt].vcNum = loopCnt;

        if (TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[0]->type)
        {
            prms->createPrms.chCfg[loopCnt].inCsiDataType =
                FVID2_CSI2_DF_RAW12;
        }
        else
        {
            prms->createPrms.chCfg[loopCnt].inCsiDataType =
                tivxCaptureExtractInCsiDataType(format);
        }
        prms->createPrms.chCfg[loopCnt].outFmt.width =
            width;
        prms->createPrms.chCfg[loopCnt].outFmt.height =
            height;
        for (i = 0; i < planes; i ++)
        {
            prms->createPrms.chCfg[loopCnt].outFmt.pitch[i] =
                stride[i];
        }

        prms->createPrms.chCfg[loopCnt].outFmt.dataFormat =
            tivxCaptureExtractDataFormat(format);
        prms->createPrms.chCfg[loopCnt].outFmt.ccsFormat =
            tivxCaptureExtractCcsFormat(format);
    }
    /* set module configuration parameters */
    prms->createPrms.instCfg.enableCsiv2p0Support = params->enableCsiv2p0Support;
    prms->createPrms.instCfg.numDataLanes = params->numDataLanes;
    prms->createPrms.instCfg.enableErrbypass = (uint32_t)FALSE;
    for (loopCnt = 0U ;
         loopCnt < prms->createPrms.instCfg.numDataLanes ;
         loopCnt++)
    {
        prms->createPrms.instCfg.dataLanesMap[loopCnt] = params->dataLanesMap[loopCnt];
    }
    /* set frame drop buffer parameters */
    prms->createPrms.frameDropBufLen =
        CAPTURE_FRAME_DROP_LEN;
    prms->createPrms.frameDropBuf = (uint64_t)tivxMemAlloc(prms->createPrms.frameDropBufLen, TIVX_MEM_EXTERNAL);

    tivxMemBufferUnmap(capture_config_target_ptr,
       obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
       VX_READ_ONLY);
}

static vx_status VX_CALLBACK tivxCaptureProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxCaptureParams *prms = NULL;
    tivx_obj_desc_object_array_t *output_desc;
    static Fvid2_FrameList frmList;
    vx_uint32 size, frmIdx = 0U, chId = 0U;
    vx_enum state;
    Fvid2_Frame *fvid2Frame;
    tivx_obj_desc_object_array_t *desc;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCaptureParams) != size))
        {
            status = VX_FAILURE;
        }
        else
        {
            status = tivxGetTargetKernelInstanceState(kernel, &state);
        }
    }

    if(VX_SUCCESS == status)
    {
        /* Steady state: receives a buffer and returns a buffer */
        if (VX_NODE_STATE_STEADY == state)
        {
            /* Providing buffers to capture source */
            status = tivxCaptureEnqueueFrameToDriver(output_desc, prms);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Enqueue Frame to Driver failed !!!\n");
            }

            /* Starts FVID2 on initial frame */
            if (VX_SUCCESS == status)
            {
                if (0U == prms->steady_state_started)
                {
                    status = Fvid2_start(prms->drvHandle, NULL);
                    if (VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could not start FVID2 !!!\n");
                    }
                    prms->steady_state_started = 1;
                }
            }

            /* Pends until a frame is available then dequeue frames from capture driver */
            if (VX_SUCCESS == status)
            {
                tivx_obj_desc_t *tmp_desc[TIVX_CAPTURE_MAX_CH];

                uint32_t is_all_ch_frame_available = 0;

                for(chId=0; chId<prms->numCh; chId++)
                {
                    tmp_desc[chId] = NULL;
                }

                while(!is_all_ch_frame_available)
                {
                    is_all_ch_frame_available = 1;
                    for(chId=0; chId<prms->numCh; chId++)
                    {
                        tivxQueuePeek(&prms->pendingFrameQ[chId], (uintptr_t*)&tmp_desc[chId]);
                        if(NULL==tmp_desc[chId])
                            is_all_ch_frame_available = 0;
                    }

                    if(!is_all_ch_frame_available)
                    {
                        tivxEventWait(prms->frame_available, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

                        status = Fvid2_dequeue(
                            prms->drvHandle,
                            &frmList,
                            0,
                            FVID2_TIMEOUT_NONE);

                        if(status==VX_SUCCESS)
                        {
                            for(frmIdx=0; frmIdx < frmList.numFrames; frmIdx++)
                            {
                                fvid2Frame = frmList.frames[frmIdx];

                                chId = fvid2Frame->chNum;

                                desc = (tivx_obj_desc_object_array_t *)fvid2Frame->appData;

                                tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);
                                tivxQueuePut(&prms->pendingFrameQ[chId], (uintptr_t)desc, TIVX_EVENT_TIMEOUT_NO_WAIT);
                            }
                        }
                    }
                }
                for(chId=0; chId<prms->numCh; chId++)
                {
                    tivxQueueGet(&prms->pendingFrameQ[chId], (uintptr_t*)&tmp_desc[chId], TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
                /* all values in tmp_desc[] should be same */
                obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX] = (tivx_obj_desc_t *)tmp_desc[0];
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
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *input_obj_desc;
    tivx_obj_desc_object_array_t *output_desc;
    tivxCaptureParams *prms = NULL;
    uint32_t chId, bufId;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        input_obj_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX];
        output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxCaptureParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxCaptureParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could allocate memory !!!\n");
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            /* Initialize steady_state_started to 0 */
            prms->steady_state_started = 0;
            /* Initialize raw capture to 0 */
            prms->raw_capture = 0;
            /* set instance to be used for capture */
            prms->instId = CSIRX_INSTANCE_ID_0;
            /* Set number of channels to number of items in object array */
            prms->numCh = output_desc->num_items;

            if (prms->numCh > TIVX_CAPTURE_MAX_CH)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Object descriptor number of channels exceeds max value allowed by capture!!!\r\n");
            }
        }

        /* Setting CSIRX capture parameters */
        if (VX_SUCCESS == status)
        {
            tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                           prms->numCh);

            tivxCaptureSetCreateParams(prms, input_obj_desc);
        }

        /* Creating frame available event */
        if (VX_SUCCESS == status)
        {
            status = tivxEventCreate(&prms->frame_available);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Event creation failed in capture!!!\r\n");
            }
        }

        /* Creating FVID2 handle */
        if (VX_SUCCESS == status)
        {
            Fvid2CbParams_init(&prms->drvCbPrms);

            prms->drvCbPrms.cbFxn   = (Fvid2_CbFxn) &captDrvCallback;
            prms->drvCbPrms.appData = prms;

            prms->drvHandle = Fvid2_create(
                CSIRX_CAPT_DRV_ID,
                prms->instId,
                &prms->createPrms,
                &prms->createStatus,
                &prms->drvCbPrms);

            if ((NULL == prms->drvHandle) ||
                (prms->createStatus.retVal != FVID2_SOK))
            {
                VX_PRINT(VX_ZONE_ERROR, ": Capture Create Failed!!!\r\n");
                status = prms->createStatus.retVal;
            }
            else
            {
                /* Set CSIRX D-PHY configuration parameters */
                Csirx_initDPhyCfg(&prms->dphyCfg);
                prms->dphyCfg.inst = prms->instId;
                status = Fvid2_control(
                    prms->drvHandle, IOCTL_CSIRX_SET_DPHY_CONFIG,
                    &prms->dphyCfg, NULL);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Failed to set PHY Parameters!!!\r\n");
                }
            }
        }

        /* Creating FVID2 frame Q */
        if (VX_SUCCESS == status)
        {
            for(chId=0; chId<prms->numCh; chId++)
            {
                status = tivxQueueCreate(&prms->freeFvid2FrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->fvid2_free_q_mem[chId], 0);

                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture queue create failed!!!\r\n");
                    break;
                }

                for(bufId = 0; bufId < (TIVX_CAPTURE_MAX_NUM_BUFS); bufId++)
                {
                    tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)&prms->fvid2Frames[chId][bufId], TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
            }
        }

        /* Creating pending frame Q */
        if (VX_SUCCESS == status)
        {
            for(chId=0; chId<prms->numCh; chId++)
            {
                tivxQueueCreate(&prms->pendingFrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_free_q_mem[chId], 0);

                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture queue create failed!!!\r\n");
                    break;
                }
            }
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxCaptureParams));
        }
        else
        {
            if (NULL != prms->drvHandle)
            {
                Fvid2_delete(prms->drvHandle, NULL);
                prms->drvHandle = NULL;
            }

            if (NULL != prms->frame_available)
            {
                tivxEventDelete(&prms->frame_available);
            }

            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxCaptureParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static void tivxCapturePrintStatus(tivxCaptureParams *prms)
{
    int32_t status;
    uint32_t cnt;

    if (NULL != prms)
    {
        status = Fvid2_control(prms->drvHandle,
                                IOCTL_CSIRX_GET_INST_STATUS,
                                &prms->captStatus,
                                NULL);
        if (FVID2_SOK == status)
        {
            VX_PRINT(VX_ZONE_INFO,
                "\n\r==========================================================\r\n");
            VX_PRINT(VX_ZONE_INFO,
                      ": Capture Status:\r\n");
            VX_PRINT(VX_ZONE_INFO,
                      "==========================================================\r\n");
            VX_PRINT(VX_ZONE_INFO,
                      ": FIFO Overflow Count: %d\r\n",
                      prms->captStatus.overflowCount);
            VX_PRINT(VX_ZONE_INFO,
                      ": Spurious UDMA interrupt count: %d\r\n",
                      prms->captStatus.spuriousUdmaIntrCount);

            VX_PRINT(VX_ZONE_INFO,
                "  [Channel No] | Frame Queue Count |"
                " Frame De-queue Count | Frame Drop Count |\n");
            for(cnt = 0U ; cnt < prms->numCh ; cnt ++)
            {
                VX_PRINT(VX_ZONE_INFO,
                      "\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                      cnt,
                      prms->captStatus.queueCount[cnt],
                      prms->captStatus.dequeueCount[cnt],
                      prms->captStatus.dropCount[cnt]);
            }
        }
    }
}

static vx_status VX_CALLBACK tivxCaptureDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS, retVal = VX_SUCCESS;
    tivxCaptureParams *prms = NULL;
    static Fvid2_FrameList frmList;
    uint32_t size, chId;

    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could not obtain kernel instance context !!!\n");
        }

        /* Stopping FVID2 Capture */
        if (VX_SUCCESS == status)
        {
            status = Fvid2_stop(prms->drvHandle, NULL);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Capture not stopped !!!\n");
            }
        }

        /* Dequeue all the request from the driver */
        if (VX_SUCCESS == status)
        {
            Fvid2FrameList_init(&frmList);
            do
            {
                retVal = Fvid2_dequeue(
                    prms->drvHandle,
                    &frmList,
                    0,
                    FVID2_TIMEOUT_NONE);
            } while (FVID2_SOK == retVal);

            if ((FVID2_SOK != retVal) && (FVID2_ENO_MORE_BUFFERS != retVal))
            {
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Capture Dequeue Failed !!!\n");
                status = retVal;
            }
        }

        if (VX_SUCCESS == status)
        {
            tivxCapturePrintStatus(prms);
        }

        /* Deleting FVID2 handle */
        if (VX_SUCCESS == status)
        {
            status = Fvid2_delete(prms->drvHandle, NULL);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Delete Failed !!!\n");
            }
        }

        /* Deleting FVID2 frame Q */
        if (VX_SUCCESS == status)
        {
            for(chId=0; chId<prms->numCh; chId++)
            {
                tivxQueueDelete(&prms->freeFvid2FrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        if (VX_SUCCESS == status)
        {
            for(chId=0; chId<prms->numCh; chId++)
            {
                tivxQueueDelete(&prms->pendingFrameQ[chId]);
            }
        }

        /* Deleting event */
        if (VX_SUCCESS == status)
        {
            tivxEventDelete(&prms->frame_available);
        }

        /* Free-ing kernel instance params */
        if (VX_SUCCESS == status)
        {
            prms->drvHandle = NULL;

            if ((NULL != prms) && (sizeof(tivxCaptureParams) == size))
            {
                tivxMemFree(prms, sizeof(tivxCaptureParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

void tivxAddTargetKernelCapture(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_CAPTURE1, TIVX_TARGET_MAX_NAME);

        vx_capture_target_kernel1 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_CAPTURE_NAME,
                            target_name,
                            tivxCaptureProcess,
                            tivxCaptureCreate,
                            tivxCaptureDelete,
                            NULL,
                            NULL);

        strncpy(target_name, TIVX_TARGET_CAPTURE2,
            TIVX_TARGET_MAX_NAME);

        vx_capture_target_kernel2 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_CAPTURE_NAME,
                            target_name,
                            tivxCaptureProcess,
                            tivxCaptureCreate,
                            tivxCaptureDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelCapture(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_capture_target_kernel1);
    if(status == VX_SUCCESS)
    {
        vx_capture_target_kernel1 = NULL;
    }
    status = tivxRemoveTargetKernel(vx_capture_target_kernel2);
    if(status == VX_SUCCESS)
    {
        vx_capture_target_kernel2 = NULL;
    }
}


