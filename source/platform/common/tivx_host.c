/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
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

#include <vx_internal.h>
#include <tivx_target_config.h>

void tivxRegisterOpenVXCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);
void tivxRegisterOpenVXExtKernels(void);
void tivxUnRegisterOpenVXExtKernels(void);
void tivxPlatformResetObjDescTableInfo(void);

static uint8_t gInitCount = 0U;

static void tivxHostInitLocal(void);
static void tivxHostDeInitLocal(void);

#if defined(LINUX) || defined(QNX)
#include <pthread.h>

/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void tivxHostInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxHostInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);

}

void tivxHostDeInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxHostDeInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);
}

#else
void tivxHostInit(void)
{
    tivxHostInitLocal();
}

void tivxHostDeInit(void)
{
    tivxHostDeInitLocal();
}
#endif // defined(LINUX) || defined(QNX)

static void tivxHostInitLocal(void)
{
    if (0U == gInitCount)
    {
        /* Dont init Obj Desc table here, since its done during system time by RTOS core
         * This API will be called by each TIOVX linux process.
         * So if obj desc are init every time a TIOVX process is created
         * then some other TIOVX process running in background would lose its state and things will go wrong
         */
        /* tivxPlatformResetObjDescTableInfo(); */

        /* This function registers functions that only makes sense on the host.  This is done to optimize
           memory footprint so the linker on the non-host cores can optimize out symbols needed on the host.
         */
        ownRegisterFunctionsForHost();
        (void)ownObjectInit();
        tivxRegisterOpenVXCoreKernels();
        tivxRegisterOpenVXExtKernels();
        tivxPlatformSetHostTargetId((tivx_cpu_id_e)tivxGetSelfCpuId());

        VX_PRINT(VX_ZONE_INFO, "Initialization Done for HOST !!!\n");
    }

    gInitCount++;
}

static void tivxHostDeInitLocal(void)
{
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_HOST_UBR002
<justification end> */
    if (0U != gInitCount)
/* LDRA_JUSTIFY_END */
    {
        gInitCount--;

        if (0U == gInitCount)
        {
            VX_PRINT(VX_ZONE_INFO, "De-Initialization Done for HOST !!!\n");
            (void)ownObjectDeInit();
            tivxUnRegisterOpenVXExtKernels();
            tivxUnRegisterOpenVXCoreKernels();
        }
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_UM003
<justification end> */
    else
    {
        /* ERROR. */
        VX_PRINT(VX_ZONE_ERROR, "De-Initialization Error !!!\n");
    }
/* LDRA_JUSTIFY_END */
}
