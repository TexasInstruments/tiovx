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

void ownPlatformCreateTargetsC7(void)
{
    /*
     * Note: All CPU tasks should be at a lower priority than APP_IPC_RPMESSAGE_RX_TASK_PRI, otherwise
     *       new messages would be starved from getting enqueued into the various priority
     *       worker threads.
     * */

    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1,       0u, "TIVX_C71_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2, 1u, "TIVX_C71_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3, 2u, "TIVX_C71_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4, 3u, "TIVX_C71_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5, 4u, "TIVX_C71_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6, 5u, "TIVX_C71_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7, 6u, "TIVX_C71_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8, 7u, "TIVX_C71_P8", 2u);

    #if defined(SOC_J721S2)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP1,           0u, "TIVX_CPU",       8u);
    #elif (C7X_COUNT > 1U)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2,       0u, "TIVX_C72_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2, 1u, "TIVX_C72_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3, 2u, "TIVX_C72_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4, 3u, "TIVX_C72_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5, 4u, "TIVX_C72_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6, 5u, "TIVX_C72_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7, 6u, "TIVX_C72_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8, 7u, "TIVX_C72_P8", 2u);
    #endif

    #if (C7X_COUNT > 2U)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3,       0u, "TIVX_C73_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2, 1u, "TIVX_C73_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3, 2u, "TIVX_C73_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4, 3u, "TIVX_C73_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5, 4u, "TIVX_C73_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6, 5u, "TIVX_C73_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7, 6u, "TIVX_C73_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8, 7u, "TIVX_C73_P8", 2u);
    #endif

    #if (C7X_COUNT > 3U)
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4,       0u, "TIVX_C74_P1", 9u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2, 1u, "TIVX_C74_P2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3, 2u, "TIVX_C74_P3", 7u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4, 3u, "TIVX_C74_P4", 6u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5, 4u, "TIVX_C74_P5", 5u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6, 5u, "TIVX_C74_P6", 4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7, 6u, "TIVX_C74_P7", 3u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8, 7u, "TIVX_C74_P8", 2u);
    #endif
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void ownPlatformDeleteTargetsC7.* <function end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_CONFIG_C7x_UM001
<justification end> */
void ownPlatformDeleteTargetsC7(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8);

    #if defined(SOC_J721S2)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP1);
    #elif (C7X_COUNT > 1U)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8);
    #endif

    #if (C7X_COUNT > 2U)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8);
    #endif

    #if (C7X_COUNT > 3U)
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8);
    #endif
}

#ifndef PC
void ownPlatformCreateTargets(void)
{
    ownPlatformCreateTargetsC7();
}

void ownPlatformDeleteTargets(void)
{
    ownPlatformDeleteTargetsC7();
}
#endif
