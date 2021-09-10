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

#include <vx_vpac_ldc_target_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacLdcSetParamsFromDcc(
    const tivxVpacLdcObj                   *ldc_obj,
    const tivx_obj_desc_user_data_object_t *dcc_buf_desc)
{
    Ldc_Config *ldc_cfg = (Ldc_Config *)&ldc_obj->ldc_cfg;
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != dcc_buf_desc)
    {
        dcc_parser_output_params_t *pout = tivxMemAlloc(sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL == pout)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate DCC parser output buffer \n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            vpac_ldc_dcc_cfg_t    *pcfg   = &pout->vpacLdcCfg;
            vpac_ldc_dcc_params_t *params = &pcfg->ldc_dcc_params;

            void *target_ptr_dcc;
            target_ptr_dcc = tivxMemShared2TargetPtr(&dcc_buf_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr_dcc, dcc_buf_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            uint8_t * dcc_ldc_buf = (uint8_t *)target_ptr_dcc;
            int32_t dcc_buf_size = (int32_t)dcc_buf_desc->mem_size;

            dcc_parser_input_params_t parser_input = {
                dcc_ldc_buf,
                (uint32_t)dcc_buf_size,
                0,
                0,
                0,
                ldc_obj->sensor_dcc_id
            };

            pout->useVpacLdcCfg = 0;
            dcc_update(&parser_input, pout);

            if (1U == pout->useVpacLdcCfg)
            {
                ldc_cfg->perspTrnsformCfg.enableWarp = params->pwarpen;
                ldc_cfg->perspTrnsformCfg.coeffA     = params->affine_a;
                ldc_cfg->perspTrnsformCfg.coeffB     = params->affine_b;
                ldc_cfg->perspTrnsformCfg.coeffC     = params->affine_c;
                ldc_cfg->perspTrnsformCfg.coeffD     = params->affine_d;
                ldc_cfg->perspTrnsformCfg.coeffE     = params->affine_e;
                ldc_cfg->perspTrnsformCfg.coeffF     = params->affine_f;
                ldc_cfg->perspTrnsformCfg.coeffG     = params->affine_g;
                ldc_cfg->perspTrnsformCfg.coeffH     = params->affine_h;

                ldc_cfg->enableMultiRegions          = params->regmode_en;
                ldc_cfg->outputBlockWidth            = params->ld_obw;
                ldc_cfg->outputBlockHeight           = params->ld_obh;
                ldc_cfg->pixelPad                    = params->ld_pad;
                ldc_cfg->outputStartX                = params->ld_initx;
                ldc_cfg->outputStartY                = params->ld_inity;

                if (1U == ldc_cfg->enableMultiRegions)
                {
                    int32_t cnt1, cnt2;
                    for (cnt1 = 0; cnt1 < (int32_t)LDC_MAX_HORZ_REGIONS; cnt1 ++)
                    {
                        ldc_cfg->regCfg.width[cnt1] = params->ld_sf_width[cnt1];
                    }

                    for (cnt1 = 0; cnt1 < (int32_t)LDC_MAX_VERT_REGIONS; cnt1 ++)
                    {
                        ldc_cfg->regCfg.height[cnt1] = params->ld_sf_height[cnt1];
                    }

                    for (cnt1 = 0; cnt1 < (int32_t)LDC_MAX_VERT_REGIONS; cnt1 ++)
                    {
                        for (cnt2 = 0; cnt2 < (int32_t)LDC_MAX_HORZ_REGIONS; cnt2 ++)
                        {
                            ldc_cfg->regCfg.enable[cnt1][cnt2]      = params->ld_sf_en[cnt1][cnt2];
                            ldc_cfg->regCfg.blockWidth[cnt1][cnt2]  = params->ld_sf_obw[cnt1][cnt2];
                            ldc_cfg->regCfg.blockHeight[cnt1][cnt2] = params->ld_sf_obh[cnt1][cnt2];
                            ldc_cfg->regCfg.pixelPad[cnt1][cnt2]    = params->ld_sf_pad[cnt1][cnt2];
                        }
                    }
                }

                if (0U != params->ldmapen)
                {
                    Ldc_LutCfg *lut_cfg        = &ldc_cfg->lutCfg;

                    ldc_cfg->enableBackMapping = (uint32_t)TRUE;
                    lut_cfg->width             = params->mesh_frame_w;
                    lut_cfg->height            = params->mesh_frame_h;
                    lut_cfg->dsFactor          = params->table_m;
                    lut_cfg->lineOffset        = params->mesh_table_pitch;

                    lut_cfg->address = (uint64_t)pcfg->mesh_table;
                }
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr_dcc, dcc_buf_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            tivxMemFree(pout, sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}
