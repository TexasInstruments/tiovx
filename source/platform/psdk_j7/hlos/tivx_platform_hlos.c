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
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

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

    /*! \brief POSIX semaphore to protect memory
     */
    sem_t *semaphore;
    /*! \brief POSIX semaphore to protect Data Reference
     */
    sem_t *semaphore_data_ref;
    /*! \brief POSIX semaphore to protect Log Memory
     */
    sem_t *semaphore_log_mem;
} tivx_platform_info_t;


/*! \brief Global instance of platform information
 * \ingroup group_tivx_platform
 */
static tivx_platform_info_t g_tivx_platform_info =
{
    TIVX_TARGET_INFO
};

tivx_obj_desc_shm_entry_t *gTivxObjDescShmEntry = NULL;

void tivxPlatformResetObjDescTableInfo(void);

vx_status ownPlatformInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t shmSize = 0;
    int32_t i = 0, retVal;

    /* Build time check to see if the structure size is 8byte aligned and size of the elements is not more than  */
    BUILD_ASSERT(
    ((sizeof(tivx_obj_desc_shm_entry_t)) % (TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN))
        == 0U);
    BUILD_ASSERT(
    (sizeof(tivx_obj_desc_shm_entry_t)) <= TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE);

    retVal = appIpcGetTiovxObjDescSharedMemInfo( (void **) &gTivxObjDescShmEntry, &shmSize);

    if( (0 != retVal) || (gTivxObjDescShmEntry == NULL)
        || ((uint32_t)shmSize < (TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST*(uint32_t)sizeof(tivx_obj_desc_shm_entry_t))))
    {
        /* insufficient shared memory size */
        VX_PRINT(VX_ZONE_ERROR, "insufficient shared memory size\n");
        status = (vx_status)VX_FAILURE;
    }
    if(status==(vx_status)VX_SUCCESS)
    {
        for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
        {
            status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);

            if ((vx_status)VX_SUCCESS != status)
            {
                ownPlatformDeInit();
                break;
            }
        }
        ownIpcInit();
        ownLogRtInit();

        /* create a named semaphore that is used by all TIOVX process
         * to serialize access to critical resources shared between processes
         * example, obj desc shared memory
         * mode/permissions = 00700 octal = 0x01C0
         * mode/permissions = 00777 octal = 0x01ff
         */
        /* TIOVX-1165 : If OpenVX needs to be accessed by multiple applications with different
         * user accounts, then the spinlock semaphors should be created with read/write access
         * for all accounts, not just the account that initializes OpenVX. */
        #define TIVX_SEM_OPEN_MODE  (0x01ff)

        g_tivx_platform_info.semaphore = sem_open("/tiovxsem", (O_CREAT), (TIVX_SEM_OPEN_MODE), 1);

        if (SEM_FAILED == g_tivx_platform_info.semaphore)
        {
            VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore create failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            g_tivx_platform_info.semaphore_data_ref = sem_open("/tiovxsem_data_ref", (O_CREAT), (TIVX_SEM_OPEN_MODE), 1);

            if (SEM_FAILED == g_tivx_platform_info.semaphore_data_ref)
            {
                VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore_data_ref create failed\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                g_tivx_platform_info.semaphore_log_mem = sem_open("/tiovxsem_log_mem", (O_CREAT), (TIVX_SEM_OPEN_MODE), 1);

                if (SEM_FAILED == g_tivx_platform_info.semaphore_log_mem)
                {
                    VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore_log_mem create failed\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
        }
    }

    return (status);
}


void ownPlatformDeInit(void)
{
    int32_t i;

    sem_close(g_tivx_platform_info.semaphore);
    sem_close(g_tivx_platform_info.semaphore_data_ref);
    sem_close(g_tivx_platform_info.semaphore_log_mem);

    ownIpcDeInit();

    for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
    }
}

void ownPlatformSystemLock(vx_enum lock_id)
{
    if (lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);

        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* for data ref queue lock, need to take a multi processor lock,
             * since multiple CPUs could be trying to queue/dequeue from the same
             * data ref queue.
             * This lock in this platform is implemented via HW spinlock
             */
            sem_wait(g_tivx_platform_info.semaphore_data_ref);
            appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
        {
            sem_wait(g_tivx_platform_info.semaphore_log_mem);
            appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            sem_wait(g_tivx_platform_info.semaphore);
            appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else
        {
            /* do nothing */
        }
    }
}

void ownPlatformSystemUnlock(vx_enum lock_id)
{
    if (lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* release the lock taken during ownPlatformSystemLock */
            appIpcHwLockRelease(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID);
            sem_post(g_tivx_platform_info.semaphore_data_ref);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
        {
            appIpcHwLockRelease(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID);
            sem_post(g_tivx_platform_info.semaphore_log_mem);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            appIpcHwLockRelease(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID);
            sem_post(g_tivx_platform_info.semaphore);
        }
        else
        {
            /* do nothing */
        }

        tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
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

vx_bool ownPlatformTargetMatch(
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

    for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
    {
        if (0 == strncmp(
                g_tivx_platform_info.target_info[i].target_name,
                TIVX_TARGET_HOST,
                TIVX_TARGET_MAX_NAME))
        {
            /* update target_id for TIVX_TARGET_HOST */
            g_tivx_platform_info.target_info[i].target_id = (int32_t)host_target_id;
            break;
        }
    }
}

void ownPlatformGetTargetName(vx_enum target_id, char *target_name)
{
    uint32_t i;

    snprintf(target_name, TIVX_TARGET_MAX_NAME, "UNKNOWN");

    if(target_id!=(vx_enum)TIVX_TARGET_ID_INVALID)
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

void ownPlatformGetLogRtShmInfo(void **shm_base, uint32_t *shm_size)
{
    if(shm_base)
    {
        *shm_base = NULL;
    }
    if(shm_size)
    {
        *shm_size = 0;
    }
    if(shm_base && shm_size)
    {
        appIpcGetTiovxLogRtSharedMemInfo(shm_base, shm_size);
    }
}

void ownPlatformTaskInit()
{
}
