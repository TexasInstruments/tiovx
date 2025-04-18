/*
*
* Copyright (c) 2021 - 2025 Texas Instruments Incorporated
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

#ifndef TIVX_TARGET_CONFIG_H_
#define TIVX_TARGET_CONFIG_H_

#include <vx_internal.h>

#if defined(SOC_J721E)
#include <soc/tivx_target_config_j721e.h>
#elif defined(SOC_J721S2)
#include <soc/tivx_target_config_j721s2.h>
#elif defined(SOC_J784S4)
#include <soc/tivx_target_config_j784s4.h>
#elif defined(SOC_AM62A)
#include <soc/tivx_target_config_am62a.h>
#elif defined(SOC_J722S)
#include <soc/tivx_target_config_j722s.h>
#elif defined(SOC_J742S2)
#include <soc/tivx_target_config_j742s2.h>
#else
#error Must define SOC_<SOC_NAME>, options: SOC_J721E, SOC_J721S2, SOC_J784S4, SOC_AM62A, SOC_J722S, SOC_J742S2
#endif

/*! \brief Create target ID.
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri);

/*! \brief Delete target ID.
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformDeleteTargetId(vx_enum target_id);

/*! \brief Set target ID for HOST.
 *
 *         Called during tivxHostInit()
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformSetHostTargetId(tivx_target_id_e host_target_id);

void ownPlatformCreateTargetsMpu(void);
void ownPlatformCreateTargetsC7(void);
void ownPlatformCreateTargetsR5f(void);
void ownPlatformCreateTargetsC66(void);

void ownPlatformDeleteTargetsMpu(void);
void ownPlatformDeleteTargetsC7(void);
void ownPlatformDeleteTargetsR5f(void);
void ownPlatformDeleteTargetsC66(void);

#endif
