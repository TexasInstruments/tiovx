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



#include <vx_internal.h>

/*! \brief Pointer to the IPC notify event handler.
 *         It can be registered using #tivxIpcRegisterHandler API
 * \ingroup group_tivx_ipc
 */
static tivx_ipc_handler_f g_ipc_handler = NULL;
static vx_enum g_cpu_id = TIVX_CPU_ID_DSP1;

void tivxIpcRegisterHandler(tivx_ipc_handler_f notifyCb)
{
    g_ipc_handler = notifyCb;
}

vx_status tivxIpcSendMsg(
    vx_enum cpu_id, uint32_t payload)
{
    vx_status status = VX_SUCCESS;

    if( cpu_id < TIVX_CPU_ID_MAX && g_ipc_handler != NULL)
    {
        g_ipc_handler(payload);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxIpcSendMsg: IPC send msg failed\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

void tivxSetSelfCpuId(vx_enum cpu_id)
{
    g_cpu_id = cpu_id;
}

vx_enum tivxGetSelfCpuId(void)
{
    return (g_cpu_id);
}

void tivxIpcInit(void)
{
}

void tivxIpcDeInit(void)
{
}

vx_bool tivxIsTargetEnabled(char target_name[])
{
    return (vx_true_e);
}
