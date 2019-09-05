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

#include "utils/perf_stats/include/app_perf_stats.h"
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


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
static vx_status tivxVpacVissSetInputParams(tivxVpacVissObj *vissObj,
    tivx_obj_desc_raw_image_t *raw_img_desc);
static vx_status tivxVpacVissSetOutputParams(tivxVpacVissObj *vissObj,
    tivx_vpac_viss_params_t *vissPrms,
    tivx_obj_desc_image_t *obj_desc[]);
static vx_status tivxVpacVissMapFormat(uint32_t *fmt, uint32_t *ccsFmt,
    uint32_t out_id, uint32_t vxFmt, uint32_t mux_val);
static vx_status tivxVpacVissMapStorageFormat(uint32_t *ccsFmt, uint32_t vxFmt);
static vx_status tivxVpacVissCheckInputDesc(uint16_t num_params,
    tivx_obj_desc_t *obj_desc[]);
static vx_status tivxVpacVissMapUserDesc(void **target_ptr,
    tivx_obj_desc_user_data_object_t *desc, uint32_t size);
static void tivxVpacVissUnmapUserDesc(void **target_ptr,
    tivx_obj_desc_user_data_object_t *desc);
static vx_status vhwaVissAllocMemForCtx(tivxVpacVissObj *vissObj,
    tivx_vpac_viss_params_t *vissPrms);
static void vhwaVissFreeCtxMem(tivxVpacVissObj *vissObj);
static void vhwaVissRestoreCtx(tivxVpacVissObj *vissObj);
static void vhwaVissSaveCtx(tivxVpacVissObj *vissObj);

int32_t tivxVpacVissFrameComplCb(Fvid2_Handle handle, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_viss_target_kernel = NULL;
tivxVpacVissInstObj gTivxVpacVissInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacViss(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxAddTargetKernelVpacViss: Invalid CPU ID\n");
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        memset(&gTivxVpacVissInstObj, 0x0, sizeof(tivxVpacVissInstObj));

        status = tivxMutexCreate(&gTivxVpacVissInstObj.lock);

        if (VX_SUCCESS != status)
        {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxAddTargetKernelVpacViss: Failed to Allocate lock \n");
        }
    }

    if (status == VX_SUCCESS)
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
    vx_status status = VX_SUCCESS;

    if (NULL != gTivxVpacVissInstObj.lock)
    {
        tivxMutexDelete(&gTivxVpacVissInstObj.lock);
    }

    status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxRemoveTargetKernelVpacViss: Failed to Remove Viss TargetKernel\n");
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
    vx_status                  status = VX_SUCCESS;
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
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissCreate: Input Descriptor Error\n");
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
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCreate: Alloc Failed for Viss Object\n");
            status = VX_ERROR_NO_RESOURCES;
        }

        if (VX_SUCCESS == status)
        {
            if(config_desc->mem_size != sizeof(tivx_vpac_viss_params_t))
            {
                status = VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: tivx_vpac_viss_params_t, host size (%d) != target size (%d)\n",
                    config_desc->mem_size, sizeof(tivx_vpac_viss_params_t));
            }
        }

        if ((VX_SUCCESS == status) && (NULL != aewb_res_desc))
        {
            if(aewb_res_desc->mem_size != sizeof(tivx_ae_awb_params_t))
            {
                status = VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: tivx_ae_awb_params_t, host size (%d) != target size (%d)\n",
                    aewb_res_desc->mem_size, sizeof(tivx_ae_awb_params_t));
            }
        }

        if ((VX_SUCCESS == status) && (NULL != h3a_out_desc))
        {
            if(h3a_out_desc->mem_size != sizeof(tivx_h3a_data_t))
            {
                status = VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: tivx_h3a_data_t, host size (%d) != target size (%d)\n",
                    h3a_out_desc->mem_size, sizeof(tivx_h3a_data_t));
            }
            vissObj->h3a_out_enabled = vx_true_e;
        }

        if (VX_SUCCESS == status)
        {
            status = tivxMutexCreate(&vissObj->config_lock);
        }

        /* Now Map config Desc and get VISS Parameters */
        if (VX_SUCCESS == status)
        {
            status = tivxVpacVissMapUserDesc(&vissObj->viss_prms_target_ptr,
                config_desc, sizeof(tivx_vpac_viss_params_t));
            if (VX_SUCCESS == status)
            {
                vissPrms = (tivx_vpac_viss_params_t *)
                    vissObj->viss_prms_target_ptr;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: Failed to Map VISS Parameters Descriptor\n");
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCreate: Failed to allocate mutex\n");
        }

        /* Extract AEWB Result parameters, it might be needed in
         * setting some of the VISS configuration */
        if ((VX_SUCCESS == status) && (NULL != aewb_res_desc))
        {
            status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                aewb_res_desc, sizeof(tivx_ae_awb_params_t));
            if (VX_SUCCESS == status)
            {
                ae_awb_result = (tivx_ae_awb_params_t *)
                    vissObj->aewb_res_target_ptr;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: Failed to Map AEWB Result Descriptor\n");
            }
        }
    }

    /* Now allocate the require resources and open the FVID2 driver */
    if (VX_SUCCESS == status)
    {
        Vhwa_m2mVissCreateArgsInit(&vissObj->createArgs);

        status = tivxEventCreate(&vissObj->waitForProcessCmpl);
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCreate: Failed to allocate Event\n");
        }

        if (VX_SUCCESS == status)
        {
            vissObj->cbPrms.cbFxn   = tivxVpacVissFrameComplCb;
            vissObj->cbPrms.appData = vissObj;

            vissObj->handle = Fvid2_create(FVID2_VHWA_M2M_VISS_DRV_ID,
                VHWA_M2M_VISS_DRV_INST0, &vissObj->createArgs,
                NULL, &vissObj->cbPrms);

            if (NULL == vissObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: Failed to Open Driver\n");
                status = VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCreate: Failed to Create Mutex\n");
        }
    }

    /* Allocate memory for the GLBCE Statistics */
    if (VX_SUCCESS == status)
    {
        status = vhwaVissAllocMemForCtx(vissObj, vissPrms);
    }

    /* Extract the format information from the config descriptor
     * and output images and set the format in Driver using
     * SET_PARAMS ioctl  */
    if (VX_SUCCESS == status)
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
        vissDrvPrms->edgeEnhancerMode = vissPrms->ee_mode;

        /* Set the input image format and number of inputs from
         * raw image descriptor */
        status = tivxVpacVissSetInputParams(vissObj, raw_img_desc);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCreate: Failed to set Input Params\n");
        }
        else
        {
            /* Set the output image format from the output images
             * this function also maps the vx_image format to
             * Fvid2_format and Fvid2_storage format and sets it in
             * viss output format */
            status = tivxVpacVissSetOutputParams(
                vissObj, vissPrms, img_desc);
            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: Failed to set Output Params\n");
            }
        }

        if (VX_SUCCESS == status)
        {
            /* All Formats, frame size, module enables are set in
             * viss parameters, call this ioctl to set validate and set
             * them in the driver */
            fvid2_status = Fvid2_control(vissObj->handle,
                IOCTL_VHWA_M2M_VISS_SET_PARAMS, (void *)vissDrvPrms, NULL);

            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: Failed to set Params in driver\n");
                status = VX_FAILURE;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Set H3A Source parameters, this mainly sets up the
         * input source to the h3a. */
        tivxVpacVissSetH3aSrcParams(vissObj, vissPrms);
    }

    /* Extract individual module specific parameters from the DCC data
     * base and set it in the driver */
    if (VX_SUCCESS == status)
    {
        if (NULL != dcc_buf_desc)
        {
            vissObj->use_dcc = 1u;

            status = tivxVpacVissInitDcc(vissObj, vissPrms);

            if (VX_SUCCESS == status)
            {
                /* Parse DCC Database and store the output in local variables */
                status = tivxVpacVissSetParamsFromDcc(
                    vissObj, dcc_buf_desc, h3a_out_desc, ae_awb_result);
                if (VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissCreate: Failed to Parse and Set DCC Params\n");
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissCreate: Failed to Parse and Set DCC Params\n");
            }
        }
        else
        {
            /* set defaults */
            tivxVpacVissDccMapRfeParams(vissObj);
            tivxVpacVissDccMapFlexCFAParamsDefaults(vissObj);
            tivxVpacVissDccMapFlexCCParams(vissObj);
            tivxVpacVissDccMapEeParams(vissObj);
        }
    }

    /* Now Set the parsed parameters in the VISS Driver,
     * This is required even for the non-DCC parameters, like
     * H3A Input source */
    if (VX_SUCCESS == status)
    {
        status = tivxVpacVissSetConfigInDrv(vissObj);

        if (VX_SUCCESS == status)
        {
            /* Reset this flag, as the config is already applied to driver */
            vissObj->isConfigUpdated = 0U;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCreate: Failed to Parse and Set non-DCC Params\n");
        }
    }

    /* Unmap descriptor memories */
    if ( (VX_SUCCESS == status) && (NULL != vissObj))
    {
        /* If the target pointer is non null, descriptor is also non null,
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if (NULL != vissObj->aewb_res_target_ptr)
        {
            tivxVpacVissUnmapUserDesc(&vissObj->aewb_res_target_ptr,
                aewb_res_desc);
        }

        /* If the target pointer is non null, descriptor is also non null
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if (NULL != vissObj->viss_prms_target_ptr)
        {
            tivxVpacVissUnmapUserDesc(&vissObj->viss_prms_target_ptr,
                config_desc);
        }
    }

    if (VX_SUCCESS == status)
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

        vissObj->bypass_nsf4 = vissPrms->bypass_nsf4;
        vissObj->sensor_dcc_id = vissPrms->sensor_dcc_id;
    }

    if (VX_SUCCESS != status)
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
    vx_status                status = VX_SUCCESS;
    uint32_t                 size;
    tivxVpacVissObj         *vissObj = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissDelete: Input Descriptor Error\n");
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&vissObj, &size);

        /* Check the validity of context object */
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissDelete: Incorrect kernel instance context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissDelete: Incorrect Object Size\n");
            status = VX_FAILURE;
        }
        else
        {
            tivxVpacVissDeInitDcc(vissObj);

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
    vx_status                  status = VX_SUCCESS;
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
    uint64_t cur_time;

    /* Check for mandatory descriptor */
    status = tivxVpacVissCheckInputDesc(num_params, obj_desc);
    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissProcess: Input Descriptor Error\n");
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&vissObj, &size);

        /* Check the validity of context object */
        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissProcess: Incorrect kernel instance context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissProcess: Incorrect Object Size\n");
            status = VX_FAILURE;
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

    if (VX_SUCCESS == status)
    {
        /* Config Desc Cannot be null */
        status = tivxVpacVissMapUserDesc(&vissObj->viss_prms_target_ptr,
            config_desc, sizeof(tivx_vpac_viss_params_t));
        if (VX_SUCCESS == status)
        {
            vissPrms = (tivx_vpac_viss_params_t *)vissObj->viss_prms_target_ptr;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissProcess: Failed to Map VISS Parameters Descriptor\n");
        }

        /* AEWB Result is optional parameter */
        if ((VX_SUCCESS == status) && (NULL != aewb_res_desc))
        {
            status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                aewb_res_desc, sizeof(tivx_ae_awb_params_t));
            if (VX_SUCCESS == status)
            {
                ae_awb_result =
                    (tivx_ae_awb_params_t *)vissObj->aewb_res_target_ptr;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissProcess: Failed to Map AEWB Result Descriptor\n");
            }
        }

        if (VX_SUCCESS == status)
        {
            if (NULL != h3a_out_desc)
            {
                status = tivxVpacVissMapUserDesc(&vissObj->h3a_out_target_ptr,
                    h3a_out_desc, sizeof(tivx_h3a_data_t));
                if (VX_SUCCESS == status)
                {
                    h3a_out =
                        (tivx_h3a_data_t *)vissObj->h3a_out_target_ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissProcess: Failed to Map H3A Result Descriptor\n");
                }
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Update Configuration in Driver */
        tivxMutexLock(vissObj->config_lock);

        /* Check if there is any change in H3A Input Source */
        tivxVpacVissSetH3aSrcParams(vissObj, vissPrms);

        if (NULL != ae_awb_result)
        {
            status = tivxVpacVissApplyAEWBParams(vissObj, ae_awb_result);
            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissProcess: Failed to apply AEWB Result\n");
            }
        }

        if ((VX_SUCCESS == status) && (1u == vissObj->isConfigUpdated))
        {
            status = tivxVpacVissSetConfigInDrv(vissObj);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissProcess: Failed to Set Config in Driver\n");
            }
        }
        tivxMutexUnlock(vissObj->config_lock);
    }

    if (VX_SUCCESS == status)
    {
        /* Set the buffer address in the input buffer */
        for (cnt = 0u; cnt < vissObj->num_in_buf; cnt ++)
        {
            vissObj->inFrm[cnt].addr[0u] = tivxMemShared2PhysPtr(
                raw_img_desc->img_ptr[cnt].shared_ptr,
                raw_img_desc->img_ptr[cnt].mem_heap_region);
        }

        /* Set the buffer address in the output buffer */
        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
        {
            for (buf_cnt = 0U; buf_cnt < vissObj->num_out_buf_addr[cnt];
                    buf_cnt ++)
            {
                vissObj->outFrm[cnt].addr[buf_cnt] = tivxMemShared2PhysPtr(
                    img_desc[cnt]->mem_ptr[buf_cnt].shared_ptr,
                    img_desc[cnt]->mem_ptr[buf_cnt].mem_heap_region);
            }
        }

        if (NULL != h3a_out)
        {
            h3a_out->aew_af_mode = vissPrms->h3a_aewb_af_mode;
            h3a_out->h3a_source_data = vissPrms->h3a_in;
            h3a_out->size = vissObj->h3a_output_size;

            if(0 == vissPrms->h3a_aewb_af_mode)
            {
                /* TI 2A Node may not need the aew config since it gets it from DCC, but this is copied
                 * in case third party 2A nodes which don't use DCC can easily see this information */
                memcpy(&h3a_out->aew_config, &vissObj->aew_config, sizeof(tivx_h3a_aew_config));
            }

            vissObj->outFrm[VHWA_M2M_VISS_OUT_H3A_IDX].addr[0u] =
                (uint64_t)h3a_out->data;
        }
    }

    if (VX_SUCCESS == status)
    {
        vhwaVissRestoreCtx(vissObj);

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit the request to the driver */
        fvid2_status = Fvid2_processRequest(vissObj->handle, &vissObj->inFrmList,
            &vissObj->outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissProcess: Failed to Submit Request\n");
            status = VX_FAILURE;
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissProcess: Failed to Get Processed Request\n");
                status = VX_FAILURE;
            }
        }

        vhwaVissSaveCtx(vissObj);
    }

    if (VX_SUCCESS == status)
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        /* TODO: Figure out how to calculate pixels here */
        appPerfStatsHwaUpdateLoad(APP_PERF_HWA_VISS,
            cur_time,
            raw_img_desc->params.width*raw_img_desc->params.height /* pixels processed */
            );
    }

    /* If the target pointer is non null, descriptor is also non null,
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    if ((VX_SUCCESS == status) && (NULL != aewb_res_desc))
    {
        tivxVpacVissUnmapUserDesc(&vissObj->aewb_res_target_ptr, aewb_res_desc);
    }

    /* If the target pointer is non null, descriptor is also non null
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    if ((VX_SUCCESS == status) && (NULL != config_desc))
    {
        tivxVpacVissUnmapUserDesc(&vissObj->viss_prms_target_ptr, config_desc);
    }

    if ((VX_SUCCESS == status) && (NULL != h3a_out_desc))
    {
        tivxVpacVissUnmapUserDesc(&vissObj->h3a_out_target_ptr, h3a_out_desc);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    uint32_t                          size;
    tivxVpacVissObj                  *vissObj = NULL;

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&vissObj, &size);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissControl: Failed to Get Target Kernel Instance Context\n");
        }
        else if ((NULL == vissObj) ||
            (sizeof(tivxVpacVissObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissControl: Incorrect Object Size\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissControl: Invalid Node Command Id\n");
                status = VX_FAILURE;
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

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->vissObj[cnt].isAlloc)
        {
            vissObj = &instObj->vissObj[cnt];
            memset(vissObj, 0x0, sizeof(tivxVpacVissObj));
            instObj->vissObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Initialize few members to values, other than 0 */
    vissObj->lastH3aInSrc = RFE_H3A_IN_SEL_MAX;

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
    tivx_vpac_viss_params_t *vissPrms,
    tivx_obj_desc_image_t *obj_desc[])
{
    vx_status                 status = VX_SUCCESS;
    uint32_t                  cnt;
    uint32_t                  out_cnt;
    uint32_t                  out_start;
    uint32_t                  mux_val[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    Vhwa_M2mVissOutputParams *outPrms = NULL;
    Vhwa_M2mVissParams       *vissDrvPrms;
    tivx_obj_desc_image_t    *im_desc;

    mux_val[0U] = vissPrms->mux_output0;
    mux_val[1U] = vissPrms->mux_output1;
    mux_val[2U] = vissPrms->mux_output2;
    mux_val[3U] = vissPrms->mux_output3;
    mux_val[4U] = vissPrms->mux_output4;
    vissDrvPrms = &vissObj->vissPrms;

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

            if (VX_SUCCESS == status)
            {
                outPrms->enable = TRUE;
                outPrms->fmt.width = im_desc->width;
                outPrms->fmt.height = im_desc->height;

                for (cnt = 0u; cnt < TIVX_IMAGE_MAX_PLANES; cnt ++)
                {
                    outPrms->fmt.pitch[cnt] =
                        im_desc->imagepatch_addr[cnt].stride_y;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissSetOutputParams: Failed to map format for output%d\n", out_cnt);
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

        if (VX_SUCCESS != status)
        {
            break;
        }

        out_start ++;
    }

    /* H3A is one of the output for the Driver,
     * Enable it if required */
    if (VX_SUCCESS == status)
    {
        if (vx_true_e == vissObj->h3a_out_enabled)
        {
            outPrms = &vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_H3A_IDX];
            outPrms->enable = (uint32_t)TRUE;
            outPrms->fmt.dataFormat = FVID2_DF_RAW;
        }
    }

    return (status);
}

static vx_status tivxVpacVissSetInputParams(tivxVpacVissObj *vissObj,
    tivx_obj_desc_raw_image_t *raw_img_desc)
{
    vx_status            status = VX_SUCCESS;
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

    /* Set the Input Format */
    fmt->width = raw_img_desc->params.width;
    fmt->height = raw_img_desc->params.height;
    fmt->pitch[0] = raw_img_desc->imagepatch_addr[0U].stride_y;
    fmt->dataFormat = FVID2_DF_RAW;

    switch (raw_img_desc->params.format[0U].pixel_container)
    {
        case TIVX_RAW_IMAGE_8_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        case TIVX_RAW_IMAGE_16_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            break;
        case TIVX_RAW_IMAGE_P12_BIT:
            fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissSetInputParams: Invalid Input Format\n");
            status = VX_ERROR_INVALID_PARAMETERS;
            break;
    }

    if (VX_SUCCESS == status)
    {
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
    }

    return (status);
}

static vx_status tivxVpacVissMapStorageFormat(uint32_t *ccsFmt, uint32_t vxFmt)
{
    vx_status status = VX_SUCCESS;

    if (VX_DF_IMAGE_U16 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS12_UNPACKED16;
    }
    else if (TIVX_DF_IMAGE_P12 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS12_PACKED;
    }
    else if (VX_DF_IMAGE_U8 == vxFmt)
    {
        *ccsFmt = FVID2_CCSF_BITS8_PACKED;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissMapStorageFormat: Invalid Storage Format \n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxVpacVissMapFormat(uint32_t *fmt, uint32_t *ccsFmt,
    uint32_t out_id, uint32_t vxFmt, uint32_t mux_val)
{
    vx_status status = VX_SUCCESS;

    switch (mux_val)
    {
        case 0U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: Map Storage Format Failed\n");
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: mux0 not supported on output4\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 1U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: Map Storage Format Failed\n");
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: mux1 not supported on output0/1\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 2U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: Map Storage Format Failed\n");
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: mux2 not supported on output0\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 3U:
        {
            /* Map single plane storage format */
            status = tivxVpacVissMapStorageFormat(ccsFmt, vxFmt);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: Map Storage Format Failed\n");
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: mux3 not supported on output1/3\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case 4U:
        {
            if (VX_DF_IMAGE_NV12 == vxFmt)
            {
                *ccsFmt = FVID2_CCSF_BITS8_PACKED;
            }
            else if (TIVX_DF_IMAGE_NV12_P12 == vxFmt)
            {
                *ccsFmt = FVID2_CCSF_BITS12_PACKED;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: only NV12 supported on mux4\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }

            if (VX_SUCCESS == status)
            {
                if ((TIVX_KERNEL_VPAC_VISS_OUT0_IDX == out_id) ||
                    (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id))
                {
                    *fmt = FVID2_DF_YUV420SP_UV;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissMapFormat: only output0/2 supports on mux4\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }
            break;
        }
        case 5U:
        {
            *ccsFmt = FVID2_CCSF_BITS8_PACKED;

            if (TIVX_KERNEL_VPAC_VISS_OUT2_IDX == out_id)
            {
                if (VX_DF_IMAGE_UYVY == vxFmt)
                {
                    *fmt = FVID2_DF_YUV422I_UYVY;
                }
                else if (VX_DF_IMAGE_YUYV == vxFmt)
                {
                    *fmt = FVID2_DF_YUV422I_YUYV;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissMapFormat: only UYVY/YUYV formats supported \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissMapFormat: mux5 is supported only on output2 \n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        default:
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissMapFormat: Invalid value of mux \n");
            status = VX_ERROR_INVALID_PARAMETERS;
            break;
        }
    }

    return (status);
}

static vx_status tivxVpacVissMapUserDesc(void **target_ptr,
    tivx_obj_desc_user_data_object_t *desc, uint32_t size)
{
    vx_status status = VX_SUCCESS;

    if (desc->mem_size == size)
    {
        *target_ptr = tivxMemShared2TargetPtr(
            desc->mem_ptr.shared_ptr, desc->mem_ptr.mem_heap_region);

        tivxMemBufferMap(*target_ptr, desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissMapUserDesc: Incorrect descriptor\n");
        status = VX_FAILURE;
    }

    return (status);
}

static void tivxVpacVissUnmapUserDesc(void **target_ptr,
    tivx_obj_desc_user_data_object_t *desc)
{
    tivxMemBufferUnmap(*target_ptr, desc->mem_size, VX_MEMORY_TYPE_HOST,
        VX_READ_ONLY);
    *target_ptr = NULL;
}

static vx_status tivxVpacVissCheckInputDesc(uint16_t num_params,
    tivx_obj_desc_t *obj_desc[])
{
    vx_status status = VX_SUCCESS;
    uint32_t cnt;
    uint32_t out_start;

    if (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissCheckInputDesc: Num params incorrect, = %d\n", num_params);
    }

    if ((NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissCheckInputDesc: Invalid Descriptor\n");
        status = VX_FAILURE;

        if (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCheckInputDesc: Configuration is NULL\n");
        }
        if (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCheckInputDesc: Raw input is NULL\n");
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
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissCheckInputDesc: Atleast one output must be enabled\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return (status);
}

static vx_status vhwaVissAllocMemForCtx(tivxVpacVissObj *vissObj,
    tivx_vpac_viss_params_t *vissPrms)
{
    vx_status           status = VX_SUCCESS;
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
                status = VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR,
                    "vhwaVissAllocMemForCtx: Failed to get GLBCE Stats Info!!!\n");
            }
            else
            {
                tivxMemBufferAlloc(&vissObj->ctx_mem_ptr,
                    vissObj->glbceStatInfo.size, TIVX_MEM_EXTERNAL);
                if (NULL == vissObj->ctx_mem_ptr.host_ptr)
                {
                    vissObj->ctx_mem_phys_ptr = 0u;
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR,
                        "vhwaVissAllocMemForCtx: Failed to allocate memory!!!\n");
                }
                else
                {
                    vissObj->ctx_mem_phys_ptr = tivxMemShared2PhysPtr(
                        vissObj->ctx_mem_ptr.shared_ptr,
                        vissObj->ctx_mem_ptr.mem_heap_region);
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
        status = VX_FAILURE;
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

static void vhwaVissRestoreCtx(tivxVpacVissObj *vissObj)
{
    int32_t status;
    app_udma_copy_1d_prms_t prms;

    if ((NULL != vissObj) && (0u != vissObj->ctx_mem_phys_ptr))
    {
        prms.src_addr = vissObj->ctx_mem_phys_ptr;
        prms.dest_addr = vissObj->glbceStatInfo.addr;
        prms.length = vissObj->glbceStatInfo.size;
        status = appUdmaCopy1D(NULL, &prms);

        if (0u != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "vhwaVissRestoreCtx: Failed to restore Context !!!\n");
        }
    }
}

static void vhwaVissSaveCtx(tivxVpacVissObj *vissObj)
{
    int32_t status;
    app_udma_copy_1d_prms_t prms;

    if ((NULL != vissObj) && (0u != vissObj->ctx_mem_phys_ptr))
    {
        prms.src_addr = vissObj->glbceStatInfo.addr;
        prms.dest_addr = vissObj->ctx_mem_phys_ptr;
        prms.length = vissObj->glbceStatInfo.size;
        status = appUdmaCopy1D(NULL, &prms);

        if (0u != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "vhwaVissSaveCtx: Failed to restore Context !!!\n");
        }
    }
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


