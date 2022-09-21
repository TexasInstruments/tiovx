/*
 *
 * Copyright (c) 2021-2021 Texas Instruments Incorporated
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

#ifndef TIVX_SOC_J721S2_H_
#define TIVX_SOC_J721S2_H_

#ifndef SOC_J721S2
#define SOC_J721S2
#endif

#ifndef VPAC3
#define VPAC3
#endif

#include <TI/tivx.h>
#include <TI/j7_kernels.h>
#include <TI/j7_nodes.h>
#include <TI/j7_vpac_ldc.h>
#include <TI/j7_vpac_msc.h>
#include <TI/j7_vpac_viss.h>
#include <TI/j7_vpac_nf.h>
#include <TI/j7_dmpac_dof.h>
#include <TI/j7_dmpac_sde.h>
#include <TI/j7_capture.h>
#include <TI/j7_csitx.h>
#include <TI/j7_display.h>
#include <TI/j7_display_m2m.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to j721s2 soc TI extension APIs
 */

/*! \brief Target name for A72_0
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_A72_0      "A72-0"

/*! \brief Target name for A72_1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_A72_1      "A72-1"

/*! \brief Target name for A72_2
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_A72_2      "A72-2"

/*! \brief Target name for A72_3
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_A72_3      "A72-3"

/*! \brief Target name for DSP_C7_1
 *         This target task is first in priority.
 *         Each of the C7X targets are assigned a different
 *         task priority.  Subsequent C7X targets are assigned
 *         lower priority than the preceding target (i.e.,
 *         \ref TIVX_TARGET_DSP_C7_1_PRI_2 is lower priority
 *         than \ref TIVX_TARGET_DSP_C7_1_PRI_1).  Therefore,
 *         the \ref TIVX_TARGET_DSP_C7_1_PRI_2 target will be
 *         preempted by \ref TIVX_TARGET_DSP_C7_1_PRI_1 once
 *         the higher priority target is unblocked to execute.
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1    "DSP_C7-1"

/*! \brief Target name for DSP_C7_1
 *         This target task is first in priority.
 *         This aliases to the same task as \ref TIVX_TARGET_DSP_C7_1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_1    TIVX_TARGET_DSP_C7_1

/*! \brief Target name for DSP_C7-1_PRI_2
 *         This target task is second in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_2    "DSP_C7-1_PRI_2"

/*! \brief Target name for DSP_C7-1_PRI_3
 *         This target task is third in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_3    "DSP_C7-1_PRI_3"

/*! \brief Target name for DSP_C7-1_PRI_4
 *         This target task is fourth in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_4    "DSP_C7-1_PRI_4"

/*! \brief Target name for DSP_C7-1_PRI_5
 *         This target task is fifth in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_5    "DSP_C7-1_PRI_5"

/*! \brief Target name for DSP_C7-1_PRI_6
 *         This target task is sixth in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_6    "DSP_C7-1_PRI_6"

/*! \brief Target name for DSP_C7-1_PRI_7
 *         This target task is seventh in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_7    "DSP_C7-1_PRI_7"

/*! \brief Target name for DSP_C7-1_PRI_8
 *         This target task is eighth in priority
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP_C7_1_PRI_8    "DSP_C7-1_PRI_8"

/*! \brief Name for DSP target class, instance 1
 *
 *   Assigned to DSP_C7_2 core (C7x without an MMA)
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP1        "DSP-1"

/*! \brief Name for MCU2 target class, core 0
 *
 *   Assigned to MCU2_0 core
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_MCU2_0      "MCU2-0"

/*! \brief Name for IPU1 target class, core 0
 *
 *   Assigned to IPU1_0 core
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_IPU1_0      "MCU2-0"

/*! \brief Name for MCU2 target class, core 1
 *
 *   Assigned to MCU2_1 core
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_MCU2_1      "MCU2-1"

/*! \brief Name for IPU1 target class, core 1
 *
 *   Assigned to IPU1_1 core
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_IPU1_1      "MCU2-1"

/*! \brief Name for MCU3_0 target class, core 0
 *
 *   Assigned to MCU3_0 core
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_MCU3_0      "MCU3-0"

/*! \brief Name for MCU3_1 target class, core 1
 *
 *   Assigned to MCU3_1 core
 *
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_MCU3_1      "MCU3-1"

/* This is a compatibility feature used only for
 * testing purposes and is aliased to the standalone
 * C7 DSP
 */
#define TIVX_TARGET_DSP2        TIVX_TARGET_DSP1

/*! \brief Target name for Capture
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE1     "CAPTURE1"

/*! \brief Target name for Capture
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE2     "CAPTURE2"

/*! \brief Target name for Capture Node Instance 3
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE3     "CAPTURE3"

/*! \brief Target name for Capture Node Instance 4
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE4     "CAPTURE4"

/*! \brief Target name for Capture Node Instance 5
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE5     "CAPTURE5"

/*! \brief Target name for Capture Node Instance 6
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE6     "CAPTURE6"

/*! \brief Target name for Capture Node Instance 7
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE7     "CAPTURE7"

/*! \brief Target name for Capture Node Instance 8
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CAPTURE8     "CAPTURE8"

/*! \brief Target name for CSITX
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CSITX     "CSITX"

/*! \brief Target name for CSITX2
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_CSITX2     "CSITX2"

/*! \brief Target name for Display M2M Node Instance 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DISPLAY_M2M1     "DSS_M2M1"

/*! \brief Target name for Display M2M Node Instance 2
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DISPLAY_M2M2     "DSS_M2M2"

/*! \brief Target name for Display M2M Node Instance 3
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DISPLAY_M2M3     "DSS_M2M3"

/*! \brief Target name for Display M2M Node Instance 4
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DISPLAY_M2M4     "DSS_M2M4"

/*! \brief Target name for Display
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DISPLAY1     "DISPLAY1"

/*! \brief Target name for Display
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DISPLAY2     "DISPLAY2"

/*! \brief Target name for DMPAC DOF
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DMPAC_DOF    "DMPAC_DOF"

/*! \brief Target name for DMPAC SDE
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DMPAC_SDE    "DMPAC_SDE"

/*! \brief Target name for VPAC LDC1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_VPAC_LDC1    "VPAC_LDC1"

/*! \brief Target name for VPAC MSC1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_VPAC_MSC1    "VPAC_MSC1"

/*! \brief Target name for VPAC MSC2
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_VPAC_MSC2    "VPAC_MSC2"

/*! \brief Target name for VPAC NF
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_VPAC_NF      "VPAC_NF"

/*! \brief Target name for VPAC VISS1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_VPAC_VISS1   "VPAC_VISS1"

/*! \brief CPU ID for supported CPUs
 *
 *         CPU ID is defined in platform module since
 *         depending on platform the CPUs could be different
 *
 *         Current CPU IDs are defined assuming J721S2 SoC
 *
 *         Caution: This enum is used as index into the array
 *         #g_ipc_cpu_id_map, so change in this enum will require
 *         change in this array as well.
 *
 *
 * \ingroup group_tivx_ext_host
 */
typedef enum _tivx_cpu_id_e {

    /*! \brief CPU ID for DSP1 */
    TIVX_CPU_ID_DSP1 = 0,

    /*! \brief CPU ID for C7_1 */
    TIVX_CPU_ID_DSP_C7_1 = 1,

    /*! \brief CPU ID for MCU2_0 */
    TIVX_CPU_ID_MCU2_0 = 2,

    /*! \brief CPU ID for IPU1_0 */
    TIVX_CPU_ID_IPU1_0 = TIVX_CPU_ID_MCU2_0,

    /*! \brief CPU ID for MCU2_1 */
    TIVX_CPU_ID_MCU2_1 = 3,

    /*! \brief CPU ID for IPU1_1 */
    TIVX_CPU_ID_IPU1_1 = TIVX_CPU_ID_MCU2_1,

    /*! \brief CPU ID for A72-0 */
    TIVX_CPU_ID_A72_0 = 4,

    /*! \brief CPU ID for MCU3_0 */
    TIVX_CPU_ID_MCU3_0 = 5,

    /*! \brief CPU ID for MCU3_1 */
    TIVX_CPU_ID_MCU3_1 = 6,

    /*! \brief Max value of CPU ID  */
    TIVX_CPU_ID_MAX = 7,

    /*! \brief Invalid CPU ID */
    TIVX_CPU_ID_INVALID = 0xFF

} tivx_cpu_id_e;


#ifdef __cplusplus
}
#endif

#endif
