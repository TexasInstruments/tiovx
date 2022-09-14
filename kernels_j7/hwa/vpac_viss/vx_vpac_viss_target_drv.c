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

#include <vx_vpac_viss_target_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status tivxVpacVissSetRfeConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacVissSetGlbceConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacVissSetNsf4Config(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
static vx_status tivxVpacVissSetFcpConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
#if defined(VPAC3L)
static vx_status tivxVpacVissSetPcidConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissSetConfigInDrv(tivxVpacVissObj *vissObj)
{
    vx_status status;

    status = tivxVpacVissSetRfeConfig(vissObj, &vissObj->vissCfgRef);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetGlbceConfig(vissObj, &vissObj->vissCfgRef);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetNsf4Config(vissObj, &vissObj->vissCfgRef);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetFcpConfig(vissObj, &vissObj->vissCfgRef);
    }

#if defined(VPAC3L)
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetPcidConfig(vissObj, &vissObj->vissCfgRef);
    }
#endif

    return (status);
}

vx_status tivxVpacVissSetConfigBuffer(tivxVpacVissObj *vissObj)
{
    vx_status status =      (vx_status) VX_SUCCESS;

#ifndef SOC_AM62A
    int32_t fvid2_status =  FVID2_SOK;

    /* Initialize buffer pointer to NULL */
    vissObj->configurationBuffer.bufferPtr = NULL;
    /* Initialize length to zero */
    vissObj->configurationBuffer.length = 0;
    /* Initialize configThroughUDMA to false */
    vissObj->configurationBuffer.configThroughUdmaFlag = false;

    /* Get configuration buffer information */
    fvid2_status = Fvid2_control(vissObj->handle,
    IOCTL_VHWA_M2M_VISS_GET_BUFF_INFO, (void*) &vissObj->configurationBuffer,
            NULL);

    if (FVID2_SOK != fvid2_status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get config buff info from driver\n");
    }

    if (FVID2_SOK == fvid2_status)
    {
        /* configThroughUdmaFlag is true, allocate config buffer */
        if (true == vissObj->configurationBuffer.configThroughUdmaFlag)
        {
            /* allocate buffer for the length requested */
            vissObj->configurationBuffer.bufferPtr = (uint32_t*) tivxMemAlloc(
                    vissObj->configurationBuffer.length,
                    (vx_enum) TIVX_MEM_EXTERNAL);
            if (NULL == vissObj->configurationBuffer.bufferPtr)
            {
                VX_PRINT(VX_ZONE_ERROR, "failed to allocate %d bytes !!!\n",
                        vissObj->configurationBuffer.length);
                fvid2_status = FVID2_EALLOC;
            }
            else
            {
                /* Set configuration buffer information */
                fvid2_status = Fvid2_control(vissObj->handle,
                IOCTL_VHWA_M2M_VISS_SET_BUFF_INFO,
                        (void*) &vissObj->configurationBuffer, NULL);

                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                            "Failed to set config buff info into driver\n");
                }
            }
        }
    }
    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status) VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = (vx_status) VX_SUCCESS;
    }

#endif

    return (status);
}

void tivxVpacVissDeleteConfigBuffer(tivxVpacVissObj *vissObj)
{
#ifndef SOC_AM62A
    if (NULL != vissObj->configurationBuffer.bufferPtr)
    {
        tivxMemFree((void*) vissObj->configurationBuffer.bufferPtr,
                vissObj->configurationBuffer.length,
                (vx_enum) TIVX_MEM_EXTERNAL);
    }
#endif
}

static vx_status tivxVpacVissSetRfeConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Rfe_Control         rfeCtrl;

    if (NULL != vissCfgRef->lPwlCfg)
    {
        /* PWL for Long Input */
        rfeCtrl.module  = RFE_MODULE_PWL1;
        rfeCtrl.pwl1Cfg = vissCfgRef->lPwlCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL1 Config !!!\n");
        }

        /* Reset Pointer */
        vissCfgRef->lPwlCfg = NULL;
    }

    if ((NULL != vissCfgRef->sPwlCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL for Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL2;
        rfeCtrl.pwl2Cfg = vissCfgRef->sPwlCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL2 Config !!!\n");
        }

        /* Reset Pointer */
        vissCfgRef->sPwlCfg = NULL;
    }

    if ((NULL != vissCfgRef->vsPwlCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL for Very Short Input */
        rfeCtrl.module  = RFE_MODULE_PWL3;
        rfeCtrl.pwl3Cfg = vissCfgRef->vsPwlCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL3 Config !!!\n");
        }

        vissCfgRef->vsPwlCfg = NULL;
    }

    if ((NULL != vissCfgRef->lLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL Lut for Long Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT1;
        rfeCtrl.decomp1Cfg      = vissCfgRef->lLutCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL1_LUT Config !!!\n");
        }

        vissCfgRef->lLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->sLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL Lut for Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT2;
        rfeCtrl.decomp2Cfg      = vissCfgRef->sLutCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL2_LUT Config !!!\n");
        }

        vissCfgRef->sLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->vsLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* PWL Lut for Very Short Input */
        rfeCtrl.module          = RFE_MODULE_DECOMP_LUT3;
        rfeCtrl.decomp3Cfg      = vissCfgRef->vsLutCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PWL3_LUT Config !!!\n");
        }

        vissCfgRef->vsLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->wdr1Cfg) && (FVID2_SOK == fvid2_status))
    {
        /* WDR Merge 1 block configuration */
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA1;
        rfeCtrl.wdrMergeMa1  = vissCfgRef->wdr1Cfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set WDR_MERGE1 Config !!!\n");
        }

        vissCfgRef->wdr1Cfg = NULL;
    }

    if ((NULL != vissCfgRef->wdr2Cfg) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_WDR_MERGE_MA2;
        rfeCtrl.wdrMergeMa2  = vissCfgRef->wdr2Cfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set WDR_MERGE2 Config !!!\n");
        }

        vissCfgRef->wdr2Cfg = NULL;
    }

    if ((NULL != vissCfgRef->comp20To16LutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* Set Companding Lut to convert from 20 to 16bits */
        rfeCtrl.module       = RFE_MODULE_COMP_LUT;
        rfeCtrl.compCfg      = vissCfgRef->comp20To16LutCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set COMP_LUT Config !!!\n");
        }

        vissCfgRef->comp20To16LutCfg = NULL;
    }

    if ((NULL != vissCfgRef->dpcOtf) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_DPC_OTF;
        rfeCtrl.dpcOtfCfg    = vissCfgRef->dpcOtf;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set DPC_OTF Config !!!\n");
        }

        vissCfgRef->dpcOtf = NULL;
    }

    if ((NULL != vissCfgRef->dpcLut) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_DPC_LUT;
        rfeCtrl.dpcLutCfg    = vissCfgRef->dpcLut;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);

        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set DPC_LUT Config !!!\n");
        }

        vissCfgRef->dpcLut = NULL;
    }

    if ((NULL != vissCfgRef->lscCfg) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module      = RFE_MODULE_LSC;
        rfeCtrl.lscConfig   = vissCfgRef->lscCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set LSC Config !!!\n");
        }

        vissCfgRef->lscCfg = NULL;
    }

    if ((NULL != vissCfgRef->wbCfg) && (FVID2_SOK == fvid2_status))
    {
        rfeCtrl.module       = RFE_MODULE_GAIN_OFST;
        rfeCtrl.wbConfig     = vissCfgRef->wbCfg;
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set WhiteBalance Config !!!\n");
        }

        vissCfgRef->wbCfg = NULL;
    }

    if ((NULL != vissCfgRef->rfeH3aInCfg) && (FVID2_SOK == fvid2_status))
    {
        /* H3A Input Selection and Lut configuration */
        rfeCtrl.module   = RFE_MODULE_H3A;
        rfeCtrl.h3aInCfg = vissCfgRef->rfeH3aInCfg;

        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set H3A Input Config !!!\n");
        }

        vissCfgRef->rfeH3aInCfg = NULL;
    }

    if ((NULL != vissCfgRef->h3aLutCfg) && (FVID2_SOK == fvid2_status))
    {
        /* H3A Input Selection and Lut configuration */
        rfeCtrl.module   = RFE_MODULE_H3A_LUT;
        rfeCtrl.h3aLutCfg = vissCfgRef->h3aLutCfg;

        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_RFE_SET_CONFIG,
            (void *)&rfeCtrl, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set H3A Input Config !!!\n");
        }

        vissCfgRef->h3aLutCfg = NULL;
    }

    if ((NULL != vissCfgRef->h3aCfg) && (FVID2_SOK == fvid2_status))
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_H3A_SET_CONFIG,
            (void *)vissCfgRef->h3aCfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set H3A Config !!!\n");
        }
        else
        {
            vissObj->h3a_output_size = vissCfgRef->h3aCfg->outputSize;
        }

        vissCfgRef->h3aCfg = NULL;
    }

    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacVissSetGlbceConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;
    Glbce_Control       glbceCtrl;

    vissDrvPrms = &vissObj->vissPrms;

    /* GLBCE Parameters can be set only if it is enabled at SET_PARAMS time */
    if ((uint32_t)TRUE == vissDrvPrms->enableGlbce)
    {
        if (NULL != vissCfgRef->glbceCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_GLBCE;
            glbceCtrl.glbceCfg = vissCfgRef->glbceCfg;
            fvid2_status = Fvid2_control(vissObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set GLBCE Config !!!\n");
            }

            vissCfgRef->glbceCfg = NULL;
        }

        if (NULL != vissCfgRef->fwdPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_FWD_PERCEPT;
            glbceCtrl.fwdPrcptCfg = vissCfgRef->fwdPrcpCfg;
            fvid2_status = Fvid2_control(vissObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set GLBCE_FWD_PERCEPT Config !!!\n");
            }

            vissCfgRef->fwdPrcpCfg = NULL;
        }

        if (NULL != vissCfgRef->revPrcpCfg)
        {
            glbceCtrl.module = GLBCE_MODULE_REV_PERCEPT;
            glbceCtrl.revPrcptCfg = vissCfgRef->revPrcpCfg;
            fvid2_status = Fvid2_control(vissObj->handle, IOCTL_GLBCE_SET_CONFIG,
                (void *)&glbceCtrl, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set GLBCE_REV_PERCEPT Config !!!\n");
            }

            vissCfgRef->revPrcpCfg = NULL;
        }
    }

    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacVissSetNsf4Config(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &vissObj->vissPrms;

    if (((uint32_t)TRUE == vissDrvPrms->enableNsf4) &&
        (NULL != vissCfgRef->nsf4Cfg))
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_NSF4_SET_CONFIG,
            (void *)vissCfgRef->nsf4Cfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set NFS4 Config !!!\n");
        }

        vissCfgRef->nsf4Cfg = NULL;
    }

    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

static vx_status tivxVpacVissSetFcpConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    int32_t             status = (vx_status)VX_SUCCESS;
    Fcp_Control         fcpCtrl;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &vissObj->vissPrms;

    if (NULL != vissCfgRef->fcpCfg[0].cfaLut16to12Cfg)
    {
        fcpCtrl.module = FCP_MODULE_COMPANDING;
        fcpCtrl.inComp = vissCfgRef->fcpCfg[0].cfaLut16to12Cfg;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set COMP Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].cfaLut16to12Cfg = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].cfaCfg)
    {
        fcpCtrl.module          = FCP_MODULE_CFA;
        fcpCtrl.cfa             = vissCfgRef->fcpCfg[0].cfaCfg;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set CFA Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].cfaCfg = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].ccm)
    {
        fcpCtrl.module      = FCP_MODULE_CCM;
        fcpCtrl.ccm         = vissCfgRef->fcpCfg[0].ccm;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set CCM Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].ccm = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].gamma)
    {
        fcpCtrl.module = FCP_MODULE_GAMMA;
        fcpCtrl.gamma  = vissCfgRef->fcpCfg[0].gamma;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Gamma Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].gamma = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].rgb2Hsv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2HSV;
        fcpCtrl.rgb2Hsv         = vissCfgRef->fcpCfg[0].rgb2Hsv;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set RGB2HSV Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].rgb2Hsv = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].rgb2yuv)
    {
        fcpCtrl.module          = FCP_MODULE_RGB2YUV;
        fcpCtrl.rgb2Yuv         = vissCfgRef->fcpCfg[0].rgb2yuv;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set RGB2YUV Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].rgb2yuv = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].yuvSatLutCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_YUV_SAT_LUT;
        fcpCtrl.yuvSatLut               = vissCfgRef->fcpCfg[0].yuvSatLutCfg;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set YUV_LUT Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].yuvSatLutCfg = NULL;
    }

    if (NULL != vissCfgRef->fcpCfg[0].histCfg)
    {
        fcpCtrl.module                  = FCP_MODULE_HISTOGRAM;
        fcpCtrl.hist                    = vissCfgRef->fcpCfg[0].histCfg;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Histogram Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].histCfg = NULL;
    }

    if (((VHWA_M2M_VISS_EE_ON_LUMA12 == vissDrvPrms->edgeEnhancerMode) ||
         (VHWA_M2M_VISS_EE_ON_LUMA8 == vissDrvPrms->edgeEnhancerMode)) &&
        (NULL != vissCfgRef->fcpCfg[0].eeCfg))
    {
        if (VHWA_M2M_VISS_EE_ON_LUMA12 == vissDrvPrms->edgeEnhancerMode)
        {
            vissCfgRef->fcpCfg[0].eeCfg->bypassY12 = FALSE;
            vissCfgRef->fcpCfg[0].eeCfg->bypassC12 = TRUE;
            vissCfgRef->fcpCfg[0].eeCfg->eeForY12OrY8 = 0u;

            if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_12B_IDX].fmt.dataFormat) ||
                 (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_12B_IDX].fmt.dataFormat) )
            {
                vissCfgRef->fcpCfg[0].eeCfg->alignY12withChroma = TRUE;
                vissCfgRef->fcpCfg[0].eeCfg->bypassC12 = FALSE;
            }
        }
        else
        {
            vissCfgRef->fcpCfg[0].eeCfg->bypassY12 = TRUE;
        }

        if (VHWA_M2M_VISS_EE_ON_LUMA8 == vissDrvPrms->edgeEnhancerMode)
        {
            vissCfgRef->fcpCfg[0].eeCfg->bypassY8 = FALSE;
            vissCfgRef->fcpCfg[0].eeCfg->bypassC8 = TRUE;
            vissCfgRef->fcpCfg[0].eeCfg->eeForY12OrY8 = 1u;
            vissCfgRef->fcpCfg[0].eeCfg->leftShift = 2u;
            vissCfgRef->fcpCfg[0].eeCfg->rightShift = 2u;

            if ( (FVID2_DF_YUV422I_UYVY == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_8B_IDX].fmt.dataFormat) ||
                 (FVID2_DF_YUV422I_YUYV == vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_YUV422_8B_IDX].fmt.dataFormat) )
            {
                vissCfgRef->fcpCfg[0].eeCfg->alignY8withChroma = TRUE;
                vissCfgRef->fcpCfg[0].eeCfg->bypassC8 = FALSE;
            }
        }
        else
        {
            vissCfgRef->fcpCfg[0].eeCfg->bypassY8 = TRUE;
        }

        fcpCtrl.module              = FCP_MODULE_EE;
        fcpCtrl.eeCfg               = vissCfgRef->fcpCfg[0].eeCfg;
        status = Fvid2_control(vissObj->handle, IOCTL_FCP_SET_CONFIG,
            (void *)&fcpCtrl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set YEE Config !!!\n");
        }

        vissCfgRef->fcpCfg[0].eeCfg = NULL;
    }

    return (status);
}

#if defined(VPAC3L)
static vx_status tivxVpacVissSetPcidConfig(tivxVpacVissObj *vissObj,
    tivxVpacVissConfigRef *vissCfgRef)
{
    vx_status           status = (vx_status)VX_SUCCESS;
    int32_t             fvid2_status = FVID2_SOK;
    Vhwa_M2mVissParams *vissDrvPrms = NULL;

    vissDrvPrms = &vissObj->vissPrms;

    if (((uint32_t)TRUE == vissDrvPrms->enablePcid) &&
        (NULL != vissCfgRef->pcidCfg))
    {
        fvid2_status = Fvid2_control(vissObj->handle, IOCTL_PCID_SET_CONFIG,
            (void *)vissCfgRef->pcidCfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set PCID Config !!!\n");
        }

        vissCfgRef->pcidCfg = NULL;
    }

    /* Convert FVID2 status to OpenVX Status */
    if (FVID2_SOK != fvid2_status)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}
#endif


