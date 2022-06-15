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
#include "VX/vx.h"
#include "TI/tivx_event.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_display.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_display_priv.h"
#include <TI/tivx_queue.h>
#include <ti/drv/dss/dss.h>

#define DISPLAY_MAX_VALID_PLANES                      2U
#define DISPLAY_MAX_COPY_BUFFERS                      2U

typedef struct
{
    Fvid2_PosConfig     posCfg;
    Dss_FrameRtParams   inFrmPrms;
    Dss_FrameRtParams   outFrmPrms;
    Dss_ScRtParams      scPrms;
    Dss_DispRtParams    rtPrms;
    tivx_obj_desc_t     *obj_desc;
} tivxDisplayRtParams;

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
    tivxDisplayRtParams dispRtPrms[TIVX_DISPLAY_MAX_NUM_BUFS];

    uint64_t chromaBufAddr;
    /**< Chroma Buffer Address, used when input frame format is U16 */
    uint32_t chromaBufSize;
    /**< Size of the chroma buffer */
    tivx_shared_mem_ptr_t chroma_mem_ptr;

    /**< Id of active channel */
    uint32_t active_channel;

    uint32_t offset0;
    uint32_t offset1;
    tivx_display_crop_params_t cropPrms;
    uint32_t bpp;

    uint32_t isRtPrmsUpdated;
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
static vx_status tivxDisplayExtractFvid2Format(
    tivxDisplayParams *dispPrms, tivx_display_params_t *inDispPrms,
    const tivx_obj_desc_image_t *obj_desc_img, Fvid2_Format *format);
static int32_t tivxDisplayCallback(Fvid2_Handle handle, void *appData);
static uint32_t tivxDisplayGetPipeType(uint32_t drvId);
static int32_t tivxDisplayGetImageSize(const tivx_obj_desc_image_t *obj_desc_image,
                                       uint32_t *copySize0,
                                       uint32_t *copySize1);
static vx_status tivxDisplayAllocChromaBuff(tivxDisplayParams *dispPrms,
                                            const Fvid2_Format *fmt,
                                            const tivx_obj_desc_image_t *obj_desc_img);
static void tivxDisplayCalcOffset(tivxDisplayParams *dispPrms);

static vx_status VX_CALLBACK tivxDisplayControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxDisplayFreeChromaBuff(tivxDisplayParams *dispPrms);
static vx_status tivxDisplaySwitchChannel(tivxDisplayParams *dispPrms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDisplaySetCropParams(tivxDisplayParams *dispPrms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);

static vx_status tivxDisplaySwitchChannel(tivxDisplayParams *dispPrms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                             status = (vx_status)VX_SUCCESS;
    tivx_display_select_channel_params_t *ch_prms = NULL;
    void                                 *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_display_select_channel_params_t) ==
                usr_data_obj->mem_size)
        {
            ch_prms = (tivx_display_select_channel_params_t *)target_ptr;
            dispPrms->active_channel = ch_prms->active_channel_id;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxDisplaySetCropParams(tivxDisplayParams *dispPrms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                   status = (vx_status)VX_SUCCESS;
    tivx_display_crop_params_t *ch_prms = NULL;
    void                       *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_display_crop_params_t) ==
                usr_data_obj->mem_size)
        {
            ch_prms = (tivx_display_crop_params_t *)target_ptr;
            memcpy(&dispPrms->cropPrms, ch_prms, sizeof(tivx_display_crop_params_t));
            dispPrms->isRtPrmsUpdated = 1;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxDisplayExtractFvid2Format(
    tivxDisplayParams *dispPrms, tivx_display_params_t *inDispPrms,
    const tivx_obj_desc_image_t *obj_desc_img, Fvid2_Format *format)
{
    vx_status status = (vx_status)VX_SUCCESS;

    Fvid2Format_init(format);

    if (inDispPrms->enableCropping)
    {
        format->width = inDispPrms->cropPrms.width;
        format->height = inDispPrms->cropPrms.height;

        memcpy(&dispPrms->cropPrms,
            &inDispPrms->cropPrms, sizeof(dispPrms->cropPrms));
    }
    else
    {
        format->width = obj_desc_img->imagepatch_addr[0].dim_x;
        format->height = obj_desc_img->imagepatch_addr[0].dim_y;
    }
    format->ccsFormat = FVID2_CCSF_BITS8_PACKED;
    format->scanFormat = FVID2_SF_PROGRESSIVE;

    switch (obj_desc_img->format)
    {
        case (vx_df_image)TIVX_DF_IMAGE_RGB565:
            format->dataFormat = FVID2_DF_BGR16_565;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 2;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGB:
            format->dataFormat = FVID2_DF_RGB24_888;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 3;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGBX:
            format->dataFormat = FVID2_DF_RGBX24_8888;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 4;
            break;
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
            format->dataFormat = FVID2_DF_BGRX32_8888;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 4;
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
            format->dataFormat = FVID2_DF_YUV422I_UYVY;
            format->pitch[FVID2_YUV_INT_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 2;
            break;
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            format->dataFormat = FVID2_DF_YUV422I_YUYV;
            format->pitch[FVID2_YUV_INT_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 2;
            break;
        case (vx_df_image)VX_DF_IMAGE_NV12:
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[1].stride_y;
            dispPrms->bpp = 1;
            break;
        case (vx_df_image)VX_DF_IMAGE_U16:
            format->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 2;
            break;
        case (vx_df_image)VX_DF_IMAGE_U8:
            format->ccsFormat = FVID2_CCSF_BITS8_PACKED;
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            dispPrms->bpp = 1;
            break;
        default:
            status = (vx_status)VX_FAILURE;
            break;
    }

    return status;
}

static vx_status tivxDisplayAllocChromaBuff(tivxDisplayParams *dispPrms,
                                            const Fvid2_Format *fmt,
                                            const tivx_obj_desc_image_t *obj_desc_img)
{
    int32_t status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    void *chroma_target_ptr;
    uint16_t *chroma_ptr16;
    uint8_t *chroma_ptr8;

    if (((vx_df_image)VX_DF_IMAGE_U16 == obj_desc_img->format)||((vx_df_image)VX_DF_IMAGE_U8 == obj_desc_img->format))
    {
        dispPrms->chromaBufSize =
            (fmt->pitch[1] * fmt->height) / 2u;
        status = tivxMemBufferAlloc(&dispPrms->chroma_mem_ptr,
            dispPrms->chromaBufSize, (vx_enum)TIVX_MEM_EXTERNAL);
        if ((vx_status)VX_SUCCESS == status)
        {
            dispPrms->chromaBufAddr =
                tivxMemShared2PhysPtr(dispPrms->chroma_mem_ptr.shared_ptr,
                (int32_t)dispPrms->chroma_mem_ptr.mem_heap_region);

            chroma_target_ptr = tivxMemShared2TargetPtr(&dispPrms->chroma_mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(chroma_target_ptr, dispPrms->chromaBufSize,
                             (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

            if ((vx_df_image)VX_DF_IMAGE_U16 == obj_desc_img->format)
            {
                chroma_ptr16 = (uint16_t *)chroma_target_ptr;
                for (cnt = 0; cnt < (dispPrms->chromaBufSize / 2U); cnt ++)
                {
                       *chroma_ptr16 = 0x800u;
                       chroma_ptr16 ++;
                }
            }
            else if ((vx_df_image)VX_DF_IMAGE_U8 == obj_desc_img->format)
            {
                chroma_ptr8 = (uint8_t *)chroma_target_ptr;
                for (cnt = 0; cnt < dispPrms->chromaBufSize  ; cnt ++)
                {
                       *chroma_ptr8 = 0x80u;
                       chroma_ptr8 ++;
                }
            }
            else
            {
                /* do nothing */
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(chroma_target_ptr, dispPrms->chromaBufSize,
                             (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
        }
    }
    else
    {
        dispPrms->chromaBufAddr = (uint64_t)NULL;
        dispPrms->chromaBufSize = 0x0;
    }

    return (status);
}

static void tivxDisplayFreeChromaBuff(tivxDisplayParams *dispPrms)
{
    if (dispPrms->chromaBufSize > 0u)
    {
        tivxMemBufferFree(&dispPrms->chroma_mem_ptr, dispPrms->chromaBufSize);
    }
}

static int32_t tivxDisplayCallback(Fvid2_Handle handle, void *appData)
{
    tivxDisplayParams *displayParams = (tivxDisplayParams *)(appData);
    SemaphoreP_post(displayParams->waitSem);
    return (vx_status)VX_SUCCESS;
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

static int32_t tivxDisplayGetImageSize(const tivx_obj_desc_image_t *obj_desc_image,
                                       uint32_t *copySize0,
                                       uint32_t *copySize1)
{
    vx_status status = (vx_status)VX_SUCCESS;
    switch (obj_desc_image->format)
    {
        case (vx_df_image)VX_DF_IMAGE_RGB:
            *copySize0 = (uint32_t)obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
            *copySize0 = (uint32_t)obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGBX:
            *copySize0 = (uint32_t)obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case (vx_df_image)VX_DF_IMAGE_YUYV:
        case (vx_df_image)VX_DF_IMAGE_UYVY:
            *copySize0 = (uint32_t)obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            break;
        case (vx_df_image)VX_DF_IMAGE_NV12:
            *copySize0 = (uint32_t)obj_desc_image->imagepatch_addr[0].stride_y * obj_desc_image->height;
            *copySize1 = (uint32_t)obj_desc_image->imagepatch_addr[1].stride_y * obj_desc_image->height;
            break;
        default:
            *copySize0 = 0;
            *copySize1 = 0;
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            break;
    }
    return status;
}

static vx_status VX_CALLBACK tivxDisplayCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
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
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        obj_desc_configuration = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX];
        obj_desc_image = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];

        displayParams = tivxMemAlloc(sizeof(tivxDisplayParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if(NULL != displayParams)
        {
            memset(displayParams, 0, sizeof(tivxDisplayParams));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Couldn't allocate memory!\r\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if(sizeof(tivx_display_params_t) != obj_desc_configuration->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Display params size is not correct!\r\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        if((vx_status)VX_SUCCESS == status)
        {
            display_config_target_ptr = tivxMemShared2TargetPtr(&obj_desc_configuration->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(display_config_target_ptr,
                             obj_desc_configuration->mem_size,
                             (vx_enum)VX_MEMORY_TYPE_HOST,
                             (vx_enum)VX_READ_ONLY));

            params = (tivx_display_params_t *)display_config_target_ptr;

            drvId = params->pipeId;
            if(drvId >= DSS_DISP_INST_MAX)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Invalid pipe id used by display!\r\n");
            }
        }
        if((vx_status)VX_SUCCESS == status)
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
                status = (vx_status)VX_FAILURE;
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
                   (FVID2_SOK != displayParams->createStatus.retVal))
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Display Create Failed!\r\n");
                }
            }
        }
        if((vx_status)VX_SUCCESS == status)
        {
            Dss_dispParamsInit(&displayParams->dispParams);
            displayParams->dispParams.pipeCfg.pipeType = tivxDisplayGetPipeType(drvId);
            displayParams->dispParams.pipeCfg.outWidth = params->outWidth;
            displayParams->dispParams.pipeCfg.outHeight = params->outHeight;
            displayParams->dispParams.layerPos.startX = params->posX;
            displayParams->dispParams.layerPos.startY = params->posY;
            status = tivxDisplayExtractFvid2Format(displayParams,
                params, obj_desc_image,
                &displayParams->dispParams.pipeCfg.inFmt);
            if((displayParams->dispParams.pipeCfg.inFmt.width != displayParams->dispParams.pipeCfg.outWidth) ||
               (displayParams->dispParams.pipeCfg.inFmt.height != displayParams->dispParams.pipeCfg.outHeight))
            {
                displayParams->dispParams.pipeCfg.scEnable = TRUE;
            }

            tivxDisplayCalcOffset(displayParams);
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxDisplayAllocChromaBuff(displayParams,
                &displayParams->dispParams.pipeCfg.inFmt,
                obj_desc_image);
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            if(TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE == params->opMode)
            {
                status = tivxDisplayGetImageSize(obj_desc_image,
                                                 &displayParams->copyImageSize[0],
                                                 &displayParams->copyImageSize[1]);

                if((vx_status)VX_SUCCESS == status)
                {
                    if(displayParams->copyImageSize[0] != 0U)
                    {
                        displayParams->copyImagePtr[0][0] = tivxMemAlloc(displayParams->copyImageSize[0], (vx_enum)TIVX_MEM_EXTERNAL);
                        if(displayParams->copyImagePtr[0][0] == NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Couldn't allocate memory for copy buffer!\r\n");
                        }
                        displayParams->copyImagePtr[1][0] = tivxMemAlloc(displayParams->copyImageSize[0], (vx_enum)TIVX_MEM_EXTERNAL);
                        if(displayParams->copyImagePtr[1][0] == NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Couldn't allocate memory for copy buffer!\r\n");
                        }
                    }
                    if((displayParams->copyImageSize[1] != 0U) && ((vx_df_image)VX_DF_IMAGE_NV12 == obj_desc_image->format))
                    {
                        displayParams->copyImagePtr[0][1] = tivxMemAlloc(displayParams->copyImageSize[1], (vx_enum)TIVX_MEM_EXTERNAL);
                        if(displayParams->copyImagePtr[0][1] == NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Couldn't allocate memory for copy buffer!\r\n");
                        }
                        displayParams->copyImagePtr[1][1] = tivxMemAlloc(displayParams->copyImageSize[1], (vx_enum)TIVX_MEM_EXTERNAL);
                        if(displayParams->copyImagePtr[1][1] == NULL)
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Couldn't allocate memory for copy buffer!\r\n");
                        }
                    }
                }
                displayParams->currIdx = 0;
            }
        }
        if((vx_status)VX_SUCCESS == status)
        {
            fvid2_status = Fvid2_control(displayParams->drvHandle,
                                   IOCTL_DSS_DISP_SET_DSS_PARAMS,
                                   &displayParams->dispParams,
                                   NULL);
            if(FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Display Set Parameters Failed!\r\n");
            }
        }
        /* Creating FVID2 frame Q */
        if(((vx_status)VX_SUCCESS == status) && (TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE == displayParams->opMode))
        {
            uint32_t bufId;
            status = tivxQueueCreate(&displayParams->fvid2FrameQ, TIVX_DISPLAY_MAX_NUM_BUFS, displayParams->fvid2FrameQMem, 0);
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Fvid2 queue create failed!\r\n");
            }

            for(bufId = 0; bufId <TIVX_DISPLAY_MAX_NUM_BUFS; bufId++)
            {
                displayParams->fvid2Frames[bufId].appData =
                    &displayParams->dispRtPrms[bufId];
                tivxQueuePut(&displayParams->fvid2FrameQ, (uintptr_t)&displayParams->fvid2Frames[bufId], TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
        }

        if((vx_status)VX_SUCCESS == status)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(display_config_target_ptr,
                               obj_desc_configuration->mem_size,
                               (vx_enum)VX_MEMORY_TYPE_HOST,
                               (vx_enum)VX_READ_ONLY));
            tivxSetTargetKernelInstanceContext(kernel,
                                               displayParams,
                                               sizeof(tivxDisplayParams));
        }
        else
        {
            if(NULL != displayParams)
            {
                tivxMemFree(displayParams, sizeof(tivxDisplayParams), (vx_enum)TIVX_MEM_EXTERNAL);
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
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivxDisplayParams *displayParams = NULL;
    Fvid2_FrameList frmList;
    uint32_t size;
    tivx_obj_desc_image_t *obj_desc_image;

    if((num_params != TIVX_KERNEL_DISPLAY_MAX_PARAMS) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX]))
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Invalid parameters!\r\n");
    }
    else
    {
        obj_desc_image = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX];
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **) &displayParams,
                                                    &size);

        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Could not obtain display kernel instance context!\r\n");
        }

        if (NULL == displayParams)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Display params is NULL!\r\n");
        }

        /* Stop Display */
        if((vx_status)VX_SUCCESS == status)
        {
            fvid2_status = Fvid2_stop(displayParams->drvHandle, NULL);

            if(FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: FVID2 Stop Failed!\r\n");
            }
        }

        /* Dequeue all the request from the driver */
        if((vx_status)VX_SUCCESS == status)
        {
            do
            {
                fvid2_status = Fvid2_dequeue(displayParams->drvHandle,
                                       &frmList,
                                       0,
                                       FVID2_TIMEOUT_NONE);
            } while(FVID2_SOK == fvid2_status);

            /* Delete FVID2 handle */
            fvid2_status = Fvid2_delete(displayParams->drvHandle, NULL);

            if(FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: FVID2 Delete Failed!\r\n");
            }
        }

        /* Deleting FVID2 frame Q */
        if(((vx_status)VX_SUCCESS == status) && (TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE == displayParams->opMode))
        {
            tivxQueueDelete(&displayParams->fvid2FrameQ);
        }

        /* Delete the wait semaphore */
        if(((vx_status)VX_SUCCESS == status) && (NULL != displayParams->waitSem))
        {
            SemaphoreP_delete(displayParams->waitSem);
            displayParams->waitSem = NULL;
        }

        /* Delete kernel instance params object */
        if(((vx_status)VX_SUCCESS == status) && (NULL != displayParams))
        {
            displayParams->drvHandle = NULL;

            tivxDisplayFreeChromaBuff(displayParams);

            if(TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE == displayParams->opMode)
            {
                if(displayParams->copyImageSize[0] != 0U)
                {
                    if(displayParams->copyImagePtr[0][0])
                    {
                        tivxMemFree(displayParams->copyImagePtr[0][0], displayParams->copyImageSize[0], (vx_enum)TIVX_MEM_EXTERNAL);
                    }
                    if(displayParams->copyImagePtr[1][0])
                    {
                        tivxMemFree(displayParams->copyImagePtr[1][0], displayParams->copyImageSize[0], (vx_enum)TIVX_MEM_EXTERNAL);
                    }
                }
                if((displayParams->copyImageSize[1] != 0U) && ((vx_df_image)VX_DF_IMAGE_NV12 == obj_desc_image->format))
                {
                    if(displayParams->copyImagePtr[0][1])
                    {
                        tivxMemFree(displayParams->copyImagePtr[0][1], displayParams->copyImageSize[1], (vx_enum)TIVX_MEM_EXTERNAL);
                    }
                    if(displayParams->copyImagePtr[1][1])
                    {
                        tivxMemFree(displayParams->copyImagePtr[1][1], displayParams->copyImageSize[1], (vx_enum)TIVX_MEM_EXTERNAL);
                    }
                }
            }

            if(sizeof(tivxDisplayParams) == size)
            {
                tivxMemFree(displayParams, sizeof(tivxDisplayParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = (vx_status)VX_SUCCESS;
    uint32_t             size;
    tivxDisplayParams   *dispPrms = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&dispPrms, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == dispPrms) ||
        (sizeof(tivxDisplayParams) != size))
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
            case TIVX_DISPLAY_SELECT_CHANNEL:
            {
                status = tivxDisplaySwitchChannel(dispPrms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DISPLAY_SET_CROP_PARAMS:
            {
                status = tivxDisplaySetCropParams(dispPrms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
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

    return (status);
}

static void tivxDisplayCalcOffset(tivxDisplayParams *dispPrms)
{
    Fvid2_Format *fmt;

    if (NULL != dispPrms)
    {
        fmt = &dispPrms->dispParams.pipeCfg.inFmt;

        dispPrms->offset0 = (dispPrms->cropPrms.startY * fmt->pitch[0]) +
            dispPrms->cropPrms.startX * dispPrms->bpp;
        if (fmt->dataFormat == FVID2_DF_YUV420SP_UV)
        {
            dispPrms->offset1 =
                ((dispPrms->cropPrms.startY / 2) * fmt->pitch[0]) +
                dispPrms->cropPrms.startX * dispPrms->bpp;
        }
        else
        {
            dispPrms->offset1 = dispPrms->offset0;
        }
    }
}

void tivxDisplayUpdateRtParams(tivxDisplayParams *dispPrms, Fvid2_Frame *frm)
{
    uint32_t             cnt;
    tivxDisplayRtParams *rtPrms;
    Fvid2_Format        *fmt;

    if (dispPrms->isRtPrmsUpdated)
    {
        rtPrms = (tivxDisplayRtParams *)frm->appData;
        fmt = &dispPrms->dispParams.pipeCfg.inFmt;

        if (NULL != rtPrms)
        {
            rtPrms->inFrmPrms.width = dispPrms->cropPrms.width;
            rtPrms->inFrmPrms.height = dispPrms->cropPrms.height;
            for (cnt = 0; cnt < FVID2_MAX_PLANES; cnt ++)
            {
                rtPrms->inFrmPrms.pitch[cnt] = fmt->pitch[cnt];
            }
            rtPrms->inFrmPrms.dataFormat = fmt->dataFormat;

            memset(&rtPrms->rtPrms, 0, sizeof(Dss_DispRtParams));
            rtPrms->rtPrms.inFrmParams = &rtPrms->inFrmPrms;
            frm->perFrameCfg = (void *)&rtPrms->rtPrms;

            tivxDisplayCalcOffset(dispPrms);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Appdata is null !!!\n");
        }

        dispPrms->isRtPrmsUpdated = 0u;
    }
}

static vx_status VX_CALLBACK tivxDisplayProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivxDisplayParams *displayParams = NULL;
    tivx_obj_desc_image_t *obj_desc_image;
    void *image_target_ptr1, *image_target_ptr2 = NULL;
    uint32_t size;
    Fvid2_FrameList frmList;
    Fvid2_Frame *frm;
    tivxDisplayRtParams *rtPrms;

    if((num_params != TIVX_KERNEL_DISPLAY_MAX_PARAMS) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_CONFIGURATION_IDX]) ||
       (NULL == obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX]))
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Parameters are NULL!\r\n");
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void *)&displayParams,
                                                    &size);

        if(((vx_status)VX_SUCCESS != status) ||
           (NULL == displayParams) ||
           (sizeof(tivxDisplayParams) != size))
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Instance context is NULL!\r\n");
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        uint32_t active_channel;

        active_channel = displayParams->active_channel;

        obj_desc_image = (tivx_obj_desc_image_t *)
            tivxGetObjDescElement(obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX],
                                  active_channel);

        if (obj_desc_image == NULL)
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        image_target_ptr1 = tivxMemShared2TargetPtr(&obj_desc_image->mem_ptr[0]);
        if((vx_df_image)VX_DF_IMAGE_NV12 == obj_desc_image->format)
        {
            image_target_ptr2 = tivxMemShared2TargetPtr(&obj_desc_image->mem_ptr[1]);

        }

        if(TIVX_KERNEL_DISPLAY_ZERO_BUFFER_COPY_MODE == displayParams->opMode)
        {
            /* Get frame from queue */
            tivxQueueGet(&displayParams->fvid2FrameQ, (uintptr_t*)&frm, TIVX_EVENT_TIMEOUT_NO_WAIT);
            if(NULL != frm)
            {
                tivxDisplayUpdateRtParams(displayParams, frm);

                /* Assign buffer addresses */
                frm->addr[0U] = ((uint64_t)image_target_ptr1) +
                    displayParams->offset0;
                if((vx_df_image)VX_DF_IMAGE_NV12 == obj_desc_image->format)
                {
                    frm->addr[1U] = (uint64_t)image_target_ptr2 +
                        displayParams->offset1;
                }

                if (((vx_df_image)VX_DF_IMAGE_U16 == obj_desc_image->format)||((vx_df_image)VX_DF_IMAGE_U8 == obj_desc_image->format))
                {
                    frm->addr[1U] = displayParams->chromaBufAddr;
                }
                frm->fid = FVID2_FID_FRAME;

                rtPrms = (tivxDisplayRtParams *)frm->appData;
                rtPrms->obj_desc = (tivx_obj_desc_t*)obj_desc_image;

                /* Create frame list */
                frmList.numFrames  = 1U;
                frmList.frames[0U] = frm;
                /* Call Fvid2 Queue */
                fvid2_status = Fvid2_queue(displayParams->drvHandle, &frmList, 0U);
                if(FVID2_SOK != fvid2_status)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Unable to queue frame!\r\n");
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Could not get frame from queue!\r\n");
            }

            if(TRUE == displayParams->firstFrameDisplay)
            {
                fvid2_status = Fvid2_start(displayParams->drvHandle, NULL);
                if(FVID2_SOK != fvid2_status)
                {
                    status = (vx_status)VX_FAILURE;
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
                    fvid2_status = Fvid2_dequeue(displayParams->drvHandle,
                                        &frmList,
                                        0U,
                                        FVID2_TIMEOUT_NONE);
                } while(FVID2_EAGAIN == fvid2_status);
                if((1U == frmList.numFrames) && (FVID2_SOK == fvid2_status))
                {
                    frm = frmList.frames[0U];

                    frm->perFrameCfg = NULL;
                    rtPrms = (tivxDisplayRtParams *)frm->appData;
                    /* Return frame */
                    obj_desc[TIVX_KERNEL_DISPLAY_INPUT_IMAGE_IDX] = rtPrms->obj_desc;
                    tivxQueuePut(&displayParams->fvid2FrameQ, (uintptr_t)frm, TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
                else /* (1U != frmList.numFrames) || (((vx_status)VX_SUCCESS != status) && (FVID2_EAGAIN != status))*/
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Dequeue operation failed!\r\n");
                }
            }
        }
        else if(TIVX_KERNEL_DISPLAY_BUFFER_COPY_MODE == displayParams->opMode)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(
                displayParams->copyImagePtr[displayParams->currIdx][0],
                displayParams->copyImageSize[0],
                (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(
                image_target_ptr1,
                displayParams->copyImageSize[0],
                (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            /* Copy  and assign buffers */
            memcpy(displayParams->copyImagePtr[displayParams->currIdx][0], image_target_ptr1, displayParams->copyImageSize[0]);

            tivxCheckStatus(&status, tivxMemBufferUnmap(
                displayParams->copyImagePtr[displayParams->currIdx][0],
                displayParams->copyImageSize[0],
                (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(
                image_target_ptr1,
                displayParams->copyImageSize[0],
                (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            displayParams->copyFrame[displayParams->currIdx].addr[0] = (uint64_t)displayParams->copyImagePtr[displayParams->currIdx][0];
            if((vx_df_image)VX_DF_IMAGE_NV12 == obj_desc_image->format)
            {
                tivxCheckStatus(&status, tivxMemBufferMap(
                    displayParams->copyImagePtr[displayParams->currIdx][1],
                    displayParams->copyImageSize[1],
                    (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
                tivxCheckStatus(&status, tivxMemBufferMap(
                    image_target_ptr2,
                    displayParams->copyImageSize[1],
                    (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));

                if (NULL != image_target_ptr2)
                {
                    memcpy(displayParams->copyImagePtr[displayParams->currIdx][1], image_target_ptr2, displayParams->copyImageSize[1]);
                }

                tivxCheckStatus(&status, tivxMemBufferUnmap(
                    displayParams->copyImagePtr[displayParams->currIdx][1],
                    displayParams->copyImageSize[1],
                    (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
                tivxCheckStatus(&status, tivxMemBufferUnmap(
                    image_target_ptr2,
                    displayParams->copyImageSize[1],
                    (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));

                displayParams->copyFrame[displayParams->currIdx].addr[1] = (uint64_t)displayParams->copyImagePtr[displayParams->currIdx][1];
            }
            displayParams->copyFrame[displayParams->currIdx].fid = FVID2_FID_FRAME;
            displayParams->copyFrame[displayParams->currIdx].appData = NULL;

            /* Create frame list */
            frmList.numFrames  = 1U;
            frmList.frames[0U] = &displayParams->copyFrame[displayParams->currIdx];
            /* Call Fvid2 Queue */
            fvid2_status = Fvid2_queue(displayParams->drvHandle, &frmList, 0U);
            if(FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Unable to queue copy frame!\r\n");
            }
            if(TRUE == displayParams->firstFrameDisplay)
            {
                fvid2_status = Fvid2_start(displayParams->drvHandle, NULL);
                if(FVID2_SOK != fvid2_status)
                {
                    status = (vx_status)VX_FAILURE;
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
                    fvid2_status = Fvid2_dequeue(displayParams->drvHandle,
                                           &frmList,
                                           0U,
                                           FVID2_TIMEOUT_NONE);
                } while(FVID2_EAGAIN == fvid2_status);
                if((1U == frmList.numFrames) && (FVID2_SOK == fvid2_status))
                {
                    /* Change Curr Index */
                    if(displayParams->currIdx == 0U)
                    {
                        displayParams->currIdx = 1;
                    }
                    else
                    {
                        displayParams->currIdx = 0;
                    }
                }
                else /* (1U != frmList.numFrames) || (((vx_status)VX_SUCCESS != status) && (FVID2_EAGAIN != status))*/
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Dequeue operation failed!\r\n");
                }
            }
        }
        else
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY: ERROR: Wrong Operation Mode Selected!\r\n");
        }
    }

    return status;
}

void tivxAddTargetKernelDisplay(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
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

void tivxRemoveTargetKernelDisplay(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_display_target_kernel1);
    if(status == (vx_status)VX_SUCCESS)
    {
        vx_display_target_kernel1 = NULL;
    }
    status = tivxRemoveTargetKernel(vx_display_target_kernel2);
    if(status == (vx_status)VX_SUCCESS)
    {
        vx_display_target_kernel2 = NULL;
    }
}
