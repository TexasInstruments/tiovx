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



#ifndef TIVX_TARGET_CONFIG_J721E_H_
#define TIVX_TARGET_CONFIG_J721E_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Platform Specific Target Configurations
 */

/*! \brief Max number of targets on a given R5F
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_R5F_MAX            (24U)

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0u),

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP2, 0u),

    /*! \brief target ID for DSP_C7_1 */
    TIVX_TARGET_ID_DSP_C7_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 0u),

    /*! \brief target ID for DSP_C7_1_PRI_2 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 1u),

    /*! \brief target ID for DSP_C7_1_PRI_3 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 2u),

    /*! \brief target ID for DSP_C7_1_PRI_4 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 3u),

    /*! \brief target ID for DSP_C7_1_PRI_5 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 4u),

    /*! \brief target ID for DSP_C7_1_PRI_6 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 5u),

    /*! \brief target ID for DSP_C7_1_PRI_7 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 6u),

    /*! \brief target ID for DSP_C7_1_PRI_8 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 7u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 0u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 1u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 2u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 3u),

    /*! \brief target ID for MCU2-0 */
    TIVX_TARGET_ID_MCU2_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 0u),

    /*! \brief target ID for NF */
    TIVX_TARGET_ID_VPAC_NF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 1u),

    /*! \brief target ID for LDC1 */
    TIVX_TARGET_ID_VPAC_LDC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 2u),

    /*! \brief target ID for MSC1 */
    TIVX_TARGET_ID_VPAC_MSC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 3u),

    /*! \brief target ID for MSC2 */
    TIVX_TARGET_ID_VPAC_MSC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 4u),

    /*! \brief target ID for VISS1 */
    TIVX_TARGET_ID_VPAC_VISS1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 5u),

    /*! \brief target ID for Capture1 */
    TIVX_TARGET_ID_CAPTURE1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 6u),

    /*! \brief target ID for Capture2 */
    TIVX_TARGET_ID_CAPTURE2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 7u),

    /*! \brief target ID for Display1 */
    TIVX_TARGET_ID_DISPLAY1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 8u),

    /*! \brief target ID for Display2 */
    TIVX_TARGET_ID_DISPLAY2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 9u),

    /*! \brief target ID for CSITX */
    TIVX_TARGET_ID_CSITX = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 10u),

    /*! \brief target ID for Capture3 */
    TIVX_TARGET_ID_CAPTURE3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 11u),

    /*! \brief target ID for Capture4 */
    TIVX_TARGET_ID_CAPTURE4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 12u),

    /*! \brief target ID for Capture5 */
    TIVX_TARGET_ID_CAPTURE5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 13u),

    /*! \brief target ID for Capture6 */
    TIVX_TARGET_ID_CAPTURE6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 14u),

    /*! \brief target ID for Capture7 */
    TIVX_TARGET_ID_CAPTURE7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 15u),

    /*! \brief target ID for Capture8 */
    TIVX_TARGET_ID_CAPTURE8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 16u),

    /*! \brief target ID for Display M2M1 */
    TIVX_TARGET_ID_DISPLAY_M2M1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 17u),
    
    /*! \brief target ID for Display M2M2 */
    TIVX_TARGET_ID_DISPLAY_M2M2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 18u),
    
    /*! \brief target ID for Display M2M3 */
    TIVX_TARGET_ID_DISPLAY_M2M3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 19u),
    
    /*! \brief target ID for Display M2M4 */
    TIVX_TARGET_ID_DISPLAY_M2M4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 20u),

    /*! \brief target ID for MCU2-1 */
    TIVX_TARGET_ID_MCU2_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_1, 0u),

    /*! \brief target ID for SDE */
    TIVX_TARGET_ID_DMPAC_SDE = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_1, 1u),

    /*! \brief target ID for DOF */
    TIVX_TARGET_ID_DMPAC_DOF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_1, 2u),

    /*! \brief target ID for MCU3_0 */
    TIVX_TARGET_ID_MCU3_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU3_0, 0u),

    /*! \brief target ID for MCU3_1 */
    TIVX_TARGET_ID_MCU3_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU3_1, 0u),
    
} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, (vx_enum)TIVX_TARGET_ID_DSP1},                                   \
    {TIVX_TARGET_DSP2, (vx_enum)TIVX_TARGET_ID_DSP2},                                   \
    {TIVX_TARGET_DSP_C7_1, (vx_enum)TIVX_TARGET_ID_DSP_C7_1},                           \
    {TIVX_TARGET_DSP_C7_1_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2},               \
    {TIVX_TARGET_DSP_C7_1_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3},               \
    {TIVX_TARGET_DSP_C7_1_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4},               \
    {TIVX_TARGET_DSP_C7_1_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5},               \
    {TIVX_TARGET_DSP_C7_1_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6},               \
    {TIVX_TARGET_DSP_C7_1_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7},               \
    {TIVX_TARGET_DSP_C7_1_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8},               \
    {TIVX_TARGET_MCU2_0, (vx_enum)TIVX_TARGET_ID_MCU2_0},                               \
    {TIVX_TARGET_MPU_0, (vx_enum)TIVX_TARGET_ID_MPU_0},                                 \
    {TIVX_TARGET_MPU_1, (vx_enum)TIVX_TARGET_ID_MPU_1},                                 \
    {TIVX_TARGET_MPU_2, (vx_enum)TIVX_TARGET_ID_MPU_2},                                 \
    {TIVX_TARGET_MPU_3, (vx_enum)TIVX_TARGET_ID_MPU_3},                                 \
    {TIVX_TARGET_VPAC_NF, (vx_enum)TIVX_TARGET_ID_VPAC_NF},                             \
    {TIVX_TARGET_VPAC_LDC1, (vx_enum)TIVX_TARGET_ID_VPAC_LDC1},                         \
    {TIVX_TARGET_VPAC_MSC1, (vx_enum)TIVX_TARGET_ID_VPAC_MSC1},                         \
    {TIVX_TARGET_VPAC_MSC2, (vx_enum)TIVX_TARGET_ID_VPAC_MSC2},                         \
    {TIVX_TARGET_VPAC_VISS1, (vx_enum)TIVX_TARGET_ID_VPAC_VISS1},                       \
    {TIVX_TARGET_CAPTURE1, (vx_enum)TIVX_TARGET_ID_CAPTURE1},                           \
    {TIVX_TARGET_CAPTURE2, (vx_enum)TIVX_TARGET_ID_CAPTURE2},                           \
    {TIVX_TARGET_DISPLAY1, (vx_enum)TIVX_TARGET_ID_DISPLAY1},                           \
    {TIVX_TARGET_DISPLAY2, (vx_enum)TIVX_TARGET_ID_DISPLAY2},                           \
    {TIVX_TARGET_CSITX, (vx_enum)TIVX_TARGET_ID_CSITX},                                 \
    {TIVX_TARGET_CAPTURE3, (vx_enum)TIVX_TARGET_ID_CAPTURE3},                           \
    {TIVX_TARGET_CAPTURE4, (vx_enum)TIVX_TARGET_ID_CAPTURE4},                           \
    {TIVX_TARGET_CAPTURE5, (vx_enum)TIVX_TARGET_ID_CAPTURE5},                           \
    {TIVX_TARGET_CAPTURE6, (vx_enum)TIVX_TARGET_ID_CAPTURE6},                           \
    {TIVX_TARGET_CAPTURE7, (vx_enum)TIVX_TARGET_ID_CAPTURE7},                           \
    {TIVX_TARGET_CAPTURE8, (vx_enum)TIVX_TARGET_ID_CAPTURE8},                           \
    {TIVX_TARGET_DISPLAY_M2M1, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1},                   \
    {TIVX_TARGET_DISPLAY_M2M2, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2},                   \
    {TIVX_TARGET_DISPLAY_M2M3, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3},                   \
    {TIVX_TARGET_DISPLAY_M2M4, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4},                   \
    {TIVX_TARGET_MCU2_1, (vx_enum)TIVX_TARGET_ID_MCU2_1},                               \
    {TIVX_TARGET_DMPAC_SDE, (vx_enum)TIVX_TARGET_ID_DMPAC_SDE},                         \
    {TIVX_TARGET_DMPAC_DOF, (vx_enum)TIVX_TARGET_ID_DMPAC_DOF},                         \
    {TIVX_TARGET_MCU3_0, (vx_enum)TIVX_TARGET_ID_MCU3_0},                               \
    {TIVX_TARGET_MCU3_1, (vx_enum)TIVX_TARGET_ID_MCU3_1},                               \
    /* TIVX_TARGET_HOST will be filled later during tivxHostInit()             \
     * by calling function tivxPlatformSetHostTargetId                         \
     */                                                                        \
    {TIVX_TARGET_HOST, (vx_enum)TIVX_TARGET_ID_INVALID}                                 \
}

#ifdef __cplusplus
}
#endif

#endif
