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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "TI/tivx.h"
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_video_encoder.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "mm_enc.h"
#include <utils/mem/include/app_mem.h>
#include "utils/udma/include/app_udma.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define VIDEO_ENCODER_MAX_HANDLES      (2U)
#define MM_SUCCESS                     (0U)
#define HW_ALIGN                       (64U)
#define ALIGN_SIZE(x,y)                (((x + (y-1)) / y) * y)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    uint32_t                            channel_id;
    struct mm_buffer                    in_buff;
    struct mm_buffer                    out_buff;
    uint8_t                             processFlag;
    tivx_event                          waitForProcessCmpl;
    uint8_t                             which_buff;
} tivxVideoEncoderObj;

typedef struct
{
    tivx_mutex lock;
    tivxVideoEncoderObj videoEncoderObj[VIDEO_ENCODER_MAX_HANDLES];
} tivxVideoEncoderInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVideoEncoderProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVideoEncoderCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVideoEncoderDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVideoEncoderControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static tivxVideoEncoderObj *tivxVideoEncoderAllocObject(
       tivxVideoEncoderInstObj *instObj);
static void tivxVideoEncoderFreeObject(
       tivxVideoEncoderInstObj *instObj,
       tivxVideoEncoderObj *encoder_obj);
void tivxVideoEncoderErrorCb();

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_video_encoder_target_kernel = NULL;

static tivxVideoEncoderInstObj gTivxVideoEncoderInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVideoEncoder(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VENC1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxAddTargetKernelVideoEncoder: Invalid CPU\n");
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_video_encoder_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VIDEO_ENCODER_NAME,
                            target_name,
                            tivxVideoEncoderProcess,
                            tivxVideoEncoderCreate,
                            tivxVideoEncoderDelete,
                            tivxVideoEncoderControl,
                            NULL);
        if (NULL != vx_video_encoder_target_kernel)
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxVideoEncoderInstObj.lock);
            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelVideoEncoder: Failed to create Mutex\n");
            }
            else
            {
                memset(&gTivxVideoEncoderInstObj.videoEncoderObj, 0x0U,
                    sizeof(tivxVideoEncoderObj) * VIDEO_ENCODER_MAX_HANDLES);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxAddTargetKernelVideoEncoder: Failed to Add Video Encoder TargetKernel\n");
            status = VX_FAILURE;
        }

    }
}

void tivxRemoveTargetKernelVideoEncoder(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_video_encoder_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_video_encoder_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxRemoveTargetKernelVideoEncoder: Failed to Remove Video Encoder TargetKernel\n");
    }
    if (0 != gTivxVideoEncoderInstObj.lock)
    {
        tivxMutexDelete(&gTivxVideoEncoderInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVideoEncoderProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    int32_t                           mm_status = MM_SUCCESS;
    uint32_t                          size;
    tivxVideoEncoderObj              *encoder_obj = NULL;
    uint8_t                          *bitstream;
    tivx_obj_desc_image_t            *input_image_desc;
    tivx_obj_desc_user_data_object_t *output_bitstream_desc;
    void                             *input_image_target_ptr_y;
    void 			     *input_image_target_ptr_uv;
    void                             *output_bitstream_target_ptr;

    if ( (num_params != TIVX_KERNEL_VIDEO_ENCODER_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_ENCODER_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_ENCODER_INPUT_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_ENCODER_OUTPUT_BITSTREAM_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderProcess: Invalid Descriptor\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&encoder_obj, &size);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderProcess: Null Desc\n");
        }
        else if (sizeof(tivxVideoEncoderObj) != size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderProcess: Incorrect object size\n");
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VIDEO_ENCODER_INPUT_IMAGE_IDX];
        output_bitstream_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VIDEO_ENCODER_OUTPUT_BITSTREAM_IDX];

        input_image_target_ptr_y = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);

        input_image_target_ptr_uv = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);

        output_bitstream_target_ptr = tivxMemShared2TargetPtr(&output_bitstream_desc->mem_ptr);

        tivxMemBufferMap(input_image_target_ptr_y, input_image_desc->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferMap(input_image_target_ptr_uv, input_image_desc->mem_size[1],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferMap(output_bitstream_target_ptr, output_bitstream_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        bitstream = (uint8_t*) output_bitstream_target_ptr;
    }

    if (VX_SUCCESS == status)
    {
        encoder_obj->processFlag = 0;

        encoder_obj->in_buff.chId = encoder_obj->channel_id;
        encoder_obj->in_buff.type = MM_BUF_TYPE_VIDEO_INPUT;
	encoder_obj->in_buff.num_planes = input_image_desc->planes;
        encoder_obj->in_buff.size[0] = input_image_desc->mem_size[0];
	encoder_obj->in_buff.size[1] = input_image_desc->mem_size[1];
        encoder_obj->in_buff.buf_addr[0] = input_image_target_ptr_y;
	encoder_obj->in_buff.buf_addr[1] = input_image_target_ptr_uv;

        encoder_obj->out_buff.chId = encoder_obj->channel_id;
        encoder_obj->out_buff.type = MM_BUF_TYPE_VIDEO_OUTPUT;
	encoder_obj->out_buff.num_planes = 1;
        encoder_obj->out_buff.size[0] = output_bitstream_desc->mem_size;
        encoder_obj->out_buff.buf_addr[0] = bitstream;

        mm_status = MM_ENC_BufPrepare(&encoder_obj->in_buff, encoder_obj->channel_id);
        mm_status |= MM_ENC_BufPrepare(&encoder_obj->out_buff, encoder_obj->channel_id);

        if(MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderProcess: MM_ENC Buf Prepare failed\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        mm_status = MM_ENC_Process(&encoder_obj->in_buff, &encoder_obj->out_buff, encoder_obj->channel_id);

        if (MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderProcess: MM_ENC Process failed\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxEventWait(encoder_obj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderProcess: tivxEventWait failed\n");
		}
    }

    if(VX_SUCCESS == status)
    {
        output_bitstream_desc->valid_mem_size = encoder_obj->out_buff.size[0];

        tivxMemBufferUnmap(input_image_target_ptr_y, input_image_desc->mem_size[0],
           VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr_uv, input_image_desc->mem_size[1],
           VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(output_bitstream_target_ptr, output_bitstream_desc->mem_size,
           VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }
    return status;
}

static vx_status VX_CALLBACK tivxVideoEncoderCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    int32_t                           mm_status = MM_SUCCESS;
    vx_df_image                       input_image_fmt;
    mm_vid_create_params             venc_params;
    tivxVideoEncoderObj              *encoder_obj = NULL;
    tivx_video_encoder_params_t      *encoder_params;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t            *input_image_desc;
    void                             *configuration_target_ptr;

    if ( (num_params != TIVX_KERNEL_VIDEO_ENCODER_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_ENCODER_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_ENCODER_INPUT_IMAGE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_ENCODER_OUTPUT_BITSTREAM_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderCreate: Invalid Descriptor\n");
        status = VX_FAILURE;
    }
    else
    {
        encoder_obj = tivxVideoEncoderAllocObject(&gTivxVideoEncoderInstObj);

        if (NULL != encoder_obj)
        {
            configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VIDEO_ENCODER_CONFIGURATION_IDX];
            input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VIDEO_ENCODER_INPUT_IMAGE_IDX];

            configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: Failed to Alloc Video Encoder Object\n");
            status = VX_ERROR_NO_RESOURCES;
        }
    }

    if (VX_SUCCESS == status)
    {
        if (configuration_desc->mem_size != sizeof(tivx_video_encoder_params_t))
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderCreate: User data object size on target does not match the size on host, possibly due to misalignment in data structure\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxEventCreate(&encoder_obj->waitForProcessCmpl);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: Failed to allocate Event\n");
        }
    }

    if (VX_SUCCESS == status)
    {
        encoder_params = (tivx_video_encoder_params_t *) configuration_target_ptr;
        input_image_fmt = input_image_desc->format;

        venc_params.width = input_image_desc->imagepatch_addr[0].dim_x;
        venc_params.height = input_image_desc->imagepatch_addr[0].dim_y;

        if (VX_DF_IMAGE_NV12 == input_image_fmt)
        {
            venc_params.in_pixelformat = MM_PIX_FMT_NV12M;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: Invalid input format\n");
            status = VX_FAILURE;
        }

        if (TIVX_BITSTREAM_FORMAT_H264 == encoder_params->bitstream_format)
        {
            venc_params.out_pixelformat = MM_PIX_FMT_H264;
        }
        else if (TIVX_BITSTREAM_FORMAT_HEVC == encoder_params->bitstream_format)
        {
            venc_params.out_pixelformat = MM_PIX_FMT_HEVC;
        }
        else if (TIVX_BITSTREAM_FORMAT_MJPEG == encoder_params->bitstream_format)
        {
            venc_params.out_pixelformat = MM_PIX_FMT_MJPEG;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: Invalid output format\n");
            status = VX_FAILURE;
        }

        mm_status = MM_ENC_Create(&venc_params, &encoder_obj->channel_id);

        if(MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: MM_ENC Create failed\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        mm_status = MM_ENC_RegisterCb(tivxVideoEncoderErrorCb, encoder_obj->channel_id);

        if(MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: MM_ENC Register Callback failed\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        mm_status = MM_ENC_StartStreaming(encoder_obj->channel_id, MM_BUF_TYPE_VIDEO_OUTPUT);
        mm_status |= MM_ENC_StartStreaming(encoder_obj->channel_id, MM_BUF_TYPE_VIDEO_INPUT);

        if (MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderCreate: MM_ENC Start Streaming failed\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, encoder_obj,
            sizeof(tivxVideoEncoderObj));
    }

    return status;
}

static vx_status VX_CALLBACK tivxVideoEncoderDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = VX_SUCCESS;
    int32_t              mm_status = MM_SUCCESS;
    uint32_t             size;
    tivxVideoEncoderObj *encoder_obj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&encoder_obj, &size);

    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderDelete: Null Desc\n");
    }
    else if (sizeof(tivxVideoEncoderObj) != size)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoEncoderDelete: Incorrect object size\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        mm_status = MM_ENC_StopStreaming(encoder_obj->channel_id, MM_BUF_TYPE_VIDEO_INPUT);
        mm_status |= MM_ENC_StopStreaming(encoder_obj->channel_id, MM_BUF_TYPE_VIDEO_OUTPUT);

        if(MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderDelete: MM_ENC_StopStreaming failed\n");
            status = VX_FAILURE;
        }

        mm_status |= MM_ENC_Destroy(encoder_obj->channel_id);

        if(MM_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoEncoderDelete: MM_ENC_Destroy failed\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        if (NULL != encoder_obj->waitForProcessCmpl)
        {
            tivxEventDelete(&encoder_obj->waitForProcessCmpl);
        }
        

        tivxVideoEncoderFreeObject(&gTivxVideoEncoderInstObj, encoder_obj);
    }

    return status;
}

static vx_status VX_CALLBACK tivxVideoEncoderControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVideoEncoderObj *tivxVideoEncoderAllocObject(
       tivxVideoEncoderInstObj *instObj)
{
    uint32_t        cnt;
    tivxVideoEncoderObj *encoder_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VIDEO_ENCODER_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->videoEncoderObj[cnt].isAlloc)
        {
            encoder_obj = &instObj->videoEncoderObj[cnt];
            memset(encoder_obj, 0x0, sizeof(tivxVideoEncoderObj));
            instObj->videoEncoderObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (encoder_obj);
}

static void tivxVideoEncoderFreeObject(tivxVideoEncoderInstObj *instObj,
    tivxVideoEncoderObj *encoder_obj)
{
    uint32_t cnt;

    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VIDEO_ENCODER_MAX_HANDLES; cnt ++)
    {
        if (encoder_obj == &instObj->videoEncoderObj[cnt])
        {
            encoder_obj->isAlloc = 0U;
            break;
        }
    }

    tivxMutexUnlock(instObj->lock);
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

void tivxVideoEncoderErrorCb(struct mm_buffer *buff, mm_enc_process_cb cb_type)
{
    tivxVideoEncoderObj *encoder_obj = NULL;
    uint32_t             cnt;

    /* Lock instance mutex */
    if (NULL != buff)
    {
        tivxMutexLock(gTivxVideoEncoderInstObj.lock);
        for (cnt = 0U; cnt < VIDEO_ENCODER_MAX_HANDLES; cnt ++)
        {
            if (1U == gTivxVideoEncoderInstObj.videoEncoderObj[cnt].isAlloc)
            {
                encoder_obj = &gTivxVideoEncoderInstObj.videoEncoderObj[cnt];
                if(encoder_obj->channel_id == buff->chId)
                {
                    break;
                }
            }
        }
        /* Release instance mutex */
        tivxMutexUnlock(gTivxVideoEncoderInstObj.lock);

        if (NULL != encoder_obj)
        {
            if (encoder_obj->channel_id == buff->chId)
            {
                switch (cb_type) {
                case MM_CB_SRC_FRAME_RELEASE:
                    encoder_obj->processFlag |= 1;
                    break;
                case MM_CB_CODED_BUFF_READY:
                    encoder_obj->processFlag |= 2;
                    break;
                case MM_CB_ENC_STR_END:
                    break;
                case MM_CB_ENC_ERROR_FATAL:
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVideoEncoderErrorCb: MM_CB_ERROR_FATAL\n");
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVideoEncoderErrorCb: default\n");
                    break;
                }
                if (3 == encoder_obj->processFlag)
                {
                    tivxEventPost(encoder_obj->waitForProcessCmpl);
                }

            }
        }
    }
}
