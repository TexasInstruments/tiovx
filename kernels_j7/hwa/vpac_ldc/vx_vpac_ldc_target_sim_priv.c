/*
 *
 * Copyright (c) 2021-2021 Texas Instruments Incorporated
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

#include "vx_vpac_ldc_target_sim_priv.h"

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacLdcSetConfigInSim(tivxVpacLdcParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != prms)
    {
        ldc_settings       *params = &prms->config.settings;
        Ldc_Config *ldc_cfg = (Ldc_Config *)&prms->ldcObj.ldc_cfg;

        params->pwarpen     = ldc_cfg->perspTrnsformCfg.enableWarp;
        params->affine_a    = ldc_cfg->perspTrnsformCfg.coeffA;
        params->affine_b    = ldc_cfg->perspTrnsformCfg.coeffB;
        params->affine_c    = ldc_cfg->perspTrnsformCfg.coeffC;
        params->affine_d    = ldc_cfg->perspTrnsformCfg.coeffD;
        params->affine_e    = ldc_cfg->perspTrnsformCfg.coeffE;
        params->affine_f    = ldc_cfg->perspTrnsformCfg.coeffF;
        params->affine_g    = ldc_cfg->perspTrnsformCfg.coeffG;
        params->affine_h    = ldc_cfg->perspTrnsformCfg.coeffH;

        params->regmode_en  = ldc_cfg->enableMultiRegions;
        params->ld_obw      = ldc_cfg->outputBlockWidth;
        params->ld_obh      = ldc_cfg->outputBlockHeight;
        params->ld_pad      = ldc_cfg->pixelPad;
        params->ld_initx    = ldc_cfg->outputStartX;
        params->ld_inity    = ldc_cfg->outputStartY;

        if (1U == ldc_cfg->enableMultiRegions)
        {
            int32_t cnt1, cnt2;
            for (cnt1 = 0; cnt1 < (int32_t)LDC_MAX_HORZ_REGIONS; cnt1 ++)
            {
                params->ld_sf_width[cnt1] = ldc_cfg->regCfg.width[cnt1];
            }

            for (cnt1 = 0; cnt1 < (int32_t)LDC_MAX_VERT_REGIONS; cnt1 ++)
            {
                params->ld_sf_height[cnt1] = ldc_cfg->regCfg.height[cnt1];
            }

            for (cnt1 = 0; cnt1 < (int32_t)LDC_MAX_VERT_REGIONS; cnt1 ++)
            {
                for (cnt2 = 0; cnt2 < (int32_t)LDC_MAX_HORZ_REGIONS; cnt2 ++)
                {
                    params->ld_sf_en[cnt1*LDC_MAX_HORZ_REGIONS+cnt2]  = ldc_cfg->regCfg.enable[cnt1][cnt2];
                    params->ld_sf_obw[cnt1*LDC_MAX_HORZ_REGIONS+cnt2] = ldc_cfg->regCfg.blockWidth[cnt1][cnt2];
                    params->ld_sf_obh[cnt1*LDC_MAX_HORZ_REGIONS+cnt2] = ldc_cfg->regCfg.blockHeight[cnt1][cnt2];
                    params->ld_sf_pad[cnt1*LDC_MAX_HORZ_REGIONS+cnt2] = ldc_cfg->regCfg.pixelPad[cnt1][cnt2];
                }
            }
        }

        if (0U != ldc_cfg->enableBackMapping)
        {
            Ldc_LutCfg *lut_cfg        = &ldc_cfg->lutCfg;
            int factor = 1<<lut_cfg->dsFactor;

            params->ldmapen          = (uint32_t)1;
            params->mesh_frame_w     = lut_cfg->width;
            params->mesh_frame_h     = lut_cfg->height;
            params->table_m          = lut_cfg->dsFactor;

            params->table_width      = ((lut_cfg->width+(factor-1))>>lut_cfg->dsFactor) + 1;
            params->table_height     = ((lut_cfg->height+(factor-1))>>lut_cfg->dsFactor) + 1;

            prms->mesh_buffer_size = params->table_width * params->table_height * 4;
            prms->mesh = tivxMemAlloc(prms->mesh_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->mesh)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
            else
            {
                int32_t j;
                int32_t *meshPtr = (int32_t *)lut_cfg->address;

                for(j = 0; j < params->table_height; j++) {
                    uint32_t input_index = (lut_cfg->lineOffset>>2)*j;
                    uint32_t output_index = params->table_width*j;
                    xyMeshSwapCpy((uint32_t*)&prms->mesh[output_index], (uint32_t*)&meshPtr[input_index],  params->table_width);
                }
            }
            params->mesh_table = (int *)prms->mesh;
        }
    }
    else
    {
        status = VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }

    return (status);
}

void xyMeshSwapCpy(uint32_t *output, uint32_t *input, uint32_t num_points)
{
    uint32_t i, in_val, out_val;

    for(i=0; i<num_points; i++)
    {
        in_val = input[i];
        out_val = ((in_val & 0xFFFF) << 16) | ((in_val >> 16) & 0xFFFF);
        output[i] = out_val;
    }

    return;
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */
