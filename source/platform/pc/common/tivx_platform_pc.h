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




#ifndef _TIVX_PLATFORM_PC_H_
#define _TIVX_PLATFORM_PC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief PC Platform APIs
 */

/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_config.h
 * \ingroup group_tivx_platform
 */
#if defined (SOC_J784S4)
#define TIVX_PLATFORM_MAX_TARGETS            (75u)
#elif defined (SOC_J721E)
#define TIVX_PLATFORM_MAX_TARGETS            (43u)
#elif defined (SOC_J721S2)
#define TIVX_PLATFORM_MAX_TARGETS            (43u)
#elif defined (SOC_AM62A)
#define TIVX_PLATFORM_MAX_TARGETS            (20u)
#elif defined (SOC_J722S)
#define TIVX_PLATFORM_MAX_TARGETS            (40u)
#elif defined (SOC_J742S2)
#define TIVX_PLATFORM_MAX_TARGETS            (63u)
#endif


#if defined (SOC_J721E)

/*! \brief Mapping of Target names with Task names and Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                    \
{                                                                           \
    {TIVX_TARGET_DSP1, "TIVX_DSP1", 0},                                      \
    {TIVX_TARGET_DSP2, "TIVX_DSP2", 1},                                      \
    {TIVX_TARGET_DSP_C7_1, "TIVX_C71_P1", 2},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, "TIVX_C71_P2", 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, "TIVX_C71_P3", 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, "TIVX_C71_P4", 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, "TIVX_C71_P5", 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, "TIVX_C71_P6", 7},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, "TIVX_C71_P7", 8},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, "TIVX_C71_P8", 9},                         \
    {TIVX_TARGET_IPU1_0, "TIVX_IPU1_0", 10},                                 \
    {TIVX_TARGET_IPU1_1, "TIVX_IPU1_1", 11},                                 \
    {TIVX_TARGET_MCU2_0, "TIVX_MCU2_0", 12},                                 \
    {TIVX_TARGET_MCU2_1, "TIVX_MCU2_1", 13},                                 \
    {TIVX_TARGET_MCU3_0, "TIVX_MCU3_0", 14},                                \
    {TIVX_TARGET_MCU3_1, "TIVX_MCU3_1", 15},                                \
    {TIVX_TARGET_MPU_0, "TIVX_MPU0", 16},                                   \
    {TIVX_TARGET_MPU_1, "TIVX_MPU1", 17},                                   \
    {TIVX_TARGET_MPU_2, "TIVX_MPU2", 18},                                   \
    {TIVX_TARGET_MPU_3, "TIVX_MPU3", 19},                                   \
    {TIVX_TARGET_VPAC_NF, "TIVX_VNF", 20},                                 \
    {TIVX_TARGET_VPAC_LDC1, "TIVX_VLDC", 21},                              \
    {TIVX_TARGET_VPAC_MSC1, "TIVX_VMSC1", 22},                             \
    {TIVX_TARGET_VPAC_MSC2, "TIVX_VMSC2", 23},                             \
    {TIVX_TARGET_DMPAC_SDE, "TIVX_SDE", 24},                                \
    {TIVX_TARGET_DMPAC_DOF, "TIVX_DOF", 25},                                \
    {TIVX_TARGET_VPAC_VISS1, "TIVX_VVISS", 26},                            \
    {TIVX_TARGET_CAPTURE1, "TIVX_CAPT1", 27},                               \
    {TIVX_TARGET_CAPTURE2, "TIVX_CAPT2", 28},                               \
    {TIVX_TARGET_CAPTURE3, "TIVX_CAPT3", 29},                               \
    {TIVX_TARGET_CAPTURE4, "TIVX_CAPT4", 30},                               \
    {TIVX_TARGET_CAPTURE5, "TIVX_CAPT5", 31},                               \
    {TIVX_TARGET_CAPTURE6, "TIVX_CAPT6", 32},                               \
    {TIVX_TARGET_CAPTURE7, "TIVX_CAPT7", 33},                               \
    {TIVX_TARGET_CAPTURE8, "TIVX_CAPT8", 34},                               \
    {TIVX_TARGET_DISPLAY1, "TIVX_DISP1", 35},                               \
    {TIVX_TARGET_DISPLAY2, "TIVX_DISP2", 36},                               \
    {TIVX_TARGET_DISPLAY_M2M1, "TIVX_DPM2M1", 37},                          \
    {TIVX_TARGET_DISPLAY_M2M2, "TIVX_DPM2M2", 38},                          \
    {TIVX_TARGET_DISPLAY_M2M3, "TIVX_DPM2M3", 39},                          \
    {TIVX_TARGET_DISPLAY_M2M4, "TIVX_DPM2M4", 40},                          \
    {TIVX_TARGET_CSITX, "TIVX_CSITX", 41},                                  \
    /* TIVX_TARGET_HOST should point to the number associated with          \
     * the host target thread in list above                                 \
     */                                                                     \
    {TIVX_TARGET_HOST, "TIVX_MPU0", 16},                                    \
}
#elif defined (SOC_J721S2)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                    \
{                                                                           \
    {TIVX_TARGET_DSP1, "TIVX_DSP1", 0},                                      \
    {TIVX_TARGET_DSP_C7_1, "TIVX_C71_P1", 1},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, "TIVX_C71_P2", 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, "TIVX_C71_P3", 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, "TIVX_C71_P4", 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, "TIVX_C71_P5", 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, "TIVX_C71_P6", 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, "TIVX_C71_P7", 7},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, "TIVX_C71_P8", 8},                         \
    {TIVX_TARGET_IPU1_0, "TIVX_IPU1_0", 9},                                  \
    {TIVX_TARGET_IPU1_1, "TIVX_IPU1_1", 10},                                 \
    {TIVX_TARGET_MCU2_0, "TIVX_MCU2_0", 11},                                 \
    {TIVX_TARGET_MCU2_1, "TIVX_MCU2_1", 12},                                 \
    {TIVX_TARGET_MCU3_0, "TIVX_MCU3_0", 13},                                \
    {TIVX_TARGET_MCU3_1, "TIVX_MCU3_1", 14},                                \
    {TIVX_TARGET_MPU_0, "TIVX_MPU0", 15},                                   \
    {TIVX_TARGET_MPU_1, "TIVX_MPU1", 16},                                   \
    {TIVX_TARGET_MPU_2, "TIVX_MPU2", 17},                                   \
    {TIVX_TARGET_MPU_3, "TIVX_MPU3", 18},                                   \
    {TIVX_TARGET_VPAC_NF, "TIVX_VNF", 19},                                 \
    {TIVX_TARGET_VPAC_LDC1, "TIVX_VLDC", 20},                              \
    {TIVX_TARGET_VPAC_MSC1, "TIVX_VMSC1", 21},                             \
    {TIVX_TARGET_VPAC_MSC2, "TIVX_VMSC2", 22},                             \
    {TIVX_TARGET_DMPAC_SDE, "TIVX_SDE", 23},                                \
    {TIVX_TARGET_DMPAC_DOF, "TIVX_DOF", 24},                                \
    {TIVX_TARGET_VPAC_VISS1, "TIVX_VVISS", 25},                            \
    {TIVX_TARGET_CAPTURE1, "TIVX_CAPT1", 26},                               \
    {TIVX_TARGET_CAPTURE2, "TIVX_CAPT2", 27},                               \
    {TIVX_TARGET_CAPTURE3, "TIVX_CAPT3", 28},                               \
    {TIVX_TARGET_CAPTURE4, "TIVX_CAPT4", 29},                               \
    {TIVX_TARGET_CAPTURE5, "TIVX_CAPT5", 30},                               \
    {TIVX_TARGET_CAPTURE6, "TIVX_CAPT6", 31},                               \
    {TIVX_TARGET_CAPTURE7, "TIVX_CAPT7", 32},                               \
    {TIVX_TARGET_CAPTURE8, "TIVX_CAPT8", 33},                               \
    {TIVX_TARGET_DISPLAY1, "TIVX_DISP1", 34},                               \
    {TIVX_TARGET_DISPLAY2, "TIVX_DISP2", 35},                               \
    {TIVX_TARGET_DISPLAY_M2M1, "TIVX_DPM2M1", 36},                          \
    {TIVX_TARGET_DISPLAY_M2M2, "TIVX_DPM2M2", 37},                          \
    {TIVX_TARGET_DISPLAY_M2M3, "TIVX_DPM2M3", 38},                          \
    {TIVX_TARGET_DISPLAY_M2M4, "TIVX_DPM2M4", 39},                          \
    {TIVX_TARGET_CSITX, "TIVX_CSITX", 40},                                  \
    {TIVX_TARGET_CSITX2, "TIVX_CSITX2", 41},                                  \
    /* TIVX_TARGET_HOST should point to the number associated with          \
     * the host target thread in list above                                 \
     */                                                                     \
    {TIVX_TARGET_HOST, "TIVX_MPU0", 15},                                    \
}

#elif defined (SOC_J784S4)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                    \
{                                                                           \
    {TIVX_TARGET_DSP_C7_1, "TIVX_C71_P1", 0},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, "TIVX_C71_P2", 1},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, "TIVX_C71_P3", 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, "TIVX_C71_P4", 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, "TIVX_C71_P5", 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, "TIVX_C71_P6", 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, "TIVX_C71_P7", 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, "TIVX_C71_P8", 7},                         \
    {TIVX_TARGET_DSP_C7_2, "TIVX_C72_P1", 8},                               \
    {TIVX_TARGET_DSP_C7_2_PRI_2, "TIVX_C72_P2", 9},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_3, "TIVX_C72_P3", 10},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_4, "TIVX_C72_P4", 11},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_5, "TIVX_C72_P5", 12},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_6, "TIVX_C72_P6", 13},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_7, "TIVX_C72_P7", 14},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_8, "TIVX_C72_P8", 15},                        \
    {TIVX_TARGET_DSP_C7_3, "TIVX_C73_P1", 16},                              \
    {TIVX_TARGET_DSP_C7_3_PRI_2, "TIVX_C73_P2", 17},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_3, "TIVX_C73_P3", 18},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_4, "TIVX_C73_P4", 19},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_5, "TIVX_C73_P5", 20},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_6, "TIVX_C73_P6", 21},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_7, "TIVX_C73_P7", 22},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_8, "TIVX_C73_P8", 23},                        \
    {TIVX_TARGET_DSP_C7_4, "TIVX_C74_P1", 24},                              \
    {TIVX_TARGET_DSP_C7_4_PRI_2, "TIVX_C74_P2", 25},                        \
    {TIVX_TARGET_DSP_C7_4_PRI_3, "TIVX_C74_P3", 26},                        \
    {TIVX_TARGET_DSP_C7_4_PRI_4, "TIVX_C74_P4", 27},                        \
    {TIVX_TARGET_DSP_C7_4_PRI_5, "TIVX_C74_P5", 28},                        \
    {TIVX_TARGET_DSP_C7_4_PRI_6, "TIVX_C74_P6", 29},                        \
    {TIVX_TARGET_DSP_C7_4_PRI_7, "TIVX_C74_P7", 30},                        \
    {TIVX_TARGET_DSP_C7_4_PRI_8, "TIVX_C74_P8", 31},                        \
    {TIVX_TARGET_MCU2_0, "TIVX_MCU2_0", 32},                                 \
    {TIVX_TARGET_MCU2_1, "TIVX_MCU2_1", 33},                                 \
    {TIVX_TARGET_MCU3_0, "TIVX_MCU3_0", 34},                                \
    {TIVX_TARGET_MCU3_1, "TIVX_MCU3_1", 35},                                \
    {TIVX_TARGET_MCU4_0, "TIVX_MCU4_0", 36},                                \
    {TIVX_TARGET_MCU4_1, "TIVX_MCU4_1", 37},                                \
    {TIVX_TARGET_MPU_0, "TIVX_MPU0", 38},                                   \
    {TIVX_TARGET_MPU_1, "TIVX_MPU1", 39},                                   \
    {TIVX_TARGET_MPU_2, "TIVX_MPU2", 40},                                   \
    {TIVX_TARGET_MPU_3, "TIVX_MPU3", 41},                                   \
    {TIVX_TARGET_VPAC_NF, "TIVX_V1NF", 42},                                 \
    {TIVX_TARGET_VPAC_LDC1, "TIVX_V1LDC", 43},                              \
    {TIVX_TARGET_VPAC_MSC1, "TIVX_V1MSC1", 44},                             \
    {TIVX_TARGET_VPAC_MSC2, "TIVX_V1MSC2", 45},                             \
    {TIVX_TARGET_VPAC2_NF, "TIVX_V2NF", 46},                                \
    {TIVX_TARGET_VPAC2_LDC1, "TIVX_V2LDC", 47},                             \
    {TIVX_TARGET_VPAC2_MSC1, "TIVX_V2MSC1", 48},                            \
    {TIVX_TARGET_VPAC2_MSC2, "TIVX_V2MSC2", 49},                            \
    {TIVX_TARGET_DMPAC_SDE, "TIVX_SDE", 50},                                \
    {TIVX_TARGET_DMPAC_DOF, "TIVX_DOF", 51},                                \
    {TIVX_TARGET_VPAC_VISS1, "TIVX_V1VISS", 52},                            \
    {TIVX_TARGET_VPAC2_VISS1, "TIVX_V2VISS", 53},                           \
    {TIVX_TARGET_CAPTURE1, "TIVX_CAPT1", 54},                               \
    {TIVX_TARGET_CAPTURE2, "TIVX_CAPT2", 55},                               \
    {TIVX_TARGET_CAPTURE3, "TIVX_CAPT3", 56},                               \
    {TIVX_TARGET_CAPTURE4, "TIVX_CAPT4", 57},                               \
    {TIVX_TARGET_CAPTURE5, "TIVX_CAPT5", 58},                               \
    {TIVX_TARGET_CAPTURE6, "TIVX_CAPT6", 59},                               \
    {TIVX_TARGET_CAPTURE7, "TIVX_CAPT7", 60},                               \
    {TIVX_TARGET_CAPTURE8, "TIVX_CAPT8", 61},                               \
    {TIVX_TARGET_CAPTURE9, "TIVX_CAPT9", 62},                               \
    {TIVX_TARGET_CAPTURE10, "TIVX_CAPT10", 63},                             \
    {TIVX_TARGET_CAPTURE11, "TIVX_CAPT11", 64},                             \
    {TIVX_TARGET_CAPTURE12, "TIVX_CAPT12", 65},                             \
    {TIVX_TARGET_DISPLAY1, "TIVX_DISP1", 66},                               \
    {TIVX_TARGET_DISPLAY2, "TIVX_DISP2", 67},                               \
    {TIVX_TARGET_DISPLAY_M2M1, "TIVX_DPM2M1", 68},                          \
    {TIVX_TARGET_DISPLAY_M2M2, "TIVX_DPM2M2", 69},                          \
    {TIVX_TARGET_DISPLAY_M2M3, "TIVX_DPM2M3", 70},                          \
    {TIVX_TARGET_DISPLAY_M2M4, "TIVX_DPM2M4", 71},                          \
    {TIVX_TARGET_CSITX, "TIVX_CSITX", 72},                                  \
    {TIVX_TARGET_CSITX2, "TIVX_CSITX2", 73},                                \
    /* TIVX_TARGET_HOST should point to the number associated with          \
     * the host target thread in list above                                 \
     */                                                                     \
    {TIVX_TARGET_HOST, "TIVX_MPU0", 38},                                    \
}

#elif defined (SOC_J742S2)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                    \
{                                                                           \
    {TIVX_TARGET_DSP_C7_1, "TIVX_C71_P1", 0},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, "TIVX_C71_P2", 1},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, "TIVX_C71_P3", 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, "TIVX_C71_P4", 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, "TIVX_C71_P5", 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, "TIVX_C71_P6", 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, "TIVX_C71_P7", 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, "TIVX_C71_P8", 7},                         \
    {TIVX_TARGET_DSP_C7_2, "TIVX_C72_P1", 8},                               \
    {TIVX_TARGET_DSP_C7_2_PRI_2, "TIVX_C72_P2", 9},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_3, "TIVX_C72_P3", 10},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_4, "TIVX_C72_P4", 11},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_5, "TIVX_C72_P5", 12},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_6, "TIVX_C72_P6", 13},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_7, "TIVX_C72_P7", 14},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_8, "TIVX_C72_P8", 15},                        \
    {TIVX_TARGET_DSP_C7_3, "TIVX_C73_P1", 16},                              \
    {TIVX_TARGET_DSP_C7_3_PRI_2, "TIVX_C73_P2", 17},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_3, "TIVX_C73_P3", 18},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_4, "TIVX_C73_P4", 19},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_5, "TIVX_C73_P5", 20},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_6, "TIVX_C73_P6", 21},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_7, "TIVX_C73_P7", 22},                        \
    {TIVX_TARGET_DSP_C7_3_PRI_8, "TIVX_C73_P8", 23},                        \
    {TIVX_TARGET_MCU2_0, "TIVX_MCU2_0", 24},                                 \
    {TIVX_TARGET_MCU2_1, "TIVX_MCU2_1", 25},                                 \
    {TIVX_TARGET_MCU3_0, "TIVX_MCU3_0", 26},                                \
    {TIVX_TARGET_MCU3_1, "TIVX_MCU3_1", 27},                                \
    {TIVX_TARGET_MCU4_0, "TIVX_MCU4_0", 28},                                \
    {TIVX_TARGET_MCU4_1, "TIVX_MCU4_1", 29},                                \
    {TIVX_TARGET_MPU_0, "TIVX_MPU0", 30},                                   \
    {TIVX_TARGET_MPU_1, "TIVX_MPU1", 31},                                   \
    {TIVX_TARGET_MPU_2, "TIVX_MPU2", 32},                                   \
    {TIVX_TARGET_MPU_3, "TIVX_MPU3", 33},                                   \
    {TIVX_TARGET_VPAC_NF, "TIVX_V1NF", 34},                                 \
    {TIVX_TARGET_VPAC_LDC1, "TIVX_V1LDC", 35},                              \
    {TIVX_TARGET_VPAC_MSC1, "TIVX_V1MSC1", 36},                             \
    {TIVX_TARGET_VPAC_MSC2, "TIVX_V1MSC2", 37},                             \
    {TIVX_TARGET_VPAC2_NF, "TIVX_V2NF", 38},                                \
    {TIVX_TARGET_VPAC2_LDC1, "TIVX_V2LDC", 39},                             \
    {TIVX_TARGET_VPAC2_MSC1, "TIVX_V2MSC1", 40},                            \
    {TIVX_TARGET_VPAC2_MSC2, "TIVX_V2MSC2", 41},                            \
    {TIVX_TARGET_DMPAC_SDE, "TIVX_SDE", 42},                                \
    {TIVX_TARGET_DMPAC_DOF, "TIVX_DOF", 43},                                \
    {TIVX_TARGET_VPAC_VISS1, "TIVX_V1VISS", 44},                            \
    {TIVX_TARGET_VPAC2_VISS1, "TIVX_V2VISS", 45},                           \
    {TIVX_TARGET_CAPTURE1, "TIVX_CAPT1", 46},                               \
    {TIVX_TARGET_CAPTURE2, "TIVX_CAPT2", 47},                               \
    {TIVX_TARGET_CAPTURE3, "TIVX_CAPT3", 48},                               \
    {TIVX_TARGET_CAPTURE4, "TIVX_CAPT4", 49},                               \
    {TIVX_TARGET_CAPTURE5, "TIVX_CAPT5", 50},                               \
    {TIVX_TARGET_CAPTURE6, "TIVX_CAPT6", 51},                               \
    {TIVX_TARGET_CAPTURE7, "TIVX_CAPT7", 52},                               \
    {TIVX_TARGET_CAPTURE8, "TIVX_CAPT8", 53},                               \
    {TIVX_TARGET_DISPLAY1, "TIVX_DISP1", 54},                               \
    {TIVX_TARGET_DISPLAY2, "TIVX_DISP2", 54},                               \
    {TIVX_TARGET_DISPLAY_M2M1, "TIVX_DPM2M1", 56},                          \
    {TIVX_TARGET_DISPLAY_M2M2, "TIVX_DPM2M2", 57},                          \
    {TIVX_TARGET_DISPLAY_M2M3, "TIVX_DPM2M3", 58},                          \
    {TIVX_TARGET_DISPLAY_M2M4, "TIVX_DPM2M4", 59},                          \
    {TIVX_TARGET_CSITX, "TIVX_CSITX", 60},                                  \
    {TIVX_TARGET_CSITX2, "TIVX_CSITX2", 61},                                \
    /* TIVX_TARGET_HOST should point to the number associated with          \
     * the host target thread in list above                                 \
     */                                                                     \
    {TIVX_TARGET_HOST, "TIVX_MPU0", 30},                                    \
}

#elif defined (SOC_AM62A)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                    \
{                                                                           \
    {TIVX_TARGET_DSP1, "TIVX_DSP1", 0},                                      \
    {TIVX_TARGET_DSP_C7_1, "TIVX_C71_P1", 1},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, "TIVX_C71_P2", 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, "TIVX_C71_P3", 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, "TIVX_C71_P4", 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, "TIVX_C71_P5", 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, "TIVX_C71_P6", 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, "TIVX_C71_P7", 7},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, "TIVX_C71_P8", 8},                         \
    {TIVX_TARGET_IPU1_0, "TIVX_IPU1_0", 9},                                  \
    {TIVX_TARGET_MCU1_0, "TIVX_MCU1_0", 10},                                 \
    {TIVX_TARGET_MPU_0, "TIVX_MPU0", 11},                                   \
    {TIVX_TARGET_MPU_1, "TIVX_MPU1", 12},                                   \
    {TIVX_TARGET_MPU_2, "TIVX_MPU2", 13},                                   \
    {TIVX_TARGET_MPU_3, "TIVX_MPU3", 14},                                   \
    {TIVX_TARGET_VPAC_LDC1, "TIVX_VLDC", 15},                              \
    {TIVX_TARGET_VPAC_MSC1, "TIVX_VMSC1", 16},                              \
    {TIVX_TARGET_VPAC_MSC2, "TIVX_VMSC2", 17},                              \
    {TIVX_TARGET_VPAC_VISS1, "TIVX_VVISS", 18},                            \
    /* TIVX_TARGET_HOST should point to the number associated with          \
     * the host target thread in list above                                 \
     */                                                                     \
    {TIVX_TARGET_HOST, "TIVX_MPU0", 11},                                    \
}

#elif defined (SOC_J722S)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                    \
{                                                                           \
    {TIVX_TARGET_DSP1, "TIVX_DSP1", 0},                                      \
    {TIVX_TARGET_DSP_C7_1, "TIVX_C71_P1", 1},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, "TIVX_C71_P2", 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, "TIVX_C71_P3", 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, "TIVX_C71_P4", 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, "TIVX_C71_P5", 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, "TIVX_C71_P6", 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, "TIVX_C71_P7", 7},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, "TIVX_C71_P8", 8},                         \
    {TIVX_TARGET_DSP2, "TIVX_DSP2", 9},                                      \
    {TIVX_TARGET_DSP_C7_2, "TIVX_C72_P1", 10},                              \
    {TIVX_TARGET_DSP_C7_2_PRI_2, "TIVX_C72_P2", 11},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_3, "TIVX_C72_P3", 12},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_4, "TIVX_C72_P4", 13},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_5, "TIVX_C72_P5", 14},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_6, "TIVX_C72_P6", 15},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_7, "TIVX_C72_P7", 16},                        \
    {TIVX_TARGET_DSP_C7_2_PRI_8, "TIVX_C72_P8", 17},                        \
    {TIVX_TARGET_IPU1_0, "TIVX_IPU1_0", 18},                                 \
    {TIVX_TARGET_MCU1_0, "TIVX_MCU1_0", 19},                                \
    {TIVX_TARGET_MCU2_0, "TIVX_MCU2_0", 20},                                 \
    {TIVX_TARGET_MPU_0, "TIVX_MPU0", 21},                                   \
    {TIVX_TARGET_MPU_1, "TIVX_MPU1", 22},                                   \
    {TIVX_TARGET_MPU_2, "TIVX_MPU2", 23},                                   \
    {TIVX_TARGET_MPU_3, "TIVX_MPU3", 24},                                   \
    {TIVX_TARGET_VPAC_LDC1, "TIVX_VLDC", 25},                              \
    {TIVX_TARGET_VPAC_MSC1, "TIVX_VMSC1", 26},                              \
    {TIVX_TARGET_VPAC_MSC2, "TIVX_VMSC2", 27},                              \
    {TIVX_TARGET_DMPAC_SDE, "TIVX_SDE", 28},                                \
    {TIVX_TARGET_DMPAC_DOF, "TIVX_DOF", 29},                                \
    {TIVX_TARGET_VPAC_VISS1, "TIVX_VVISS", 30},                            \
    {TIVX_TARGET_CAPTURE1, "TIVX_CAPT1", 31},                               \
    {TIVX_TARGET_CAPTURE2, "TIVX_CAPT2", 32},                               \
    {TIVX_TARGET_CAPTURE3, "TIVX_CAPT3", 33},                               \
    {TIVX_TARGET_CAPTURE4, "TIVX_CAPT4", 34},                               \
    {TIVX_TARGET_DISPLAY1, "TIVX_DISP1", 35},                               \
    {TIVX_TARGET_DISPLAY2, "TIVX_DISP2", 36},                               \
    {TIVX_TARGET_CSITX, "TIVX_CSITX", 37},                                  \
    {TIVX_TARGET_CSITX2, "TIVX_CSITX2", 38},                                \
    /* TIVX_TARGET_HOST should point to the number associated with          \
     * the host target thread in list above                                 \
     */                                                                     \
    {TIVX_TARGET_HOST, "TIVX_MPU0", 21},                                    \
}

#endif


/*! \brief Function to trick target kernels into believing they are running
 *   on a DSP or EVE or such core in PC emulation mode
 *
 * \ingroup group_tivx_platform
 */
void tivxSetSelfCpuId(vx_enum cpu_id);

/*! \brief Convert a specific target ID to a task name
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformGetTaskName(vx_enum target_id, char *task_name);

#ifdef __cplusplus
}
#endif

#endif
