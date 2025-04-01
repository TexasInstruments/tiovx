/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
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
#include <tivx_platform_common.h>
#include <utils/ipc/include/app_ipc.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

void tivxPlatformResetObjDescTableInfo(void);

/*! \brief Structure for keeping track of platform locks
 * \ingroup group_tivx_platform
 */
typedef struct tivx_platform_info
{
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
    {NULL}, NULL, NULL, NULL
};

tivx_obj_desc_shm_entry_t *gTivxObjDescShmEntry = NULL;

int32_t tivxPlatformGetShmSize(uint32_t * shm_size)
{
    return appIpcGetTiovxObjDescSharedMemInfo((void**)&gTivxObjDescShmEntry, shm_size);
}

vx_status ownPlatformInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t shmSize = 0;
    int32_t i = 0, retVal;

#if defined(LDRA_UNTESTABLE_CODE)
    /* Build time check to see if the structure size is 8byte aligned and size of the elements is not more than  */
    BUILD_ASSERT(
    (((sizeof(tivx_obj_desc_shm_entry_t)) % (TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN))
        == 0U) ? 1U : 0U);
    BUILD_ASSERT(
    ((sizeof(tivx_obj_desc_shm_entry_t)) <= (TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE)) ? 1U : 0U);
    /* This is required to be <= 32 per framework requirements since we are using a bitmask with a uint32_t type */
    BUILD_ASSERT(
    (TIVX_KERNEL_MAX_PARAMS <= 32U) ? 1U : 0U);
#endif

    retVal = appIpcGetTiovxObjDescSharedMemInfo( (void **) &gTivxObjDescShmEntry, &shmSize);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM001
<justification end> */
    if( (0 != retVal) || (gTivxObjDescShmEntry == NULL)
        || ((uint32_t)shmSize < (TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST*(uint32_t)sizeof(tivx_obj_desc_shm_entry_t))))
    {
        /* insufficient shared memory size */
        VX_PRINT(VX_ZONE_ERROR, "insufficient shared memory size\n");
        status = (vx_status)VX_FAILURE;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_PLATFORM_HLOS_UBR001
<justification end> */
    if(status==(vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
    {
        for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
        {
            status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM002
<justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                ownPlatformDeInit();
                break;
            }
/* LDRA_JUSTIFY_END */
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM003 <justification end> */
        if (SEM_FAILED == g_tivx_platform_info.semaphore)
        {
            VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore create failed\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM003 <justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            g_tivx_platform_info.semaphore_data_ref = sem_open("/tiovxsem_data_ref", (O_CREAT), (TIVX_SEM_OPEN_MODE), 1);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM004 <justification end> */
            if (SEM_FAILED == g_tivx_platform_info.semaphore_data_ref)
            {
                VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore_data_ref create failed\n");
                status = (vx_status)VX_FAILURE;
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM004 <justification end> */
            else
/* LDRA_JUSTIFY_END */
            {
                g_tivx_platform_info.semaphore_log_mem = sem_open("/tiovxsem_log_mem", (O_CREAT), (TIVX_SEM_OPEN_MODE), 1);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM005
<justification end> */
                if (SEM_FAILED == g_tivx_platform_info.semaphore_log_mem)
                {
                    VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore_log_mem create failed\n");
                    status = (vx_status)VX_FAILURE;
                }
/* LDRA_JUSTIFY_END */
            }
        }
    }

    return (status);
}


void ownPlatformDeInit(void)
{
    int32_t i;

    (void)sem_close(g_tivx_platform_info.semaphore);
    (void)sem_close(g_tivx_platform_info.semaphore_data_ref);
    (void)sem_close(g_tivx_platform_info.semaphore_log_mem);

    ownIpcDeInit();

    for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_PLATFORM_HLOS_UBR002
<justification end> */
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            (void)tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
/* LDRA_JUSTIFY_END */
    }
}

void ownPlatformSystemLock(vx_enum lock_id)
{
    if (lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        (void)tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);

        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* for data ref queue lock, need to take a multi processor lock,
             * since multiple CPUs could be trying to queue/dequeue from the same
             * data ref queue.
             * This lock in this platform is implemented via HW spinlock
             */
            (void)sem_wait(g_tivx_platform_info.semaphore_data_ref);
            (void)appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
        {
            (void)sem_wait(g_tivx_platform_info.semaphore_log_mem);
            (void)appIpcHwLockAcquire(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID, APP_IPC_WAIT_FOREVER);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            (void)sem_wait(g_tivx_platform_info.semaphore);
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
    if (lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
        {
            /* release the lock taken during ownPlatformSystemLock */
            (void)appIpcHwLockRelease(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID);
            (void)sem_post(g_tivx_platform_info.semaphore_data_ref);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
        {
            (void)appIpcHwLockRelease(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID);
            (void)sem_post(g_tivx_platform_info.semaphore_log_mem);
        }
        else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
        {
            (void)appIpcHwLockRelease(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID);
            (void)sem_post(g_tivx_platform_info.semaphore);
        }
        else
        {
            /* do nothing */
        }

        (void)tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
            (uint32_t)lock_id]);
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void tivxPlatformResetObjDescTableInfo.* <function end>
<justification start> TIOVX_CODE_COVERAGE_PLATFORM_HLOS_UM006
<justification end> */
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
    }
}

void ownPlatformTaskInit(void)
{
}
