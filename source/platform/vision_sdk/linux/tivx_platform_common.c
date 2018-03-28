/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <src/hlos/system/system_priv_openvx.h>
#include <tivx_platform_vision_sdk.h>
#include "tivx_platform_priv.h"

#include <sys/time.h>

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
    tivx_mutex g_platform_lock[TIVX_PLATFORM_LOCK_MAX];
} tivx_platform_info_t;


/*! \brief Global instance of platform information
 * \ingroup group_tivx_platform
 */
static tivx_platform_info_t g_tivx_platform_info =
{
    TIVX_TARGET_INFO
};

tivx_obj_desc_shm_entry_t *gTivxObjDescShmEntry;

vx_status tivxPlatformInit(void)
{
    vx_status status;
    uint32_t i = 0;

    gTivxObjDescShmEntry = (tivx_obj_desc_shm_entry_t *)
        System_ovxGetObjDescShm();

    if (0 == gTivxObjDescShmEntry)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxPlatformInit: Object descriptor SHM entry is 0\n");
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0; i < TIVX_PLATFORM_LOCK_MAX; i ++)
        {
            status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);

            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxPlatformInit: Mutex creation failed\n");
                tivxPlatformDeInit();
                break;
            }
        }
    }

    tivxMemInit();
    tivxIpcInit();

    return (status);
}


void tivxPlatformDeInit(void)
{
    uint32_t i;

    tivxMemDeInit();
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

        if(lock_id==TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* for data ref queue lock, need to take a multi processor lock,
             * since multiple CPUs could be trying to queue/dequeue from the same
             * data ref queue.
             * This lock in this platform is implemented via HW spinlock
             */
            System_openvxHwSpinLockAcquire(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID);
        }
    }
}

void tivxPlatformSystemUnlock(vx_enum lock_id)
{
    if ((uint32_t)lock_id < TIVX_PLATFORM_LOCK_MAX)
    {
        if(lock_id==TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* release the lock taken during tivxPlatformSystemLock */
            System_openvxHwSpinLockRelease(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID);
        }

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
    tivx_obj_desc_t *tmp_obj_desc = NULL;
    uint32_t i;

    if (NULL != table_info)
    {
        table_info->table_base = gTivxObjDescShmEntry;
        table_info->num_entries = TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST;

        /* Change this according available entries*/
        table_info->last_alloc_index = 0U;

        /* Initializing all desc to be available */
        for(i=0; i<table_info->num_entries; i++)
        {
            tmp_obj_desc = (tivx_obj_desc_t*)&table_info->table_base[i];
            tmp_obj_desc->type = TIVX_OBJ_DESC_INVALID;
        }

        table_info->last_alloc_index = 0;
    }
}

void tivxPlatformPrintf(const char *format)
{
    printf(format);
}

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    uint64_t timeInUsecs = 0;
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0)
    {
        timeInUsecs = 0;
    }
    else
    {
        timeInUsecs = tv.tv_sec * 1000000ull + tv.tv_usec;
    }

    return timeInUsecs;
}

void tivxPlatformActivate()
{
}

void tivxPlatformDeactivate()
{
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
            g_tivx_platform_info.target_info[i].target_id = host_target_id;
            break;
        }
    }
}

void tivxPlatformGetTargetName(vx_enum target_id, char *target_name)
{
    uint32_t i;

    snprintf(target_name, TIVX_TARGET_MAX_NAME, "UNKNOWN");

    if(target_id!=TIVX_TARGET_ID_INVALID)
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
