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

#include <utils/ipc/include/app_ipc.h>

static void tivxIpcHandler(uint32_t src_cpu_id, uint32_t payload);

/*! \brief An array of vision sdk CPU Ids. This is used for mapping
 *   tivx_cpu_id_e to vision sdk cpu id. This mapping is required for
 *   sending notify event using vision sdk API.
 *   vx_enum tivx_cpu_id_e is used as index into this array to get
 *   vision sdk cpu id.
 * \ingroup group_tivx_ipc
 */
#if defined (SOC_J721E)
uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C6x_1,
    APP_IPC_CPU_C6x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1
};
#endif

#if defined (SOC_J721S2)
uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1
};
#endif

#if defined (SOC_J784S4)
uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_MCU4_0,
    APP_IPC_CPU_MCU4_1,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4
};
#endif

#if defined (SOC_AM62A)
uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MPU1_0
};
#endif

#if defined (SOC_J722S)
uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_MCU1_0,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MPU1_0
};
#endif

#if defined (SOC_J742S2)
uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_MCU2_0,
    APP_IPC_CPU_MCU2_1,
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_MCU3_0,
    APP_IPC_CPU_MCU3_1,
    APP_IPC_CPU_MCU4_0,
    APP_IPC_CPU_MCU4_1,
    APP_IPC_CPU_C7x_3
};
#endif

/*! \brief Pointer to the IPC notify event handler.
 *         It can be registered using #ownIpcRegisterHandler API
 * \ingroup group_tivx_ipc
 */
static tivx_ipc_handler_f g_ipc_handler = NULL;


/*! \brief Global IPC handler
 * \ingroup group_tivx_ipc
 */
static void tivxIpcHandler(uint32_t src_cpu_id, uint32_t payload)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IPC_UBR001
<justification end> */
    if (NULL != g_ipc_handler)
    {
        g_ipc_handler(payload);
    }
/* LDRA_JUSTIFY_END */
}

void ownIpcRegisterHandler(tivx_ipc_handler_f notifyCb)
{
    g_ipc_handler = notifyCb;
}

vx_status ownIpcSendMsg(
    vx_enum cpu_id, uint32_t payload, uint32_t host_cpu_id, uint32_t host_port_id)
{
    /* convert OpenVX CPU ID to VSDK CPU ID */
    uint32_t vsdk_cpu_id;
    vx_status status;

    if( cpu_id < (vx_enum)TIVX_CPU_ID_MAX)
    {
        vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

        /* if dest CPU and the host CPU is same then send IPC message to
         * the specific host port ID so that the process (Linux) which
         * created the object descriptor gets the response
         *
         * if dest CPU is not host CPU then this is some other RTOS CPU,
         * in this case send to the common global TIOVX port ID used by all
         * RTOS
         */
        if(cpu_id==(vx_enum)host_cpu_id)
        {
            status = appIpcSendNotifyPort(
                vsdk_cpu_id,
                payload,
                host_port_id);
        }
        else
        {
            status = appIpcSendNotify(
                vsdk_cpu_id,
                payload);
        }

        if( status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "OpenVX send notification failed\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "CPU ID invalid\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_enum tivxGetSelfCpuId(void)
{
    vx_enum cpu_id = (vx_enum)TIVX_CPU_ID_INVALID;
    uint32_t i, vsdk_cpu_id;

    vsdk_cpu_id =  appIpcGetSelfCpuId();

    for (i = 0;
#if defined(C7X_FAMILY) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IPC_UBR002
<justification end> */
#endif
            i < dimof(g_ipc_cpu_id_map);
#if defined(C7X_FAMILY) || defined(C66)
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_C7X_UM001
<justification end> */
#endif
            i ++)
#if defined(C7X_FAMILY) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
    {
#if defined(C7X_FAMILY) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_C7X_TIVX_IPC_UBR001
<justification end> */
#endif
        if (vsdk_cpu_id == g_ipc_cpu_id_map[i])
        {
            cpu_id = (vx_enum)i;
            break;
        }
#if defined(C7X_FAMILY) || defined(C66)
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_C7X_UM001
<justification end> */
#endif
    }
#if defined(C7X_FAMILY) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif

    return (cpu_id);
}

uint16_t ownIpcGetHostPortId(uint16_t cpu_id)
{
    /* convert OpenVX CPU ID to VSDK CPU ID */
    uint32_t vsdk_cpu_id;
    uint32_t host_port_id = 0;

    if((int32_t)cpu_id < (vx_enum)TIVX_CPU_ID_MAX)
    {
        vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

        host_port_id = appIpcGetHostPortId((uint16_t)vsdk_cpu_id);
    }

    /* host port ID is 16b max to this type conversion is ok */
    return (uint16_t)host_port_id;
}

void ownIpcInit(void)
{
    /* Register IPC Handler */
    (void)appIpcRegisterNotifyHandler(tivxIpcHandler);
}

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY
<metric start> statement <metric end>
<function start> void ownIpcDeInit.* <function end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_IPC_UM001
<justification end> */
#endif
void ownIpcDeInit(void)
{
    /* Un-Register IPC Handler */
    (void)appIpcRegisterNotifyHandler(NULL);
}

vx_bool ownIsCpuEnabled(uint32_t app_cpu_id)
{
    uint32_t vsdk_cpu_id;

    vsdk_cpu_id = app_cpu_id;
    return (vx_bool)appIpcIsCpuEnabled(vsdk_cpu_id);
}

vx_bool tivxIsTargetEnabled(const char target_name[])
{
    vx_bool isEnabled = (vx_bool)vx_false_e;
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;
    vx_enum cpu_id;
    uint32_t vsdk_cpu_id;
    uint32_t vsdk_isenabled;

    if (NULL != target_name)
    {
        /* Get the targetId */
        target_id = ownPlatformGetTargetId(target_name);
        if (target_id != (vx_enum)TIVX_TARGET_ID_INVALID)
        {
            cpu_id = ownTargetGetCpuId(target_id);
            if( cpu_id < (vx_enum)TIVX_CPU_ID_MAX) /* TIOVX-1948- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_IPC_UBR004 */
            {
                vsdk_cpu_id  = g_ipc_cpu_id_map[cpu_id];

                vsdk_isenabled = appIpcIsCpuEnabled(vsdk_cpu_id);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_IPC_UBR005
<justification end> */
                if (1U == vsdk_isenabled)
/* LDRA_JUSTIFY_END */
                {
                    isEnabled = (vx_bool)vx_true_e;
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_IPC_UM001
<justification end> */
                else
                {
                    isEnabled = (vx_bool)vx_false_e;
                }
/* LDRA_JUSTIFY_END */
            }
        }
    }
    return (isEnabled);
}
