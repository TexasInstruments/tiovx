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

#include <tivx_target_config.h>

void ownPlatformCreateTargetsR5f(void)
{
    /* MCU2-0 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0,     0u, "TIVX_CPU_0",  4u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1,  1u, "TIVX_VLDC1", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1,  2u, "TIVX_VMSC1",  8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2,  3u, "TIVX_VMSC2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1, 4u, "TIVX_VVISS1", 13u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1,   5u, "TIVX_CAPT1", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2,   6u, "TIVX_CAPT2", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3,   7u, "TIVX_CAPT3", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4,   8u, "TIVX_CAPT4", 15u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1,   9u, "TIVX_DISP1", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2,  10u, "TIVX_DISP2", 14u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX,     11u, "TIVX_CSITX",  8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_CSITX2,    12u, "TIVX_CSITX2", 8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE, 13u, "TIVX_SDE",    8u);
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF, 14u, "TIVX_DOF",    8u);

    /* MCU1-0 Targets */
    tivxPlatformCreateTargetId((vx_enum)TIVX_TARGET_ID_MCU1_0, 0u, "TIVX_MCU1_0", 4u);
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void ownPlatformDeleteTargetsR5f.* <function end>
<justification start> TIOVX_CODE_COVERAGE_TARGET_CONFIG_R5F_UM001
<justification end> */
void ownPlatformDeleteTargetsR5f(void)
{
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU2_0);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_LDC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_MSC2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_VPAC_VISS1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE3);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CAPTURE4);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY1);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DISPLAY2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_CSITX2);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_SDE);
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_DMPAC_DOF);

    /* MCU1-0 targets */
    tivxPlatformDeleteTargetId((vx_enum)TIVX_TARGET_ID_MCU1_0);
}

#ifndef PC
void ownPlatformCreateTargets(void)
{
    ownPlatformCreateTargetsR5f();
}

void ownPlatformDeleteTargets(void)
{
    ownPlatformDeleteTargetsR5f();
}
#endif
