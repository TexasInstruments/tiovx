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
#include <TI/tda4x.h>
#include "tivx_hwa_kernels.h"
#include <stdio.h>

static vx_status VX_CALLBACK tivxPublishKernels(vx_context context);
static vx_status VX_CALLBACK tivxUnPublishKernels(vx_context context);

typedef vx_status (*tivxHostKernel_Fxn) (vx_context context);

typedef struct {
    tivxHostKernel_Fxn    add_kernel;
    tivxHostKernel_Fxn    remove_kernel;
} Tivx_Host_Kernel_List;


static uint32_t gIsHwaKernelsLoad = 0u;

//vx_status tivxAddHWAKernelVPAC_NF_bilateral(vx_context context);
vx_status tivxAddKernel_VPAC_NF_generic(vx_context context);

//vx_status tivxRemoveHWAKernelVPAC_NF_bilateral(vx_context context);
vx_status tivxRemoveKernel_VPAC_NF_generic(vx_context context);

static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {tivxAddKernel_VPAC_NF_generic, tivxRemoveKernel_VPAC_NF_generic}
};

static vx_status VX_CALLBACK tivxPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i <
        (sizeof(gTivx_host_kernel_list)/sizeof(Tivx_Host_Kernel_List)); i ++)
    {
        if (gTivx_host_kernel_list[i].add_kernel)
        {
            status = gTivx_host_kernel_list[i].add_kernel(context);
        }

        if (VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxUnPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i <
        (sizeof(gTivx_host_kernel_list)/sizeof(Tivx_Host_Kernel_List)); i ++)
    {
        if (gTivx_host_kernel_list[i].remove_kernel)
        {
            status = gTivx_host_kernel_list[i].remove_kernel(context);
        }

        if (VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

void tivxRegisterHWAKernels(void)
{
    tivxRegisterModule(TIVX_MODULE_NAME_HWA, tivxPublishKernels, tivxUnPublishKernels);
}

void tivxUnRegisterHWAKernels(void)
{
    tivxUnRegisterModule(TIVX_MODULE_NAME_HWA);
}

void hwaPrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName)
{
    printf("[ %c%c ] Execution time for %9d pixels (avg = %4d.%-6d ms, min = %4d.%-6d ms, max = %4d.%-6d ms)\n",
        testName[0], testName[1],
        (int)numPixels,
        (int)(performance.avg/1000000),
        (int)(performance.avg%1000000),
        (int)(performance.min/1000000),
        (int)(performance.min%1000000),
        (int)(performance.max/1000000),
        (int)(performance.max%1000000)
        );
}

void hwaLoadKernels(vx_context context)
{
    if ((0 == gIsHwaKernelsLoad) &&
        (NULL != context))
    {
        tivxRegisterHWAKernels();
        vxLoadKernels(context, TIVX_MODULE_NAME_HWA);

        /* These three lines only work on PC emulation mode ...
         * this will need to be updated when moving to target */
        tivxSetSelfCpuId(TIVX_CPU_ID_IPU1_0);
        tivxRegisterHWATargetKernels();
        tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);

        gIsHwaKernelsLoad = 1U;
    }
}

void hwaUnLoadKernels(vx_context context)
{
    if ((1u == gIsHwaKernelsLoad) &&
        (NULL != context))
    {
        vxUnloadKernels(context, TIVX_MODULE_NAME_HWA);
        tivxUnRegisterHWAKernels();

        /* This line only work on PC emulation mode ...
         * this will need to be updated when moving to target */
        tivxUnRegisterHWATargetKernels();

        gIsHwaKernelsLoad = 0U;
    }
}
