/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_vision_sdk.h>

/*! \brief Structure for keeping track of platform information
 *         Currently it is mainly used for mapping target id and target name
 * \ingroup group_tivx_platform
 */
typedef struct tivx_platform_info
{
    struct {
        /*! \brief Name of the target, defined in tivx.h file
         */
        char target_name[TIVX_TARGET_MAX_NAME];
        /*! \brief Id of the target defined in #tivx_target_id_e in the
         *   file tivx_platform_vision_sdk.h
         */
        vx_enum target_id;
    } target_info[TIVX_PLATFORM_MAX_TARGETS];

    /*! \brief Platform locks to protect access to the descriptor id
     *   TODO: Replace this with the H/W spin lock
     */
    tivx_mutex g_platform_lock[TIVX_PLATFORM_LOCK_MAX];
} tivx_platform_info_t;


/*! \brief Global instance of platform information
 * \ingroup group_tivx_platform
 */
static tivx_platform_info_t g_tivx_platform_info =
{
    TIVX_TARGET_INFO
};

tivx_obj_desc_shm_entry_t gTivxObjDescShmEntry
    [TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST];
#pragma DATA_SECTION(gTivxObjDescShmEntry, ".bss:extMemNonCache:tiovxObjDescShm");
#pragma DATA_ALIGN(gTivxObjDescShmEntry, 32);

vx_status tivxPlatformInit(void)
{
    vx_status status;
    uint32_t i = 0;

    /* Build time check to see if the structure size is 8byte aligned and size of the elements is not more than  */
    BUILD_ASSERT(
    ((sizeof(tivx_obj_desc_shm_entry_t)) % (TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN))
        == 0);
    BUILD_ASSERT(
    (sizeof(tivx_obj_desc_shm_entry_t)) <= TIVX_PLATFORM_MAX_SHM_ENTRY_SIZE);

    for (i = 0; i < TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);

        if (VX_SUCCESS != status)
        {
            tivxPlatformDeInit();
            break;
        }
    }

    tivxIpcInit();

    return (status);
}


void tivxPlatformDeInit(void)
{
    uint32_t i;

    tivxIpcDeInit();

    for (i = 0; i < TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
    }
}

void tivxPlatformSystemLock(vx_enum lock_id)
{
    if ((uint32_t)lock_id < TIVX_PLATFORM_LOCK_MAX)
    {
        tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);
    }
}

void tivxPlatformSystemUnlock(vx_enum lock_id)
{
    if ((uint32_t)lock_id < TIVX_PLATFORM_LOCK_MAX)
    {
        tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
            (uint32_t)lock_id]);
    }
}

vx_enum tivxPlatformGetTargetId(const char *target_name)
{
    uint32_t i;
    vx_enum target_id = TIVX_TARGET_ID_INVALID;

    if (NULL != target_name)
    {
        for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
        {
            if (0 == strncmp(g_tivx_platform_info.target_info[i].target_name,
                    target_name,
                    TIVX_TARGET_MAX_NAME))
            {
                target_id = g_tivx_platform_info.target_info[i].target_id;
                break;
            }
        }
    }

    return (target_id);
}

vx_bool tivxPlatformTargetMatch(
    const char *kernel_target_name, const char *target_string)
{
    vx_bool status = vx_false_e;
    uint32_t i;

    if ((NULL != kernel_target_name) && (NULL != target_string))
    {
        if (0 == strncmp(kernel_target_name, target_string,
            TIVX_TARGET_MAX_NAME))
        {
            for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
            {
                if (0 == strncmp(
                        g_tivx_platform_info.target_info[i].target_name,
                        target_string,
                        TIVX_TARGET_MAX_NAME))
                {
                    status = vx_true_e;
                    break;
                }
            }
        }
    }

    return (status);
}

void tivxPlatformGetObjDescTableInfo(tivx_obj_desc_table_info_t *table_info)
{
    if (NULL != table_info)
    {
        table_info->table_base = gTivxObjDescShmEntry;
        table_info->num_entries = TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST;

        /* Change this according available entries*/
        table_info->last_alloc_index = 0U;
    }

#if defined (M4)
    {
        tivx_obj_desc_t *tmp_obj_desc = NULL;
        uint32_t i;
        /* Initializing all desc to be available */
        for(i=0; i<table_info->num_entries; i++)
        {
            tmp_obj_desc = (tivx_obj_desc_t*)&table_info->table_base[i];
            tmp_obj_desc->type = TIVX_OBJ_DESC_INVALID;
        }

        table_info->last_alloc_index = 0;
    }
#endif

}
