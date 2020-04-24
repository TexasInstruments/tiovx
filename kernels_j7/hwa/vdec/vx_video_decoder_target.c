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
#include "tivx_kernel_video_decoder.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vdec_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "mm_dec.h"
#include <utils/mem/include/app_mem.h>
#include "utils/udma/include/app_udma.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define VIDEO_DECODER_MAX_HANDLES (2U)
#define MM_DEC_SUCCESS            (0U)
#define HW_ALIGN                  (64U)
#define ALIGN_SIZE(x,y)           (((x + (y-1U)) / y) * y)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    uint32_t                            channel_id;
    struct mm_buffer                    in_buff;
    struct mm_buffer                    in_buff_2;
    struct mm_buffer                    out_buff;
    struct mm_buffer                    out_buff_2;
    uint8_t                             processFlag;
    tivx_event                          waitForProcessCmpl;
    uint8_t                            *internal_buff_1;
    uint8_t                            *internal_buff_2;
    uint8_t                             which_buff;
    uint32_t                            internal_size;
    uint8_t                             first_process;
} tivxVideoDecoderObj;

typedef struct
{
    tivx_mutex lock;
    tivxVideoDecoderObj videoDecoderObj[VIDEO_DECODER_MAX_HANDLES];
} tivxVideoDecoderInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVideoDecoderProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVideoDecoderCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVideoDecoderDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVideoDecoderControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static tivxVideoDecoderObj *tivxVideoDecoderAllocObject(
       tivxVideoDecoderInstObj *instObj);
static void tivxVideoDecoderFreeObject(
       tivxVideoDecoderInstObj *instObj,
       tivxVideoDecoderObj *decoder_obj);
static void memcpyDMA(const uint8_t *pOut, const uint8_t *pIn, uint32_t length);
void tivxVideoDecoderErrorCb(struct mm_buffer *buff, mm_dec_process_cb cb_type);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_video_decoder_target_kernel = NULL;

static tivxVideoDecoderInstObj gTivxVideoDecoderInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVideoDecoder(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_0) || (self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VDEC1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxAddTargetKernelVideoDecoder: Invalid CPU\n");
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_video_decoder_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VIDEO_DECODER_NAME,
                            target_name,
                            tivxVideoDecoderProcess,
                            tivxVideoDecoderCreate,
                            tivxVideoDecoderDelete,
                            tivxVideoDecoderControl,
                            NULL);
        if (NULL != vx_video_decoder_target_kernel)
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxVideoDecoderInstObj.lock);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelVideoDecoder: Failed to create Mutex\n");
            }
            else
            {
                memset(&gTivxVideoDecoderInstObj.videoDecoderObj, 0x0,
                    sizeof(tivxVideoDecoderObj) * VIDEO_DECODER_MAX_HANDLES);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxAddTargetKernelVideoDecoder: Failed to Add Video Decoder TargetKernel\n");
            status = (vx_status)VX_FAILURE;
        }

    }
}

void tivxRemoveTargetKernelVideoDecoder(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_video_decoder_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_video_decoder_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxRemoveTargetKernelVideoDecoder: Failed to Remove Video Decoder TargetKernel\n");
    }
    if (0 != gTivxVideoDecoderInstObj.lock)
    {
        tivxMutexDelete(&gTivxVideoDecoderInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */
static vx_status VX_CALLBACK tivxVideoDecoderProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           mm_status = (int32_t)MM_DEC_SUCCESS;
    uint32_t                          size;
    tivxVideoDecoderObj              *decoder_obj = NULL;
    uint8_t                          *bitstream;
    tivx_obj_desc_user_data_object_t *input_bitstream_desc;
    tivx_obj_desc_image_t            *output_image_desc;
    void                             *input_bitstream_target_ptr;
    void                             *output_image_target_ptr_y;
    void                             *output_image_target_ptr_uv;

    if ( (num_params != TIVX_KERNEL_VIDEO_DECODER_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_DECODER_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_DECODER_INPUT_BITSTREAM_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderProcess: Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&decoder_obj, &size);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderProcess: Null Desc\n");
        }
        else if (sizeof(tivxVideoDecoderObj) != size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderProcess: Incorrect object size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* do nothing */
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input_bitstream_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VIDEO_DECODER_INPUT_BITSTREAM_IDX];
        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX];

        input_bitstream_target_ptr = tivxMemShared2TargetPtr(&input_bitstream_desc->mem_ptr);
        output_image_target_ptr_y = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[0]);
        output_image_target_ptr_uv = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[1]);

        tivxMemBufferMap(input_bitstream_target_ptr, input_bitstream_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        bitstream = (uint8_t*) input_bitstream_target_ptr;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t temp_mm_status;

        decoder_obj->in_buff.chId = decoder_obj->channel_id;
        decoder_obj->in_buff.type = MM_BUF_TYPE_VIDEO_INPUT;
        decoder_obj->in_buff.size[0] = decoder_obj->internal_size;
        decoder_obj->in_buff.buf_addr[0] = bitstream;

        decoder_obj->in_buff_2.chId = decoder_obj->channel_id;
        decoder_obj->in_buff_2.type = MM_BUF_TYPE_VIDEO_INPUT;
        decoder_obj->in_buff_2.size[0] = decoder_obj->internal_size;
        decoder_obj->in_buff_2.buf_addr[0] = bitstream;

        decoder_obj->out_buff.chId = decoder_obj->channel_id;
        decoder_obj->out_buff.type = MM_BUF_TYPE_VIDEO_OUTPUT;
        decoder_obj->out_buff.size[0] = decoder_obj->internal_size;
        decoder_obj->out_buff.buf_addr[0] = decoder_obj->internal_buff_1;

        decoder_obj->out_buff_2.chId = decoder_obj->channel_id;
        decoder_obj->out_buff_2.type = MM_BUF_TYPE_VIDEO_OUTPUT;
        decoder_obj->out_buff_2.size[0] = decoder_obj->internal_size;
        decoder_obj->out_buff_2.buf_addr[0] = decoder_obj->internal_buff_2;

        mm_status = MM_DEC_BufPrepare(&decoder_obj->in_buff, decoder_obj->channel_id);
        temp_mm_status = (uint32_t)mm_status;
        temp_mm_status |= (uint32_t)MM_DEC_BufPrepare(&decoder_obj->in_buff_2, decoder_obj->channel_id);

        if (0U != decoder_obj->first_process)
        {
            temp_mm_status |= (uint32_t)MM_DEC_BufPrepare(&decoder_obj->out_buff, decoder_obj->channel_id);
            temp_mm_status |= (uint32_t)MM_DEC_BufPrepare(&decoder_obj->out_buff_2, decoder_obj->channel_id);
        }

        mm_status = (int32_t)temp_mm_status;

        if((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderProcess: MM_DEC Buf Prepare failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        decoder_obj->processFlag = 0;
        decoder_obj->in_buff.size[0] = input_bitstream_desc->valid_mem_size;
        decoder_obj->in_buff_2.size[0] = input_bitstream_desc->valid_mem_size;

        if (1U == decoder_obj->which_buff)
        {
            mm_status = MM_DEC_Process(&decoder_obj->in_buff, &decoder_obj->out_buff, decoder_obj->channel_id);
        }
        else
        {
            mm_status = MM_DEC_Process(&decoder_obj->in_buff_2, &decoder_obj->out_buff_2, decoder_obj->channel_id);
        }

        if ((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderProcess: MM_DEC Process failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {

        uint32_t padded_size_y = ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_x, (uint32_t)HW_ALIGN)
                                * ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_y, (uint32_t)HW_ALIGN);

        uint32_t padded_size_uv = (ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_x, (uint32_t)HW_ALIGN)
                                * ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_y, (uint32_t)HW_ALIGN) * 1U )/ 2U;

        tivxEventWait(decoder_obj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        if (0U != decoder_obj->first_process)
        {
            decoder_obj->first_process = 0;

            tivxMemBufferMap(output_image_target_ptr_y, output_image_desc->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

            tivxMemBufferMap(output_image_target_ptr_uv, output_image_desc->mem_size[1],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

            if (1U == decoder_obj->which_buff)
            {
                decoder_obj->which_buff = 2;
            }
            else
            {
                decoder_obj->which_buff = 1;
            }
            memset(output_image_target_ptr_y, 0, padded_size_y);
            memset(output_image_target_ptr_uv, 0, padded_size_uv);

            tivxMemBufferUnmap(output_image_target_ptr_y, output_image_desc->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
            tivxMemBufferUnmap(output_image_target_ptr_uv, output_image_desc->mem_size[1],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
        }
        else
        {
            if (1U == decoder_obj->which_buff)
            {
                decoder_obj->which_buff = 2;

                memcpyDMA((uint8_t *) output_image_target_ptr_y,
                        (uint8_t *)decoder_obj->internal_buff_2,
                        output_image_desc->mem_size[0]);

                memcpyDMA((uint8_t *) output_image_target_ptr_uv,
                        (uint8_t *)decoder_obj->internal_buff_2 + padded_size_y,
                        output_image_desc->mem_size[1]);
            }
            else
            {
                decoder_obj->which_buff = 1;

                memcpyDMA((uint8_t *) output_image_target_ptr_y,
                        (uint8_t *)decoder_obj->internal_buff_1,
                        output_image_desc->mem_size[0]);

                memcpyDMA((uint8_t *) output_image_target_ptr_uv,
                        (uint8_t *)decoder_obj->internal_buff_1 + padded_size_y,
                        output_image_desc->mem_size[1]);
            }
        }
    }


    if ((vx_status)VX_SUCCESS == status)
    {
        tivxMemBufferUnmap(input_bitstream_target_ptr, input_bitstream_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
    }

    return status;

}

static vx_status VX_CALLBACK tivxVideoDecoderCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           mm_status = (int32_t)MM_DEC_SUCCESS;
    uint32_t                          temp_mm_status;
    vx_df_image                       output_image_fmt;
    mm_vid_create_params             vdec_params;
    tivxVideoDecoderObj              *decoder_obj = NULL;
    tivx_video_decoder_params_t      *decoder_params;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t            *output_image_desc;
    void                             *configuration_target_ptr;

    if ( (num_params != TIVX_KERNEL_VIDEO_DECODER_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_DECODER_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_DECODER_INPUT_BITSTREAM_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderCreate: Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        decoder_obj = tivxVideoDecoderAllocObject(&gTivxVideoDecoderInstObj);

        if (NULL != decoder_obj)
        {
            configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VIDEO_DECODER_CONFIGURATION_IDX];
            output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX];

            configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: Failed to Alloc Video Decoder Object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        decoder_obj->first_process = 1;
        decoder_obj->which_buff = 1;
        decoder_obj->internal_size = (ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_x, (uint32_t)HW_ALIGN)
            * ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_y, (uint32_t)HW_ALIGN) * 3U) / 2U;

        decoder_obj->internal_buff_1 = (uint8_t*)appMemAlloc(APP_MEM_HEAP_DDR, decoder_obj->internal_size, 4096);

        if (NULL == decoder_obj->internal_buff_1)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderCreate: tivxMemAlloc failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        decoder_obj->internal_buff_2 = (uint8_t*)appMemAlloc(APP_MEM_HEAP_DDR, decoder_obj->internal_size, 4096);

        if (NULL == decoder_obj->internal_buff_2)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderCreate: tivxMemAlloc failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (configuration_desc->mem_size != sizeof(tivx_video_decoder_params_t))
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderCreate: User data object size on target does not match the size on host, possibly due to misalignment in data structure\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxEventCreate(&decoder_obj->waitForProcessCmpl);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: Failed to allocate Event\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        decoder_params = (tivx_video_decoder_params_t *) configuration_target_ptr;
        output_image_fmt = output_image_desc->format;

        vdec_params.width = ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_x, HW_ALIGN);
        vdec_params.height = ALIGN_SIZE(output_image_desc->imagepatch_addr[0].dim_y, HW_ALIGN);

        if (TIVX_BITSTREAM_FORMAT_H264 == decoder_params->bitstream_format)
        {
            vdec_params.in_pixelformat = (uint32_t)MM_PIX_FMT_H264;
        }
        else if (TIVX_BITSTREAM_FORMAT_HEVC == decoder_params->bitstream_format)
        {
            vdec_params.in_pixelformat = (uint32_t)MM_PIX_FMT_HEVC;
        }
        else if (TIVX_BITSTREAM_FORMAT_MJPEG == decoder_params->bitstream_format)
        {
            vdec_params.in_pixelformat = (uint32_t)MM_PIX_FMT_MJPEG;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: Invalid input format\n");
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_df_image)VX_DF_IMAGE_NV12 == output_image_fmt)
        {
            vdec_params.out_pixelformat = (uint32_t)MM_PIX_FMT_NV12;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: Invalid output format\n");
            status = (vx_status)VX_FAILURE;
        }

        mm_status = MM_DEC_Create(&vdec_params, &decoder_obj->channel_id);

        if((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: MM_DEC Create failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        mm_status = MM_DEC_RegisterCb(tivxVideoDecoderErrorCb, decoder_obj->channel_id);

        if((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: M_DEC Register Callback failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        mm_status = MM_DEC_StartStreaming(decoder_obj->channel_id, MM_BUF_TYPE_VIDEO_OUTPUT);
        temp_mm_status = (uint32_t)mm_status;
        temp_mm_status |= (uint32_t)MM_DEC_StartStreaming(decoder_obj->channel_id, MM_BUF_TYPE_VIDEO_INPUT);
        mm_status = (int32_t)temp_mm_status;

        if((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderCreate: MM_DEC Start Streaming failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, decoder_obj,
            sizeof(tivxVideoDecoderObj));
    }

    return status;
}

static vx_status VX_CALLBACK tivxVideoDecoderDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = (vx_status)VX_SUCCESS;
    int32_t              mm_status = (int32_t)MM_DEC_SUCCESS;
    uint32_t             temp_mm_status;
    uint32_t             size;
    tivxVideoDecoderObj *decoder_obj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&decoder_obj, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderDelete: Null Desc\n");
    }
    else if (sizeof(tivxVideoDecoderObj) != size)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVideoDecoderDelete: Incorrect object size\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing */
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        mm_status = MM_DEC_StopStreaming(decoder_obj->channel_id, MM_BUF_TYPE_VIDEO_INPUT);
        temp_mm_status = (uint32_t)mm_status;
        temp_mm_status |= (uint32_t)MM_DEC_StopStreaming(decoder_obj->channel_id, MM_BUF_TYPE_VIDEO_OUTPUT);
        mm_status = (int32_t)temp_mm_status;

        if((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderDelete: MM_DEC_StopStreaming failed\n");
            status = (vx_status)VX_FAILURE;
        }

        temp_mm_status |= (uint32_t)MM_DEC_Destroy(decoder_obj->channel_id);
        mm_status = (int32_t)temp_mm_status;

        if((int32_t)MM_DEC_SUCCESS != mm_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVideoDecoderDelete: MM_DEC_Destroy failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != decoder_obj->waitForProcessCmpl)
        {
            tivxEventDelete(&decoder_obj->waitForProcessCmpl);
        }

        if (NULL != decoder_obj->internal_buff_1)
        {
            appMemFree(APP_MEM_HEAP_DDR, decoder_obj->internal_buff_1, decoder_obj->internal_size);
            decoder_obj->internal_buff_1 = NULL;
        }

        if (NULL != decoder_obj->internal_buff_2)
        {
            appMemFree(APP_MEM_HEAP_DDR, decoder_obj->internal_buff_2, decoder_obj->internal_size);
            decoder_obj->internal_buff_2 = NULL;
        }

        tivxVideoDecoderFreeObject(&gTivxVideoDecoderInstObj, decoder_obj);
    }

    return status;
}

static vx_status VX_CALLBACK tivxVideoDecoderControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVideoDecoderObj *tivxVideoDecoderAllocObject(
       tivxVideoDecoderInstObj *instObj)
{
    uint32_t        cnt;
    tivxVideoDecoderObj *decoder_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VIDEO_DECODER_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->videoDecoderObj[cnt].isAlloc)
        {
            decoder_obj = &instObj->videoDecoderObj[cnt];
            memset(decoder_obj, 0x0, sizeof(tivxVideoDecoderObj));
            instObj->videoDecoderObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (decoder_obj);
}

static void tivxVideoDecoderFreeObject(tivxVideoDecoderInstObj *instObj,
    tivxVideoDecoderObj *decoder_obj)
{
    uint32_t cnt;

    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VIDEO_DECODER_MAX_HANDLES; cnt ++)
    {
        if (decoder_obj == &instObj->videoDecoderObj[cnt])
        {
            decoder_obj->isAlloc = 0U;
            break;
        }
    }

    tivxMutexUnlock(instObj->lock);
}

static void memcpyDMA(const uint8_t *pOut, const uint8_t *pIn, uint32_t length)
{
    app_udma_copy_1d_prms_t prms_1d;

    appUdmaCopy1DPrms_Init(&prms_1d);
    prms_1d.dest_addr    = appMemGetVirt2PhyBufPtr((uint64_t) pOut, APP_MEM_HEAP_DDR);
    prms_1d.src_addr     = appMemGetVirt2PhyBufPtr((uint64_t) pIn, APP_MEM_HEAP_DDR);
    prms_1d.length       = length;
    appUdmaCopy1D(NULL, &prms_1d);

}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

void tivxVideoDecoderErrorCb(struct mm_buffer *buff, mm_dec_process_cb cb_type)
{
    tivxVideoDecoderObj *decoder_obj = NULL;
    uint32_t             cnt;

    /* Lock instance mutex */
    if (NULL != buff)
    {
        tivxMutexLock(gTivxVideoDecoderInstObj.lock);
        for (cnt = 0U; cnt < VIDEO_DECODER_MAX_HANDLES; cnt ++)
        {
            if (1U == gTivxVideoDecoderInstObj.videoDecoderObj[cnt].isAlloc)
            {
                decoder_obj = &gTivxVideoDecoderInstObj.videoDecoderObj[cnt];
                if(decoder_obj->channel_id == buff->chId)
                {
                    break;
                }
            }
        }
        /* Release instance mutex */
        tivxMutexUnlock(gTivxVideoDecoderInstObj.lock);

        if (NULL != decoder_obj)
        {
            if (decoder_obj->channel_id == buff->chId)
            {
                switch (cb_type) {
                case MM_CB_STRUNIT_PROCESSED:
                    decoder_obj->processFlag |= 1U;
                    break;
                case MM_CB_PICT_DECODED:
                    break;
                case MM_CB_PICT_DISPLAY:
                    decoder_obj->processFlag |= 2U;
                    break;
                case MM_CB_PICT_RELEASE:
                    decoder_obj->processFlag |= 4U;
                    break;
                case MM_CB_PICT_END:
                    break;
                case MM_CB_STR_END:
                    break;
                case MM_CB_ERROR_FATAL:
                    break;
                default:
                    break;
                }

                if ((7U == decoder_obj->processFlag) || ((0U != decoder_obj->first_process) && (3U == decoder_obj->processFlag)))
                {
                    tivxEventPost(decoder_obj->waitForProcessCmpl);
                }
            }
        }
    }
}
