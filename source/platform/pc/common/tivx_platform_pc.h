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




#ifndef _TIVX_PLATFORM_WINDOWS_H_
#define _TIVX_PLATFORM_WINDOWS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Windows Platform APIs
 */


/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_target.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS            (22u)


/*! \brief Maximum number obj descriptors that are present in shared memory
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST  (1024U)


/*! \brief Target ID for supported targets
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for CPU1 */
    TIVX_TARGET_ID_CPU1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0),

    /*! \brief target ID for CPU2 */
    TIVX_TARGET_ID_CPU2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 1),

    /*! \brief target ID for CPU3 */
    TIVX_TARGET_ID_CPU3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 2),

    /*! \brief target ID for CPU2 */
    TIVX_TARGET_ID_CPU4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 3),

} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_DSP2, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_EVE1, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_EVE2, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_EVE3, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_EVE4, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_IPU1_0, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_IPU1_1, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_IPU2, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_A15_0, TIVX_TARGET_ID_CPU1},                                  \
    {TIVX_TARGET_HOST, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_RESV00, TIVX_TARGET_ID_CPU2},                                 \
    {TIVX_TARGET_RESV01, TIVX_TARGET_ID_CPU3},                                 \
    {TIVX_TARGET_RESV02, TIVX_TARGET_ID_CPU3},                                 \
    {TIVX_TARGET_RESV03, TIVX_TARGET_ID_CPU4},                                 \
    {TIVX_TARGET_RESV04, TIVX_TARGET_ID_CPU4},                                 \
    {TIVX_TARGET_RESV05, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_RESV06, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_RESV07, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_RESV08, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_RESV09, TIVX_TARGET_ID_CPU1}                                 \
}


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
