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
#define TIVX_PLATFORM_MAX_TARGETS            (44u)
#elif defined (SOC_J721S2)
#define TIVX_PLATFORM_MAX_TARGETS            (43u)
#elif defined (SOC_J6)
#define TIVX_PLATFORM_MAX_TARGETS            (11u)
#elif defined (SOC_AM62A)
#define TIVX_PLATFORM_MAX_TARGETS            (20u)
#endif

/*! \brief Maximum number obj descriptors that are present in shared memory
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST  (1024U)


#if defined(SOC_J6)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                     \
{                                                            \
    {TIVX_TARGET_DSP1,   0},                                 \
    {TIVX_TARGET_DSP2,   1},                                 \
    {TIVX_TARGET_EVE1,   2},                                 \
    {TIVX_TARGET_EVE2,   3},                                 \
    {TIVX_TARGET_EVE3,   4},                                 \
    {TIVX_TARGET_EVE4,   5},                                 \
    {TIVX_TARGET_IPU1_0, 6},                                 \
    {TIVX_TARGET_IPU1_1, 7},                                 \
    {TIVX_TARGET_IPU2,   8},                                 \
    {TIVX_TARGET_A15_0,  9},                                 \
    /* TIVX_TARGET_HOST should point to the number associated with  \
     * the host target thread in list above                         \
     */                                                             \
    {TIVX_TARGET_HOST,  9},                                 \
}

#elif defined (SOC_J721E)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, 0},                                   \
    {TIVX_TARGET_DSP2, 1},                                   \
    {TIVX_TARGET_DSP_C7_1, 2},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, 7},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, 8},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, 9},                         \
    {TIVX_TARGET_IPU1_0, 10},                                 \
    {TIVX_TARGET_IPU1_1, 11},                                 \
    {TIVX_TARGET_MCU2_0, 12},                                 \
    {TIVX_TARGET_MCU2_1, 13},                                 \
    {TIVX_TARGET_MCU3_0, 14},                                 \
    {TIVX_TARGET_MCU3_1, 15},                                 \
    {TIVX_TARGET_A72_0, 16},                                  \
    {TIVX_TARGET_A72_1, 17},                                  \
    {TIVX_TARGET_A72_2, 18},                                  \
    {TIVX_TARGET_A72_3, 19},                                  \
    {TIVX_TARGET_VPAC_NF, 20},                                \
    {TIVX_TARGET_VPAC_LDC1, 21},                              \
    {TIVX_TARGET_VPAC_MSC1, 22},                              \
    {TIVX_TARGET_VPAC_MSC2, 23},                              \
    {TIVX_TARGET_DMPAC_SDE, 24},                              \
    {TIVX_TARGET_DMPAC_DOF, 25},                              \
    {TIVX_TARGET_VPAC_VISS1, 26},                             \
    {TIVX_TARGET_CAPTURE1, 27},                               \
    {TIVX_TARGET_CAPTURE2, 28},                               \
    {TIVX_TARGET_CAPTURE3, 29},                               \
    {TIVX_TARGET_CAPTURE4, 30},                               \
    {TIVX_TARGET_CAPTURE5, 31},                               \
    {TIVX_TARGET_CAPTURE6, 32},                               \
    {TIVX_TARGET_CAPTURE7, 33},                               \
    {TIVX_TARGET_CAPTURE8, 34},                               \
    {TIVX_TARGET_DISPLAY1, 35},                               \
    {TIVX_TARGET_DISPLAY2, 36},                               \
    {TIVX_TARGET_DISPLAY_M2M1, 37},                           \
    {TIVX_TARGET_DISPLAY_M2M2, 38},                           \
    {TIVX_TARGET_DISPLAY_M2M3, 39},                           \
    {TIVX_TARGET_DISPLAY_M2M4, 40},                           \
    {TIVX_TARGET_CSITX, 41},                                  \
    /* TIVX_TARGET_HOST should point to the number associated with  \
     * the host target thread in list above                         \
     */                                                             \
    {TIVX_TARGET_HOST, 16},                                   \
}

#elif defined (SOC_J721S2)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, 0},                                   \
    {TIVX_TARGET_DSP_C7_1, 1},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, 7},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, 8},                         \
    {TIVX_TARGET_IPU1_0, 9},                                 \
    {TIVX_TARGET_IPU1_1, 10},                                \
    {TIVX_TARGET_MCU2_0, 11},                                \
    {TIVX_TARGET_MCU2_1, 12},                                \
    {TIVX_TARGET_MCU3_0, 13},                                \
    {TIVX_TARGET_MCU3_1, 14},                                \
    {TIVX_TARGET_A72_0, 15},                                 \
    {TIVX_TARGET_A72_1, 16},                                 \
    {TIVX_TARGET_A72_2, 17},                                 \
    {TIVX_TARGET_A72_3, 18},                                 \
    {TIVX_TARGET_VPAC_NF, 19},                               \
    {TIVX_TARGET_VPAC_LDC1, 20},                             \
    {TIVX_TARGET_VPAC_MSC1, 21},                             \
    {TIVX_TARGET_VPAC_MSC2, 22},                             \
    {TIVX_TARGET_DMPAC_SDE, 23},                             \
    {TIVX_TARGET_DMPAC_DOF, 24},                             \
    {TIVX_TARGET_VPAC_VISS1, 25},                            \
    {TIVX_TARGET_CAPTURE1, 26},                              \
    {TIVX_TARGET_CAPTURE2, 27},                              \
    {TIVX_TARGET_CAPTURE3, 28},                              \
    {TIVX_TARGET_CAPTURE4, 29},                              \
    {TIVX_TARGET_CAPTURE5, 30},                              \
    {TIVX_TARGET_CAPTURE6, 31},                              \
    {TIVX_TARGET_CAPTURE7, 32},                              \
    {TIVX_TARGET_CAPTURE8, 33},                              \
    {TIVX_TARGET_DISPLAY1, 34},                              \
    {TIVX_TARGET_DISPLAY2, 35},                              \
    {TIVX_TARGET_DISPLAY_M2M1, 36},                          \
    {TIVX_TARGET_DISPLAY_M2M2, 37},                          \
    {TIVX_TARGET_DISPLAY_M2M3, 38},                          \
    {TIVX_TARGET_DISPLAY_M2M4, 39},                          \
    {TIVX_TARGET_CSITX, 40},                                 \
    {TIVX_TARGET_CSITX2, 41},                                \
    /* TIVX_TARGET_HOST should point to the number associated with  \
     * the host target thread in list above                         \
     */                                                             \
    {TIVX_TARGET_HOST, 15},                                   \
}

#elif defined (SOC_J784S4)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP_C7_1, 0},                               \
    {TIVX_TARGET_DSP_C7_1_PRI_2, 1},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_3, 2},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_4, 3},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_5, 4},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_6, 5},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_7, 6},                         \
    {TIVX_TARGET_DSP_C7_1_PRI_8, 7},                         \
    {TIVX_TARGET_DSP_C7_2, 8},                               \
    {TIVX_TARGET_DSP_C7_2_PRI_2, 9},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_3, 10},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_4, 11},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_5, 12},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_6, 13},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_7, 14},                         \
    {TIVX_TARGET_DSP_C7_2_PRI_8, 15},                         \
    {TIVX_TARGET_DSP_C7_3, 16},                               \
    {TIVX_TARGET_DSP_C7_3_PRI_2, 17},                         \
    {TIVX_TARGET_DSP_C7_3_PRI_3, 18},                         \
    {TIVX_TARGET_DSP_C7_3_PRI_4, 19},                         \
    {TIVX_TARGET_DSP_C7_3_PRI_5, 20},                         \
    {TIVX_TARGET_DSP_C7_3_PRI_6, 21},                         \
    {TIVX_TARGET_DSP_C7_3_PRI_7, 22},                         \
    {TIVX_TARGET_DSP_C7_3_PRI_8, 23},                         \
    {TIVX_TARGET_DSP_C7_4, 24},                               \
    {TIVX_TARGET_DSP_C7_4_PRI_2, 25},                         \
    {TIVX_TARGET_DSP_C7_4_PRI_3, 26},                         \
    {TIVX_TARGET_DSP_C7_4_PRI_4, 27},                         \
    {TIVX_TARGET_DSP_C7_4_PRI_5, 28},                         \
    {TIVX_TARGET_DSP_C7_4_PRI_6, 29},                         \
    {TIVX_TARGET_DSP_C7_4_PRI_7, 30},                         \
    {TIVX_TARGET_DSP_C7_4_PRI_8, 31},                         \
    {TIVX_TARGET_MCU2_0, 32},                                 \
    {TIVX_TARGET_MCU2_1, 33},                                 \
    {TIVX_TARGET_MCU3_0, 34},                                 \
    {TIVX_TARGET_MCU3_1, 35},                                 \
    {TIVX_TARGET_MCU4_0, 36},                                 \
    {TIVX_TARGET_MCU4_1, 37},                                 \
    {TIVX_TARGET_A72_0, 38},                                  \
    {TIVX_TARGET_A72_1, 39},                                  \
    {TIVX_TARGET_A72_2, 40},                                  \
    {TIVX_TARGET_A72_3, 41},                                  \
    {TIVX_TARGET_VPAC_NF, 42},                                \
    {TIVX_TARGET_VPAC_LDC1, 43},                              \
    {TIVX_TARGET_VPAC_MSC1, 44},                              \
    {TIVX_TARGET_VPAC_MSC2, 45},                              \
    {TIVX_TARGET_VPAC2_NF, 46},                               \
    {TIVX_TARGET_VPAC2_LDC1, 47},                             \
    {TIVX_TARGET_VPAC2_MSC1, 48},                             \
    {TIVX_TARGET_VPAC2_MSC2, 49},                             \
    {TIVX_TARGET_DMPAC_SDE, 50},                              \
    {TIVX_TARGET_DMPAC_DOF, 51},                              \
    {TIVX_TARGET_VPAC_VISS1, 52},                             \
    {TIVX_TARGET_VPAC2_VISS1, 53},                             \
    {TIVX_TARGET_CAPTURE1, 54},                               \
    {TIVX_TARGET_CAPTURE2, 55},                               \
    {TIVX_TARGET_CAPTURE3, 56},                               \
    {TIVX_TARGET_CAPTURE4, 57},                               \
    {TIVX_TARGET_CAPTURE5, 58},                               \
    {TIVX_TARGET_CAPTURE6, 59},                               \
    {TIVX_TARGET_CAPTURE7, 60},                               \
    {TIVX_TARGET_CAPTURE8, 61},                               \
    {TIVX_TARGET_CAPTURE9, 62},                               \
    {TIVX_TARGET_CAPTURE10, 63},                               \
    {TIVX_TARGET_CAPTURE11, 64},                               \
    {TIVX_TARGET_CAPTURE12, 65},                               \
    {TIVX_TARGET_DISPLAY1, 66},                               \
    {TIVX_TARGET_DISPLAY2, 67},                               \
    {TIVX_TARGET_DISPLAY_M2M1, 68},                          \
    {TIVX_TARGET_DISPLAY_M2M2, 69},                          \
    {TIVX_TARGET_DISPLAY_M2M3, 70},                          \
    {TIVX_TARGET_DISPLAY_M2M4, 71},                          \
    {TIVX_TARGET_CSITX, 72},                                 \
    {TIVX_TARGET_CSITX2, 73},                                 \
    /* TIVX_TARGET_HOST should point to the number associated with  \
     * the host target thread in list above                         \
     */                                                             \
    {TIVX_TARGET_HOST, 38},                                  \
}

#elif defined (SOC_AM62A)

/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, 0},                                    \
    {TIVX_TARGET_DSP_C7_1, 1},                                \
    {TIVX_TARGET_DSP_C7_1_PRI_2, 2},                          \
    {TIVX_TARGET_DSP_C7_1_PRI_3, 3},                          \
    {TIVX_TARGET_DSP_C7_1_PRI_4, 4},                          \
    {TIVX_TARGET_DSP_C7_1_PRI_5, 5},                          \
    {TIVX_TARGET_DSP_C7_1_PRI_6, 6},                          \
    {TIVX_TARGET_DSP_C7_1_PRI_7, 7},                          \
    {TIVX_TARGET_DSP_C7_1_PRI_8, 8},                          \
    {TIVX_TARGET_IPU1_0, 9},                                  \
    {TIVX_TARGET_MCU1_0, 10},                                  \
    {TIVX_TARGET_A72_0, 11},                                  \
    {TIVX_TARGET_A72_1, 12},                                  \
    {TIVX_TARGET_A72_2, 13},                                  \
    {TIVX_TARGET_A72_3, 14},                                  \
    {TIVX_TARGET_VPAC_LDC1, 15},                              \
    {TIVX_TARGET_VPAC_MSC1, 16},                              \
    {TIVX_TARGET_VPAC_MSC2, 17},                              \
    {TIVX_TARGET_VPAC_VISS1, 18},                             \
    /* TIVX_TARGET_HOST should point to the number associated with  \
     * the host target thread in list above                         \
     */                                                             \
    {TIVX_TARGET_HOST, 11},                                   \
}

#endif


/*! \brief Function to trick target kernels into beliving they are running
 *   on a DSP or EVE or such core in PC emulation mode
 *
 * \ingroup group_tivx_platform
 */
void tivxSetSelfCpuId(vx_enum cpu_id);

#ifdef __cplusplus
}
#endif

#endif
