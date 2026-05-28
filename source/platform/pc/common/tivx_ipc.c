/*
*
* Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include <tivx_platform_pc.h>
#include <utils/ipc/include/app_ipc.h>
#include <sys/prctl.h>
#include <stdio.h>

#if defined (SOC_FAMILY_TDA5)
#include <Std_Types.h>
#include <Ipc_Notify_Hal_Defaults.h>

uint32_t g_ipc_cpu_id_map[TIVX_CPU_ID_MAX] = {
    APP_IPC_CPU_C7x_1,
    APP_IPC_CPU_C7x_2,
    APP_IPC_CPU_C7x_3,
    APP_IPC_CPU_C7x_4,
    APP_IPC_CPU_MPU1_0,
    APP_IPC_CPU_RMCU0_0,
    APP_IPC_CPU_RMCU0_1,
    APP_IPC_CPU_RMCU1_0,
    APP_IPC_CPU_RMCU1_1,
    APP_IPC_CPU_RMCU2_0,
    APP_IPC_CPU_RMCU2_1,
    APP_IPC_CPU_MCU0_M55,
    APP_IPC_CPU_MCU1_M55,
    APP_IPC_CPU_MCU2_M55,
    APP_IPC_CPU_MCU3_M55,
    APP_IPC_CPU_MCU4_M55
};

uint32_t g_ipc_csl_id_map[TIVX_CPU_ID_MAX] = {
    CORE_ID_DSP0,
    CORE_ID_DSP1,
    CORE_ID_DSP2,
    CORE_ID_DSP3,
    CORE_ID_A720_0,
    CORE_ID_RMCU0_0,
    CORE_ID_RMCU0_1,
    CORE_ID_RMCU1_0,
    CORE_ID_RMCU1_1,
    CORE_ID_RMCU2_0,
    CORE_ID_RMCU2_1,
    CORE_ID_MCU0,
    CORE_ID_MCU1,
    CORE_ID_MCU2,
    CORE_ID_MCU3,
    CORE_ID_MCU4
};

static char* g_app_ipc_name_map[APP_IPC_CPU_MAX] =
{
    "INVALID\0",
    "TIVX_MCU0\0",
    "TIVX_MCU1\0",
    "TIVX_MCU2\0",
    "TIVX_MCU3\0",
    "TIVX_MCU4\0",
    "TIVX_R52P0\0",
    "INVALID\0",
    "INVALID\0",
    "INVALID\0",
    "INVALID\0",
    "INVALID\0",
    "TIVX_C71\0",
    "TIVX_C72\0",
    "TIVX_C73\0",
    "TIVX_C74\0",
};


static char* g_ipc_csl_name_map[CORE_ID_MAX] = {
    "INVALID",
    "TIVX_MCU0",
    "TIVX_MCU1",
    "TIVX_MCU2",
    "TIVX_MCU3",
    "TIVX_MCU4",
    "TIVX_C71",
    "TIVX_C72",
    "TIVX_C73",
    "TIVX_C74",
    "INVALID",
    "INVALID",
    "INVALID",
    "INVALID",
    "TIVX_R52P0",
    "INVALID",
    "INVALID",
    "INVALID",
    "INVALID",
    "INVALID",
};

static tivx_vdk_ipc_send_mbox_f gVdkIpcSendMbox = NULL;

void ownUpdateIpcSendMboxFunctionPtr(tivx_vdk_ipc_send_mbox_f ptr)
{
    gVdkIpcSendMbox = ptr;
}
#endif

/*! \brief Pointer to the IPC notify event handler.
 *         It can be registered using #ownIpcRegisterHandler API
 * \ingroup group_tivx_ipc
 */
static tivx_ipc_handler_f g_ipc_handler = NULL;
static vx_enum g_cpu_id = (vx_enum)TIVX_CPU_ID_DSP1;

#if defined (SOC_FAMILY_TDA5)
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
#endif

void ownIpcRegisterHandler(tivx_ipc_handler_f notifyCb)
{
    g_ipc_handler = notifyCb;
}

void ownIpcInit(void)
{
#if defined (SOC_FAMILY_TDA5)
    if (NULL != gVdkIpcSendMbox)
    {
        /* Register IPC Handler */
        (void)appIpcRegisterNotifyHandler(tivxIpcHandler);
    }
#endif
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
#if defined (SOC_FAMILY_TDA5)
    if(NULL != gVdkIpcSendMbox)
    {
        /* Un-Register IPC Handler */
        (void)appIpcRegisterNotifyHandler(NULL);
    }
#endif
}

vx_status ownIpcSendMsg(
    vx_enum cpu_id, uint32_t payload, uint32_t host_cpu_id, uint32_t host_port_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if( (cpu_id < (vx_enum)TIVX_CPU_ID_MAX) && (NULL != g_ipc_handler))
    {
#if defined (SOC_FAMILY_TDA5)
        if(NULL == gVdkIpcSendMbox)
        {
            g_ipc_handler(payload);
        }
        else
        {
            uint32_t app_cpu_id = g_ipc_cpu_id_map[cpu_id];
            if(cpu_id==(vx_enum)host_cpu_id)
            {
                status = appIpcSendNotifyPort(
                    app_cpu_id,
                    payload,
                    host_port_id);
            }
            else
            {
                status = appIpcSendNotify(
                    app_cpu_id,
                    payload);
            }
        }
#else
            g_ipc_handler(payload);
#endif
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "IPC send msg failed\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

void tivxSetSelfCpuId(vx_enum cpu_id)
{
    g_cpu_id = cpu_id;
}

uint16_t ownIpcGetHostPortId(uint16_t cpu_id)
{
    /* NOT used */
    return 0;
}

vx_enum tivxGetSelfCpuId(void)
{
    vx_enum self_cpu_id;

    #if defined (SOC_FAMILY_TDA5)
    if (NULL != gVdkIpcSendMbox)
    {
        self_cpu_id = (vx_enum)tivxVdkGetSelfOvxIpcCpuId();
    }
    else
    #endif /* #if defined (SOC_FAMILY_TDA5) */
    {
        self_cpu_id = g_cpu_id;
    }

    return self_cpu_id;
}

vx_bool ownIsCpuEnabled(uint32_t app_cpu_id)
{
    return ((vx_bool)vx_true_e);
}

void ownIpcGetCpuMap(uint32_t blank_map[TIVX_CPU_ID_MAX])
{
    uint32_t i;

    for (i = 0; i < TIVX_CPU_ID_MAX; i++)
    {
        blank_map[i] = TIVX_CPU_ID_DSP1;
    }
}

vx_bool tivxIsTargetEnabled(const char target_name[])
{
    return ((vx_bool)vx_true_e);
}

#if defined (SOC_FAMILY_TDA5)
int16_t tivxVdkGetSelfOvxIpcCpuId(void)
{
    int16_t cpu_id;
    char thread_name[16];

    if (prctl(PR_GET_NAME, thread_name) != 0U)
    {
        printf("prctl(PR_GET_NAME) failed\n");
        cpu_id = -1;
    }
    else
    {
        if (strncmp(thread_name, "TIVX_C7\0", 7) == 0U)
        {
            cpu_id = TIVX_CPU_ID_DSP1;
        }
        else
        {
            if (strncmp(thread_name, "ACK\0", 3) == 0U)
            {
                cpu_id = -1;
            }
            else
            {
                cpu_id = TIVX_CPU_ID_MCU0;
            }
        }
    }

    return cpu_id;
}

int16_t tivxVdkGetSelfAppIpcCpuId(void)
{
    int16_t cpu_id;
    char thread_name[16];

    if (prctl(PR_GET_NAME, thread_name) != 0U)
    {
        printf("prctl(PR_GET_NAME) failed\n");
        cpu_id = -1;
    }
    else
    {
        if (strncmp(thread_name, "TIVX_C7\0", 7) == 0U)
        {
            cpu_id = g_ipc_cpu_id_map[TIVX_CPU_ID_DSP_C7_1];
        }
        else
        {
            if (strncmp(thread_name, "ACK\0", 3) == 0U)
            {
                cpu_id = -1;
            }
            else
            {
                cpu_id = g_ipc_cpu_id_map[TIVX_CPU_ID_MCU0];

            }
        }
    }

    return cpu_id;
}

int16_t tivxVdkGetSelfCslIpcCpuId(void)
{
    int16_t cpu_id;
    char thread_name[16];

    if (prctl(PR_GET_NAME, thread_name) != 0U)
    {
        printf("prctl(PR_GET_NAME) failed\n");
        cpu_id = -1;
    }
    else
    {
        if (strncmp(thread_name, "TIVX_C7\0", 7) == 0U)
        {
            cpu_id = g_ipc_csl_id_map[TIVX_CPU_ID_DSP_C7_1];
        }
        else
        {
            if (strncmp(thread_name, "ACK\0", 3) == 0U)
            {
                cpu_id = -1;
            }
            else
            {
                cpu_id = g_ipc_csl_id_map[TIVX_CPU_ID_MCU0];

            }
        }
    }

    return cpu_id;
}
#endif
