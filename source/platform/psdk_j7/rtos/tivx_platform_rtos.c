/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/misc/include/app_misc.h>

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

int32_t tivxPlatformGetShmSize(uint32_t * shm_size)
{
    return appIpcGetTiovxObjDescSharedMemInfo((void**)&gTivxObjDescShmEntry, shm_size);
}

vx_status ownPlatformInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t i = 0;
    uint32_t shmSize = 0;
    int32_t retVal;

    /* Build time check to see if the structure size is 8byte aligned and size of the elements is not more than  */
    BUILD_ASSERT((((sizeof(tivx_obj_desc_shm_entry_t)) % (TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN)) == 0U )? 1U : 0U);
    BUILD_ASSERT(((sizeof(tivx_obj_desc_shm_entry_t)) <= (TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE )) ? 1U : 0U);

    retVal = appIpcGetTiovxObjDescSharedMemInfo( (void **) &gTivxObjDescShmEntry, &shmSize);

#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1772- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PLATFORM_RTOS_UM001 */
    if( (0 != retVal) || (gTivxObjDescShmEntry == NULL)
        || (shmSize < (TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST*(uint32_t)sizeof(tivx_obj_desc_shm_entry_t))))
    {
        /* insufficient shared memory size */
        VX_PRINT(VX_ZONE_ERROR, "insufficient shared memory size\n");
        status = (vx_status)VX_FAILURE;
    }
#endif
    if(status==(vx_status)VX_SUCCESS) /* TIOVX-1957- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_PLATFORM_RTOS_UBR001 */
    {
        /* init obj desc on RTOS side, it is assumed that linux starts after RTOS, so linux need not init the object descriptors */
        tivxPlatformResetObjDescTableInfo();

        for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
        {
            status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1772- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PLATFORM_RTOS_UM002 */
            if ((vx_status)VX_SUCCESS != status)
            {
                ownPlatformDeInit();
                break;
            }
#endif
        }
        ownIpcInit();
        ownLogRtInit();
    }

    return (status);
}

/*LDRA_NOANALYSIS*/
/* TIOVX-1772- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_PLATFORM_RTOS_UM003 */
void ownPlatformDeInit(void)
{
    int32_t i;

    ownIpcDeInit();

    for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            (void)tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
    }
}
/*LDRA_ANALYSIS*/

void ownPlatformSystemLock(vx_enum lock_id)
{

    if ((vx_enum)lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        (void)tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);

        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* for data ref queue lock, need to take a multi processor lock,
             * since multiple CPUs could be trying to queue/dequeue from the same
             * data ref queue.
             * This lock in this platform is implemented via HW spinlock
             */
            (void)appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
        {
            (void)appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            (void)appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else
        {
            /* do nothing */
        }
    }
}

void ownPlatformSystemUnlock(vx_enum lock_id)
{

    if ((vx_enum)lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* release the lock taken during ownPlatformSystemLock */
            (void)appIpcHwLockRelease(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
        {
            (void)appIpcHwLockRelease(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            (void)appIpcHwLockRelease(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID);
        }
        else
        {
            /* do nothing */
        }

        (void)tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
            (uint32_t)lock_id]);
    }
}

vx_enum ownPlatformGetTargetId(const char *target_name)
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

void ownPlatformGetObjDescTableInfo(tivx_obj_desc_table_info_t *table_info)
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

    for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS;/* TIOVX-1957- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_PLATFORM_RTOS_UBR002 */ i ++)
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

void ownPlatformGetTargetName(vx_enum target_id, char *target_name)
{
    uint32_t i;

    (void)snprintf(target_name, TIVX_TARGET_MAX_NAME, "UNKNOWN");

    if(target_id != (vx_enum)TIVX_TARGET_ID_INVALID)
    {
        for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
        {
            if (target_id == g_tivx_platform_info.target_info[i].target_id)
            {
                (void)snprintf(target_name, TIVX_TARGET_MAX_NAME, "%s", g_tivx_platform_info.target_info[i].target_name);
                break;
            }
        }
    }
}

void ownPlatformGetLogRtShmInfo(void **shm_base, uint32_t *shm_size)
{
    if(shm_base != NULL)
    {
        *shm_base = NULL;
    }
    if(shm_size != NULL)
    {
        *shm_size = 0;
    }
    if(shm_base && shm_size)
    {
        appIpcGetTiovxLogRtSharedMemInfo(shm_base, shm_size);
        /* Needs to be called once by someone, size RTOS boots first, we call it in RTOS side */
        if(*shm_base != NULL) /* TIOVX-1957- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_RTOS_TIVX_PLATFORM_RTOS_UBR003 */
        {
            ownLogRtResetShm(*shm_base);
        }
    }
}

void ownPlatformTaskInit(void)
{
    appUtilsTaskInit();
}
