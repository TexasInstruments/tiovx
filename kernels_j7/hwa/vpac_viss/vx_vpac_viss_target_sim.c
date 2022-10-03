/*
 *
 * Copyright (c) 2019-2021 Texas Instruments Incorporated
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
#include <stdio.h>
#include "vx_vpac_viss_target_sim_priv.h"

#include "tivx_hwa_vpac_viss_priv.h"
#include "vx_kernels_hwa_target.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define NO_SHIFT 0
#define UP_SHIFT 1
#define DOWN_SHIFT 2

#define Y12_PRE_COPY  NO_SHIFT
#define Y12_POST_COPY NO_SHIFT
#define Y8_PRE_COPY   UP_SHIFT
#define Y8_POST_COPY  DOWN_SHIFT

#define VISS_MAX_PATH_SIZE 256

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

static vx_status tivxVpacVissAllocMem(tivxVpacVissParams *prms);
static void tivxVpacVissFreeMem(tivxVpacVissParams *prms);
static void tivxVpacVissCopyShift(uint16_t src[], uint16_t dst[], int32_t size, uint16_t shift_policy);
static vx_status tivxVpacVissConfigSimDataPath(tivxVpacVissParams *prms, tivx_vpac_viss_params_t *vissPrms,
                                               tivx_obj_desc_t *obj_desc[], uint16_t num_params);

tivx_ae_awb_params_t g_ae_awb_result;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_vpac_viss_target_kernel[2] = {NULL};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacViss(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_MAX_NAME);
        vx_vpac_viss_target_kernel[0] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_VISS_NAME,
                            target_name,
                            tivxVpacVissProcess,
                            tivxVpacVissCreate,
                            tivxVpacVissDelete,
                            tivxVpacVissControl,
                            NULL);
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_VISS1, TIVX_TARGET_MAX_NAME);
        vx_vpac_viss_target_kernel[1] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_VISS_NAME,
                            target_name,
                            tivxVpacVissProcess,
                            tivxVpacVissCreate,
                            tivxVpacVissDelete,
                            tivxVpacVissControl,
                            NULL);

    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
    }
}

void tivxRemoveTargetKernelVpacViss(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel[0]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_viss_target_kernel[0] = NULL;
        }
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel[1]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_viss_target_kernel[1] = NULL;
        }
    }
    #endif
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxVpacVissObj           *vissObj = NULL;
    tivx_vpac_viss_params_t   *vissPrms = NULL;
    tivx_obj_desc_raw_image_t *raw_desc;
    tivx_ae_awb_params_t      *ae_awb_result = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t *h3a_out_desc = NULL;
    tivx_obj_desc_user_data_object_t *dcc_buf_desc = NULL;

    if ( (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        tivxVpacVissParams *prms = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        aewb_res_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_DCC_BUF_IDX];
        raw_desc = (tivx_obj_desc_raw_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
        h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

        /* Allocate node instance handle */

        prms = tivxMemAlloc(sizeof(tivxVpacVissParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxVpacVissParams));

            prms->width = raw_desc->imagepatch_addr[0].dim_x;
            prms->height = raw_desc->imagepatch_addr[0].dim_y;
            prms->buffer_size = (prms->width * prms->height) * 2;

            vissObj = &prms->vissObj;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Alloc Failed for Viss Object\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        /* Error checking for if the user data object sizes match between host and target side */

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
        }

        /* VISS modules are divided into different simulators and intermediate buffer storage
         * is required for PC simulator because of this. Allocate intermediate data buffers here. */

        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxVpacVissAllocMem(prms);
        }

        /* Map and dereference the config and aewb results data structures from the node input params */

        if ((vx_status)VX_SUCCESS == status)
        {
            void *configuration_target_ptr;

            configuration_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr, config_desc->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            vissPrms = (tivx_vpac_viss_params_t *)configuration_target_ptr;
        }
        if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
        {
            void *aewb_res_target_ptr;

            aewb_res_target_ptr = tivxMemShared2TargetPtr(&aewb_res_desc->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(aewb_res_target_ptr, aewb_res_desc->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            ae_awb_result = (tivx_ae_awb_params_t *)aewb_res_target_ptr;
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
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* In one shot, syncronize the drv data structures to the sim data structures */
            status = tivxVpacVissSetConfigInSim(prms);

            if ((vx_status)VX_SUCCESS == status)
            {
                /* Reset this flag, as the config is already applied to simulator */
                vissObj->isConfigUpdated = 0U;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "Failed to Parse and Set viss Params to simulator data structures\n");
            }
        }

        /* Configure the data path parameters and connections between the VISS submodules
         * based on the node parameter configurations (config input, as well as the data
         * formats and existance of optional parameters) */

        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxVpacVissConfigSimDataPath(prms, vissPrms, obj_desc, num_params);
        }

        /* Unmap descriptor memories */
        if ((vx_status)VX_SUCCESS == status)
        {
            /* If the target pointer is non null, descriptor is also non null,
             * Even if there is any error, if this pointer is non-null,
             * unmap must be called */
            if ((NULL != aewb_res_desc) && (NULL != ae_awb_result))
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap((void *)ae_awb_result, aewb_res_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }

            /* If the target pointer is non null, descriptor is also non null
             * Even if there is any error, if this pointer is non-null,
             * unmap must be called */
            if (NULL != vissPrms)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap((void *)vissPrms, config_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
        }

        /* Save node instance handle, or if any failures, free memory allocations */

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxVpacVissParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVpacVissDeInitDcc(&prms->vissObj);
                tivxVpacVissFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxVpacVissParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacVissParams) == size))
        {
            tivxVpacVissDeInitDcc(&prms->vissObj);
            tivxVpacVissFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *config_desc;
    tivx_obj_desc_user_data_object_t *ae_awb_result_desc;
    tivx_obj_desc_raw_image_t *raw_desc;
    tivx_obj_desc_image_t *y12_desc;
    tivx_obj_desc_image_t *uv12_c1_desc;
    tivx_obj_desc_image_t *y8_r8_c2_desc;
    tivx_obj_desc_image_t *uv8_g8_c3_desc;
    tivx_obj_desc_image_t *s8_b8_c4_desc;
    tivx_obj_desc_distribution_t *histogram0_desc;
    tivx_obj_desc_distribution_t *histogram1_desc;
    tivx_obj_desc_distribution_t *raw_histogram_desc;
    tivx_obj_desc_user_data_object_t *h3a_out_desc;
    tivxVpacVissParams *prms = NULL;
    tivxVpacVissObj    *vissObj = NULL;
    tivx_vpac_viss_params_t *vissPrms;

    if ( (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw_desc = (tivx_obj_desc_raw_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
        y12_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT0_IDX];
        uv12_c1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT1_IDX];
        y8_r8_c2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT2_IDX];
        uv8_g8_c3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT3_IDX];
        s8_b8_c4_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT4_IDX];
        histogram0_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX];
        histogram1_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM1_IDX];
        raw_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_HISTOGRAM_IDX];
        h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxVpacVissParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        int32_t i;
        void *configuration_target_ptr;
        void *ae_awb_result_target_ptr;
        void *raw_target_ptr[3];
        void *raw_mem_target_ptr[3];
        void *y12_target_ptr = NULL;
        void *uv12_c1_target_ptr = NULL;
        void *y8_r8_c2_target_ptr = NULL;
        void *uv8_g8_c3_target_ptr = NULL;
        void *s8_b8_c4_target_ptr = NULL;
        void *histogram0_target_ptr = NULL;
        void *histogram1_target_ptr = NULL;
        void *raw_histogram_target_ptr = NULL;
        void *h3a_aew_af_target_ptr = NULL;
        uint32_t num_exposures = raw_desc->params.num_exposures;
        tivx_ae_awb_params_t * ae_awb_result = NULL;

        raw_target_ptr[0] = NULL;
        raw_target_ptr[1] = NULL;
        raw_target_ptr[2] = NULL;
        raw_mem_target_ptr[0] = NULL;
        raw_mem_target_ptr[1] = NULL;
        raw_mem_target_ptr[2] = NULL;

        vissObj = &prms->vissObj;

        configuration_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr,
           config_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        if( NULL != ae_awb_result_desc)
        {
            ae_awb_result_target_ptr = tivxMemShared2TargetPtr(&ae_awb_result_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(ae_awb_result_target_ptr,
               ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));

            ae_awb_result = (tivx_ae_awb_params_t *)ae_awb_result_target_ptr;
        }
        else
        {
            ae_awb_result = &g_ae_awb_result;
        }
        /* Get image pointer(s) */
        for(i=0; i < num_exposures; i++)
        {
            raw_target_ptr[i] = tivxMemShared2TargetPtr(&raw_desc->img_ptr[i]);
        }

        /* Map buffer(s) */
        raw_mem_target_ptr[0] = tivxMemShared2TargetPtr(&raw_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(raw_mem_target_ptr[0],
            raw_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        if (!raw_desc->params.line_interleaved)
        {
            for(i=1; i < num_exposures; i++)
            {
                raw_mem_target_ptr[i] = tivxMemShared2TargetPtr(&raw_desc->mem_ptr[i]);
                tivxCheckStatus(&status, tivxMemBufferMap(raw_mem_target_ptr[i],
                    raw_desc->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_READ_ONLY));
            }
        }

        if( y12_desc != NULL)
        {
            y12_target_ptr = tivxMemShared2TargetPtr(&y12_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(y12_target_ptr,
               y12_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y12_desc->format)
            {
                uv12_c1_target_ptr = tivxMemShared2TargetPtr(&y12_desc->mem_ptr[1]);
                tivxCheckStatus(&status, tivxMemBufferMap(uv12_c1_target_ptr,
                  y12_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                   (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv12_c1_desc != NULL)
        {
            uv12_c1_target_ptr = tivxMemShared2TargetPtr(&uv12_c1_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(uv12_c1_target_ptr,
               uv12_c1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( y8_r8_c2_desc != NULL)
        {
            y8_r8_c2_target_ptr = tivxMemShared2TargetPtr(&y8_r8_c2_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(y8_r8_c2_target_ptr,
               y8_r8_c2_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            if ((vx_df_image)VX_DF_IMAGE_NV12 == y8_r8_c2_desc->format)
            {
                uv8_g8_c3_target_ptr = tivxMemShared2TargetPtr(&y8_r8_c2_desc->mem_ptr[1]);
                tivxCheckStatus(&status, tivxMemBufferMap(uv8_g8_c3_target_ptr,
                  y8_r8_c2_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                   (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv8_g8_c3_desc != NULL)
        {
            uv8_g8_c3_target_ptr = tivxMemShared2TargetPtr(&uv8_g8_c3_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(uv8_g8_c3_target_ptr,
               uv8_g8_c3_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( s8_b8_c4_desc != NULL)
        {
            s8_b8_c4_target_ptr = tivxMemShared2TargetPtr(&s8_b8_c4_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(s8_b8_c4_target_ptr,
               s8_b8_c4_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( histogram0_desc != NULL)
        {
            histogram0_target_ptr = tivxMemShared2TargetPtr(&histogram0_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(histogram0_target_ptr,
               histogram0_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( histogram1_desc != NULL)
        {
            histogram1_target_ptr = tivxMemShared2TargetPtr(&histogram1_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(histogram1_target_ptr,
               histogram1_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( raw_histogram_desc != NULL)
        {
            raw_histogram_target_ptr = tivxMemShared2TargetPtr(&raw_histogram_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(raw_histogram_target_ptr,
               raw_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( h3a_out_desc != NULL)
        {
            h3a_aew_af_target_ptr = tivxMemShared2TargetPtr(&h3a_out_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(h3a_aew_af_target_ptr,
               h3a_out_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        vissPrms = (tivx_vpac_viss_params_t *)configuration_target_ptr;

        /* call kernel processing function */

        /* Read non-NUll input buffers (up to 3) */
        lse_reformat_in_viss(raw_desc, raw_target_ptr[0], prms->raw0_16, 0);
        if (num_exposures > 1U)
        {
            lse_reformat_in_viss(raw_desc, raw_target_ptr[1], prms->raw1_16, 1);
        }
        if (num_exposures > 2U)
        {
            lse_reformat_in_viss(raw_desc, raw_target_ptr[2], prms->raw2_16, 2);
        }

        /* PROCESSING */

        /*
         * FCP Instance
         * ------------
         *
         *          |-------------------->|
         *          |        |----------->|--[5]->
         *   FLEXCFA -> FLEXCC -> EE ---->|
         *                   |
         *                   '--> HIST-->
         *
         *
         * VPAC_VISS1
         * ----------
         *
         * RAWFE -> NSF4 -> GLBCE -> FCP[0]--[5]->
         *     '--> H3A --->            '--> HIST -->
         *
         *
         * VPAC_VISS3
         * ----------
         *
         *     |-------------------------------->|        |---------> HIST[1]
         *     |       |------------------------>|        |
         *     |       |            |----------->|---> FCP[1]--[5]-> |
         *     |       |            |        |-->|                   |--[5]->
         * RAWFE -> CAC -> NSF4+DWB -> GLBCE --------> FCP[0]--[5]-> |
         *     |       '---> RawHist ---->                |
         *     '--> H3A --->                              '---------> HIST[0]
         *
         */

        /* RAWFE */

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
            status = tivxVpacVissSetConfigInSim(prms);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Set Config in Simulator\n");
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            rawfe_main(&prms->rawfe_params, prms->raw2_16, prms->raw1_16, prms->raw0_16, prms->scratch_rawfe_raw_out, prms->scratch_rawfe_h3a_out);

            /* H3A */
            if( h3a_out_desc != NULL)
            {
                h3a_top(&prms->h3a_in, &prms->h3a_params, prms->scratch_af_result, prms->scratch_aew_result);
            }

#ifdef VPAC3
            /* CAC */
            if(0 == prms->bypass_cac)
            {
                cac_raw(1, prms->cac_params.colorEnable,
                        prms->cac_lut[0], prms->cac_lut[1], prms->cac_lut[2], prms->cac_lut[3],
                        prms->cac_params.blkSize, prms->cac_params.blkGridSize.hCnt, prms->cac_params.blkGridSize.vCnt,
                        prms->scratch_rawfe_raw_out, prms->scratch_cac_out, prms->width, prms->height);
            }

            if( raw_histogram_desc != NULL)
            {
                RawHistogram_top (&prms->raw_hist_params, prms->pScratch_cac_out, prms->scratch_raw_hist_out);
            }
#endif

            /* NSF4 */
            if(0 == prms->bypass_nsf4)
            {
                nsf4_main(&prms->nsf4_params, prms->pScratch_cac_out, prms->scratch_nsf4v_out);
            }

#ifdef VPAC3
            /* NFS4_DWB */
            if(0 == prms->bypass_dwb)
            {
                void *dwbLineWeights = &prms->dwb_params.dwbLineWeights;
                wb_lut(1, dwbLineWeights, prms->dwb_x, prms->dwb_y, prms->dwb_s,
                       prms->pScratch_nsf4v_out, prms->scratch_dwb_out, prms->width, prms->height);
            }
#endif

            /* GLBCE */
            if(0 == prms->bypass_glbce)
            {
                int retval;

                retval = glbce_process(prms->hGlbce, prms->pScratch_dwb_out, prms->scratch_glbce_out);

                if(retval < 0)
                {
                    VX_PRINT(VX_ZONE_ERROR, "glbce_process returned failure\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            int k;

            for(k=0; k < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; k++)
            {
                /* Convert glbce 16-bit output to 32-bit input to CFA */
                for(i=0; i < (prms->buffer_size/2); i++)
                {
                    prms->fcp[k].scratch_cfa_in[i] = (uint32_t)prms->fcp[k].pScratch_cfa_in_16[i];
                }

                /* FLEXCFA */
                FLXD_Demosaic(prms->fcp[k].scratch_cfa_in, prms->fcp[k].scratch_cfa_out, raw_desc->params.width, raw_desc->params.height, 12, &prms->flexcfa_params[k]);


                /* FLEXCC */
                if(0 == prms->fcp[k].bypass_cc)
                {
                    flexcc_top_processing(prms->fcp[k].scratch_cfa_out, prms->fcp[k].scratch_cc_out, prms->fcp[k].scratch_hist, &prms->flexcc_params[k]);
                }

                /* EE */
                if(0 == prms->fcp[k].bypass_ee)
                {
                    tivxVpacVissCopyShift(prms->fcp[k].scratch_ee_shift_in, prms->fcp[k].scratch_ee_in, prms->buffer_size, prms->fcp[k].pre_copy);
                    ee_top(&prms->ee_params[k], prms->fcp[k].scratch_ee_in, prms->fcp[k].scratch_ee_out);
                    tivxVpacVissCopyShift(prms->fcp[k].scratch_ee_out, prms->fcp[k].scratch_ee_shift_out, prms->buffer_size, prms->fcp[k].post_copy);
                }
            }

            /* Fill non-NULL output buffers (up to 7) */

            if( y12_desc != NULL)
            {
                if (((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y12_desc->format) ||
                     ((vx_df_image)VX_DF_IMAGE_NV12 == y12_desc->format) ||
                     ((vx_df_image)VX_DF_IMAGE_YUYV == y12_desc->format) ||
                     ((vx_df_image)VX_DF_IMAGE_UYVY == y12_desc->format))
                {
                    lse_reformat_out_viss(raw_desc, y12_desc, y12_target_ptr, uv12_c1_target_ptr, prms->out_0_16, prms->out_1_16, prms->out_0_align);
                }
                else
                {
                    lse_reformat_out_viss(raw_desc, y12_desc, y12_target_ptr, NULL, prms->out_0_16, NULL, prms->out_0_align);
                }
            }
            if( uv12_c1_desc != NULL)
            {
                lse_reformat_out_viss(raw_desc, uv12_c1_desc, uv12_c1_target_ptr, NULL, prms->out_1_16, NULL, prms->out_1_align);
            }
            if( y8_r8_c2_desc != NULL)
            {
                if (((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y8_r8_c2_desc->format) ||
                    ((vx_df_image)VX_DF_IMAGE_NV12 == y8_r8_c2_desc->format) ||
                    ((vx_df_image)VX_DF_IMAGE_YUYV == y8_r8_c2_desc->format) ||
                    ((vx_df_image)VX_DF_IMAGE_UYVY == y8_r8_c2_desc->format))
                {
                    lse_reformat_out_viss(raw_desc, y8_r8_c2_desc, y8_r8_c2_target_ptr, uv8_g8_c3_target_ptr, prms->out_2_16, prms->out_3_16, prms->out_2_align);
                }
                else
                {
                    lse_reformat_out_viss(raw_desc, y8_r8_c2_desc, y8_r8_c2_target_ptr, NULL, prms->out_2_16, NULL, prms->out_2_align);
                }
            }
            if( uv8_g8_c3_desc != NULL)
            {
                lse_reformat_out_viss(raw_desc, uv8_g8_c3_desc, uv8_g8_c3_target_ptr, NULL, prms->out_3_16, NULL, prms->out_3_align);
            }
            if( s8_b8_c4_desc != NULL)
            {
                lse_reformat_out_viss(raw_desc, s8_b8_c4_desc, s8_b8_c4_target_ptr, NULL, prms->out_4_16, NULL, prms->out_4_align);
            }
            if( histogram0_desc != NULL)
            {
                memcpy(histogram0_target_ptr, prms->fcp[0].scratch_hist, 256*sizeof(uint32_t));
            }
            if( histogram1_desc != NULL)
            {
                memcpy(histogram1_target_ptr, prms->fcp[1].scratch_hist, 256*sizeof(uint32_t));
            }
            if( raw_histogram_desc != NULL)
            {
                memcpy(raw_histogram_target_ptr, prms->scratch_raw_hist_out, 128*sizeof(uint32_t));
            }
            if( h3a_out_desc != NULL)
            {
                tivx_h3a_data_t *pH3a_buf = (tivx_h3a_data_t*)h3a_aew_af_target_ptr;
                pH3a_buf->aew_af_mode = vissPrms->h3a_aewb_af_mode;
                pH3a_buf->h3a_source_data = vissPrms->h3a_in;

                if( (0 == vissPrms->h3a_aewb_af_mode) && (NULL != prms->scratch_aew_result))
                {
                    /* TI 2A Node may not need the aew config since it gets it from DCC, but this is copied
                     * in case third party 2A nodes which don't use DCC can easily see this information */
                    memcpy(&pH3a_buf->aew_config, &prms->aew_config, sizeof(tivx_h3a_aew_config));
                    pH3a_buf->size = prms->aew_buffer_size;
                    memcpy((void *)pH3a_buf->data, prms->scratch_aew_result, prms->aew_buffer_size);
                }
                else if( (1 == vissPrms->h3a_aewb_af_mode) && (NULL != prms->scratch_af_result) )
                {
                    pH3a_buf->size = prms->af_buffer_size;
                    memcpy((void *)pH3a_buf->data, prms->scratch_af_result, prms->af_buffer_size);
                }
                h3a_out_desc->valid_mem_size = pH3a_buf->size + TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN;
            }

            prms->viss_frame_count++;
        }

        /* kernel processing function complete */

        tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr,
           config_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        if( NULL != ae_awb_result_desc)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(ae_awb_result_target_ptr,
               ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(raw_mem_target_ptr[0],
            raw_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        if (!raw_desc->params.line_interleaved)
        {
            for(i=1; i < num_exposures; i++)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(raw_mem_target_ptr[i],
                    raw_desc->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_READ_ONLY));
            }
        }

        if( y12_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(y12_target_ptr,
               y12_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if (((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y12_desc->format) ||
                ((vx_df_image)VX_DF_IMAGE_NV12 == y12_desc->format))
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(uv12_c1_target_ptr,
                   y12_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv12_c1_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(uv12_c1_target_ptr,
               uv12_c1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( y8_r8_c2_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(y8_r8_c2_target_ptr,
               y8_r8_c2_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if (((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y8_r8_c2_desc->format) ||
                ((vx_df_image)VX_DF_IMAGE_NV12 == y8_r8_c2_desc->format))
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(uv8_g8_c3_target_ptr,
                   y8_r8_c2_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv8_g8_c3_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(uv8_g8_c3_target_ptr,
               uv8_g8_c3_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( s8_b8_c4_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(s8_b8_c4_target_ptr,
               s8_b8_c4_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( histogram0_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(histogram0_target_ptr,
               histogram0_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( histogram1_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(histogram1_target_ptr,
               histogram1_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( raw_histogram_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(raw_histogram_target_ptr,
               raw_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( h3a_out_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(h3a_aew_af_target_ptr,
               h3a_out_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacVissParams                *prms = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == prms) ||
        (sizeof(tivxVpacVissParams) != size))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "Incorrect Object Size\n");
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
                /* Update Configuration in Simulator */
                status = tivxVpacVissSetParamsFromDcc(&prms->vissObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U], NULL, NULL);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "Invalid Node Command Id\n");
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

static vx_status tivxVpacVissAllocMem(tivxVpacVissParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != prms)
    {
        uint32_t i, k;

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->raw0_16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->raw0_16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status) /* always allocate as per cmodel */
        {
            prms->raw1_16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->raw1_16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status) /* always allocate as per cmodel */
        {
            prms->raw2_16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->raw2_16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_rawfe_raw_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_rawfe_raw_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_rawfe_h3a_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_rawfe_h3a_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_cac_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_cac_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_dwb_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_dwb_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_nsf4v_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_nsf4v_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_glbce_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_glbce_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->scratch_raw_hist_out = tivxMemAlloc(128*sizeof(uint32_t), (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->scratch_raw_hist_out)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        for (k=0; k<TIVX_VPAC_VISS_FCP_NUM_INSTANCES;k++)
        {
            if ((vx_status)VX_SUCCESS == status)
            {
                prms->fcp[k].scratch_cfa_in = tivxMemAlloc(prms->buffer_size*2, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->fcp[k].scratch_cfa_in)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            for (i = 0; i < FLXD_NUM_FIR; i++)
            {
                if ((vx_status)VX_SUCCESS == status)
                {
                    prms->fcp[k].scratch_cfa_out[i] = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->fcp[k].scratch_cfa_out[i])
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                }
            }

            for (i = 0; i < 8; i++)
            {
                if ((vx_status)VX_SUCCESS == status)
                {
                    prms->fcp[k].scratch_cc_out[i] = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->fcp[k].scratch_cc_out[i])
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->fcp[k].scratch_hist = tivxMemAlloc(256*sizeof(uint32_t), (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->fcp[k].scratch_hist)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->fcp[k].scratch_ee_in = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->fcp[k].scratch_ee_in)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->fcp[k].scratch_ee_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->fcp[k].scratch_ee_out)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->fcp[k].scratch_ee_shift_out = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->fcp[k].scratch_ee_shift_out)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static void tivxVpacVissFreeMem(tivxVpacVissParams *prms)
{
    int retval;

    if (NULL != prms)
    {
        uint32_t i, k;

        if (NULL != prms->raw0_16)
        {
            tivxMemFree(prms->raw0_16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->raw0_16 = NULL;
        }
        if (NULL != prms->raw1_16)
        {
            tivxMemFree(prms->raw1_16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->raw1_16 = NULL;
        }
        if (NULL != prms->raw2_16)
        {
            tivxMemFree(prms->raw2_16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->raw2_16 = NULL;
        }
        if (NULL != prms->scratch_rawfe_raw_out)
        {
            tivxMemFree(prms->scratch_rawfe_raw_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_rawfe_raw_out = NULL;
        }
        if (NULL != prms->scratch_rawfe_h3a_out)
        {
            tivxMemFree(prms->scratch_rawfe_h3a_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_rawfe_h3a_out = NULL;
        }
        if (NULL != prms->scratch_aew_result)
        {
            tivxMemFree(prms->scratch_aew_result, prms->aew_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_aew_result = NULL;
        }
        if (NULL != prms->scratch_af_result)
        {
            tivxMemFree(prms->scratch_af_result, prms->af_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_af_result = NULL;
        }
        if (NULL != prms->scratch_cac_out)
        {
            tivxMemFree(prms->scratch_cac_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_cac_out = NULL;
        }
        if (NULL != prms->scratch_dwb_out)
        {
            tivxMemFree(prms->scratch_dwb_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_dwb_out = NULL;
        }
        if (NULL != prms->scratch_nsf4v_out)
        {
            tivxMemFree(prms->scratch_nsf4v_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_nsf4v_out = NULL;
        }
        if (NULL != prms->scratch_glbce_out)
        {
            tivxMemFree(prms->scratch_glbce_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_glbce_out = NULL;
        }
        if (NULL != prms->scratch_raw_hist_out)
        {
            tivxMemFree(prms->scratch_raw_hist_out, 128*sizeof(uint32_t), (vx_enum)TIVX_MEM_EXTERNAL);
            prms->scratch_raw_hist_out = NULL;
        }
        for(k=0;k<TIVX_VPAC_VISS_FCP_NUM_INSTANCES;k++)
        {
            if (NULL != prms->fcp[k].scratch_cfa_in)
            {
                tivxMemFree(prms->fcp[k].scratch_cfa_in, prms->buffer_size*2, (vx_enum)TIVX_MEM_EXTERNAL);
                prms->fcp[k].scratch_cfa_in = NULL;
            }
            for (i = 0; i < FLXD_NUM_FIR; i++)
            {
                if (NULL != prms->fcp[k].scratch_cfa_out[i])
                {
                    tivxMemFree(prms->fcp[k].scratch_cfa_out[i], prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    prms->fcp[k].scratch_cfa_out[i] = NULL;
                }
            }
            for (i = 0; i < 8; i++)
            {
                if (NULL != prms->fcp[k].scratch_cc_out[i])
                {
                    tivxMemFree(prms->fcp[k].scratch_cc_out[i], prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    prms->fcp[k].scratch_cc_out[i] = NULL;
                }
            }
            if (NULL != prms->fcp[k].scratch_hist)
            {
                tivxMemFree(prms->fcp[k].scratch_hist, 256*sizeof(uint32_t), (vx_enum)TIVX_MEM_EXTERNAL);
                prms->fcp[k].scratch_hist = NULL;
            }
            if (NULL != prms->fcp[k].scratch_ee_in)
            {
                tivxMemFree(prms->fcp[k].scratch_ee_in, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                prms->fcp[k].scratch_ee_in = NULL;
            }
            if (NULL != prms->fcp[k].scratch_ee_out)
            {
                tivxMemFree(prms->fcp[k].scratch_ee_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                prms->fcp[k].scratch_ee_out = NULL;
            }
            if (NULL != prms->fcp[k].scratch_ee_shift_out)
            {
                tivxMemFree(prms->fcp[k].scratch_ee_shift_out, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                prms->fcp[k].scratch_ee_shift_out = NULL;
            }

            prms->fcp[k].out_y12_16 = NULL;
            prms->fcp[k].out_uv12_c1_16 = NULL;
            prms->fcp[k].out_y8_r8_c2_16 = NULL;
            prms->fcp[k].out_uv8_g8_c3_16 = NULL;
            prms->fcp[k].out_s8_b8_c4_16 = NULL;
            prms->fcp[k].scratch_ee_shift_in = NULL;
            prms->fcp[k].pScratch_cfa_in_16 = NULL;
        }

        if(NULL != prms->dcc_out_buf)
        {
            tivxMemFree(prms->dcc_out_buf, prms->dcc_out_numbytes, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dcc_out_buf = NULL;
        }

        if(NULL != prms->dcc_input_params)
        {
            tivxMemFree(prms->dcc_input_params, sizeof(dcc_parser_input_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dcc_input_params = NULL;
        }

        if(NULL != prms->dcc_output_params)
        {
            tivxMemFree(prms->dcc_output_params, sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dcc_output_params = NULL;
        }

        prms->out_0_16 = NULL;
        prms->out_1_16 = NULL;
        prms->out_2_16 = NULL;
        prms->out_3_16 = NULL;
        prms->out_4_16 = NULL;
        prms->pScratch_nsf4v_out = NULL;
        prms->pScratch_cac_out = NULL;
        prms->pScratch_dwb_out = NULL;
        prms->pScratch_glbce_out = NULL;

        if(prms->hGlbce != NULL)
        {
            retval = glbce_delete_handle(prms->hGlbce);

            if(retval < 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "glbce_delete_handle returned failure\n");
            }
        }

        if(NULL != prms->rawfe_params.lsc.gain_table)
        {
            free(prms->rawfe_params.lsc.gain_table);
        }

        tivxMemFree(prms, sizeof(tivxVpacVissParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static void tivxVpacVissCopyShift(uint16_t src[], uint16_t dst[], int32_t size, uint16_t shift_policy)
{
    int32_t i;

    if (NO_SHIFT == shift_policy)
    {
        memcpy(dst, src, size);
    }
    else if (UP_SHIFT == shift_policy)
    {
        for(i=0; i < (size/2); i++)
        {
            dst[i] = src[i] << 4;
        }
    }
    else if (DOWN_SHIFT == shift_policy)
    {
        for(i=0; i < (size/2); i++)
        {
            dst[i] = src[i] >> 4;
        }
    }
}

static vx_status tivxVpacVissConfigSimDataPath(tivxVpacVissParams *prms, tivx_vpac_viss_params_t *vissPrms,
                                               tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_image_t *y12_desc;
    tivx_obj_desc_image_t *uv12_c1_desc;
    tivx_obj_desc_image_t *y8_r8_c2_desc;
    tivx_obj_desc_image_t *uv8_g8_c3_desc;
    tivx_obj_desc_image_t *s8_b8_c4_desc;
    tivx_obj_desc_distribution_t *histogram0_desc;
    tivx_obj_desc_distribution_t *histogram1_desc;
    tivx_obj_desc_user_data_object_t *h3a_out_desc;

    if((NULL != prms) && (NULL != vissPrms))
    {
        uint32_t i,k;

        y12_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT0_IDX];
        uv12_c1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT1_IDX];
        y8_r8_c2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT2_IDX];
        uv8_g8_c3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT3_IDX];
        s8_b8_c4_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT4_IDX];
        histogram0_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX];
        histogram1_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM1_IDX];
        h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

        /*
         * FCP Instance
         * ------------
         *
         *          |-------------------->|
         *          |        |----------->|--[5]->
         *   FLEXCFA -> FLEXCC -> EE ---->|
         *                   |
         *                   '--> HIST-->
         *
         *
         * VPAC_VISS1
         * ----------
         *
         * RAWFE -> NSF4 -> GLBCE -> FCP[0]--[5]->
         *     '--> H3A --->            '--> HIST -->
         *
         *
         * VPAC_VISS3
         * ----------
         *
         *     |-------------------------------->|        |---------> HIST[1]
         *     |       |------------------------>|        |
         *     |       |            |----------->|---> FCP[1]--[5]-> |
         *     |       |            |        |-->|                   |--[5]->
         * RAWFE -> CAC -> NSF4+DWB -> GLBCE --------> FCP[0]--[5]-> |
         *     |       '---> RawHist ---->                |
         *     '--> H3A --->                              '---------> HIST[0]
         *
         */

        /* RAWFE */

        /* H3A */
        if( h3a_out_desc != NULL)
        {
            int32_t num_aew_windows, ae_size, af_pad;
            uint32_t max_h3a_out_buffer_size;
            int16_t temp_num_aew_windows;

            prms->h3a_in.image_width = prms->width;
            prms->h3a_in.image_height = prms->height;
            prms->h3a_in.image_data = (int16_t*)prms->scratch_rawfe_h3a_out;

            prms->aew_config.aewwin1_WINH = prms->h3a_params.aewwin1_WINH;
            prms->aew_config.aewwin1_WINW = prms->h3a_params.aewwin1_WINW;
            prms->aew_config.aewwin1_WINVC = prms->h3a_params.aewwin1_WINVC;
            prms->aew_config.aewwin1_WINHC = prms->h3a_params.aewwin1_WINHC;
            prms->aew_config.aewsubwin_AEWINCV = prms->h3a_params.aewsubwin_AEWINCV;
            prms->aew_config.aewsubwin_AEWINCH = prms->h3a_params.aewsubwin_AEWINCH;

            /* Compute AEW buffer size */
            temp_num_aew_windows = (prms->h3a_params.aewwin1_WINVC+1)*(prms->h3a_params.aewwin1_WINHC);
            num_aew_windows = (int32_t)temp_num_aew_windows;
            ae_size = 0;
            for (i = 0; i < num_aew_windows; i++)
            {
                if ( (prms->h3a_params.aew_cfg_AEFMT == 0) || (prms->h3a_params.aew_cfg_AEFMT == 1))
                {
                    ae_size += 8;
                }
                else
                {
                    ae_size += 4;
                }
                if (((i%8) == 7) && (i > 0))
                {
                    ae_size += 4;
                }
                if (((i%prms->h3a_params.aewwin1_WINHC) == (prms->h3a_params.aewwin1_WINHC -1)) && ((ae_size%8) == 4) && (i != (num_aew_windows-1)))
                {
                    ae_size += 4;
                }
            }
            if ((num_aew_windows%8) != 0)
            {
                ae_size += 4;
            }
            if ((ae_size%8) != 0)
            {
                ae_size+= 4;
            }
            prms->aew_buffer_size = ae_size * sizeof(uint32_t);

            /* Compute AF buffer size */
            if (prms->h3a_params.pcr_AF_VF_EN)
            {
                int16_t temp_af_buffer_size = ((int16_t)16 * (prms->h3a_params.afpax2_PAXHC * prms->h3a_params.afpax2_PAXVC)) * (int16_t)sizeof(uint32_t);
                prms->af_buffer_size = (uint32_t)temp_af_buffer_size;
            }
            else
            {
                int16_t temp0 = prms->h3a_params.afpax2_PAXHC * prms->h3a_params.afpax2_PAXVC;
                int32_t temp1;
                af_pad = 0;
                if ((int16_t)1 == (prms->h3a_params.afpax2_PAXHC%2))
                {
                    int16_t temp_af_pad = 4*prms->h3a_params.afpax2_PAXVC;
                    af_pad = (int32_t)temp_af_pad;
                }
                temp1 = ((12 * (int32_t)temp0) + af_pad) * (int32_t)sizeof(uint32_t);
                prms->af_buffer_size = (uint32_t)temp1;
            }

            /* H3A can operate in AF or AEWB mode. */
            /* Therefore total memory needed is the max(aew_buffer_size, af_buffer_size) */
            max_h3a_out_buffer_size = ((prms->aew_buffer_size > prms->af_buffer_size) ?
                                       prms->aew_buffer_size : prms->af_buffer_size);
            if(max_h3a_out_buffer_size > TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES)
            {
                VX_PRINT(VX_ZONE_ERROR, "Required H3A output buffer size (%d bytes) is greater than TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES (%d bytes)\n",
                                         max_h3a_out_buffer_size, TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES);
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if (((vx_status)VX_SUCCESS == status) && (0U != prms->aew_buffer_size))
            {
                prms->scratch_aew_result = tivxMemAlloc(prms->aew_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->scratch_aew_result)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
            else
            {
                prms->scratch_aew_result = NULL;
            }

            if (((vx_status)VX_SUCCESS == status) && (0U != prms->af_buffer_size))
            {
                prms->scratch_af_result = tivxMemAlloc(prms->af_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->scratch_af_result)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
            else
            {
                prms->scratch_af_result = NULL;
            }
        }

        /* CAC */
        prms->pScratch_cac_out = prms->scratch_rawfe_raw_out;
#ifdef VPAC3
        if(0 == vissPrms->bypass_cac)
        {
            prms->pScratch_cac_out = prms->scratch_cac_out;
        }
        else
#endif
        {
            prms->bypass_cac = 1;
        }

        /* NSF4 */
        prms->pScratch_nsf4v_out = prms->pScratch_cac_out;
        if(0 == vissPrms->bypass_nsf4)
        {
            prms->pScratch_nsf4v_out = prms->scratch_nsf4v_out;
        }
        else
        {
            prms->bypass_nsf4 = 1;
        }

        /* DWB */
        prms->pScratch_dwb_out = prms->pScratch_nsf4v_out;
#ifdef VPAC3
        if((0 == vissPrms->bypass_dwb) && (1 == prms->dwb_params.enable))
        {
            prms->pScratch_dwb_out = prms->scratch_dwb_out;
        }
        else
#endif
        {
            prms->bypass_dwb = 1;
        }

        /* GLBCE */
        prms->pScratch_glbce_out = prms->pScratch_dwb_out;
        if(0 == vissPrms->bypass_glbce)
        {
            int retval;
            prms->pScratch_glbce_out = prms->scratch_glbce_out;
            retval = glbce_get_handle(&prms->glbce_params, &prms->hGlbce);

            if(retval < 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "glbce_get_handle returned failure\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            prms->bypass_glbce = 1;
        }

        /* FLEXCFA */
        prms->fcp[0].pScratch_cfa_in_16 = prms->pScratch_glbce_out;
#ifdef VPAC3
        prms->fcp[1].pScratch_cfa_in_16 = prms->pScratch_glbce_out;

        if(TIVX_VPAC_VISS_FCP1_INPUT_RFE == vissPrms->fcp1_config)
        {
            prms->fcp[1].pScratch_cfa_in_16 = prms->scratch_rawfe_raw_out;
        }
        else if (TIVX_VPAC_VISS_FCP1_INPUT_CAC == vissPrms->fcp1_config)
        {
            prms->fcp[1].pScratch_cfa_in_16 = prms->pScratch_cac_out;
        }
        else if (TIVX_VPAC_VISS_FCP1_INPUT_NSF4 == vissPrms->fcp1_config)
        {
            prms->fcp[1].pScratch_cfa_in_16 = prms->pScratch_nsf4v_out;
        }
#endif
        for(k=0; k<TIVX_VPAC_VISS_FCP_NUM_INSTANCES; k++)
        {
            /* FLEXCC */
            if((( NULL != y12_desc )       && (k == (vissPrms->output_fcp_mapping[0] & 1)) &&
                    ((0 == (vissPrms->output_fcp_mapping[0] & 2)) ||
                    ((2 == (vissPrms->output_fcp_mapping[0] & 2)) && (TIVX_VPAC_VISS_MUX2_C2 != vissPrms->fcp[k].mux_output2)))) ||
               (( NULL != uv12_c1_desc )   && (k == (vissPrms->output_fcp_mapping[1] & 1)) &&
                   (((0 == (vissPrms->output_fcp_mapping[1] & 2)) && (TIVX_VPAC_VISS_MUX1_C1 != vissPrms->fcp[k].mux_output1)) ||
                    ((2 == (vissPrms->output_fcp_mapping[1] & 2)) && (TIVX_VPAC_VISS_MUX3_C3 != vissPrms->fcp[k].mux_output3)))) ||
               (( NULL != y8_r8_c2_desc )  && (k == (vissPrms->output_fcp_mapping[2] & 1)) &&
                    ((0 == (vissPrms->output_fcp_mapping[2] & 2)) ||
                    ((2 == (vissPrms->output_fcp_mapping[2] & 2)) && (TIVX_VPAC_VISS_MUX2_C2 != vissPrms->fcp[k].mux_output2)))) ||
               (( NULL != uv8_g8_c3_desc ) && (k == (vissPrms->output_fcp_mapping[3] & 1)) &&
                   (((0 == (vissPrms->output_fcp_mapping[3] & 2)) && (TIVX_VPAC_VISS_MUX1_C1 != vissPrms->fcp[k].mux_output1)) ||
                    ((2 == (vissPrms->output_fcp_mapping[3] & 2)) && (TIVX_VPAC_VISS_MUX3_C3 != vissPrms->fcp[k].mux_output3)))) ||
               (( NULL != s8_b8_c4_desc )  && (k == (vissPrms->output_fcp_mapping[4] & 1)) &&
                                                                     (TIVX_VPAC_VISS_MUX4_C4 != vissPrms->fcp[k].mux_output4)) ||
               (( NULL != histogram0_desc) && (k == 0)) ||
               (( NULL != histogram1_desc) && (k == 1))
              )
            {
                if( (( NULL != y12_desc ) && (k == (vissPrms->output_fcp_mapping[0] & 1)) &&
                     ((0 == (vissPrms->output_fcp_mapping[0] & 2) && (TIVX_VPAC_VISS_MUX0_NV12_P12 == vissPrms->fcp[k].mux_output0)) ||
                      (2 == (vissPrms->output_fcp_mapping[0] & 2) && (TIVX_VPAC_VISS_MUX2_NV12 == vissPrms->fcp[k].mux_output2))))   ||
                    (( NULL != y8_r8_c2_desc ) && (k == (vissPrms->output_fcp_mapping[2] & 1)) &&
                     ((0 == (vissPrms->output_fcp_mapping[2] & 2) && (TIVX_VPAC_VISS_MUX0_NV12_P12 == vissPrms->fcp[k].mux_output0)) ||
                      (2 == (vissPrms->output_fcp_mapping[2] & 2) && (TIVX_VPAC_VISS_MUX2_NV12 == vissPrms->fcp[k].mux_output2)))))
                {
                    prms->flexcc_params[k].ChromaMode = TIVX_VPAC_VISS_CHROMA_MODE_420; /* NV12 format is chosen on at least 1 output port */
                }
                else if ((( NULL != y12_desc ) && (k == (vissPrms->output_fcp_mapping[0] & 1)) &&
                         (2 == (vissPrms->output_fcp_mapping[0] & 2) && (TIVX_VPAC_VISS_MUX2_YUV422 == vissPrms->fcp[k].mux_output2)))   ||
                        (( NULL != y8_r8_c2_desc ) && (k == (vissPrms->output_fcp_mapping[2] & 1)) &&
                         (2 == (vissPrms->output_fcp_mapping[2] & 2) && (TIVX_VPAC_VISS_MUX2_YUV422 == vissPrms->fcp[k].mux_output2))))
                {
                    prms->flexcc_params[k].ChromaMode = TIVX_VPAC_VISS_CHROMA_MODE_422; /* YUV422 interleaved is chosen */
                }
                else
                {
                    prms->flexcc_params[k].ChromaMode = vissPrms->fcp[k].chroma_mode;
                }
            }
            else
            {
                prms->fcp[k].bypass_cc = 1;
            }

            /* EE */
            if(0 != vissPrms->fcp[k].ee_mode)
            {
                uint32_t ee_mode_status[2] = {0};

                if(1 == vissPrms->fcp[k].ee_mode)
                {
                    ee_mode_status[0] = 1;
                }

                if ( (2 == vissPrms->fcp[k].ee_mode) &&
                    ((TIVX_VPAC_VISS_MUX2_Y8     == vissPrms->fcp[k].mux_output2) ||
                     (TIVX_VPAC_VISS_MUX2_NV12   == vissPrms->fcp[k].mux_output2) ||
                     (TIVX_VPAC_VISS_MUX2_YUV422 == vissPrms->fcp[k].mux_output2)))
                {
                    ee_mode_status[1] = 1;
                }


                if((( NULL != y12_desc )       && (k == (vissPrms->output_fcp_mapping[0] & 1)) &&
                       (((0 == (vissPrms->output_fcp_mapping[0] & 2)) && (1 == ee_mode_status[0])) ||
                        ((2 == (vissPrms->output_fcp_mapping[0] & 2)) && (1 == ee_mode_status[1]))))  ||
                   (( NULL != y8_r8_c2_desc )  && (k == (vissPrms->output_fcp_mapping[2] & 1)) &&
                       (((0 == (vissPrms->output_fcp_mapping[2] & 2)) && (1 == ee_mode_status[0])) ||
                        ((2 == (vissPrms->output_fcp_mapping[2] & 2)) && (1 == ee_mode_status[1])))))
                {
                    prms->fcp[k].bypass_ee = 0;
                }
                else
                {
                    prms->fcp[k].bypass_ee = 1;
                }
            }
            else
            {
                prms->fcp[k].bypass_ee = 1;
            }
        }

        /* Configure FCP output muxes */

        for(k=0; k<TIVX_VPAC_VISS_FCP_NUM_INSTANCES; k++)
        {
            uint32_t output_0_dual_channel = 0;
            uint32_t output_2_dual_channel = 0;

            /* y12 output [0] */

            if(((0 == prms->fcp[k].bypass_ee) && (vissPrms->fcp[k].ee_mode == TIVX_VPAC_VISS_EE_MODE_Y12)) &&
                (TIVX_VPAC_VISS_MUX0_VALUE12 != vissPrms->fcp[k].mux_output0))
            {
                prms->fcp[k].scratch_ee_shift_in = prms->fcp[k].scratch_cc_out[0]; /* y12 */
                prms->fcp[k].out_y12_16 = prms->fcp[k].scratch_ee_shift_out;       /* y12 */
                prms->fcp[k].pre_copy = Y12_PRE_COPY;
                prms->fcp[k].post_copy = Y12_POST_COPY;
            }
            else
            {
                prms->fcp[k].out_y12_16 = prms->fcp[k].scratch_cc_out[0];    /* y12 */
            }

            if (TIVX_VPAC_VISS_MUX0_NV12_P12 == vissPrms->fcp[k].mux_output0)  /* NV12_P12 Format */
            {
                prms->fcp[k].out_uv12_c1_16 = prms->fcp[k].scratch_cc_out[1]; /* uv12 */
                output_0_dual_channel = 1U;
            }

            if (TIVX_VPAC_VISS_MUX0_VALUE12 == vissPrms->fcp[k].mux_output0)  /* Value Format */
            {
                prms->flexcc_params[k].MuxY12Out = 2; /* HSV value */
            }
            else
            {
                prms->flexcc_params[k].MuxY12Out = 1; /* YUV Luma */
            }

            /* uv12_c1 output [1] */

            if(0 == output_0_dual_channel)
            {
                if(vissPrms->fcp[k].mux_output1 == TIVX_VPAC_VISS_MUX1_UV12)
                {
                    prms->fcp[k].out_uv12_c1_16 = prms->fcp[k].scratch_cc_out[1]; /* uv12 */
                }
                else
                {
                    prms->fcp[k].out_uv12_c1_16 = prms->fcp[k].scratch_cfa_out[0]; /* c1 */
                }
            }

            /* y8_r8_c2 output [2] */

            prms->fcp[k].out_y8_r8_c2_bit_align = 8;
            if(vissPrms->fcp[k].mux_output2 == TIVX_VPAC_VISS_MUX2_Y8)
            {
                if((0 == prms->fcp[k].bypass_ee) && (vissPrms->fcp[k].ee_mode == TIVX_VPAC_VISS_EE_MODE_Y8))
                {
                    prms->fcp[k].scratch_ee_shift_in = prms->fcp[k].scratch_cc_out[2]; /* y8 */
                    prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_ee_shift_out;  /* y8 */
                    prms->fcp[k].pre_copy = Y8_PRE_COPY;
                    prms->fcp[k].post_copy = Y8_POST_COPY;
                }
                else
                {
                    prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_cc_out[2];  /* y8 */
                }
            }
            else if (vissPrms->fcp[k].mux_output2 == TIVX_VPAC_VISS_MUX2_RED)
            {
                prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_cc_out[5]; /* r8 */
            }
            else if (vissPrms->fcp[k].mux_output2 == TIVX_VPAC_VISS_MUX2_C2)
            {
                prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_cfa_out[1]; /* c2 */
                prms->fcp[k].out_y8_r8_c2_bit_align = 12;
            }
            else if (vissPrms->fcp[k].mux_output2 == TIVX_VPAC_VISS_MUX2_VALUE8)
            {
                prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_cc_out[2];  /* y8 (value) */
            }
            else if((vissPrms->fcp[k].mux_output2 == TIVX_VPAC_VISS_MUX2_NV12) || (vissPrms->fcp[k].mux_output2 == TIVX_VPAC_VISS_MUX2_YUV422))
            {
                output_2_dual_channel = 1U;

                if((0 == prms->fcp[k].bypass_ee) && (vissPrms->fcp[k].ee_mode == TIVX_VPAC_VISS_EE_MODE_Y8))
                {
                    prms->fcp[k].scratch_ee_shift_in = prms->fcp[k].scratch_cc_out[2]; /* y8 */
                    prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_ee_shift_out;  /* y8 */
                    prms->fcp[k].pre_copy = Y8_PRE_COPY;
                    prms->fcp[k].post_copy = Y8_POST_COPY;
                }
                else
                {
                    prms->fcp[k].out_y8_r8_c2_16 = prms->fcp[k].scratch_cc_out[2];  /* y8 */
                }

                prms->fcp[k].out_uv8_g8_c3_bit_align = 8;
                prms->fcp[k].out_uv8_g8_c3_16 = prms->fcp[k].scratch_cc_out[3]; /* UV8 */
            }
            else
            {
                /* Not valid input */
            }

            if (TIVX_VPAC_VISS_MUX2_VALUE8 == vissPrms->fcp[k].mux_output2)  /* Value Format */
            {
                prms->flexcc_params[k].MuxY8Out = 2; /* HSV value */
            }
            else
            {
                prms->flexcc_params[k].MuxY8Out = 1; /* YUV Luma */
            }

            /* uv8_g8_c3 output */

            if(0 == output_2_dual_channel)
            {
                prms->fcp[k].out_uv8_g8_c3_bit_align = 8;
                if(vissPrms->fcp[k].mux_output3 == TIVX_VPAC_VISS_MUX3_UV8)
                {
                    prms->fcp[k].out_uv8_g8_c3_16 = prms->fcp[k].scratch_cc_out[3]; /* uv8 */
                }
                else if (vissPrms->fcp[k].mux_output3 == TIVX_VPAC_VISS_MUX3_GREEN)
                {
                    prms->fcp[k].out_uv8_g8_c3_16 = prms->fcp[k].scratch_cc_out[6]; /* g8 */
                }
                else
                {
                    prms->fcp[k].out_uv8_g8_c3_16 = prms->fcp[k].scratch_cfa_out[2]; /* c3 */
                    prms->fcp[k].out_uv8_g8_c3_bit_align = 12;
                }
            }

            /* s8_b8_c4 output */

            prms->fcp[k].out_s8_b8_c4_bit_align = 8;
            if(vissPrms->fcp[k].mux_output4 == TIVX_VPAC_VISS_MUX4_SAT)
            {
                prms->fcp[k].out_s8_b8_c4_16 = prms->fcp[k].scratch_cc_out[4]; /* s8 */
            }
            else if (vissPrms->fcp[k].mux_output4 == TIVX_VPAC_VISS_MUX4_BLUE)
            {
                prms->fcp[k].out_s8_b8_c4_16 = prms->fcp[k].scratch_cc_out[7]; /* b8 */
            }
            else
            {
                prms->fcp[k].out_s8_b8_c4_16 = prms->fcp[k].scratch_cfa_out[3]; /* c4 */
                prms->fcp[k].out_s8_b8_c4_bit_align = 12;
            }
        }

        /* Configure final output muxes */

        if( NULL != y12_desc )
        {
            k = vissPrms->output_fcp_mapping[0] & 1U;

            if(0 == (vissPrms->output_fcp_mapping[0] & 2)) /* [0] */
            {
                prms->out_0_16 = prms->fcp[k].out_y12_16;
                prms->out_0_align = 12U;

                if (TIVX_VPAC_VISS_MUX0_NV12_P12 == vissPrms->fcp[k].mux_output0)  /* NV12_P12 Format */
                {
                    prms->out_1_16 = prms->fcp[k].out_uv12_c1_16; /* uv12 */
                }
            }
            else /* [2] */
            {
                prms->out_0_16 = prms->fcp[k].out_y8_r8_c2_16;
                prms->out_0_align = prms->fcp[k].out_y8_r8_c2_bit_align;

                if ((TIVX_VPAC_VISS_MUX2_NV12 == vissPrms->fcp[k].mux_output2) ||    /* NV12 Format */
                    (TIVX_VPAC_VISS_MUX2_YUV422 == vissPrms->fcp[k].mux_output2) )   /* YUV422 Format */
                {
                    prms->out_1_16 = prms->fcp[k].out_uv8_g8_c3_16; /* uv8 */
                }
            }

        }

        if( NULL != uv12_c1_desc )
        {
            k = vissPrms->output_fcp_mapping[1] & 1U;

            if(0 == (vissPrms->output_fcp_mapping[1] & 2)) /* [1] */
            {
                prms->out_1_16 = prms->fcp[k].out_uv12_c1_16;
                prms->out_1_align = 12U;
            }
            else /* [3] */
            {
                prms->out_1_16 = prms->fcp[k].out_uv8_g8_c3_16;
                prms->out_1_align = prms->fcp[k].out_uv8_g8_c3_bit_align;
            }
        }

        if( NULL != y8_r8_c2_desc )
        {
            k = vissPrms->output_fcp_mapping[2] & 1U;

            if(0 == (vissPrms->output_fcp_mapping[2] & 2))  /* [0] */
            {
                prms->out_2_16 = prms->fcp[k].out_y12_16;
                prms->out_2_align = 12U;

                if (TIVX_VPAC_VISS_MUX0_NV12_P12 == vissPrms->fcp[k].mux_output0)  /* NV12_P12 Format */
                {
                    prms->out_3_16 = prms->fcp[k].out_uv12_c1_16; /* uv12 */
                }
            }
            else /* [2] */
            {
                prms->out_2_16 = prms->fcp[k].out_y8_r8_c2_16;
                prms->out_2_align = prms->fcp[k].out_y8_r8_c2_bit_align;

                if ((TIVX_VPAC_VISS_MUX2_NV12 == vissPrms->fcp[k].mux_output2) ||    /* NV12 Format */
                    (TIVX_VPAC_VISS_MUX2_YUV422 == vissPrms->fcp[k].mux_output2) )   /* YUV422 Format */
                {
                    prms->out_3_16 = prms->fcp[k].out_uv8_g8_c3_16; /* uv8 */
                }
            }
        }

        if( NULL != uv8_g8_c3_desc )
        {
            k = vissPrms->output_fcp_mapping[3] & 1U;

            if(0 == (vissPrms->output_fcp_mapping[3] & 2))  /* [1] */
            {
                prms->out_3_16 = prms->fcp[k].out_uv12_c1_16;
                prms->out_3_align = 12U;
            }
            else /* [3] */
            {
                prms->out_3_16 = prms->fcp[k].out_uv8_g8_c3_16;
                prms->out_3_align = prms->fcp[k].out_uv8_g8_c3_bit_align;
            }
        }

        if( NULL != s8_b8_c4_desc )
        {
            prms->out_4_16 = prms->fcp[vissPrms->output_fcp_mapping[4] & 1].out_s8_b8_c4_16;
            prms->out_4_align = prms->fcp[k].out_s8_b8_c4_bit_align;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}
