/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>



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

void tivxObjDescInit()
{
    tivxPlatformGetObjDescTableInfo(&g_obj_desc_table);
    tivxIpcRegisterHandler(tivxObjDescIpcHandler);
}

tivx_obj_desc_t *tivxObjDescAlloc(vx_enum type)
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
            /* free entry found */
            tmp_obj_desc->obj_desc_id = idx;
            tmp_obj_desc->type = type;

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

    if(obj_desc && *obj_desc)
    {
        if((*obj_desc)->obj_desc_id < g_obj_desc_table.num_entries)
        {
            /* valid object descriptor, free it */
            (*obj_desc)->type = TIVX_OBJ_DESC_INVALID;

            *obj_desc = NULL;

            status = VX_SUCCESS;
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

    if(    obj_desc
        && obj_desc->type == type
        && obj_desc->obj_desc_id < g_obj_desc_table.num_entries)
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
