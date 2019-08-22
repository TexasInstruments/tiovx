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

#include <TI/tivx.h>
#include <TI/j7.h>
#include "tivx_hwa_kernels.h"
#include "tivx_kernels_host_utils.h"

static vx_status VX_CALLBACK publishKernels(vx_context context);
static vx_status VX_CALLBACK unPublishKernels(vx_context context);

static uint32_t gIsHwaKernelsLoad = 0u;

vx_status tivxAddKernelVpacNfGeneric(vx_context context);
vx_status tivxAddKernelVpacNfBilateral(vx_context context);
vx_status tivxAddKernelDmpacSde(vx_context context);
vx_status tivxAddKernelVpacLdc(vx_context context);
vx_status tivxAddKernelVpacMscMultiScale(vx_context context);
vx_status tivxAddKernelVpacMscPyramid(vx_context context);
vx_status tivxAddKernelDmpacDof(vx_context context);
vx_status tivxAddKernelDofVisualize(vx_context context);
vx_status tivxAddKernelVpacViss(vx_context context);
vx_status tivxAddKernelDisplay(vx_context context);
vx_status tivxAddKernelCapture(vx_context context);
vx_status tivxAddKernelVideoDecoder(vx_context context);

vx_status tivxRemoveKernelVpacNfGeneric(vx_context context);
vx_status tivxRemoveKernelVpacNfBilateral(vx_context context);
vx_status tivxRemoveKernelDmpacSde(vx_context context);
vx_status tivxRemoveKernelVpacLdc(vx_context context);
vx_status tivxRemoveKernelVpacMscMultiScale(vx_context context);
vx_status tivxRemoveKernelVpacMscPyramid(vx_context context);
vx_status tivxRemoveKernelDmpacDof(vx_context context);
vx_status tivxRemoveKernelDofVisualize(vx_context context);
vx_status tivxRemoveKernelVpacViss(vx_context context);
vx_status tivxRemoveKernelDisplay(vx_context context);
vx_status tivxRemoveKernelCapture(vx_context context);
vx_status tivxRemoveKernelVideoDecoder(vx_context context);

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {&tivxAddKernelVpacNfGeneric, &tivxRemoveKernelVpacNfGeneric},
    {&tivxAddKernelVpacNfBilateral, &tivxRemoveKernelVpacNfBilateral},
    {&tivxAddKernelDmpacSde, &tivxRemoveKernelDmpacSde},
    {&tivxAddKernelVpacLdc, &tivxRemoveKernelVpacLdc},
    {&tivxAddKernelVpacMscMultiScale, &tivxRemoveKernelVpacMscMultiScale},
    {&tivxAddKernelVpacMscPyramid, &tivxRemoveKernelVpacMscPyramid},
    {&tivxAddKernelDmpacDof, &tivxRemoveKernelDmpacDof},
    {&tivxAddKernelDofVisualize, &tivxRemoveKernelDofVisualize},
    {&tivxAddKernelVpacViss, &tivxRemoveKernelVpacViss},
    {&tivxAddKernelDisplay, &tivxRemoveKernelDisplay},
    {&tivxAddKernelCapture, &tivxRemoveKernelCapture},
    {&tivxAddKernelVideoDecoder, &tivxRemoveKernelVideoDecoder}
};

static vx_status VX_CALLBACK publishKernels(vx_context context)
{
    return tivxPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

static vx_status VX_CALLBACK unPublishKernels(vx_context context)
{
    return tivxUnPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

void tivxRegisterHwaKernels(void)
{
    tivxRegisterModule(TIVX_MODULE_NAME_HWA, publishKernels, unPublishKernels);
}

void tivxUnRegisterHwaKernels(void)
{
    tivxUnRegisterModule(TIVX_MODULE_NAME_HWA);
}

void tivxHwaLoadKernels(vx_context context)
{
    if ((0 == gIsHwaKernelsLoad) && (NULL != context))
    {
        void tivxSetSelfCpuId(vx_enum cpu_id);

        tivxRegisterHwaKernels();
        vxLoadKernels(context, TIVX_MODULE_NAME_HWA);

        #ifdef x86_64
        /* for PC register target kernels here */
        /* for actual Si, target kernel registration will happen
         * during platform init on the CPU that supports the target
         */
        /* These three lines only work on PC emulation mode ...
         * this will need to be updated when moving to target */
        tivxSetSelfCpuId(TIVX_CPU_ID_IPU1_0);

        tivxRegisterHwaTargetVpacNfKernels();

        tivxRegisterHwaTargetDmpacSdeKernels();

        tivxRegisterHwaTargetVpacLdcKernels();

        #ifdef BUILD_HWA_DMPAC_DOF
        tivxRegisterHwaTargetDmpacDofKernels();
        #endif

        tivxRegisterHwaTargetVpacMscKernels();

        tivxRegisterHwaTargetArmKernels();

        tivxRegisterHwaTargetVpacVissKernels();

        /*tivxRegisterHwaTargetVdecKernels();*/

        tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);
        #endif

        gIsHwaKernelsLoad = 1U;
    }
}

void tivxHwaUnLoadKernels(vx_context context)
{
    if ((1u == gIsHwaKernelsLoad) && (NULL != context))
    {
        vxUnloadKernels(context, TIVX_MODULE_NAME_HWA);
        tivxUnRegisterHwaKernels();

        #ifdef x86_64
        /* This line only work on PC emulation mode ...
         * this will need to be updated when moving to target */

        tivxUnRegisterHwaTargetVpacNfKernels();

        tivxUnRegisterHwaTargetDmpacSdeKernels();

        tivxUnRegisterHwaTargetVpacLdcKernels();

        #ifdef BUILD_HWA_DMPAC_DOF
        tivxUnRegisterHwaTargetDmpacDofKernels();
        #endif

        tivxUnRegisterHwaTargetVpacMscKernels();

        tivxUnRegisterHwaTargetArmKernels();

        tivxUnRegisterHwaTargetVpacVissKernels();

        /*tivxUnRegisterHwaTargetVdecKernels();*/

        #endif

        gIsHwaKernelsLoad = 0U;
    }
}

