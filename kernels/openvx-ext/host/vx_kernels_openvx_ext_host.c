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
#include "tivx_openvx_ext_kernels.h"
#include "tivx_kernels_host_utils.h"
#include "tivx_ext_host_priv.h"

static vx_status VX_CALLBACK publishKernels(vx_context context);
static vx_status VX_CALLBACK unPublishKernels(vx_context context);

static uint32_t gIsExtKernelsLoad = 0u;

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {&tivxAddKernelObjArraySplit, &tivxRemoveKernelObjArraySplit}
};

static vx_status VX_CALLBACK publishKernels(vx_context context)
{
    return tivxPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

static vx_status VX_CALLBACK unPublishKernels(vx_context context)
{
    return tivxUnPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));
}

void tivxRegisterOpenVXExtKernels(void)
{
    tivxRegisterModule(TIVX_MODULE_NAME_OPENVX_EXT, publishKernels, unPublishKernels);
}

void tivxUnRegisterOpenVXExtKernels(void)
{
    tivxUnRegisterModule(TIVX_MODULE_NAME_OPENVX_EXT);
}

void tivxExtLoadKernels(vx_context context)
{
    if ((0U == gIsExtKernelsLoad) && (NULL != context))
    {
        void tivxSetSelfCpuId(vx_enum cpu_id);

        tivxRegisterOpenVXExtKernels();
        vxLoadKernels(context, TIVX_MODULE_NAME_OPENVX_EXT);

        #ifdef x86_64

        #if defined(SOC_AM62A)
        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_MCU1_0);
        #else
        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_MCU2_0);
        #endif
        tivxRegisterExtTargetMPUKernels();

        tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_MPU_0);
        #endif /* #ifdef x86_64 */

        tivxRegisterExtTargetMPUKernels();
    }
    gIsExtKernelsLoad++;
}

void tivxExtUnLoadKernels(vx_context context)
{
    if (gIsExtKernelsLoad > 0)
    {
        tivxUnRegisterExtTargetMPUKernels();

        gIsExtKernelsLoad--;
        if ((0u == gIsExtKernelsLoad) && (NULL != context))
        {
            void tivxSetSelfCpuId(vx_enum cpu_id);

            vxUnloadKernels(context, TIVX_MODULE_NAME_OPENVX_EXT);
            tivxUnRegisterOpenVXExtKernels();

            #ifdef x86_64

            #if defined(SOC_AM62A)
            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_MCU1_0);
            #else
            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_MCU2_0);
            #endif
            tivxUnRegisterExtTargetMPUKernels();

            tivxSetSelfCpuId((vx_enum)TIVX_CPU_ID_MPU_0);
            #endif /* #ifdef x86_64 */

            tivxUnRegisterExtTargetMPUKernels();
        }
    }
}

