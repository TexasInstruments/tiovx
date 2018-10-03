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

static inline uint32_t tivxIpcPayloadMake(
    vx_enum dst_target_id, uint16_t obj_desc_id);
static inline uint16_t tivxIpcPayloadGetObjDescId(uint32_t payload);
static inline vx_enum tivxIpcPayloadGetTargetId(uint32_t payload);
static void tivxObjDescIpcHandler(uint32_t payload);

static tivx_obj_desc_table_info_t g_obj_desc_table;


static inline uint32_t tivxIpcPayloadMake(
    vx_enum dst_target_id, uint16_t obj_desc_id)
{

    /* Keep the desc id in the upper 16bits and target id in lower 8bits */
    return (uint32_t)(
                ( ( (uint32_t)obj_desc_id & TIVX_OBJ_DESC_ID_MASK )
                        << TIVX_OBJ_DESC_ID_SHIFT
                )
                     |
                ( ( (uint32_t)dst_target_id & TIVX_TARGET_ID_MASK )
                        << TIVX_TARGET_ID_SHIFT
                )
            );
}

static inline uint16_t tivxIpcPayloadGetObjDescId(uint32_t payload)
{
    return (uint16_t)
         (
            (payload >> TIVX_OBJ_DESC_ID_SHIFT) & TIVX_OBJ_DESC_ID_MASK
         );
}

static inline vx_enum tivxIpcPayloadGetTargetId(uint32_t payload)
{
    return (payload >> TIVX_TARGET_ID_SHIFT) & TIVX_TARGET_ID_MASK;
}

static void tivxObjDescIpcHandler(uint32_t payload)
{
    vx_enum dst_target_id;
    uint16_t obj_desc_id;

    obj_desc_id = tivxIpcPayloadGetObjDescId(payload);

    dst_target_id = tivxIpcPayloadGetTargetId(payload);

    /* now this is local target hence call target API directly */
    tivxTargetQueueObjDesc(dst_target_id, obj_desc_id);
}

void tivxObjDescInit(void)
{
    tivxPlatformGetObjDescTableInfo(&g_obj_desc_table);
    tivxIpcRegisterHandler(tivxObjDescIpcHandler);
}

tivx_obj_desc_t *tivxObjDescAlloc(vx_enum type, vx_reference ref)
{
    tivx_obj_desc_t *obj_desc = NULL, *tmp_obj_desc = NULL;
    uint32_t i, idx;

    tivxPlatformSystemLock(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE);

    idx = g_obj_desc_table.last_alloc_index;

    for(i=0; i<g_obj_desc_table.num_entries; i++)
    {
        tmp_obj_desc = (tivx_obj_desc_t*)&g_obj_desc_table.table_base[idx];

        if(tmp_obj_desc->type==TIVX_OBJ_DESC_INVALID)
        {
            memset(tmp_obj_desc, 0, sizeof(tivx_obj_desc_shm_entry_t));

            /* init entry that is found */
            tmp_obj_desc->obj_desc_id = idx;
            tmp_obj_desc->type = type;
            tmp_obj_desc->host_ref = (uint64_t)ref;

            g_obj_desc_table.last_alloc_index
                = (idx+1)%g_obj_desc_table.num_entries;

            obj_desc = tmp_obj_desc;
            break;
        }

        idx = (idx+1)%g_obj_desc_table.num_entries;
    }

    tivxPlatformSystemUnlock(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE);

    return obj_desc;
}

vx_status tivxObjDescFree(tivx_obj_desc_t **obj_desc)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if((NULL != obj_desc) && (NULL != *obj_desc))
    {
        if((*obj_desc)->obj_desc_id < g_obj_desc_table.num_entries)
        {
            /* valid object descriptor, free it */
            (*obj_desc)->type = TIVX_OBJ_DESC_INVALID;

            *obj_desc = NULL;

            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"tivxObjDescFree: object descriptor ID is greater than number of object descriptor table entries\n");
        }
    }

    return status;
}

tivx_obj_desc_t *tivxObjDescGet(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = NULL;

    if(obj_desc_id < g_obj_desc_table.num_entries)
    {
        obj_desc = (tivx_obj_desc_t*)&g_obj_desc_table.table_base[obj_desc_id];
    }

    return obj_desc;
}

vx_bool tivxObjDescIsValidType(tivx_obj_desc_t *obj_desc, tivx_obj_desc_type_e type)
{
    vx_bool is_valid = vx_false_e;

    if(    (NULL != obj_desc)
        && (obj_desc->type == type)
        && (obj_desc->obj_desc_id < g_obj_desc_table.num_entries))
    {
        is_valid = vx_true_e;
    }

    return is_valid;
}

vx_status tivxObjDescSend(uint32_t dst_target_id, uint16_t obj_desc_id)
{
    vx_enum cpu_id;
    uint32_t ipc_payload;
    vx_status status = VX_SUCCESS;

    cpu_id = tivxTargetGetCpuId(dst_target_id);

    if(cpu_id == tivxGetSelfCpuId())
    {
        /* target is on same CPU queue obj_desc using target APIs */
        status = tivxTargetQueueObjDesc(dst_target_id, obj_desc_id);
    }
    else
    {
        ipc_payload = tivxIpcPayloadMake(dst_target_id, obj_desc_id);

        /* target is on remote CPU, send using IPC */
        status = tivxIpcSendMsg(cpu_id, ipc_payload);
    }

    return status;
}

uint16_t tivxReferenceGetObjDescId(vx_reference ref)
{
    uint16_t obj_desc_id = TIVX_OBJ_DESC_INVALID;

    if (NULL != ref)
    {
        obj_desc_id = ref->obj_desc->obj_desc_id;
    }

    return (obj_desc_id);
}

void tivxGetObjDescList(uint16_t obj_desc_id[],
    tivx_obj_desc_t *obj_desc[], uint32_t num_desc_id)
{
    vx_uint32 i;

    for (i = 0u; i < num_desc_id; i ++)
    {
        if (obj_desc_id[i] < g_obj_desc_table.num_entries)
        {
            obj_desc[i] =
                (tivx_obj_desc_t*)&g_obj_desc_table.table_base[obj_desc_id[i]];
        }
    }
}
