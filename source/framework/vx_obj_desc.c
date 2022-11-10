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

static inline uint32_t ownIpcPayloadMake(
    vx_enum dst_target_id, uint16_t obj_desc_id);
static inline uint16_t ownIpcPayloadGetObjDescId(uint32_t payload);
static inline vx_enum ownIpcPayloadGetTargetId(uint32_t payload);
static void ownObjDescIpcHandler(uint32_t payload);

static tivx_obj_desc_table_info_t g_obj_desc_table;


static inline uint32_t ownIpcPayloadMake(
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

static inline uint16_t ownIpcPayloadGetObjDescId(uint32_t payload)
{
    return (uint16_t)
         (
            (payload >> TIVX_OBJ_DESC_ID_SHIFT) & TIVX_OBJ_DESC_ID_MASK
         );
}

static inline vx_enum ownIpcPayloadGetTargetId(uint32_t payload)
{
    vx_uint32 returnVal = (payload >> (uint32_t)TIVX_TARGET_ID_SHIFT) & (uint32_t)TIVX_TARGET_ID_MASK;
    return (vx_enum)returnVal;
}

static void ownObjDescIpcHandler(uint32_t payload)
{
    vx_enum dst_target_id;
    uint16_t obj_desc_id;
    vx_status status;

    obj_desc_id = ownIpcPayloadGetObjDescId(payload);

    dst_target_id = ownIpcPayloadGetTargetId(payload);

    /* now this is local target hence call target API directly */
    status = ownTargetQueueObjDesc(dst_target_id, obj_desc_id);

    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR,"ownTargetQueueObjDesc failed\n");
    }
}

void ownObjDescInit(void)
{
    ownPlatformGetObjDescTableInfo(&g_obj_desc_table);
    ownIpcRegisterHandler(ownObjDescIpcHandler);
}

tivx_obj_desc_t *ownObjDescAlloc(vx_enum type, vx_reference ref)
{
    tivx_obj_desc_t *obj_desc = NULL, *tmp_obj_desc = NULL;
    uint32_t i, idx, cpu_id;

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE);

    idx = g_obj_desc_table.last_alloc_index;

    for(i=0; i<g_obj_desc_table.num_entries; i++)
    {
        tmp_obj_desc = (tivx_obj_desc_t*)&g_obj_desc_table.table_base[idx];

        if((vx_enum)tmp_obj_desc->type==(vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            tivx_obj_desc_memset(tmp_obj_desc, 0, (uint32_t)sizeof(tivx_obj_desc_shm_entry_t));

            /* init entry that is found */
            tmp_obj_desc->obj_desc_id = (uint16_t)idx;
            tmp_obj_desc->scope_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            tmp_obj_desc->in_node_done_cnt = 0;
            tmp_obj_desc->element_idx = 0;
            tmp_obj_desc->type = (uint16_t)type;
            tmp_obj_desc->host_ref = (uint64_t)(uintptr_t)ref;
            for(cpu_id = 0; cpu_id<TIVX_OBJ_DESC_MAX_HOST_PORT_ID_CPU; cpu_id++)
            {
                tmp_obj_desc->host_port_id[cpu_id] = ownIpcGetHostPortId(cpu_id);
            }
            tmp_obj_desc->host_cpu_id  = (uint32_t)tivxGetSelfCpuId();
            tmp_obj_desc->timestamp = 0;

            g_obj_desc_table.last_alloc_index
                = (idx+1U)%g_obj_desc_table.num_entries;

            obj_desc = tmp_obj_desc;
            break;
        }

        idx = (idx+1U)%g_obj_desc_table.num_entries;
    }

    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE);

    return obj_desc;
}

vx_status ownObjDescFree(tivx_obj_desc_t **obj_desc)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE);

    if((NULL != obj_desc) && (NULL != *obj_desc))
    {
        if((*obj_desc)->obj_desc_id < g_obj_desc_table.num_entries)
        {
            /* valid object descriptor, free it */
            (*obj_desc)->type = (vx_enum)TIVX_OBJ_DESC_INVALID;

            *obj_desc = NULL;

            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "object descriptor ID is greater than number of object descriptor table entries\n");
        }
    }

    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE);

    return status;
}

tivx_obj_desc_t *ownObjDescGet(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = NULL;

    if(obj_desc_id < g_obj_desc_table.num_entries)
    {
        obj_desc = (tivx_obj_desc_t*)&g_obj_desc_table.table_base[obj_desc_id];
    }

    return obj_desc;
}

vx_bool ownObjDescIsValidType(const tivx_obj_desc_t *obj_desc, tivx_obj_desc_type_e type)
{
    vx_bool is_valid = (vx_bool)vx_false_e;

    if(    (NULL != obj_desc)
        && (obj_desc->type == (uint32_t)type)
        && (obj_desc->obj_desc_id < g_obj_desc_table.num_entries))
    {
        is_valid = (vx_bool)vx_true_e;
    }

    return is_valid;
}

vx_status ownObjDescSend(uint32_t dst_target_id, uint16_t obj_desc_id)
{
    vx_enum cpu_id, self_cpu_id;
    uint32_t ipc_payload;
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc;

    cpu_id = ownTargetGetCpuId((int32_t)dst_target_id);

    self_cpu_id = tivxGetSelfCpuId();

    if(cpu_id == self_cpu_id)
    {
        /* target is on same CPU queue obj_desc using target APIs */
        status = ownTargetQueueObjDesc((int32_t)dst_target_id, obj_desc_id);

        if(status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"ownTargetQueueObjDesc failed\n");
        }
    }
    else
    {
        ipc_payload = ownIpcPayloadMake((int32_t)dst_target_id, obj_desc_id);

        obj_desc = ownObjDescGet(obj_desc_id);

        if (NULL != obj_desc)
        {
            if(self_cpu_id < TIVX_OBJ_DESC_MAX_HOST_PORT_ID_CPU)
            {
                /* target is on remote CPU, send using IPC */
                status = ownIpcSendMsg(cpu_id, ipc_payload, obj_desc->host_cpu_id, obj_desc->host_port_id[self_cpu_id]);
            }
            else
            {
                status = VX_FAILURE;
            }

            if(status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,"ownIpcSendMsg failed\n");
            }
        }
        else
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR,"ownObjDescGet failed\n");
        }
    }

    return status;
}

uint16_t ownReferenceGetObjDescId(vx_reference ref)
{
    uint16_t obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if (NULL != ref)
    {
        obj_desc_id = ref->obj_desc->obj_desc_id;
    }

    return (obj_desc_id);
}

void tivxGetObjDescList(volatile uint16_t obj_desc_id[],
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

tivx_obj_desc_t *tivxGetObjDescElement(tivx_obj_desc_t *obj_desc, uint16_t elem_idx)
{
    tivx_obj_desc_t *elem = NULL;

    /* Check the type of the object. */
    if((vx_enum)obj_desc->type==(vx_enum)TIVX_OBJ_DESC_OBJARRAY)
    {
        tivx_obj_desc_object_array_t *obj_desc_obj_array;

        obj_desc_obj_array = (tivx_obj_desc_object_array_t *)obj_desc;

        /* Validate the element index requested. */
        if (elem_idx < obj_desc_obj_array->num_items)
        {
            tivxGetObjDescList(&obj_desc_obj_array->obj_desc_id[elem_idx],
                               (tivx_obj_desc_t**)&elem, 1);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "Requested element index [%d] out of range [0-%d]\n",
                     elem_idx, obj_desc_obj_array->num_items-1);
        }
    }
    else
    {
        if ((vx_enum)TIVX_OBJ_DESC_INVALID != (vx_enum)obj_desc->scope_obj_desc_id)
        {
            tivx_obj_desc_object_array_t *parent_obj_desc = NULL;

            tivxGetObjDescList(
                &obj_desc->scope_obj_desc_id,
                (tivx_obj_desc_t**)&parent_obj_desc, 1);

            if (parent_obj_desc != NULL)
            {
                tivxGetObjDescList(
                    &parent_obj_desc->obj_desc_id[elem_idx],
                    (tivx_obj_desc_t**)&elem, 1);
            }
        }
        else
        {
            /* The object passed is of the type tivx_obj_desc_t. Just return it. */
            elem = obj_desc;
        }
    }

    return elem;
}

void tivx_obj_desc_strncpy(volatile void *dst, volatile void *src, uint32_t size)
{
    volatile uint8_t *d=(volatile uint8_t*)dst;
    volatile uint8_t *s=(volatile uint8_t*)src;
    uint32_t i;

    for(i=0; i<(size-1U); i++)
    {
        d[i] = s[i];
        if(s[i]==0U)
        {
            break;
        }
    }
    d[i] = 0;
}

void tivx_obj_desc_memcpy(volatile void *dst, volatile void *src, uint32_t size)
{
    volatile uint8_t *d=(volatile uint8_t*)dst;
    volatile uint8_t *s=(volatile uint8_t*)src;
    uint32_t i;

    for(i=0; i<size; i++)
    {
        d[i] = s[i];
    }
}

void tivx_obj_desc_memset(volatile void *dst, uint8_t val, uint32_t size)
{
    volatile uint8_t *d=(volatile uint8_t*)dst;
    uint32_t i;

    for(i=0; i<size; i++)
    {
        d[i] = val;
    }
}

int32_t tivx_obj_desc_strncmp(volatile void *dst, volatile void *src, uint32_t size)
{
    volatile uint8_t *d=(volatile uint8_t*)dst;
    volatile uint8_t *s=(volatile uint8_t*)src;
    uint32_t i;
    int32_t ret = 0;

    for(i=0; i<size; i++)
    {
        if((d[i] != s[i]) || (d[i] == 0U) || (s[i] == 0U))
        {
            ret = ((int32_t)d[i] - (int32_t)s[i]);
            break;
        }
    }
    return ret;
}

int32_t tivx_obj_desc_strncmp_delim(volatile void *dst, volatile void *src, uint32_t size, char delim)
{
    volatile uint8_t *d=(volatile uint8_t*)dst;
    volatile uint8_t *s=(volatile uint8_t*)src;
    uint32_t i;
    int32_t ret = 0;

    for(i=0; i<size; i++)
    {
        if((d[i] != s[i]) || (d[i] == 0U) || (s[i] == 0U) || (d[i] == delim) || (s[i] == delim))
        {
            if((d[i] != delim) && (s[i] != delim))
            {
                ret = ((int32_t)d[i] - (int32_t)s[i]);
            }
            break;
        }
    }
    return ret;
}

