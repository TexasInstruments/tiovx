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
#include <vx_vpac_viss_target_priv.h>
#include "tivx_hwa_vpac_viss_priv.h"
#include <utils/ipc/include/app_ipc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* #undef below to see performance without VISS context save/restore */
#define VHWA_VISS_CTX_SAVE_RESTORE_ENABLE

/* #undef below to see performance using CPU for VISS context save/restore */
#define VHWA_VISS_CTX_SAVE_RESTORE_USE_DMA


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);


static tivxVpacVissObj *tivxVpacVissAllocObject(tivxVpacVissInstObj *instObj);
static void tivxVpacVissFreeObject(tivxVpacVissInstObj *instObj,
    tivxVpacVissObj *vissObj);
static void tivxVpacVissSetInputParams(tivxVpacVissObj *vissObj,
    const tivx_obj_desc_raw_image_t *raw_img_desc);
static vx_status tivxVpacVissSetOutputParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms,
    tivx_obj_desc_image_t *obj_desc[]);
static vx_status tivxVpacVissMapFormat(uint32_t *fmt, uint32_t *ccsFmt,
    uint32_t out_id, uint32_t vxFmt, uint32_t mux_val);
static vx_status tivxVpacVissMapStorageFormat(uint32_t *ccsFmt, uint32_t vxFmt);
static vx_status tivxVpacVissCheckInputDesc(uint16_t num_params,
    tivx_obj_desc_t *obj_desc[]);
static vx_status tivxVpacVissMapUserDesc(void **target_ptr,
    const tivx_obj_desc_user_data_object_t *desc, uint32_t size);
static vx_status tivxVpacVissUnmapUserDesc(void **target_ptr,
    const tivx_obj_desc_user_data_object_t *desc);
static vx_status vhwaVissAllocMemForCtx(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms);
static void vhwaVissFreeCtxMem(tivxVpacVissObj *vissObj);
static void vhwaVissRestoreCtx(const tivxVpacVissObj *vissObj);
static void vhwaVissSaveCtx(const tivxVpacVissObj *vissObj);
static void tivxVpacVissSetIsInvalidFlag(tivx_obj_desc_t *obj_desc[]);

int32_t tivxVpacVissFrameComplCb(Fvid2_Handle handle, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_viss_target_kernel = NULL;
tivxVpacVissInstObj gTivxVpacVissInstObj;

extern tivx_mutex             viss_aewb_lock[VHWA_M2M_VISS_MAX_HANDLES];
extern tivx_ae_awb_params_t   viss_aewb_results[VHWA_M2M_VISS_MAX_HANDLES];
extern uint32_t               viss_aewb_channel[VHWA_M2M_VISS_MAX_HANDLES];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacViss(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

#ifdef SOC_AM62A
    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
#else
    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
#endif
    {
        strncpy(target_name, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        memset(&gTivxVpacVissInstObj, 0x0, sizeof(tivxVpacVissInstObj));

        status = tivxMutexCreate(&gTivxVpacVissInstObj.lock);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Allocate lock \n");
        }
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_VISS_NAME,
                            target_name,
                            tivxVpacVissProcess,
                            tivxVpacVissCreate,
                            tivxVpacVissDelete,
                            tivxVpacVissControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelVpacViss(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != gTivxVpacVissInstObj.lock)
    {
        tivxMutexDelete(&gTivxVpacVissInstObj.lock);
    }

    status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Viss TargetKernel\n");
    }
}

/* Setting the output descriptor flags to invalid if received raw_image is invalid */
static void tivxVpacVissSetIsInvalidFlag(tivx_obj_desc_t *obj_desc[])
{
    uint32_t                   cnt;
    uint32_t                   out_start;

    if (tivxFlagIsBitSet(obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX]->flags, TIVX_REF_FLAG_IS_INVALID) == 1U)
    {
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
            if (NULL != obj_desc[out_start])
            {
                tivxFlagBitSet(&obj_desc[out_start]->flags, TIVX_REF_FLAG_IS_INVALID);
            }
            out_start ++;
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX])
        {
            tivxFlagBitSet(&obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX])
        {
            tivxFlagBitSet(&obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
    }
    else
    {
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
            if (NULL != obj_desc[out_start])
            {
                tivxFlagBitClear(&obj_desc[out_start]->flags, TIVX_REF_FLAG_IS_INVALID);
            }
            out_start ++;
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX])
        {
            tivxFlagBitClear(&obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX])
        {
            tivxFlagBitClear(&obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    fvid2_status = FVID2_SOK;
    uint32_t                   cnt;
    uint32_t                   out_start;
    tivxVpacVissObj           *vissObj = NULL;
    Vhwa_M2mVissParams        *vissDrvPrms = NULL;
    tivx_vpac_viss_params_t   *vissPrms;
    tivx_obj_desc_raw_image_t *raw_img_desc;
    tivx_obj_desc_image_t     *img_desc[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    tivx_ae_awb_params_t      *ae_awb_result = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t *h3a_out_desc = NULL;
    tivx_obj_desc_user_data_object_t *dcc_buf_desc = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    else
    {
        vissObj = tivxVpacVissAllocObject(&gTivxVpacVissInstObj);

        if (NULL != vissObj)
        {
            /* Assign object descriptors */
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
            dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_DCC_BUF_IDX];
            raw_img_desc = (tivx_obj_desc_raw_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
            h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

            /* Get All output image object descriptors */
            out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Alloc Failed for Viss Object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            if(config_desc->mem_size != sizeof(tivx_vpac_viss_params_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_vpac_viss_params_t, host size (%d) != target size (%d)\n",
                    config_desc->mem_size, sizeof(tivx_vpac_viss_params_t));
            }
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
        {
            if(aewb_res_desc->mem_size != sizeof(tivx_ae_awb_params_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_ae_awb_params_t, host size (%d) != target size (%d)\n",
                    aewb_res_desc->mem_size, sizeof(tivx_ae_awb_params_t));
            }
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != h3a_out_desc))
        {
            if(h3a_out_desc->mem_size != sizeof(tivx_h3a_data_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_h3a_data_t, host size (%d) != target size (%d)\n",
                    h3a_out_desc->mem_size, sizeof(tivx_h3a_data_t));
            }
            if (NULL != dcc_buf_desc)
            {
                vissObj->h3a_out_enabled = (vx_bool)vx_true_e;
            }
            else
            {
                VX_PRINT(VX_ZONE_WARNING,
                    "VISS H3A output is not generated due to DCC not being enabled\n");
            }

            /* This is taking the app ipc cpu number, not the tivx mapped number, so it can
             * be used by AWB node for direct usage without translation */
            vissObj->cpu_id = appIpcGetSelfCpuId();

        }

        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxMutexCreate(&vissObj->config_lock);
        }

        /* Now Map config Desc and get VISS Parameters */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxVpacVissMapUserDesc(&vissObj->viss_prms_target_ptr,
                config_desc, sizeof(tivx_vpac_viss_params_t));
            if ((vx_status)VX_SUCCESS == status)
            {
                vissPrms = (tivx_vpac_viss_params_t *)
                    vissObj->viss_prms_target_ptr;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Map VISS Parameters Descriptor\n");
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate mutex\n");
        }

        /* Extract AEWB Result parameters, it might be needed in
         * setting some of the VISS configuration */
        if ((vx_status)VX_SUCCESS == status)
        {
            if(NULL != aewb_res_desc)
            {
                status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                    aewb_res_desc, sizeof(tivx_ae_awb_params_t));
                if ((vx_status)VX_SUCCESS == status)
                {
                    ae_awb_result = (tivx_ae_awb_params_t *)
                        vissObj->aewb_res_target_ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map AEWB Result Descriptor\n");
                }
            }
            else
            {
                uint32_t i, loop_break = 0;

                /* AEWB Result sent by the graph is NULL */
                /* VISS needs to use the results sent by AEWB node through VISS_CMD_SET_2A_PARAMS command */
                /* RemoteService command is supported only on target*/
                for(i=0;i<VHWA_M2M_VISS_MAX_HANDLES;i++)
                {
                    status = tivxMutexLock(viss_aewb_lock[i]);
                    if((vx_status)VX_SUCCESS == status)
                    {
                        if(0 == viss_aewb_channel[i])
                        {
                            viss_aewb_channel[i] = 1u;
                            vissObj->channel_id = i;
                            memset(&viss_aewb_results[vissObj->channel_id], 0x0, sizeof(tivx_ae_awb_params_t));
                            loop_break = 1;
                        }
                        status = tivxMutexUnlock(viss_aewb_lock[i]);
                    }

                    if(((vx_status)VX_SUCCESS != status) || (loop_break == 1))
                    {
                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "viss_aewb_lock[%d] failed\n", i);
                        }
                        break;
                    }
                }

                if(((vx_status)VX_SUCCESS == status) && (loop_break == 0))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Number of instances has exceeded VHWA_M2M_VISS_MAX_HANDLES\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
            }
        }
    }

    /* Now allocate the require resources and open the FVID2 driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_m2mVissCreateArgsInit(&vissObj->createArgs);

        status = tivxEventCreate(&vissObj->waitForProcessCmpl);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            vissObj->cbPrms.cbFxn   = tivxVpacVissFrameComplCb;
            vissObj->cbPrms.appData = vissObj;

            vissObj->handle = Fvid2_create(FVID2_VHWA_M2M_VISS_DRV_ID,
                vissObj->viss_drv_inst_id, &vissObj->createArgs,
                NULL, &vissObj->cbPrms);

            if (NULL == vissObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Open Driver\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }
    }

    /* Allocate memory for the GLBCE Statistics */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = vhwaVissAllocMemForCtx(vissObj, vissPrms);
    }

    /* Extract the format information from the config descriptor
     * and output images and set the format in Driver using
     * SET_PARAMS ioctl  */
    if ((vx_status)VX_SUCCESS == status)
    {
        vissDrvPrms = &vissObj->vissPrms;

        /* Fill up the VISS Parameters and Set it in the driver */
        Vhwa_m2mVissParamsInit(vissDrvPrms);

        if (0U == vissPrms->bypass_glbce)
        {
            vissDrvPrms->enableGlbce = (uint32_t)TRUE;
        }
        else
        {
            vissDrvPrms->enableGlbce = (uint32_t)FALSE;
        }

        if (0U == vissPrms->bypass_nsf4)
        {
            vissDrvPrms->enableNsf4 = (uint32_t)TRUE;
        }
        else
        {
            vissDrvPrms->enableNsf4 = (uint32_t)FALSE;
        }

        #ifdef VPAC3L
        if (0U == vissPrms->bypass_pcid)
        {
            vissDrvPrms->enablePcid = (uint32_t)TRUE;
        }
        else
        {
            vissDrvPrms->enablePcid = (uint32_t)FALSE;
        }
        #endif
        vissDrvPrms->edgeEnhancerMode = vissPrms->fcp[0].ee_mode;

        /* Set the input image format and number of inputs from
         * raw image descriptor */
        tivxVpacVissSetInputParams(vissObj, raw_img_desc);

        {
            /* Set the output image format from the output images
             * this function also maps the vx_image format to
             * Fvid2_format and Fvid2_storage format and sets it in
             * viss output format */
            status = tivxVpacVissSetOutputParams(
                vissObj, vissPrms, img_desc);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set Output Params\n");
            }
        }
    }

    /* Set default values for ALL viss configuration parameters */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetDefaultParams(vissObj, vissPrms, NULL);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set H3A Source parameters, this mainly sets up the
         * input source to the h3a. */
        tivxVpacVissSetH3aSrcParams(vissObj, vissPrms);
    }

    /* Extract individual module specific parameters from the DCC data
     * base and set it in the driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != dcc_buf_desc)
        {
            vissObj->use_dcc = 1u;

            status = tivxVpacVissInitDcc(vissObj, vissPrms);

            if ((vx_status)VX_SUCCESS == status)
            {
                /* Parse DCC Database and store the output in local variables */
                status = tivxVpacVissSetParamsFromDcc(
                    vissObj, dcc_buf_desc, h3a_out_desc, ae_awb_result);
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set DCC Params\n");
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set DCC Params\n");
            }

            /* Enable DPC based in driver */
            if (((uint32_t)TRUE == vissObj->vissCfg.dpcLutCfg.enable) ||
                ((uint32_t)TRUE == vissObj->vissCfg.dpcOtfCfg.enable))
            {
                vissDrvPrms->enableDpc = (uint32_t)TRUE;
            }
            else
            {
                vissDrvPrms->enableDpc = (uint32_t)FALSE;
            }
        }
    }

    /* before writing configuration set application buffer */
    if ((vx_status) VX_SUCCESS == status)
    {
        status = tivxVpacVissSetConfigBuffer(vissObj);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set ConfigBuf in driver\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* All Formats, frame size, module enables are set in
         * viss parameters, call this ioctl to set validate and set
         * them in the driver */
        fvid2_status = Fvid2_control(vissObj->handle,
            IOCTL_VHWA_M2M_VISS_SET_PARAMS, (void *)vissDrvPrms, NULL);

        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Params in driver\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Now Set the parsed parameters in the VISS Driver,
     * This is required even for the non-DCC parameters, like
     * H3A Input source */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetConfigInDrv(vissObj);

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Reset this flag, as the config is already applied to driver */
            vissObj->isConfigUpdated = 0U;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set non-DCC Params\n");
        }
    }

    /* Unmap descriptor memories */
    if ((vx_status)VX_SUCCESS == status)
    {
        /* If the target pointer is non null, descriptor is also non null,
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if ((NULL != aewb_res_desc) && (NULL != vissObj->aewb_res_target_ptr))
        {
            status = tivxVpacVissUnmapUserDesc(&vissObj->aewb_res_target_ptr,
                aewb_res_desc);
        }

        /* If the target pointer is non null, descriptor is also non null
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if (NULL != vissObj->viss_prms_target_ptr)
        {
            status = tivxVpacVissUnmapUserDesc(&vissObj->viss_prms_target_ptr,
                config_desc);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(
            kernel, vissObj, sizeof(tivxVpacVissObj));

        /* Set up the input frame list */
        for (cnt = 0U; cnt < vissObj->num_in_buf; cnt ++)
        {
            vissObj->inFrmList.frames[cnt] = &vissObj->inFrm[cnt];
        }
        vissObj->inFrmList.numFrames = vissObj->num_in_buf;

        /* Set up the output frame list */
        for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt ++)
        {
            vissObj->outFrmList.frames[cnt] = &vissObj->outFrm[cnt];
        }
        vissObj->outFrmList.numFrames = VHWA_M2M_VISS_MAX_OUTPUTS;
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        if (NULL != vissObj)
        {
            tivxVpacVissDeInitDcc(vissObj);

            if (NULL != vissObj->handle)
            {
                Fvid2_delete(vissObj->handle, NULL);
                vissObj->handle = NULL;
            }

            if (NULL != vissObj->waitForProcessCmpl)
            {
                tivxEventDelete(&vissObj->waitForProcessCmpl);
            }

            if (NULL != vissObj->config_lock)
            {
                tivxMutexDelete(&vissObj->config_lock);
            }

            tivxVpacVissFreeObject(&gTivxVpacVissInstObj, vissObj);

            vhwaVissFreeCtxMem(vissObj);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxVpacVissObj         *vissObj = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&vissObj, &size);

        /* Check the validity of context object */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect kernel instance context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;

            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];

            if(NULL == aewb_res_desc)
            {
                status = tivxMutexLock(viss_aewb_lock[vissObj->channel_id]);
                if((vx_status)VX_SUCCESS == status)
                {
                    if(0 != viss_aewb_channel[vissObj->channel_id])
                    {
                        viss_aewb_channel[vissObj->channel_id] = 0u;
                    }
                    status = tivxMutexUnlock(viss_aewb_lock[vissObj->channel_id]);
                }

                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "viss_aewb_lock[%d] failed\n", vissObj->channel_id);
                }
            }

            tivxVpacVissDeInitDcc(vissObj);

#ifndef SOC_AM62A
            if (true == vissObj->configurationBuffer.configThroughUdmaFlag)
            {
                tivxVpacVissDeleteConfigBuffer(vissObj);
            }
#endif

            Fvid2_delete(vissObj->handle, NULL);
            vissObj->handle = NULL;

            if (NULL != vissObj->waitForProcessCmpl)
            {
                tivxEventDelete(&vissObj->waitForProcessCmpl);
            }

            if (NULL != vissObj->config_lock)
            {
                tivxMutexDelete(&vissObj->config_lock);
            }

            tivxVpacVissFreeObject(&gTivxVpacVissInstObj, vissObj);

            vhwaVissFreeCtxMem(vissObj);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    fvid2_status = FVID2_SOK;
    uint32_t                   cnt;
    uint32_t                   out_start;
    uint32_t                   buf_cnt;
    uint32_t                   size;
    tivxVpacVissObj           *vissObj = NULL;
    tivx_vpac_viss_params_t   *vissPrms = NULL;
    tivx_obj_desc_raw_image_t *raw_img_desc;
    tivx_obj_desc_image_t     *img_desc[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    tivx_ae_awb_params_t      *ae_awb_result = NULL;
    tivx_h3a_data_t           *h3a_out = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t *h3a_out_desc = NULL;
    uint64_t start_time, cur_time;
    tivx_ae_awb_params_t   aewb_params;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&vissObj, &size);

        /* Check the validity of context object */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect kernel instance context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            raw_img_desc = (tivx_obj_desc_raw_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];

            /* Convert object descriptor to image descriptor */
            out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }

            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
            config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
            h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
        }
    }

    if ( (vx_status)VX_SUCCESS == status)
    {
        tivxVpacVissSetIsInvalidFlag(obj_desc);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Config Desc Cannot be null */
        status = tivxVpacVissMapUserDesc(&vissObj->viss_prms_target_ptr,
            config_desc, sizeof(tivx_vpac_viss_params_t));
        if ((vx_status)VX_SUCCESS == status)
        {
            vissPrms = (tivx_vpac_viss_params_t *)vissObj->viss_prms_target_ptr;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Map VISS Parameters Descriptor\n");
        }

        /* AEWB Result is optional parameter */
        if((vx_status)VX_SUCCESS == status)
        {
            if(NULL != aewb_res_desc)
            {
                status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                    aewb_res_desc, sizeof(tivx_ae_awb_params_t));
                if ((vx_status)VX_SUCCESS == status)
                {
                    ae_awb_result = (tivx_ae_awb_params_t *)vissObj->aewb_res_target_ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map AEWB Result Descriptor\n");
                }
            }
            else
            {
                /* AEWB Result sent by the graph is NULL */
                /* VISS needs to use the results sent by AEWB node through VISS_CMD_SET_2A_PARAMS command */
                /* RemoteService command is supported only on target*/
                uint32_t chId = vissObj->channel_id;
                status = tivxMutexLock(viss_aewb_lock[chId]);
                if((vx_status)VX_SUCCESS == status)
                {
                    ae_awb_result = &aewb_params;
                    memcpy(ae_awb_result, &viss_aewb_results[chId], sizeof(tivx_ae_awb_params_t));
                    status = tivxMutexUnlock(viss_aewb_lock[chId]);
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            if (NULL != h3a_out_desc)
            {
                if (h3a_out_desc->mem_size == sizeof(tivx_h3a_data_t))
                {
                    vissObj->h3a_out_target_ptr = tivxMemShared2TargetPtr(&h3a_out_desc->mem_ptr);

                    h3a_out = (tivx_h3a_data_t *)vissObj->h3a_out_target_ptr;

                    /* H3A output is special case, only need to map the header since rest is written by HW */
                    tivxCheckStatus(&status, tivxMemBufferMap(vissObj->h3a_out_target_ptr, offsetof(tivx_h3a_data_t, resv),
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map H3A Result Descriptor\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Update Configuration in Driver */
        tivxMutexLock(vissObj->config_lock);

        /* Check if there is any change in H3A Input Source */
        tivxVpacVissSetH3aSrcParams(vissObj, vissPrms);

        if (NULL != ae_awb_result)
        {
            status = tivxVpacVissApplyAEWBParams(vissObj, ae_awb_result);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to apply AEWB Result\n");
            }
        }

        if (((vx_status)VX_SUCCESS == status) && (1u == vissObj->isConfigUpdated))
        {
            status = tivxVpacVissSetConfigInDrv(vissObj);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Set Config in Driver\n");
            }
        }
        tivxMutexUnlock(vissObj->config_lock);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set the buffer address in the input buffer */
        for (cnt = 0u; cnt < vissObj->num_in_buf; cnt ++)
        {
            vissObj->inFrm[cnt].addr[0u] = tivxMemShared2PhysPtr(
                raw_img_desc->img_ptr[cnt].shared_ptr,
                (int32_t)raw_img_desc->img_ptr[cnt].mem_heap_region);
        }

        /* Set the buffer address in the output buffer */
        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            for (buf_cnt = 0U; buf_cnt < vissObj->num_out_buf_addr[cnt];
                    buf_cnt ++)
            {
                vissObj->outFrm[cnt].addr[buf_cnt] = tivxMemShared2PhysPtr(
                    img_desc[cnt]->mem_ptr[buf_cnt].shared_ptr,
                    (int32_t)img_desc[cnt]->mem_ptr[buf_cnt].mem_heap_region);
            }
        }

        if (NULL != h3a_out)
        {
            h3a_out->aew_af_mode = vissPrms->h3a_aewb_af_mode;
            h3a_out->h3a_source_data = vissPrms->h3a_in;
            h3a_out->cpu_id = vissObj->cpu_id;
            h3a_out->channel_id = vissObj->channel_id;
            h3a_out->size = vissObj->h3a_output_size;

            if(0U == vissPrms->h3a_aewb_af_mode)
            {
                /* TI 2A Node may not need the aew config since it gets it from DCC, but this is copied
                 * in case third party 2A nodes which don't use DCC can easily see this information */
                memcpy(&h3a_out->aew_config, &vissObj->aew_config, sizeof(tivx_h3a_aew_config));
            }

            vissObj->outFrm[VHWA_M2M_VISS_OUT_H3A_IDX].addr[0u] =
                (uint64_t)h3a_out->data;

            h3a_out_desc->valid_mem_size = vissObj->h3a_output_size + TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN;

            /* Unmap even before processing since the ARM is done, rest of buffer is HW */
            tivxCheckStatus(&status, tivxMemBufferUnmap(vissObj->h3a_out_target_ptr, offsetof(tivx_h3a_data_t, resv), (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            vissObj->h3a_out_target_ptr = NULL;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxLogRtTraceKernelInstanceExeStart(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_DMA);
        vhwaVissRestoreCtx(vissObj);
        tivxLogRtTraceKernelInstanceExeEnd(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_DMA);

        start_time = tivxPlatformGetTimeInUsecs();

        tivxLogRtTraceKernelInstanceExeStart(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA);

        /* Submit the request to the driver */
        fvid2_status = Fvid2_processRequest(vissObj->handle, &vissObj->inFrmList,
            &vissObj->outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* Wait for Frame Completion */
            tivxEventWait(vissObj->waitForProcessCmpl,
                TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
            fvid2_status = Fvid2_getProcessedRequest(vissObj->handle,
                &vissObj->inFrmList, &vissObj->outFrmList, 0);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        cur_time = tivxPlatformGetTimeInUsecs();
        tivxLogRtTraceKernelInstanceExeEnd(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA);

        tivxLogRtTraceKernelInstanceExeStart(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_DMA);
        vhwaVissSaveCtx(vissObj);
        tivxLogRtTraceKernelInstanceExeEnd(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_DMA);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        appPerfStatsHwaUpdateLoad(vissObj->hwa_perf_id,
            (uint32_t)(cur_time-start_time),
            raw_img_desc->params.width*raw_img_desc->params.height /* pixels processed */
            );
    }

    /* If the target pointer is non null, descriptor is also non null,
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
    {
        status = tivxVpacVissUnmapUserDesc(&vissObj->aewb_res_target_ptr, aewb_res_desc);
    }

    /* If the target pointer is non null, descriptor is also non null
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    if (((vx_status)VX_SUCCESS == status) && (NULL != config_desc))
    {
        status = tivxVpacVissUnmapUserDesc(&vissObj->viss_prms_target_ptr, config_desc);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacVissObj                  *vissObj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&vissObj, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == vissObj) ||
        (sizeof(tivxVpacVissObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
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
            case TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS:
            {
                /* Update Configuration in Driver */
                tivxMutexLock(vissObj->config_lock);
                status = tivxVpacVissSetParamsFromDcc(vissObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U], NULL, NULL);
                tivxMutexUnlock(vissObj->config_lock);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Node Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }
    return (status);
}


/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacVissObj *tivxVpacVissAllocObject(tivxVpacVissInstObj *instObj)
{
    uint32_t         cnt;
    tivxVpacVissObj *vissObj = NULL;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->vissObj[cnt].isAlloc)
        {
            vissObj = &instObj->vissObj[cnt];
            memset(vissObj, 0x0, sizeof(tivxVpacVissObj));
            instObj->vissObj[cnt].isAlloc = 1U;

            #ifdef SOC_AM62A
            if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
            #else
            if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
            #endif
            {
                instObj->vissObj[cnt].viss_drv_inst_id = VHWA_M2M_VISS_DRV_INST0;
                instObj->vissObj[cnt].hwa_perf_id      = APP_PERF_HWA_VPAC1_VISS;
            }
            #if defined(SOC_J784S4)
            else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
            {
                instObj->vissObj[cnt].viss_drv_inst_id = VHWA_M2M_VPAC_1_VISS_DRV_INST_ID_0;
                instObj->vissObj[cnt].hwa_perf_id      = APP_PERF_HWA_VPAC2_VISS;
            }
            #endif
            break;
        }
    }

    if (NULL != vissObj)
    {
        /* Initialize few members to values, other than 0 */
        vissObj->lastH3aInSrc = RFE_H3A_IN_SEL_MAX;
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (vissObj);
}

static void tivxVpacVissFreeObject(tivxVpacVissInstObj *instObj,
    tivxVpacVissObj *vissObj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
    {
        if (vissObj == &instObj->vissObj[cnt])
        {
            vissObj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}

static vx_status tivxVpacVissSetOutputParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms,
    tivx_obj_desc_image_t *obj_desc[])
{
    vx_status                 status = (vx_status)VX_SUCCESS;
    uint32_t                  cnt;
    uint32_t                  out_cnt;
    uint32_t                  out_start;
    uint32_t                  mux_val[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    Vhwa_M2mVissOutputParams *outPrms = NULL;
    Vhwa_M2mVissParams       *vissDrvPrms;
    tivx_obj_desc_image_t    *im_desc;

    mux_val[0U] = vissPrms->fcp[0].mux_output0;
    mux_val[1U] = vissPrms->fcp[0].mux_output1;
    mux_val[2U] = vissPrms->fcp[0].mux_output2;
    mux_val[3U] = vissPrms->fcp[0].mux_output3;
    mux_val[4U] = vissPrms->fcp[0].mux_output4;
    vissDrvPrms = &vissObj->vissPrms;

    /* Disable all Outputs first */
    for (out_cnt = 0u; out_cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; out_cnt ++)
    {
        vissDrvPrms->outPrms[out_cnt].enable = FALSE;
    }

    out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
    for (out_cnt = 0u; out_cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; out_cnt ++)
    {
        vissObj->num_out_buf_addr[out_cnt] = 0U;
        im_desc = obj_desc[out_cnt];

        if (NULL != obj_desc[out_cnt])
        {
            outPrms = &vissDrvPrms->outPrms[out_cnt];

            status = tivxVpacVissMapFormat(
                &outPrms->fmt.dataFormat, &outPrms->fmt.ccsFormat, out_start,
                im_desc->format, mux_val[out_cnt]);

            if ((vx_status)VX_SUCCESS == status)
            {
                #ifdef VPAC3L
                if(TIVX_VPAC_VISS_IR_ENABLE == vissPrms->enable_ir_op)
                {
                    outPrms->isIrOut = VHWA_VISS_IROUT_ENABLED;
                }
                else
                {
                    outPrms->isIrOut = VHWA_VISS_IROUT_DISABLED;
                }
                #endif

                outPrms->enable = TRUE;
                outPrms->fmt.width = im_desc->width;
                outPrms->fmt.height = im_desc->height;

                for (cnt = 0u; cnt < TIVX_IMAGE_MAX_PLANES; cnt ++)
                {
                    outPrms->fmt.pitch[cnt] =
                        (uint32_t)im_desc->imagepatch_addr[cnt].stride_y;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to map format for output%d\n", out_cnt);
            }

            /* TODO: See if there are any others here */
            if ((FVID2_DF_YUV420SP_UV == outPrms->fmt.dataFormat) ||
                (FVID2_DF_YUV422SP_UV == outPrms->fmt.dataFormat))
            {
                vissObj->num_out_buf_addr[out_cnt] = 2U;
            }
            else
            {
                vissObj->num_out_buf_addr[out_cnt] = 1U;
            }
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }

        out_start ++;
    }

    /* H3A is one of the output for the Driver,
     * Enable it if required */
    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_bool)vx_true_e == vissObj->h3a_out_enabled)
        {
            outPrms = &vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_H3A_IDX];
            outPrms->enable = (uint32_t)TRUE;
            outPrms->fmt.dataFormat = FVID2_DF_RAW;
        }
    }

    return (status);
}

static void tivxVpacVissSetInputParams(tivxVpacVissObj *vissObj,
    const tivx_obj_desc_raw_image_t *raw_img_desc)
{
    Fvid2_Format        *fmt;
    Vhwa_M2mVissParams  *vissDrvPrms;

    vissDrvPrms = &vissObj->vissPrms;
    fmt = &vissDrvPrms->inFmt;

    /* Set number of inputs */
    if (1U == raw_img_desc->params.num_exposures)
    {
        vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT;
    }
    else if (2U == raw_img_desc->params.num_exposures)
    {
        vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE;
    }
    else if (3U == raw_img_desc->params.num_exposures)
    {
        vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE;
    }
    else
    {
        /* do nothing */
    }

    /* Set the Input Format */
    fmt->width = raw_img_desc->params.width;
    fmt->height = raw_img_desc->params.height;
    fmt->pitch[0] = (uint32_t)raw_img_desc->imagepatch_addr[0U].stride_y;
    fmt->dataFormat = FVID2_DF_RAW;

    switch (raw_img_desc->params.format[0U].pixel_container)
    {
        vx_uint32 msb;
        case (vx_enum)TIVX_RAW_IMAGE_8_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        case (vx_enum)TIVX_RAW_IMAGE_16_BIT:
            msb = raw_img_desc->params.format[0U].msb;
            if(msb < 7U)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Format \n");
                /* do nothing */
            }
            else if ((msb <= 11U) && (msb >= 8U))
            {
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            }
            else if ((msb == 12U) || (msb == 13U))
            {
                fmt->ccsFormat = FVID2_CCSF_BITS14_UNPACKED16;
            }
            else if (msb == 15U)
            {
                fmt->ccsFormat = FVID2_CCSF_BITS16_PACKED;
            }
            else if (msb == 9U)
            {
                fmt->ccsFormat = FVID2_CCSF_BITS16_PACKED;
            }
            else
            {
                /*MSB = 7 translates to FVID2_CCSF_BITS8_UNPACKED16*/
                /*MSB = 11 translates to FVID2_CCSF_BITS12_UNPACKED16*/
                /*MSB = 14 translates to FVID2_CCSF_BITS15_UNPACKED16*/
                fmt->ccsFormat = FVID2_CCSF_BITS8_UNPACKED16 + msb - 7U;
            }
            break;
        case (vx_enum)TIVX_RAW_IMAGE_P12_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        default:
            /* do nothing */
            break;
    }

    vissObj->num_in_buf = 1u;
    switch (vissDrvPrms->inputMode)
    {
        case VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT:
            vissObj->num_in_buf = 1u;
            break;
        case VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE:
            vissObj->num_in_buf = 2u;
            break;
        case VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE:
            vissObj->num_in_buf = 3u;
            break;
        default:
            vissObj->num_in_buf = 1u;
            break;
    }

    return;
}

static vx_status tivxVpacVissMapStorageFormat(uint32_t *ccsFmt, uint32_t vxFmt)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((vx_df_image)VX_DF_IMAGE_U16 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS12_UNPACKED16;
    }
    else if ((vx_df_image)TIVX_DF_IMAGE_P12 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS12_PACKED;
    }
    else if ((vx_df_image)VX_DF_IMAGE_U8 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS8_PACKED;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Storage Format \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxVpacVissMapFormat(uint32_t *fmt, uint32_t *ccsFmt,
    uint32_t out_id, uint32_t vxFmt, uint32_t mux_val)
{
    vx_status status = (vx_status)VX_SUCCESS;

    switch (mux_val)
    {
        case 0U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
            /* Map data format on mux val0 */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
            {
                *fmt = FVID2_DF_LUMA_ONLY;
            }
            else if ((TIVX_KERNEL_VPAC_VISS_OUT1_IDX == out_id) ||
                     (TIVX_KERNEL_VPAC_VISS_OUT3_IDX == out_id))
            {
                *fmt = FVID2_DF_CHROMA_ONLY;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux0 not supported on output4\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 1U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
            /* Map data format on mux val1 */
            else if (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id)
            {
                *fmt = FVID2_DF_RED;
            }
            else if (TIVX_KERNEL_VPAC_VISS_OUT3_IDX == out_id)
            {
                *fmt = FVID2_DF_GREEN;
            }
            else if (TIVX_KERNEL_VPAC_VISS_OUT4_IDX == out_id)
            {
                *fmt = FVID2_DF_BLUE;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux1 not supported on output0/1\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 2U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
            /* Map data format on mux val2 */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT1_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT3_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT4_IDX == out_id))
            {
                *fmt = FVID2_DF_RAW;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux2 not supported on output0\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 3U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map Storage Format Failed\n");
            }
            /* Map data format on mux val2 */
            else if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
            {
                *fmt = FVID2_DF_GREY;
            }
            else if (TIVX_KERNEL_VPAC_VISS_OUT4_IDX == out_id)
            {
                *fmt = FVID2_DF_SATURATION;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux3 not supported on output1/3\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 4U:
        {
            if ((vx_df_image)VX_DF_IMAGE_NV12 == vxFmt)
            {
                *ccsFmt = FVID2_CCSF_BITS8_PACKED;
            }
            else if ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == vxFmt)
            {
                *ccsFmt = FVID2_CCSF_BITS12_PACKED;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "only NV12 supported on mux4\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                    (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
                {
                    *fmt = FVID2_DF_YUV420SP_UV;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "only output0/2 supports on mux4\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
            break;
        }
        case 5U:
        {
            *ccsFmt = FVID2_CCSF_BITS8_PACKED;

            if (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id)
            {
                if ((vx_df_image)VX_DF_IMAGE_UYVY == vxFmt)
                {
                    *fmt = FVID2_DF_YUV422I_UYVY;
                }
                else if ((vx_df_image)VX_DF_IMAGE_YUYV == vxFmt)
                {
                    *fmt = FVID2_DF_YUV422I_YUYV;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "only UYVY/YUYV formats supported \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux5 is supported only on output2 \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        #ifdef VPAC3L
        case 6U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            /* Only packed 8 and unpacked 16 IR supported in this case*/
            if (TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id)
            {
                *fmt = FVID2_DF_RAW08;
            }
            else if (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id)
            {
                *fmt = FVID2_DF_RAW12;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux6 is supported only on output0/2 \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 7U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            /* Only packed 12 IR supported in this case */
            if (TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id)
            {
                *fmt = FVID2_DF_RAW12;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "mux7 is supported only on output0 \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        #endif
        default:
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid value of mux \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            break;
        }
    }

    return (status);
}

static vx_status tivxVpacVissMapUserDesc(void **target_ptr,
    const tivx_obj_desc_user_data_object_t *desc, uint32_t size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (desc->mem_size == size)
    {
        *target_ptr = tivxMemShared2TargetPtr(&desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(*target_ptr, desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Incorrect descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxVpacVissUnmapUserDesc(void **target_ptr,
    const tivx_obj_desc_user_data_object_t *desc)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, tivxMemBufferUnmap(*target_ptr, desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
        (vx_enum)VX_READ_ONLY));

    *target_ptr = NULL;

    return status;
}

static vx_status tivxVpacVissCheckInputDesc(uint16_t num_params,
    tivx_obj_desc_t *obj_desc[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    uint32_t out_start;

    if (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Num params incorrect, = %d\n", num_params);
    }

    if ((NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;

        if (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR, "Configuration is NULL\n");
        }
        if (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR, "Raw input is NULL\n");
        }
    }
    else /* At least one output must be enabled */
    {
        out_start = TIVX_KERNEL_VPAC_VISS_OUT0_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if (NULL != obj_desc[out_start])
            {
                break;
            }
            out_start ++;
        }

        if (cnt >= TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT)
        {
            VX_PRINT(VX_ZONE_ERROR, "Atleast one output must be enabled\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return (status);
}

static vx_status vhwaVissAllocMemForCtx(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Glbce_Control       glbceCtrl;

    if ((NULL != vissObj) && (NULL != vissPrms))
    {
        if ((0U == vissPrms->bypass_glbce) && (1u == vissPrms->enable_ctx))
        {
            glbceCtrl.module = GLBCE_MODULE_GET_STATS_INFO;
            glbceCtrl.statsInfo = &vissObj->glbceStatInfo;
            fvid2_status = Fvid2_control(vissObj->handle,
                IOCTL_GLBCE_GET_CONFIG, (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                vissObj->ctx_mem_phys_ptr = 0u;
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "Failed to get GLBCE Stats Info!!!\n");
            }
            else
            {
                #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_AM62A)
                /* ADASVISION-5065: Currently using TIVX_MEM_EXTERNAL while OCMC RAM is not enabled */
                tivxMemBufferAlloc(&vissObj->ctx_mem_ptr,
                    vissObj->glbceStatInfo.size, (vx_enum)TIVX_MEM_EXTERNAL);
                #else
                tivxMemBufferAlloc(&vissObj->ctx_mem_ptr,
                    vissObj->glbceStatInfo.size, (vx_enum)TIVX_MEM_INTERNAL_L3);
                #endif
                if ((int32_t)NULL == (int32_t)vissObj->ctx_mem_ptr.host_ptr)
                {
                    vissObj->ctx_mem_phys_ptr = 0u;
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to allocate memory!!!\n");
                }
                else
                {
                    vissObj->ctx_mem_phys_ptr = tivxMemShared2PhysPtr(
                        vissObj->ctx_mem_ptr.shared_ptr,
                        (int32_t)vissObj->ctx_mem_ptr.mem_heap_region);

                    VX_PRINT(VX_ZONE_INFO, "TIOVX: VISS: GLBCE ctx mem @ 0x%08x or size %d B\n", (uint32_t)vissObj->ctx_mem_phys_ptr, vissObj->glbceStatInfo.size);
                }
            }
        }
        else
        {
            vissObj->ctx_mem_phys_ptr = 0u;
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static void vhwaVissFreeCtxMem(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj)
    {
        if (0u != vissObj->ctx_mem_phys_ptr)
        {
            tivxMemBufferFree(&vissObj->ctx_mem_ptr,
                vissObj->glbceStatInfo.size);
            vissObj->ctx_mem_phys_ptr = 0u;
        }
    }
}

static void vhwaVissRestoreCtx(const tivxVpacVissObj *vissObj)
{
#ifndef SOC_AM62A
    #ifdef VHWA_VISS_CTX_SAVE_RESTORE_ENABLE
    #ifdef VHWA_VISS_CTX_SAVE_RESTORE_USE_DMA
    int32_t status;
    #endif
    app_udma_copy_1d_prms_t prms;

    if ((NULL != vissObj) && (0u != vissObj->ctx_mem_phys_ptr))
    {
        prms.src_addr = vissObj->ctx_mem_phys_ptr;
        prms.dest_addr = vissObj->glbceStatInfo.addr;
        prms.length = vissObj->glbceStatInfo.size;
        #ifdef VHWA_VISS_CTX_SAVE_RESTORE_USE_DMA
        status = appUdmaCopy1D(NULL, &prms);

        if (0 != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to restore Context !!!\n");
        }
        #else
        memcpy((void*)(uint32_t)prms.dest_addr, (void*)(uint32_t)prms.src_addr, prms.length);
        #endif
    }
    #endif
#endif
}

static void vhwaVissSaveCtx(const tivxVpacVissObj *vissObj)
{
#ifndef SOC_AM62A
    #ifdef VHWA_VISS_CTX_SAVE_RESTORE_ENABLE
    #ifdef VHWA_VISS_CTX_SAVE_RESTORE_USE_DMA
    int32_t status;
    #endif
    app_udma_copy_1d_prms_t prms;

    if ((NULL != vissObj) && (0u != vissObj->ctx_mem_phys_ptr))
    {
        prms.src_addr = vissObj->glbceStatInfo.addr;
        prms.dest_addr = vissObj->ctx_mem_phys_ptr;
        prms.length = vissObj->glbceStatInfo.size;
        #ifdef VHWA_VISS_CTX_SAVE_RESTORE_USE_DMA
        status = appUdmaCopy1D(NULL, &prms);

        if (0 != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to restore Context !!!\n");
        }
        #else
        memcpy((void*)(uint32_t)prms.dest_addr, (void*)(uint32_t)prms.src_addr, prms.length);
        #endif
    }
    #endif
#endif
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */



/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacVissFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacVissObj *vissObj = (tivxVpacVissObj *)appData;

    if (NULL != vissObj)
    {
        tivxEventPost(vissObj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}


