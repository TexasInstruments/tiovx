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

void tivxRegisterOpenVXCoreTargetKernels(void);
void tivxUnRegisterOpenVXCoreTargetKernels(void);
void tivxRegisterTutorialTargetKernels(void);
void tivxUnRegisterTutorialTargetKernels(void);
void tivxRegisterCaptureTargetArmKernels(void);
void tivxUnRegisterCaptureTargetArmKernels(void);
void tivxRegisterTestKernelsTargetDspKernels(void);
void tivxUnRegisterTestKernelsTargetDspKernels(void);
void tivxRegisterTestKernelsTargetArmKernels(void);
void tivxUnRegisterTestKernelsTargetArmKernels(void);
void tivxRegisterExtTargetMPUKernels(void);
void tivxUnRegisterExtTargetMPUKernels(void);

static void tivxInitLocal(void);
static void tivxDeInitLocal(void);

/* Counter for tracking the {init, de-init} calls. This is also used to
 * guarantee a single init/de-init operation.
 */
static uint32_t gInitCount = 0U;

#if defined(LINUX) || defined(QNX)
#include <pthread.h>
#include <tivx_platform_posix.h>

/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void tivxInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);

}

void tivxDeInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxDeInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);
}

#else
void tivxInit(void)
{
#if defined(REMOTE_COVERAGE)
/* LDRA dynamic analysis (code coverage capture) entry point.
 * The execution history capture will begin after this call for the
 * selected and ldra instrumented files.
 */
#include "code_coverage.h"
    appLogPrintf("################### DYNAMIC CODE COVERAGE INITIALIZATION STARTED ######################\n");
    ldra_initialize();
    appLogPrintf("################### DYNAMIC CODE COVERAGE INITIALIZATION FINISHED ######################\n");
#endif /* #if defined REMOTE_COVERAGE */
    tivxInitLocal();
}

void tivxDeInit(void)
{
    tivxDeInitLocal();
}
#endif // defined(LINUX) || defined(QNX)

static void tivxInitLocal(void)
{
    if (0U == gInitCount)
    {
        tivx_set_debug_zone((int32_t)VX_ZONE_ERROR);
        tivx_set_debug_zone((int32_t)VX_ZONE_WARNING);
        tivx_set_debug_zone((int32_t)VX_ZONE_INFO);

#if defined(LINUX) || defined(QNX)
        /* Initialize the POSIX objects */
        (void)ownPosixObjectInit();
#endif

        /* Initialize resource logging */
        ownLogResourceInit();

        /* Initialize platform */
        (void)ownPlatformInit();

        /* Initialize Target */
        ownTargetInit();

        /* Initialize Host */
    #if defined (SOC_J721E)
    #if defined (C66)
        #if defined(BUILD_CORE_KERNELS)
        tivxRegisterOpenVXCoreTargetKernels();
        #endif /* #if defined(BUILD_CORE_KERNELS) */
        #ifdef BUILD_TUTORIAL
        tivxRegisterTutorialTargetKernels();
        #endif
    #endif
    #else
    #if defined (C7X_FAMILY)
        #if defined(BUILD_CORE_KERNELS)
        tivxRegisterOpenVXCoreTargetKernels();
        #endif /* #if defined(BUILD_CORE_KERNELS) */
        #ifdef BUILD_TUTORIAL
        tivxRegisterTutorialTargetKernels();
        #endif
    #endif
    #endif /* defined(SOC_J721E) */

    #ifdef BUILD_CONFORMANCE_TEST
    #if defined(BUILD_TEST_KERNELS)
    #if defined (R5F)
        tivxRegisterTestKernelsTargetArmKernels();
    #endif

    #if defined (C7X_FAMILY) || defined (C66) || defined (R5F)
    tivxRegisterCaptureTargetArmKernels();
    #endif

    #if defined (C7X_FAMILY) || defined (C66)
        tivxRegisterTestKernelsTargetDspKernels();
    #endif
    #endif /* #if defined(BUILD_TEST_KERNELS) */
    #endif  /* #ifdef BUILD_CONFORMANCE_TEST */

    #if defined (R5F)
        tivxRegisterExtTargetMPUKernels();
    #endif

        ownObjDescInit();

        ownPlatformCreateTargets();

        VX_PRINT(VX_ZONE_INFO, "Initialization Done !!!\n");
        tivx_clr_debug_zone((int32_t)VX_ZONE_INFO);
    }

    gInitCount++;
}

static void tivxDeInitLocal(void)
{
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_INIT_UBR001
<justification end> */
    if (0U != gInitCount)
/* LDRA_JUSTIFY_END */
    {
        gInitCount--;

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_INIT_UM001
<justification end> */
#endif
        if (0U == gInitCount)
        {
            ownPlatformDeleteTargets();

            /* DeInitialize Host */
        #if defined (SOC_J721E)
        #if defined (C66)
            #if defined(BUILD_CORE_KERNELS)
            tivxRegisterOpenVXCoreTargetKernels();
            #endif /* #if defined(BUILD_CORE_KERNELS) */
            #ifdef BUILD_TUTORIAL
            tivxRegisterTutorialTargetKernels();
            #endif
        #endif
        #else
        #if defined (C7X_FAMILY)
            #if defined(BUILD_CORE_KERNELS)
            tivxUnRegisterOpenVXCoreTargetKernels();
            #endif /* #if defined(BUILD_CORE_KERNELS) */
            #ifdef BUILD_TUTORIAL
            tivxUnRegisterTutorialTargetKernels();
            #endif
        #endif
        #endif /* defined(SOC_J721E) */

        #ifdef BUILD_CONFORMANCE_TEST
        #if defined(BUILD_TEST_KERNELS)
        #if defined (R5F)
            tivxUnRegisterTestKernelsTargetArmKernels();
        #endif

        #if defined (C7X_FAMILY) || defined (C66) || defined (R5F)
        tivxUnRegisterCaptureTargetArmKernels();
        #endif

        #if defined (C7X_FAMILY) || defined(C66)
            tivxUnRegisterTestKernelsTargetDspKernels();
        #endif
        #endif /* #if defined(BUILD_TEST_KERNELS) */
        #endif  /* #ifdef BUILD_CONFORMANCE_TEST */

        #if defined (R5F)
            tivxUnRegisterExtTargetMPUKernels();
        #endif

            /* DeInitialize Target */
            ownTargetDeInit();

            /* DeInitialize platform */
            ownPlatformDeInit();

            /* DeInitialize resource logging */
            ownLogResourceDeInit();

#if defined(LINUX) || defined(QNX)
            /* DeInitialize the POSIX objects */
            (void)ownPosixObjectDeInit();
#endif

            VX_PRINT(VX_ZONE_INFO, "De-Initialization Done !!!\n");
        }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_INIT_UM001
<justification end> */
    else
    {
        /* ERROR. */
        VX_PRINT(VX_ZONE_ERROR, "De-Initialization Error !!!\n");
    }
/* LDRA_JUSTIFY_END */
}
