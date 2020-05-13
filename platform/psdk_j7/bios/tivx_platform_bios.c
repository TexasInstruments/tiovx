/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk_j7.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/udma/include/app_udma.h>

void tivxPlatformResetObjDescTableInfo(void);

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
     */
    tivx_mutex g_platform_lock[(vx_enum)TIVX_PLATFORM_LOCK_MAX];
} tivx_platform_info_t;


/*! \brief Global instance of platform information
 * \ingroup group_tivx_platform
 */
static tivx_platform_info_t g_tivx_platform_info =
{
    TIVX_TARGET_INFO
};

tivx_obj_desc_shm_entry_t *gTivxObjDescShmEntry = NULL;


void *tivxPlatformGetDmaObj(void)
{
   return appUdmaGetObj();
}

vx_status tivxPlatformInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t i = 0;
    uint32_t shmSize = 0;
    int32_t retVal;

    /* Build time check to see if the structure size is 8byte aligned and size of the elements is not more than  */
    BUILD_ASSERT((sizeof(tivx_obj_desc_shm_entry_t) % TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN) == 0U);
    BUILD_ASSERT(sizeof(tivx_obj_desc_shm_entry_t) <= TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE);

    retVal = appIpcGetTiovxObjDescSharedMemInfo( (void **) &gTivxObjDescShmEntry, &shmSize);

    if( (0 != retVal) || (gTivxObjDescShmEntry == NULL)
        || (shmSize < (TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST*(uint32_t)sizeof(tivx_obj_desc_shm_entry_t))))
    {
        /* insufficient shared memory size */
        VX_PRINT(VX_ZONE_ERROR, "insufficient shared memory size\n");
        status = (vx_status)VX_FAILURE;
    }
    if(status==(vx_status)VX_SUCCESS)
    {
        /* init obj desc on RTOS side, it is assumed that linux starts after RTOS, so linux need not init the object descriptors */
        tivxPlatformResetObjDescTableInfo();

        for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
        {
            status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);

            if ((vx_status)VX_SUCCESS != status)
            {
                tivxPlatformDeInit();
                break;
            }
        }
        tivxIpcInit();
    }

    return (status);
}


void tivxPlatformDeInit(void)
{
    int32_t i;

    tivxIpcDeInit();

    for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
    }
}

void tivxPlatformSystemLock(vx_enum lock_id)
{
    if ((vx_enum)lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);

        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* for data ref queue lock, need to take a multi processor lock,
             * since multiple CPUs could be trying to queue/dequeue from the same
             * data ref queue.
             * This lock in this platform is implemented via HW spinlock
             */
            appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else
        {
            /* do nothing */
        }
    }
}

void tivxPlatformSystemUnlock(vx_enum lock_id)
{
    if ((vx_enum)lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* release the lock taken during tivxPlatformSystemLock */
            appIpcHwLockRelease(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            appIpcHwLockRelease(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID);
        }
        else
        {
            /* do nothing */
        }

        tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
            (uint32_t)lock_id]);
    }
}

vx_enum tivxPlatformGetTargetId(const char *target_name)
{
    uint32_t i;
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

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
    vx_bool status = (vx_bool)vx_false_e;
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
                    status = (vx_bool)vx_true_e;
                    break;
                }
            }
        }
    }

    return (status);
}

void tivxPlatformResetObjDescTableInfo(void)
{
    tivx_obj_desc_t *tmp_obj_desc = NULL;
    uint32_t i;
    /* Initializing all desc to be available */
    for(i=0; i<TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST; i++)
    {
        tmp_obj_desc = (tivx_obj_desc_t*)&gTivxObjDescShmEntry[i];
        tmp_obj_desc->type = (vx_enum)TIVX_OBJ_DESC_INVALID;
    }
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
}

void tivxPlatformSetHostTargetId(tivx_target_id_e host_target_id)
{
    uint32_t i;

    for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
    {
        if (0 == strncmp(
                g_tivx_platform_info.target_info[i].target_name,
                TIVX_TARGET_HOST,
                TIVX_TARGET_MAX_NAME))
        {
            /* update target_id for TIVX_TARGET_HOST */
            g_tivx_platform_info.target_info[i].target_id = (vx_enum)host_target_id;
            break;
        }
    }
}

void tivxPlatformGetTargetName(vx_enum target_id, char *target_name)
{
    uint32_t i;

    snprintf(target_name, TIVX_TARGET_MAX_NAME, "UNKNOWN");

    if(target_id != (vx_enum)TIVX_TARGET_ID_INVALID)
    {
        for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
        {
            if (target_id == g_tivx_platform_info.target_info[i].target_id)
            {
                snprintf(target_name, TIVX_TARGET_MAX_NAME, "%s", g_tivx_platform_info.target_info[i].target_name);
                break;
            }
        }
    }
}
