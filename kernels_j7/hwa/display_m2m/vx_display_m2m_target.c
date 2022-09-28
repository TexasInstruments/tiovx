/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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
#include "tivx_kernel_display_m2m.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_display_m2m_priv.h"

#include <TI/tivx_queue.h>
#include <ti/drv/fvid2/fvid2.h>
#include <ti/drv/dss/dss.h>
#include <tivx_obj_desc_priv.h>
#include <vx_reference.h>

#define DISPLAY_MAX_VALID_PLANES                      2U

#define DISPLAY_M2M_MAX_HANDLES                       (10)

typedef struct
{
    /*! IDs=> 0: Write-back pipe-line1 */
    uint32_t instId;
    /*! Number of pipe-lines used, should be set to '1' as blending is not supported currently */
    uint32_t numPipe;
    /*! IDs=> 0:VID1, 1:VIDL1, 2:VID2 and 3:VIDL2 */
    uint32_t pipeId[TIVX_DISPLAY_M2M_MAX_PIPE];
    /*! IDs=> 0:Overlay1, 1:Overlay2, 2:Overlay3 and 3:Overlay4 */
    uint32_t overlayId;
    /*! FVID2 display driver handle */
    Fvid2_Handle drvHandle;
    /*! WB pipe create parameters */
    Dss_WbCreateParams createParams;
    /*! WB pipe create status */
    Dss_WbCreateStatus createStatus;
    /*! Callback parameters */
    Fvid2_CbParams cbParams;
    /*! WB pipe status */
    Dss_WbStatus wbStatus;
    /*! WB pipe configuration */
    Dss_WbPipeCfgParams wbCfg;
    /*! WB pipe DMA configuration */
    CSL_DssWbPipeDmaCfg wbDmaCfg;
    /*! WB pipe MFlag configuration */
    Dss_WbPipeMflagParams wbMflagCfg;
    /*! WB pipe CSC configuration */
    CSL_DssCscCoeff wbCscCfg;
    /*! Display pipe configuration */
    Dss_PipeCfgParams pipeCfg[TIVX_DISPLAY_M2M_MAX_PIPE];
    /*! Display pipe MFlag configuration */
    Dss_PipeMflagParams mFlagCfg[TIVX_DISPLAY_M2M_MAX_PIPE];
    /*! Display pipe CSC configuration */
    Dss_PipeCscParams cscCfg[TIVX_DISPLAY_M2M_MAX_PIPE];
    /*! Display Overlay configuration */
    Dss_DctrlOverlayParams ovrCfg;
    /*! Display Layer configuration */
    Dss_DctrlOverlayLayerParams layerCfg;
    /*! Display Global configuration */
    Dss_DctrlGlobalDssParams globalParams;
    /*! Mutex used for waiting for process completion */
    tivx_event waitForProcessCmpl;
    /*! Display M2M Driver Input Frame List, used for providing
     *  an array of input frames */
    Fvid2_FrameList inFrmList;
    /*! Display M2M Driver Output Frame List, used for providing
     *  an array of output frames */
    Fvid2_FrameList outFrmList;
    /*! Display M2M Driver Input Frames */
    Fvid2_Frame inFrm[TIVX_DISPLAY_M2M_MAX_PIPE];
    /*! Display M2M Driver Output Frames */
    Fvid2_Frame outFrm[1U];
} tivxDisplayM2MDrvObj;

typedef struct
{
    /*! IDs=> 0: Object free, 1: allocated */
    uint32_t isAlloc;
    /*! Display M2M driver object */
    tivxDisplayM2MDrvObj drvObj;
    /*! Display M2M Node create parameters provided by application */
    tivx_display_m2m_params_t createParams;
} tivxDisplayM2MParams;

typedef struct
{
    tivx_mutex      lock;
    tivxDisplayM2MParams  m2mObj[DISPLAY_M2M_MAX_HANDLES];
} tivxDisplayM2MInstObj;

static tivx_target_kernel vx_display_m2m_target_kernel1 = NULL;
static tivx_target_kernel vx_display_m2m_target_kernel2 = NULL;
static tivx_target_kernel vx_display_m2m_target_kernel3 = NULL;
static tivx_target_kernel vx_display_m2m_target_kernel4 = NULL;

tivxDisplayM2MInstObj gTivxDispM2mInstObj;

static vx_status VX_CALLBACK tivxDisplayM2MProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayM2MCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayM2MDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDisplayM2MControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status tivxDisplayM2MSetCreateParams(
                           tivxDisplayM2MParams *prms,
                           const tivx_obj_desc_user_data_object_t *obj_desc,
                           const tivx_obj_desc_image_t *obj_desc_imageIn,
                           const tivx_obj_desc_image_t *obj_desc_imageOut);

static vx_status tivxDisplayM2MDrvStructsInit(tivxDisplayM2MDrvObj *drvObj);

static vx_status tivxDisplayM2MDrvCfg(tivxDisplayM2MDrvObj *drvObj);

static int32_t tivxDisplayM2MCallback(Fvid2_Handle handle, void *appData);

static vx_status tivxDisplayExtractFvid2Format(
                                const tivx_obj_desc_image_t *obj_desc_img,
                                Fvid2_Format *format);

static tivxDisplayM2MParams *tivxDispM2mAllocObject(tivxDisplayM2MInstObj *instObj);
static void tivxDispM2mFreeObject(tivxDisplayM2MInstObj *instObj,
    tivxDisplayM2MParams *m2mObj);


static vx_status VX_CALLBACK tivxDisplayM2MProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxDisplayM2MParams *prms = NULL;
    tivxDisplayM2MDrvObj *drvObj;
    tivx_obj_desc_image_t *input_desc;
    tivx_obj_desc_image_t *output_desc;
    void *input_target_ptr, *input_target_ptr2 = NULL;
    void *output_target_ptr, *output_target_ptr2 = NULL;
    Fvid2_Frame *frm;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t pipeIdx;

    if ( (num_params != TIVX_KERNEL_DISPLAY_M2M_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        input_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **)&prms, &size);
        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxDisplayM2MParams) != size))
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "DISPLAY M2M: ERROR: Instance context is NULL!\r\n");
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        /* Update 'input_desc' to array from only single image input to
           support blending i.e. more than 1 number of pipes. */
        input_target_ptr = tivxMemShared2TargetPtr(&input_desc->mem_ptr[0]);
        if((vx_df_image)VX_DF_IMAGE_NV12 == input_desc->format)
        {
            input_target_ptr2 = tivxMemShared2TargetPtr(&input_desc->mem_ptr[1]);
        }

        output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);

        if((vx_df_image)VX_DF_IMAGE_NV12 == output_desc->format)
        {
            output_target_ptr2 = tivxMemShared2TargetPtr(&output_desc->mem_ptr[1]);
        }

        /* call kernel processing function */

        drvObj = &prms->drvObj;
        /* Assign input buffer addresses */
        for (pipeIdx = 0U ; pipeIdx < drvObj->numPipe ; pipeIdx++)
        {
            frm = &drvObj->inFrm[pipeIdx];
            frm->addr[0U] = (uint64_t)input_target_ptr;
            if((vx_df_image)VX_DF_IMAGE_NV12 == input_desc->format)
            {
                frm->addr[1U] = (uint64_t)input_target_ptr2;
            }
        }

        /* Assign output buffer addresses */
        frm = drvObj->outFrm;
        frm->addr[0U] = (uint64_t)output_target_ptr;
        if((vx_df_image)VX_DF_IMAGE_NV12 == output_desc->format)
        {
            frm->addr[1U] = (uint64_t)output_target_ptr2;
        }

        /* Submit the request to the driver */
        fvid2_status = Fvid2_processRequest(drvObj->drvHandle,
                                           &drvObj->inFrmList,
                                           &drvObj->outFrmList,
                                           FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* Wait for Frame Completion */
            tivxEventWait(drvObj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
            fvid2_status = Fvid2_getProcessedRequest(drvObj->drvHandle,
                                                    &drvObj->inFrmList,
                                                    &drvObj->outFrmList,
                                                    0);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        /* kernel processing function complete */
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayM2MCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxDisplayM2MParams *prms = NULL;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t *obj_desc_imageIn, *obj_desc_imageOut;

    if ( (num_params != TIVX_KERNEL_DISPLAY_M2M_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX];
        obj_desc_imageIn  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX];
        obj_desc_imageOut = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX];

        if (configuration_desc->mem_size != sizeof(tivx_display_m2m_params_t))
        {
            VX_PRINT(VX_ZONE_ERROR, "User data object size on target does not match the size on host, possibly due to misalignment in data structure\n");
            status = (vx_status)VX_FAILURE;
        }

        prms = tivxDispM2mAllocObject(&gTivxDispM2mInstObj);
        if (NULL == prms)
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }

        /* Create Node object elements */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxDisplayM2MSetCreateParams(prms,
                                                   configuration_desc,
                                                   obj_desc_imageIn,
                                                   obj_desc_imageOut);
        }

        /* Create sync events */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = tivxEventCreate(&prms->drvObj.waitForProcessCmpl);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
            }
        }

        /* DSS M2M Driver create and configuration */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = tivxDisplayM2MDrvCfg(&prms->drvObj);
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms, sizeof(tivxDisplayM2MParams));
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate local memory\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayM2MDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxDisplayM2MParams *prms = NULL;
    uint32_t size;
    int32_t fvid2_status = FVID2_SOK;

    if ( (num_params != TIVX_KERNEL_DISPLAY_M2M_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DISPLAY_M2M_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
                                                    (void **)&prms,
                                                    &size);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, " DSS M2M: ERROR: Could not obtain kernel instance context !!!\n");
        }
        if(NULL == prms)
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel instance context is NULL!!!\n");
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Stop Display M2M Driver */
            fvid2_status = Fvid2_stop(prms->drvObj.drvHandle, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " DSS M2M: ERROR: FVID2 DSS M2M not stopped !!!\n");
            }
        }


        if ((vx_status)VX_SUCCESS == status)
        {
            /* Dequeue all the request from the driver */
            while ((vx_status)VX_SUCCESS == status)
            {
                fvid2_status = Fvid2_getProcessedRequest(prms->drvObj.drvHandle,
                                                        &prms->drvObj.inFrmList,
                                                        &prms->drvObj.outFrmList,
                                                        0);
                if (FVID2_SOK != fvid2_status)
                {
                    if (fvid2_status != FVID2_ENO_MORE_BUFFERS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                    }
                    status = (vx_status)VX_FAILURE;
                }
            }
            if (fvid2_status == FVID2_ENO_MORE_BUFFERS)
            {
                status = (vx_status)VX_SUCCESS;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* print status */
            fvid2_status = Fvid2_control(prms->drvObj.drvHandle,
                                         IOCTL_DSS_M2M_GET_CURRENT_STATUS,
                                         &prms->drvObj.wbStatus,
                                         NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Get status returned failure\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                printf(   "==========================================================\r\n");
                printf(   " Display M2M Status: Instance|%d\r\n", prms->drvObj.instId);
                printf(   "==========================================================\r\n");
                printf(   " Queue Count: %d\r\n", prms->drvObj.wbStatus.queueCount);
                printf(   " De-queue Count: %d\r\n", prms->drvObj.wbStatus.dequeueCount);
                printf(   " Write-back Frames Count: %d\r\n", prms->drvObj.wbStatus.wbFrmCount);
                printf(   " Underflow Count: %d\r\n", prms->drvObj.wbStatus.underflowCount);
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Delete FVID2 handle */
            fvid2_status = Fvid2_delete(prms->drvObj.drvHandle, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " DSS M2M: ERROR: FVID2 Delete Failed !!!\n");
            }
            else
            {
                prms->drvObj.drvHandle = NULL;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Delete event */
            tivxEventDelete(&prms->drvObj.waitForProcessCmpl);
        }

        if ((NULL != prms) &&
            (sizeof(tivxDisplayM2MParams) == size))
        {
            tivxDispM2mFreeObject(&gTivxDispM2mInstObj, prms);
            //tivxMemFree(prms, size, (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDisplayM2MControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t size;
    tivxDisplayM2MParams *prms = NULL;
    tivxDisplayM2MDrvObj *drvObj;
    tivx_display_m2m_statistics_t *m2m_status_prms = NULL;
    void *target_ptr;
    tivx_obj_desc_user_data_object_t *usr_data_obj;

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
    if (((vx_status)VX_SUCCESS != status)            ||
        (NULL == prms)                               ||
        (sizeof(tivxDisplayM2MParams) != size))
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        switch (node_cmd_id)
        {
            case TIVX_DISPLAY_M2M_GET_STATISTICS:
            {
                if (NULL != obj_desc[0])
                {
                    drvObj = &prms->drvObj;
                    fvid2_status = Fvid2_control(drvObj->drvHandle,
                                            IOCTL_DSS_M2M_GET_CURRENT_STATUS,
                                            &drvObj->wbStatus,
                                            NULL);
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Get status returned failure\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    else
                    {
                        /* Update return status object */
                        usr_data_obj = (tivx_obj_desc_user_data_object_t *)obj_desc[0U];
                        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);
                        tivxCheckStatus(&status,
                                        tivxMemBufferMap(target_ptr,
                                        usr_data_obj->mem_size,
                                        (vx_enum)VX_MEMORY_TYPE_HOST,
                                        (vx_enum)VX_WRITE_ONLY));
                        if (sizeof(tivx_display_m2m_statistics_t) ==
                                usr_data_obj->mem_size)
                        {
                            m2m_status_prms = (tivx_display_m2m_statistics_t *)target_ptr;
                            m2m_status_prms->queueCount     =
                                                drvObj->wbStatus.queueCount;
                            m2m_status_prms->dequeueCount   =
                                                drvObj->wbStatus.dequeueCount;
                            m2m_status_prms->wbFrmCount     =
                                                drvObj->wbStatus.wbFrmCount;
                            m2m_status_prms->underflowCount =
                                                drvObj->wbStatus.underflowCount;
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }

                        tivxCheckStatus(&status,
                                        tivxMemBufferUnmap(target_ptr,
                                        usr_data_obj->mem_size,
                                        (vx_enum)VX_MEMORY_TYPE_HOST,
                                        (vx_enum)VX_WRITE_ONLY));
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

void tivxAddTargetKernelDisplayM2M(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0 )
    {
        strncpy(target_name, TIVX_TARGET_DISPLAY_M2M1, TIVX_TARGET_MAX_NAME);
        vx_display_m2m_target_kernel1 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_M2M_NAME,
                            target_name,
                            tivxDisplayM2MProcess,
                            tivxDisplayM2MCreate,
                            tivxDisplayM2MDelete,
                            tivxDisplayM2MControl,
                            NULL);
        strncpy(target_name, TIVX_TARGET_DISPLAY_M2M2, TIVX_TARGET_MAX_NAME);
        vx_display_m2m_target_kernel2 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_M2M_NAME,
                            target_name,
                            tivxDisplayM2MProcess,
                            tivxDisplayM2MCreate,
                            tivxDisplayM2MDelete,
                            tivxDisplayM2MControl,
                            NULL);
        strncpy(target_name, TIVX_TARGET_DISPLAY_M2M3, TIVX_TARGET_MAX_NAME);
        vx_display_m2m_target_kernel3 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_M2M_NAME,
                            target_name,
                            tivxDisplayM2MProcess,
                            tivxDisplayM2MCreate,
                            tivxDisplayM2MDelete,
                            tivxDisplayM2MControl,
                            NULL);
        strncpy(target_name, TIVX_TARGET_DISPLAY_M2M4, TIVX_TARGET_MAX_NAME);
        vx_display_m2m_target_kernel4 = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DISPLAY_M2M_NAME,
                            target_name,
                            tivxDisplayM2MProcess,
                            tivxDisplayM2MCreate,
                            tivxDisplayM2MDelete,
                            tivxDisplayM2MControl,
                            NULL);

        status = tivxMutexCreate(&gTivxDispM2mInstObj.lock);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create Mutex\n");
        }
        else
        {
            memset(&gTivxDispM2mInstObj.m2mObj, 0x0,
                sizeof(tivxDisplayM2MParams) * DISPLAY_M2M_MAX_HANDLES);
        }
    }
}

void tivxRemoveTargetKernelDisplayM2M(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_display_m2m_target_kernel1);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_display_m2m_target_kernel1 = NULL;
    }

    status = tivxRemoveTargetKernel(vx_display_m2m_target_kernel2);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_display_m2m_target_kernel2 = NULL;
    }

    status = tivxRemoveTargetKernel(vx_display_m2m_target_kernel3);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_display_m2m_target_kernel3 = NULL;
    }

    status = tivxRemoveTargetKernel(vx_display_m2m_target_kernel4);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_display_m2m_target_kernel4 = NULL;
    }

    if (NULL != gTivxDispM2mInstObj.lock)
    {
        tivxMutexDelete(&gTivxDispM2mInstObj.lock);
    }
}


static vx_status tivxDisplayM2MSetCreateParams(
                       tivxDisplayM2MParams *prms,
                       const tivx_obj_desc_user_data_object_t *obj_desc,
                       const tivx_obj_desc_image_t *obj_desc_imageIn,
                       const tivx_obj_desc_image_t *obj_desc_imageOut)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *cfgPtr;
    tivx_display_m2m_params_t *createParams;
    tivxDisplayM2MDrvObj *drvObj;
    uint32_t pipeIdx, layerIdx;
    Dss_DispParams *dispParams;
    CSL_DssWbPipeCfg *wbPipeCfg;
    Dss_DctrlOverlayParams *ovrParams;
    Dss_DctrlOverlayLayerParams *layerParams;
    Fvid2_Frame *frm;

    cfgPtr = tivxMemShared2TargetPtr(&obj_desc->mem_ptr);

    tivxCheckStatus(&status, tivxMemBufferMap(cfgPtr, obj_desc->mem_size,
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    if (status == (vx_status)VX_SUCCESS)
    {
        createParams = (tivx_display_m2m_params_t *)cfgPtr;
        memcpy(&prms->createParams, createParams, sizeof(tivx_display_m2m_params_t));
        drvObj = &prms->drvObj;
        /* Set Driver object */
        drvObj->instId    = createParams->instId;
        drvObj->numPipe   = createParams->numPipe;
        drvObj->overlayId = createParams->overlayId;
        memcpy(&drvObj->pipeId[0U],
               &createParams->pipeId[0U],
               sizeof(createParams->pipeId));
    }
    /* Initialize driver object */
    if (status == (vx_status)VX_SUCCESS)
    {
        status = tivxDisplayM2MDrvStructsInit(drvObj);
    }

    /* set driver object parameters */
    if (status == (vx_status)VX_SUCCESS)
    {
        /* Callback parameters */
        drvObj->cbParams.cbFxn   = (Fvid2_CbFxn) (&tivxDisplayM2MCallback);
        drvObj->cbParams.appData = drvObj;
        drvObj->createParams.numPipe = drvObj->numPipe;
        drvObj->createParams.overlayId = drvObj->overlayId;
        /* Set Display pipeline parameters */
        for (pipeIdx = 0U ; pipeIdx < drvObj->numPipe ; pipeIdx++)
        {
            dispParams = &drvObj->pipeCfg[pipeIdx].cfgParams;
            drvObj->createParams.pipeId[pipeIdx]  = drvObj->pipeId[pipeIdx];
            drvObj->pipeCfg[pipeIdx].pipeId       = drvObj->pipeId[pipeIdx];
            drvObj->mFlagCfg[pipeIdx].pipeId      = drvObj->pipeId[pipeIdx];
            drvObj->cscCfg[pipeIdx].pipeId        = drvObj->pipeId[pipeIdx];
            dispParams->pipeCfg.pipeType          = CSL_DSS_VID_PIPE_TYPE_VID;
            dispParams->layerPos.startX           = 0U;
            dispParams->layerPos.startY           = 0U;
            dispParams->pipeCfg.scEnable          = FALSE;
            dispParams->alphaCfg.globalAlpha      = 0xFFU;
            dispParams->alphaCfg.preMultiplyAlpha = FALSE;
            status = tivxDisplayExtractFvid2Format(
                        obj_desc_imageIn,
                        &dispParams->pipeCfg.inFmt);
            if (status == (vx_status)VX_SUCCESS)
            {
                /* Set video pipe output frame dimensions same as input as
                   no scaling is done in video pipe-line */
                dispParams->pipeCfg.outWidth  = dispParams->pipeCfg.inFmt.width;
                dispParams->pipeCfg.outHeight = dispParams->pipeCfg.inFmt.height;
            }
            else
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "Invalid Input Image\n");
                break;
            }
        }

        /* Set Display WB pipeline parameters */
        if (((vx_status)VX_SUCCESS == status) && (pipeIdx > 0))
        {
            wbPipeCfg               = &drvObj->wbCfg.pipeCfg;
            /* Set WB pipe input frame dimensions same as video pipe input/output frame,
               no scaling is done in video pipe, it will be done in WB pipe-line */
            wbPipeCfg->inFmt.width  = dispParams->pipeCfg.outWidth;
            wbPipeCfg->inFmt.height = dispParams->pipeCfg.outHeight;
            wbPipeCfg->inPos.startX = 0U;
            wbPipeCfg->inPos.startY = 0U;
            status = tivxDisplayExtractFvid2Format(obj_desc_imageOut,
                                                   &wbPipeCfg->outFmt);
            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Input Image\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                if ((wbPipeCfg->inFmt.width != wbPipeCfg->outFmt.width) ||
                    (wbPipeCfg->inFmt.height != wbPipeCfg->outFmt.height))
                {
                    wbPipeCfg->scEnable = TRUE;
                }
            }
        }

        /* Set Display WB pipeline parameters */
        if ((vx_status)VX_SUCCESS == status)
        {
            ovrParams = &drvObj->ovrCfg;
            ovrParams->overlayId = drvObj->overlayId;
            ovrParams->colorbarEnable = FALSE;
            ovrParams->overlayCfg.colorKeyEnable = FALSE;
            ovrParams->overlayCfg.colorKeySel = CSL_DSS_OVERLAY_TRANS_COLOR_DEST;
            ovrParams->overlayCfg.backGroundColor = 0xc8c800U;

            layerParams = &drvObj->layerCfg;
            layerParams->overlayId = drvObj->overlayId;
            /* Set all layer to invalid first and then update only used ones */
            for(layerIdx = 0U ; layerIdx < CSL_DSS_VID_PIPE_ID_MAX ; layerIdx++)
            {
                layerParams->pipeLayerNum[layerIdx] = CSL_DSS_OVERLAY_LAYER_INVALID;
            }

            /* Currently blending is not supported so only one layer is used.
               This code needs to updated when blending is supported. */
            layerParams->pipeLayerNum[drvObj->createParams.pipeId[0U]] =
                                                CSL_DSS_OVERLAY_LAYER_NUM_0;
        }

        /* Update frame-lists */
        if ((vx_status)VX_SUCCESS == status)
        {
            drvObj->inFrmList.numFrames = drvObj->numPipe;
            for (pipeIdx = 0U ; pipeIdx < drvObj->numPipe ; pipeIdx++)
            {
                frm = (Fvid2_Frame *) &drvObj->inFrm[pipeIdx];
                frm->chNum = drvObj->createParams.pipeId[pipeIdx];
                drvObj->inFrmList.frames[pipeIdx] = frm;
            }

            frm = (Fvid2_Frame *) &drvObj->outFrm[0U];
            drvObj->outFrmList.frames[0U] = frm;
            drvObj->outFrmList.numFrames  = 1U;

        }
    }

    return status;
}

static vx_status tivxDisplayM2MDrvStructsInit(tivxDisplayM2MDrvObj *drvObj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t loopCnt;

    /* Initialize driver create parameters */
    Dss_m2mCreateParamsInit(&drvObj->createParams);
    /* Initialize driver call-back parameters */
    Fvid2CbParams_init(&drvObj->cbParams);
    /* Initialize driver pipe configuration parameters */
    for (loopCnt = 0U ; loopCnt < drvObj->numPipe ; loopCnt++)
    {
        Dss_dispParamsInit(&drvObj->pipeCfg[loopCnt].cfgParams);
        Dss_dispPipeMflagParamsInit(&drvObj->mFlagCfg[loopCnt].mFlagCfg);
        CSL_dssCscCoeffInit(&drvObj->cscCfg[loopCnt].csc);
    }
    /* Initialize WB pipeline parameters */
    Dss_m2mPipeCfgParamsInit(&drvObj->wbCfg);
    CSL_dssWbPipeDmaCfgInit(&drvObj->wbDmaCfg);
    Dss_m2mMFlagParamsInit(&drvObj->wbMflagCfg);
    CSL_dssCscCoeffInit(&drvObj->wbCscCfg);
    Dss_m2mStatusInit(&drvObj->wbStatus);

    /* Initialize Display overlay parameters */
    Dss_dctrlOverlayParamsInit(&drvObj->ovrCfg);
    Dss_dctrlOverlayLayerParamsInit(&drvObj->layerCfg);

    /* Initialize Display global parameters */
    Dss_dctrlGlobalDssParamsInit(&drvObj->globalParams);

    /* Initialize input and output frame lists */
    Fvid2FrameList_init(&drvObj->inFrmList);
    Fvid2FrameList_init(&drvObj->outFrmList);

    return status;
}

static vx_status tivxDisplayM2MDrvCfg(tivxDisplayM2MDrvObj *drvObj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t loopCnt;
    int32_t fvid2_status = FVID2_SOK;

    /* Display M2M Driver create */
    drvObj->drvHandle = Fvid2_create(DSS_M2M_DRV_ID,
                                     drvObj->instId,
                                     &drvObj->createParams,
                                     &drvObj->createStatus,
                                     &drvObj->cbParams);
    if((NULL == drvObj->drvHandle) ||
           (drvObj->createStatus.retVal != FVID2_SOK))
    {
        VX_PRINT(VX_ZONE_ERROR, ": Display M2M Create Failed!!!\r\n");
        status = (vx_status)VX_FAILURE;
    }

    /* Display M2M pipe configuration */
    if ((vx_status)VX_SUCCESS == status)
    {
        for (loopCnt = 0U ; loopCnt < drvObj->numPipe ; loopCnt++)
        {
            fvid2_status += Fvid2_control(drvObj->drvHandle,
                                          IOCTL_DSS_M2M_SET_PIPE_PARAMS,
                                          &drvObj->pipeCfg[loopCnt],
                                          NULL);
            fvid2_status += Fvid2_control(drvObj->drvHandle,
                                          IOCTL_DSS_M2M_SET_PIPE_MFLAG_PARAMS,
                                          &drvObj->mFlagCfg[loopCnt],
                                          NULL);
            fvid2_status += Fvid2_control(drvObj->drvHandle,
                                          IOCTL_DSS_M2M_SET_PIPE_CSC_COEFF,
                                          &drvObj->cscCfg[loopCnt],
                                          NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, ": Display M2M DISP IOCTL Failed!!!\r\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    /* Display M2M overlay configuration */
    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status += Fvid2_control(drvObj->drvHandle,
                                      IOCTL_DSS_DCTRL_SET_OVERLAY_PARAMS,
                                      &drvObj->ovrCfg,
                                      NULL);
        fvid2_status += Fvid2_control(drvObj->drvHandle,
                                      IOCTL_DSS_DCTRL_SET_LAYER_PARAMS,
                                      &drvObj->layerCfg,
                                      NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, ": Display M2M Overlay IOCTL Failed!!!\r\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Display M2M global configuration */
    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status += Fvid2_control(drvObj->drvHandle,
                                      IOCTL_DSS_DCTRL_SET_GLOBAL_DSS_PARAMS,
                                      &drvObj->globalParams,
                                      NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, ": Display M2M Global IOCTL Failed!!!\r\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Display M2M write-back pipe configuration */
    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status += Fvid2_control(drvObj->drvHandle,
                                      IOCTL_DSS_M2M_SET_WB_PIPE_PARAMS,
                                      &drvObj->wbCfg,
                                      NULL);
        fvid2_status += Fvid2_control(drvObj->drvHandle,
                                      IOCTL_DSS_M2M_SET_WB_PIPE_MFLAG_PARAMS,
                                      &drvObj->wbMflagCfg,
                                      NULL);
        fvid2_status += Fvid2_control(drvObj->drvHandle,
                                      IOCTL_DSS_M2M_SET_WB_PIPE_DMA_CFG,
                                      &drvObj->wbDmaCfg,
                                      NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, ": Display M2M WB IOCTL Failed!!!\r\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Start Display M2M Driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status = Fvid2_start(drvObj->drvHandle, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, ": Display M2M Driver Start Failed!!!\r\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return status;
}

static int32_t tivxDisplayM2MCallback(Fvid2_Handle handle, void *appData)
{
    tivxDisplayM2MDrvObj *drvObj = (tivxDisplayM2MDrvObj *)(appData);

    if ((NULL != drvObj) && (drvObj->waitForProcessCmpl != NULL))
    {
        tivxEventPost(drvObj->waitForProcessCmpl);
    }

    return (vx_status)VX_SUCCESS;
}

static vx_status tivxDisplayExtractFvid2Format(
                                    const tivx_obj_desc_image_t *obj_desc_img,
                                    Fvid2_Format *format)
{
    vx_status status = (vx_status)VX_SUCCESS;

    Fvid2Format_init(format);
    format->width = obj_desc_img->imagepatch_addr[0].dim_x;
    format->height = obj_desc_img->imagepatch_addr[0].dim_y;
    format->ccsFormat = FVID2_CCSF_BITS8_PACKED;
    format->scanFormat = FVID2_SF_PROGRESSIVE;

    switch (obj_desc_img->format)
    {
        case (vx_df_image)TIVX_DF_IMAGE_RGB565:
            format->dataFormat = FVID2_DF_BGR16_565;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGB:
            format->dataFormat = FVID2_DF_RGB24_888;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_RGBX:
            format->dataFormat = FVID2_DF_RGBX24_8888;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
            format->dataFormat = FVID2_DF_BGRX32_8888;
            format->pitch[FVID2_RGB_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
            format->dataFormat = FVID2_DF_YUV422I_UYVY;
            format->pitch[FVID2_YUV_INT_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            format->dataFormat = FVID2_DF_YUV422I_YUYV;
            format->pitch[FVID2_YUV_INT_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_NV12:
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[1].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_U16:
            format->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        case (vx_df_image)VX_DF_IMAGE_U8:
            format->ccsFormat = FVID2_CCSF_BITS8_PACKED;
            format->dataFormat = FVID2_DF_YUV420SP_UV;
            format->pitch[FVID2_YUV_SP_Y_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            format->pitch[FVID2_YUV_SP_CBCR_ADDR_IDX] = (uint32_t)obj_desc_img->imagepatch_addr[0].stride_y;
            break;
        default:
            status = (vx_status)VX_FAILURE;
            break;
    }

    return status;
}

static tivxDisplayM2MParams *tivxDispM2mAllocObject(tivxDisplayM2MInstObj *instObj)
{
    uint32_t                cnt;
    tivxDisplayM2MParams *m2mObj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < DISPLAY_M2M_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->m2mObj[cnt].isAlloc)
        {
            m2mObj = &instObj->m2mObj[cnt];
            memset(m2mObj, 0x0, sizeof(tivxDisplayM2MParams));
            instObj->m2mObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (m2mObj);
}

static void tivxDispM2mFreeObject(tivxDisplayM2MInstObj *instObj,
    tivxDisplayM2MParams *m2mObj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < DISPLAY_M2M_MAX_HANDLES; cnt ++)
    {
        if (m2mObj == &instObj->m2mObj[cnt])
        {
            m2mObj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}
