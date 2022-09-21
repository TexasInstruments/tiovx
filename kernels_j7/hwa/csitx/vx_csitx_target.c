/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#include "TI/tivx_event.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_csitx.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_csitx_priv.h"

#include <TI/tivx_queue.h>
#include <ti/drv/fvid2/fvid2.h>
#include <ti/drv/csitx/csitx.h>


#define CSITX_INST_ID_INVALID                         (0xFFFFU)

#define CAPTURE_OUT_CSI_DT_INVALID                    (0xFFFFFFFFU)

typedef struct tivxCsitxParams_t tivxCsitxParams;

typedef struct
{
    uint32_t instId;
    /**< Csitx Drv Instance ID. */
    uint8_t numCh;
    /**< Number of channels processed on given CSITX DRV instance. */
    uint32_t chVcMap[TIVX_CSITX_MAX_CH];
    /**< Virtual ID for channels for current csitx instance. */
    Fvid2_Handle drvHandle;
    /**< FVID2 csitx driver handle. */
    Csitx_CreateParams createPrms;
    /**< Csitx create time parameters */
    Csitx_CreateStatus createStatus;
    /**< Csitx create time status */
    Fvid2_CbParams drvCbPrms;
    /**< Csitx callback params */
    Csitx_InstStatus csitxStatus;
    /**< CSITX Transmit status. */
    tivxCsitxParams *csitxParams;
    /**< Reference to csitx node parameters. */
} tivxCsitxInstParams;

struct tivxCsitxParams_t
{
    tivxCsitxInstParams instParams[TIVX_CSITX_MAX_INST];
    /* Csitx Instance parameters */
    uint32_t numOfInstUsed;
    /**< Number of CSITX DRV instances used in current TIOVX Node. */
    uint8_t numCh;
    /**< Number of channels processed on given csitx node instance. */
    tivx_obj_desc_t *img_obj_desc[TIVX_CSITX_MAX_CH];
    /*  Transmit Images */
    uint8_t steady_state_started;
    /**< Flag indicating whether or not steady state has begun. */
    tivx_event  frame_available;
    /**< Following Queues i.e. freeFvid2FrameQ, pendingFrameQ, fvid2_free_q_mem,
     *   fvid2Frames, and pending_frame_free_q_mem are for given instance of the
     *   Node. If Node instance contains more than 1 instances of the CSITX DRV
     *   instances, then first 'n' channels are for first instance of the driver
     *   then n channels for next driver and so on... */
    /**< Event indicating when a frame is available. */
    tivx_queue freeFvid2FrameQ[TIVX_CSITX_MAX_CH];
    /**< Internal FVID2 queue */
    tivx_queue pendingFrameQ[TIVX_CSITX_MAX_CH];
    /**< Internal pending frame queue */
    uintptr_t fvid2_free_q_mem[TIVX_CSITX_MAX_CH][TIVX_CSITX_MAX_NUM_BUFS];
    /**< FVID2 queue mem */
    Fvid2_Frame fvid2Frames[TIVX_CSITX_MAX_CH][TIVX_CSITX_MAX_NUM_BUFS];
    /**< FVID2 frame structs */
    uintptr_t pending_frame_free_q_mem[TIVX_CSITX_MAX_CH][TIVX_CSITX_MAX_NUM_BUFS];
    /**< pending frame queue mem */
};

static char target_name[][TIVX_TARGET_MAX_NAME] =
{
    TIVX_TARGET_CSITX,
#if defined(SOC_J721S2) || defined(SOC_J784S4)
    TIVX_TARGET_CSITX2,
#endif
};

#define CSITX_NUM_TARGETS                             (sizeof(target_name)/sizeof(target_name[0]))

static tivx_target_kernel vx_csitx_target_kernel[CSITX_NUM_TARGETS] = {NULL};

static vx_status csitxDrvCallback(Fvid2_Handle handle, void *appData, void *reserved);
static uint32_t tivxCsitxExtractOutCsiDataType(uint32_t format);
static uint32_t tivxCsitxExtractCcsFormat(uint32_t format);
static uint32_t tivxCsitxExtractDataFormat(uint32_t format);
static uint32_t tivxCsitxGetDrvInstIndex(const tivxCsitxParams *prms,
                                           uint32_t instId);
static vx_status tivxCsitxEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *input_desc,
       tivxCsitxParams *prms);

static vx_status tivxCsitxSetCreateParams(
       tivxCsitxParams *prms,
       tivx_obj_desc_user_data_object_t *obj_desc);

static vx_status VX_CALLBACK tivxCsitxProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCsitxCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCsitxDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCsitxControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status tivxCsitxGetStatistics(tivxCsitxParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static void tivxCsitxCopyStatistics(tivxCsitxParams *prms,
    tivx_csitx_statistics_t *csitx_status_prms);
static void tivxCsitxGetChannelIndices(tivxCsitxParams *prms,
                                         uint32_t instId,
                                         uint32_t *startChIdx,
                                         uint32_t *endChIdx);
static uint32_t tivxCsitxGetNodeChannelNum(tivxCsitxParams *prms,
                                             uint32_t instId,
                                             uint32_t chId);
static uint32_t tivxCsitxMapInstId(uint32_t instId);
static void tivxCsitxPrintStatus(tivxCsitxInstParams *prms);

/**
 *******************************************************************************
 *
 * \brief Callback function from driver to application
 *
 * Callback function gets called from Driver to application on transmission of
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
static vx_status csitxDrvCallback(Fvid2_Handle handle, void *appData, void *reserved)
{
    tivxCsitxParams *prms = (tivxCsitxParams*)appData;

    tivxEventPost(prms->frame_available);

    return (vx_status)VX_SUCCESS;
}


static vx_status tivxCsitxEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *input_desc,
       tivxCsitxParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    void *input_image_target_ptr;
    uint64_t transmit_frame;
    uint32_t chId = 0U;
    static Fvid2_FrameList frmList;
    Fvid2_Frame *fvid2Frame;
    uint32_t startChIdx, endChIdx, instIdx;
    tivxCsitxInstParams *instParams;

    tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                       prms->numCh);

    /* Prepare and queue frame-list for each instance */
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];
        tivxCsitxGetChannelIndices(prms, instIdx, &startChIdx, &endChIdx);
        frmList.numFrames = instParams->numCh;
        for (chId = startChIdx ; chId < endChIdx ; chId++)
        {
            if ((vx_enum)TIVX_OBJ_DESC_RAW_IMAGE == (vx_enum)prms->img_obj_desc[0]->type)
            {
                tivx_obj_desc_raw_image_t *raw_image;

                raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[chId];

                input_image_target_ptr = tivxMemShared2TargetPtr(&raw_image->mem_ptr[0]);


                transmit_frame = ((uintptr_t)input_image_target_ptr +
                    (uint64_t)tivxComputePatchOffset(0, 0, &raw_image->imagepatch_addr[0U]));
            }
            else
            {
                tivx_obj_desc_image_t *image;
                image = (tivx_obj_desc_image_t *)prms->img_obj_desc[chId];

                input_image_target_ptr = tivxMemShared2TargetPtr(&image->mem_ptr[0]);

                transmit_frame = ((uintptr_t)input_image_target_ptr +
                    (uint64_t)tivxComputePatchOffset(0, 0, &image->imagepatch_addr[0U]));
            }

            tivxQueueGet(&prms->freeFvid2FrameQ[chId], (uintptr_t*)&fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);

            if (NULL != fvid2Frame)
            {
                /* Put into frame list as it is for same driver instance */
                frmList.frames[(chId - startChIdx)]           = fvid2Frame;
                frmList.frames[(chId - startChIdx)]->chNum    = (chId - startChIdx);
                frmList.frames[(chId - startChIdx)]->addr[0U] = transmit_frame;
                frmList.frames[(chId - startChIdx)]->appData  = input_desc;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, " CSITX: Could not retrieve buffer from buffer queue!!!\n");
            }
        }

        /* All the frames from frame-list */
        fvid2_status = Fvid2_queue(instParams->drvHandle, &frmList, 0);
        if (FVID2_SOK != fvid2_status)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: Frame could not be queued for frame %d !!!\n", chId);
            break;
        }
    }

    return status;
}


static uint32_t tivxCsitxExtractOutCsiDataType(uint32_t format)
{
    uint32_t outCsiDataType;

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_RGB:
            outCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGBX:
            outCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (uint32_t)TIVX_RAW_IMAGE_P12_BIT:
            outCsiDataType = FVID2_CSI2_DF_RAW12;
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            outCsiDataType = FVID2_CSI2_DF_YUV422_8B;
            break;
        default:
            outCsiDataType = CAPTURE_OUT_CSI_DT_INVALID;
            break;
    }

    return outCsiDataType;
}

static uint32_t tivxCsitxExtractOutCsiDataTypeFromRawImg(tivx_obj_desc_raw_image_t *raw_img)
{
    uint32_t inCsiDataType = CAPTURE_OUT_CSI_DT_INVALID;
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
        if (12u == params->format[0].msb)
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

static uint32_t tivxCsitxExtractCcsFormat(uint32_t format)
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

static uint32_t tivxCsitxMapInstId(uint32_t instId)
{
    uint32_t drvInstId = CSITX_INST_ID_INVALID;
    switch (instId)
    {
        case 0:
            drvInstId = CSITX_INSTANCE_ID_0;
            break;
#if defined (SOC_J721S2) || defined (SOC_J784S4)
        case 1:
            drvInstId = CSITX_INSTANCE_ID_1;
            break;
#endif
        default:
            /* do nothing */
            break;
    }

    return (drvInstId);
}

static uint32_t tivxCsitxExtractDataFormat(uint32_t format)
{
    uint32_t dataFormat = FVID2_DF_BGRX32_8888;

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_RGBX:
            dataFormat = FVID2_DF_BGRX32_8888;
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
            dataFormat = FVID2_DF_YUV422I_UYVY;
            break;
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            dataFormat = FVID2_DF_YUV422I_YUYV;
            break;
        default:
            /* do nothing */
            break;
    }

    return dataFormat;
}

static vx_status tivxCsitxSetCreateParams(
       tivxCsitxParams *prms,
       tivx_obj_desc_user_data_object_t *obj_desc)
{
    uint32_t loopCnt = 0U, i, format, width, height, planes, stride[TIVX_IMAGE_MAX_PLANES];
    void *csitx_config_target_ptr;
    tivx_csitx_params_t *params;
    uint32_t chIdx, instId = 0U, instIdx;
    Csitx_CreateParams *createParams;
    tivx_obj_desc_raw_image_t *raw_image = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    csitx_config_target_ptr = tivxMemShared2TargetPtr(&obj_desc->mem_ptr);

    tivxCheckStatus(&status, tivxMemBufferMap(csitx_config_target_ptr, obj_desc->mem_size,
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    params = (tivx_csitx_params_t *)csitx_config_target_ptr;

    /* Scan through all the channels provided in the Node instance and prepare CSITX DRV instance data/cfg */
    for (chIdx = 0U ; chIdx < params->numCh ; chIdx++)
    {
        instId = tivxCsitxGetDrvInstIndex(prms, params->chInstMap[chIdx]);
        prms->instParams[instId].chVcMap[prms->instParams[instId].numCh] = params->chVcNum[chIdx];
        prms->instParams[instId].numCh++;
    }

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
    }

    /* Do following for each CSITX DRV instance in the current Node */
    for (instIdx = 0U ; instIdx < params->numInst ; instIdx++)
    {
        prms->instParams[instIdx].csitxParams = prms;
        /* set instance configuration parameters */
        createParams = &prms->instParams[instIdx].createPrms;
        Csitx_createParamsInit(createParams);
        /* Initialize transmit instance status */
        Csitx_instStatusInit(&prms->instParams[instIdx].csitxStatus);

        /* set module configuration parameters */
        createParams->instCfg.rxCompEnable = params->instCfg[instIdx].rxCompEnable;
        createParams->instCfg.rxv1p3MapEnable = params->instCfg[instIdx].rxv1p3MapEnable;
        createParams->instCfg.dphyCfg.inst = params->instId[instIdx];
        createParams->instCfg.dphyCfg.laneBandSpeed = params->instCfg[instIdx].laneBandSpeed;
        createParams->instCfg.dphyCfg.laneSpeedMbps = params->instCfg[instIdx].laneSpeedMbps;
        createParams->instCfg.numDataLanes = params->instCfg[instIdx].numDataLanes;
        for (loopCnt = 0U ;
             loopCnt < createParams->instCfg.numDataLanes ;
             loopCnt++)
        {
            createParams->instCfg.lanePolarityCtrl[loopCnt] = params->instCfg[instIdx].lanePolarityCtrl[loopCnt];
        }

        createParams->numCh = prms->instParams[instIdx].numCh;
        for (loopCnt = 0U ; loopCnt < createParams->numCh ; loopCnt++)
        {
            createParams->chCfg[loopCnt].chId = loopCnt;
            createParams->chCfg[loopCnt].chType = CSITX_CH_TYPE_TX;
            createParams->chCfg[loopCnt].vcNum = prms->instParams[instIdx].chVcMap[loopCnt];

            if ((vx_enum)TIVX_OBJ_DESC_RAW_IMAGE == (vx_enum)prms->img_obj_desc[0]->type)
            {
                if (NULL != raw_image)
                {
                    createParams->chCfg[loopCnt].outCsiDataType =
                        tivxCsitxExtractOutCsiDataTypeFromRawImg(raw_image);
                }
            }
            else
            {
                createParams->chCfg[loopCnt].outCsiDataType =
                    tivxCsitxExtractOutCsiDataType(format);
            }
            createParams->chCfg[loopCnt].inFmt.width =
                width;
            createParams->chCfg[loopCnt].inFmt.height =
                height;
            for (i = 0; i < planes; i ++)
            {
                createParams->chCfg[loopCnt].inFmt.pitch[i] =
                    stride[i];
            }

            createParams->chCfg[loopCnt].inFmt.dataFormat =
                tivxCsitxExtractDataFormat(format);
            createParams->chCfg[loopCnt].inFmt.ccsFormat =
                tivxCsitxExtractCcsFormat(format);

            createParams->chCfg[loopCnt].vBlank = params->instCfg[instIdx].vBlank;
            createParams->chCfg[loopCnt].hBlank = params->instCfg[instIdx].hBlank;
            createParams->chCfg[loopCnt].startDelayPeriod = params->instCfg[instIdx].startDelayPeriod;

        }

        /* set instance to be used for csitx */
        prms->instParams[instIdx].instId = tivxCsitxMapInstId(params->instId[instIdx]);
        prms->numOfInstUsed++;
    }

    tivxCheckStatus(&status, tivxMemBufferUnmap(csitx_config_target_ptr,
       obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
       (vx_enum)VX_READ_ONLY));

    return status;
}


static vx_status VX_CALLBACK tivxCsitxProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    static Fvid2_FrameList frmList;
    Fvid2_Frame *fvid2Frame;
    tivxCsitxParams *prms = NULL;
    tivxCsitxInstParams *instParams;
    tivx_obj_desc_object_array_t *input_desc;
    tivx_obj_desc_object_array_t *desc;
    vx_uint32 size, frmIdx = 0U, chId = 0U;
    uint32_t instIdx;
    vx_enum state;

    if ( (num_params != TIVX_KERNEL_CSITX_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CSITX_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CSITX_INPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CSITX_INPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCsitxParams) != size))
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

        /* Steady state: provides a buffer and receives a buffer */
        if ((vx_enum)VX_NODE_STATE_STEADY == state)
        {
            /* Providing buffers to csitx driver */
            status = tivxCsitxEnqueueFrameToDriver(input_desc, prms);

            if ((vx_status)VX_SUCCESS != status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: Enqueue Frame to Driver failed !!!\n");
            }

            /* Starts FVID2 on initial frame */
            if ((vx_status)VX_SUCCESS == status)
            {
                if (0U == prms->steady_state_started)
                {
                    /* start all driver instances in the node */
                    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                    {
                        fvid2_status = Fvid2_start(prms->instParams[instIdx].drvHandle, NULL);
                        if (FVID2_SOK != fvid2_status)
                        {
                            status = (vx_status)VX_FAILURE;
                            VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: Could not start FVID2 !!!\n");
                            break;
                        }
                    }

                    if((vx_status)VX_SUCCESS == status)
                    {
                        prms->steady_state_started = 1;
                    }
                }
            }

            /* Pends until a frame is available then dequeue frames from csitx driver */
            if ((vx_status)VX_SUCCESS == status)
            {
                tivx_obj_desc_t *tmp_desc[TIVX_CSITX_MAX_CH] = {NULL};

                uint32_t is_all_ch_frame_available = 0;

                for(chId = 0U ; chId < prms->numCh ; chId++)
                {
                    tmp_desc[chId] = NULL;
                }

                while(is_all_ch_frame_available == 0U)
                {
                    is_all_ch_frame_available = 1;
                    for(chId = 0U ; chId < prms->numCh ; chId++)
                    {
                        tivxQueuePeek(&prms->pendingFrameQ[chId], (uintptr_t*)&tmp_desc[chId]);
                        if(NULL==tmp_desc[chId])
                        {
                            is_all_ch_frame_available = 0;
                        }
                    }

                    if(is_all_ch_frame_available == 0U)
                    {
                        tivxEventWait(prms->frame_available, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

                        for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                        {
                            instParams = &prms->instParams[instIdx];
                            fvid2_status = Fvid2_dequeue(instParams->drvHandle,
                                                         &frmList,
                                                         0,
                                                         FVID2_TIMEOUT_NONE);

                            if(FVID2_SOK == fvid2_status)
                            {
                                for(frmIdx=0; frmIdx < frmList.numFrames; frmIdx++)
                                {
                                    fvid2Frame = frmList.frames[frmIdx];
                                    chId = tivxCsitxGetNodeChannelNum(
                                                        prms,
                                                        instIdx,
                                                        fvid2Frame->chNum);
                                    desc = (tivx_obj_desc_object_array_t *)fvid2Frame->appData;

                                    tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);
                                    tivxQueuePut(&prms->pendingFrameQ[chId], (uintptr_t)desc, TIVX_EVENT_TIMEOUT_NO_WAIT);
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
                                if (FVID2_EAGAIN != fvid2_status)
                                {
                                    status = (vx_status)VX_FAILURE;
                                    VX_PRINT(VX_ZONE_ERROR,
                                        " CSITX: ERROR: FVID2 Dequeue failed !!!\n");
                                }
                            }
                        }
                    }
                }

                for(chId = 0U ; chId < prms->numCh ; chId++)
                {
                    tivxQueueGet(&prms->pendingFrameQ[chId], (uintptr_t*)&tmp_desc[chId], TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
                /* all values in tmp_desc[] should be same */
                obj_desc[TIVX_KERNEL_CSITX_INPUT_IDX] = (tivx_obj_desc_t *)tmp_desc[0];
            }
        }
        /* Pipe-up state: only provides a buffer; does not receive a buffer */
        else
        {
            status = tivxCsitxEnqueueFrameToDriver(input_desc, prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxCsitxCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_object_array_t *input_desc;
    tivxCsitxParams *prms = NULL;
    uint32_t chId, bufId, instIdx;
    tivxCsitxInstParams *instParams;

    if ( (num_params != TIVX_KERNEL_CSITX_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CSITX_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CSITX_INPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_CSITX_CONFIGURATION_IDX];
        input_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CSITX_INPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxCsitxParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxCsitxParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: Could allocate memory !!!\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Initialize steady_state_started to 0 */
            prms->steady_state_started = 0;

            /* Set number of channels to number of items in input object array */
            prms->numCh = (uint8_t)input_desc->num_items;

            if (prms->numCh > TIVX_CSITX_MAX_CH)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Object descriptor number of channels exceeds max value allowed by csitx!!!\r\n");
            }
        }

        /* Setting CSITX transmit parameters */
        if ((vx_status)VX_SUCCESS == status)
        {
            tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                           prms->numCh);

            tivxCsitxSetCreateParams(prms, configuration_desc);
        }

        /* Creating frame available event */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxEventCreate(&prms->frame_available);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Event creation failed in csitx!!!\r\n");
            }
        }

        /* Creating FVID2 handle */
        if ((vx_status)VX_SUCCESS == status)
        {
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                Fvid2CbParams_init(&instParams->drvCbPrms);

                instParams->drvCbPrms.cbFxn   = (Fvid2_CbFxn) &csitxDrvCallback;
                instParams->drvCbPrms.appData = prms;

                instParams->drvHandle = Fvid2_create(CSITX_TX_DRV_ID,
                                                     instParams->instId,
                                                     &instParams->createPrms,
                                                     &instParams->createStatus,
                                                     &instParams->drvCbPrms);

                if ((NULL == instParams->drvHandle) ||
                    (instParams->createStatus.retVal != FVID2_SOK))
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Csitx Create Failed!!!\r\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
        }

        /* Creating FVID2 frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0u ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->freeFvid2FrameQ[chId], TIVX_CSITX_MAX_NUM_BUFS, prms->fvid2_free_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Csitx free queue create failed!!!\r\n");
                    break;
                }

                for(bufId = 0u ; bufId < (TIVX_CSITX_MAX_NUM_BUFS) ; bufId++)
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
                status = tivxQueueCreate(&prms->pendingFrameQ[chId], TIVX_CSITX_MAX_NUM_BUFS, prms->pending_frame_free_q_mem[chId], 0);

                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Csitx pending queue create failed!!!\r\n");
                    break;
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms, sizeof(tivxCsitxParams));
        }
        else if (NULL != prms)
        {
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                if (NULL != instParams->drvHandle)
                {
                    Fvid2_delete(instParams->drvHandle, NULL);
                    instParams->drvHandle = NULL;
                }
            }

            if (NULL != prms->frame_available)
            {
                tivxEventDelete(&prms->frame_available);
            }

            tivxMemFree(prms, sizeof(tivxCsitxParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
        else
        {
            /* do nothing */
        }
    }

    return status;
}

static void tivxCsitxPrintStatus(tivxCsitxInstParams *prms)
{
    int32_t fvid2_status;
    uint32_t cnt;

    if (NULL != prms)
    {
        fvid2_status = Fvid2_control(prms->drvHandle,
                                IOCTL_CSITX_GET_INST_STATUS,
                                &prms->csitxStatus,
                                NULL);
        if (FVID2_SOK == fvid2_status)
        {
            printf(   "==========================================================\r\n");
            printf(
                      " Csitx Status: Instance|%d\r\n", prms->instId);
            printf(
                      "==========================================================\r\n");
            printf(
                      " FIFO Overflow Count: %d\r\n",
                      prms->csitxStatus.overflowCount);
            printf(
                "  Channel Num | Frame Queue Count |"
                " Frame De-queue Count | Frame Repeat Count |\n");
            for(cnt = 0U ; cnt < prms->numCh ; cnt ++)
            {
                printf(
                      "\t\t%d|\t\t%d|\t\t%d|\t\t%d|\n",
                      cnt,
                      prms->csitxStatus.queueCount[cnt],
                      prms->csitxStatus.dequeueCount[cnt],
                      prms->csitxStatus.frmRepeatCount[cnt]);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: FVID2 Control failed !!!\n");
        }
    }
}


static vx_status VX_CALLBACK tivxCsitxDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivxCsitxParams *prms = NULL;
    static Fvid2_FrameList frmList;
    uint32_t size, chId, instIdx;
    tivxCsitxInstParams *instParams;

    if ( (num_params != TIVX_KERNEL_CSITX_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CSITX_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CSITX_INPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: Could not obtain kernel instance context !!!\n");
        }

        if(NULL == prms)
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel instance context is NULL!!!\n");
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {

            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                /* Stopping FVID2 Csitx */
                if ((vx_status)VX_SUCCESS == status)
                {
                    fvid2_status = Fvid2_stop(instParams->drvHandle, NULL);

                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: FVID2 Csitx not stopped !!!\n");
                    }
                }

                /* Dequeue all the request from the driver */
                if ((vx_status)VX_SUCCESS == status)
                {
                    Fvid2FrameList_init(&frmList);
                    do
                    {
                        fvid2_status = Fvid2_dequeue(
                            instParams->drvHandle,
                            &frmList,
                            0,
                            FVID2_TIMEOUT_NONE);
                    } while (FVID2_SOK == fvid2_status);

                    if (FVID2_ENO_MORE_BUFFERS != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: FVID2 Csitx Dequeue Failed !!!\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }

                if ((vx_status)VX_SUCCESS == status)
                {
                    tivxCsitxPrintStatus(instParams);
                }

                /* Deleting FVID2 handle */
                if ((vx_status)VX_SUCCESS == status)
                {
                    fvid2_status = Fvid2_delete(instParams->drvHandle, NULL);

                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, " CSITX: ERROR: FVID2 Delete Failed !!!\n");
                    }
                }

                /* Free-ing kernel instance params */
                if ( ((vx_status)VX_SUCCESS == status))
                {
                    instParams->drvHandle = NULL;

                    if (sizeof(tivxCsitxParams) == size)
                    {
                        tivxMemFree(prms, sizeof(tivxCsitxParams), (vx_enum)TIVX_MEM_EXTERNAL);
                    }
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

        /* Deleting event */
        if ((vx_status)VX_SUCCESS == status)
        {
            tivxEventDelete(&prms->frame_available);
        }
    }

    return status;
}

static void tivxCsitxCopyStatistics(tivxCsitxParams *prms,
    tivx_csitx_statistics_t *csitx_status_prms)
{
    uint32_t i, instIdx;
    tivxCsitxInstParams *instParams;

    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];
        for (i = 0U ; i < instParams->numCh ; i++)
        {
            csitx_status_prms->queueCount[instIdx][i]     = instParams->csitxStatus.queueCount[i];
            csitx_status_prms->dequeueCount[instIdx][i]   = instParams->csitxStatus.dequeueCount[i];
            csitx_status_prms->frmRepeatCount[instIdx][i]      = instParams->csitxStatus.frmRepeatCount[i];
        }
        csitx_status_prms->overflowCount[instIdx]         = instParams->csitxStatus.overflowCount;
    }
}

static vx_status tivxCsitxGetStatistics(tivxCsitxParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_csitx_statistics_t *csitx_status_prms = NULL;
    void *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        if (sizeof(tivx_csitx_statistics_t) ==
                usr_data_obj->mem_size)
        {
            csitx_status_prms = (tivx_csitx_statistics_t *)target_ptr;

            tivxCsitxCopyStatistics(prms, csitx_status_prms);
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

static vx_status VX_CALLBACK tivxCsitxControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t             size, instIdx;
    tivxCsitxParams *prms = NULL;
    tivxCsitxInstParams *instParams;

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == prms) ||
        (sizeof(tivxCsitxParams) != size))
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
            case TIVX_CSITX_PRINT_STATISTICS:
            {
                for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                {
                    instParams = &prms->instParams[instIdx];
                    tivxCsitxPrintStatus(instParams);
                }
                break;
            }
            case TIVX_CSITX_GET_STATISTICS:
            {
                if (NULL != obj_desc[0])
                {
                    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                    {
                        instParams = &prms->instParams[instIdx];
                        fvid2_status = Fvid2_control(instParams->drvHandle,
                                                IOCTL_CSITX_GET_INST_STATUS,
                                                &instParams->csitxStatus,
                                                NULL);
                        if (FVID2_SOK != fvid2_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Get status returned failure\n");
                            status = (vx_status)VX_FAILURE;
                            break;
                        }
                    }
                    if (FVID2_SOK == fvid2_status)
                    {
                        status = tivxCsitxGetStatistics(prms,
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


void tivxAddTargetKernelCsitx(void)
{
    vx_enum self_cpu;
    vx_uint32 i = 0;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        for (i = 0; i < CSITX_NUM_TARGETS; i++)
        {
            vx_csitx_target_kernel[i] = tivxAddTargetKernelByName(
                                TIVX_KERNEL_CSITX_NAME,
                                target_name[i],
                                tivxCsitxProcess,
                                tivxCsitxCreate,
                                tivxCsitxDelete,
                                tivxCsitxControl,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelCsitx(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_uint32 i = 0;

    for (i = 0; i < CSITX_NUM_TARGETS; i++)
    {
        status = tivxRemoveTargetKernel(vx_csitx_target_kernel[i]);
        if ((vx_status)VX_SUCCESS == status)
        {
            vx_csitx_target_kernel[i] = NULL;
        }
    }
}

static void tivxCsitxGetChannelIndices(tivxCsitxParams *prms,
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

static uint32_t tivxCsitxGetNodeChannelNum(tivxCsitxParams *prms,
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

static uint32_t tivxCsitxGetDrvInstIndex(const tivxCsitxParams *prms,
                                           uint32_t instId)
{
    uint32_t instIdx, instVal;

    instVal = tivxCsitxMapInstId(instId);
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
