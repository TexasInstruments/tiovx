/*
*
* Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include <app_mem_map.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <tivx_platform.h>
#include <tivx_platform_pc.h>
#include <tivx_platform_common.h>
#include <semaphore.h>

#if defined(SOC_FAMILY_TDA5)
#include <utils/pc_osal/include/dpl_osal.h>
#include <Ipc_Notify_Hostemu.h>
#endif

void *gVdkObj = NULL;

uint32_t emulated_cores = 0u;

static tivx_vdk_print_log_f gVdkPrintLog = NULL;
static tivx_vdk_handle_spinlock_f gVdkHandleSpinlock = NULL;
static tivx_vdk_get_host_ptr_from_symbol_f gVdkGetHostPtrFromSymbol = NULL;
static tivx_vdk_get_host_ptr_from_phy_ptr_f gVdkGetHostPtrFromPhyPtr = NULL;

#if defined(SOC_FAMILY_TDA5)
extern IpcNotify_Hal_MailboxConfig IpcNotifyMailboxConfig[CSL_CORE_ID_MAX][CSL_CORE_ID_MAX];

int32_t tivxVdkRegisterCallbacks(
                             tivx_vdk_print_log_f print_log,
                             tivx_vdk_get_host_ptr_from_symbol_f get_host_ptr_from_symbol,
                             tivx_vdk_get_host_ptr_from_phy_ptr_f get_host_ptr_from_phy_ptr,
                             tivx_vdk_ipc_send_mbox_f ipc_send_mbox,
                             tivx_vdk_handle_spinlock_f handle_spinlock,
                             void *obj)
{
    gVdkPrintLog = print_log;
    gVdkGetHostPtrFromSymbol = get_host_ptr_from_symbol;
    gVdkGetHostPtrFromPhyPtr = get_host_ptr_from_phy_ptr;
    ownUpdateHostPtrFromPhyPtrFunctionPtr(get_host_ptr_from_phy_ptr);
    ownUpdateIpcSendMboxFunctionPtr(ipc_send_mbox);
    gVdkHandleSpinlock = handle_spinlock;

    IpcNotify_Hal_VdkInit(ipc_send_mbox, get_host_ptr_from_phy_ptr, (IpcNotify_Hal_VdkGetCpuIdCallback)tivxVdkGetSelfCslIpcCpuId, obj);

    IpcVdkPrintLogRegister(print_log);

    gVdkObj = obj;

    return VX_SUCCESS;
}

int32_t tivxVdkGetMboxConfiguration(uint32_t cpu_bitmask, tivx_mbox_config_t *mbox_config)
{
    uint32_t dst_core, i, src_core, cluster;
    uint16_t fifo_bitmask, user_bitmask;

    IpcNotify_Hal_MailboxConfig (*MailboxConfig)[CSL_CORE_ID_MAX] = IpcNotifyMailboxConfig;

    for (i = 0; i < MAX_NUM_MBOX_CLUSTERS; i++)
    {
        mbox_config->user_idx_bitmask[i/4] = 0U;
        mbox_config->fifo_id_bitmask[i] = 0U;
    }

    for (dst_core = 0; dst_core <= CSL_CORE_ID_MCU1; dst_core++)
    {
        if (cpu_bitmask & (1U << dst_core))
        {
            emulated_cores |= (1U << dst_core);
            for (src_core = 0; src_core <= CSL_CORE_ID_MCU1; src_core++)
            {
                cluster = MailboxConfig[src_core][dst_core].MailboxId;
                if (cluster != 0xFFU)
                {
                    fifo_bitmask = (1U << MailboxConfig[src_core][dst_core].HwFifoId);
                    user_bitmask = ((1U << MailboxConfig[src_core][dst_core].UserId) << (4U * (cluster % 4U)));
                    mbox_config->fifo_id_bitmask[cluster] |= fifo_bitmask;
                    mbox_config->user_idx_bitmask[cluster/4] |= user_bitmask;
                }
            }
        }
    }

    return 0;
}

int32_t tivxVdkIpcRecvMbox(uint32_t payload, uint32_t mailbox_fifo_id)
{
    int32_t status = 0;

    status = IpcNotify_Hal_VdkRecvMsg(payload, mailbox_fifo_id);

    return status;
}

void * tivxVdkGetHostPtrFromSymbol(const char * symbol)
{
    void *ptr = NULL;

    if (gVdkGetHostPtrFromSymbol != NULL)
    {
        ptr = gVdkGetHostPtrFromSymbol(symbol, gVdkObj);
    }

    return ptr;
}

uint64_t tivxVdkGetHostPtrFromPhyPtr(uint64_t phy_ptr)
{
    uint64_t ptr = 0;

    if (gVdkGetHostPtrFromPhyPtr != NULL)
    {
        ptr = gVdkGetHostPtrFromPhyPtr(phy_ptr, gVdkObj);
    }

    return ptr;
}
#endif /* #if defined(SOC_FAMILY_TDA5) */

uint32_t tivxVdkIsEnabled(void)
{
    uint32_t ret = 0;

    if (gVdkObj != NULL)
    {
        ret = 1;
    }

    return ret;
}

void * tivxGetVdkObj(void)
{
    return gVdkObj;
}

uint32_t tivxVdkGetEmulatedCores(void)
{
    return emulated_cores;
}

typedef struct tivx_platform_info
{
    /*! \brief Platform locks to protect access to the descriptor id
     */
    tivx_mutex g_platform_lock[(vx_enum)TIVX_PLATFORM_LOCK_MAX];

    /*! \brief POSIX semaphore to protect memory
     */
    sem_t semaphore;
    /*! \brief POSIX semaphore to protect Data Reference
     */
    sem_t semaphore_data_ref;
    /*! \brief POSIX semaphore to protect Log Memory
     */
    sem_t semaphore_log_mem;
} tivx_platform_info_t;

/*! \brief Global instance of platform information
 * \ingroup group_tivx_platform
 */
static tivx_platform_info_t g_tivx_platform_info;

tivx_obj_desc_shm_entry_t gTivxObjDescShmEntry
    [TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST];

int32_t tivxPlatformGetShmSize(uint32_t * shm_size)
{
    *shm_size = TIOVX_OBJ_DESC_MEM_SIZE;
    /* Always returns 0 for successful status as size is #defined */
    return 0;
}

vx_status ownPlatformInit(void)
{
    vx_status status;
    uint32_t i = 0;

    for (i = 0; i < (uint32_t)(vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        status = tivxMutexCreate(&g_tivx_platform_info.g_platform_lock[i]);

        if ((vx_status)(vx_status)VX_SUCCESS != status)
        {
            ownPlatformDeInit();
            break;
        }
    }

    ownIpcInit();
    ownLogRtInit();

#if defined(SOC_FAMILY_TDA5)
    if (sem_init(&g_tivx_platform_info.semaphore, 0, 1) != 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore create failed\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if (sem_init(&g_tivx_platform_info.semaphore_data_ref, 0, 1) != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore_data_ref create failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            if (sem_init(&g_tivx_platform_info.semaphore_log_mem, 0, 1) != 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "POSIX semaphore_log_mem create failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }
#endif

    tivxPlatformGetTimeInUsecs();

    return (status);
}


void ownPlatformDeInit(void)
{
    uint32_t i;

#if defined(SOC_FAMILY_TDA5)
    (void)sem_destroy(&g_tivx_platform_info.semaphore);
    (void)sem_destroy(&g_tivx_platform_info.semaphore_data_ref);
    (void)sem_destroy(&g_tivx_platform_info.semaphore_log_mem);
#endif

    ownIpcDeInit();

    for (i = 0; i < (vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            (void)tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
    }
}

void ownPlatformSystemLock(vx_enum lock_id)
{
    if (lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        (void)tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);

        if (NULL != gVdkHandleSpinlock)
        {
            if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
            {
                /* for data ref queue lock, need to take a multi processor lock,
                * since multiple CPUs could be trying to queue/dequeue from the same
                * data ref queue.
                * This lock in this platform is implemented via HW spinlock
                */
                (void)sem_wait(&g_tivx_platform_info.semaphore_data_ref);
                (void)gVdkHandleSpinlock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID, 1U, gVdkObj);
            }
            else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
            {
                (void)sem_wait(&g_tivx_platform_info.semaphore_log_mem);
                (void)gVdkHandleSpinlock(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID, 1U, gVdkObj);
            }
            else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
            {
                (void)sem_wait(&g_tivx_platform_info.semaphore);
                (void)gVdkHandleSpinlock(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID, 1U, gVdkObj);
            }
            else
            {
                /* do nothing */
            }
        }
    }
}

void ownPlatformSystemUnlock(vx_enum lock_id)
{
    if (lock_id < (vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        if (NULL != gVdkHandleSpinlock)
        {
            if(lock_id==(vx_enum)TIVX_PLATFORM_LOCK_DATA_REF_QUEUE)
            {
                /* release the lock taken during ownPlatformSystemLock */
                (void)gVdkHandleSpinlock(TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID, 0U, gVdkObj);
                (void)sem_post(&g_tivx_platform_info.semaphore_data_ref);
            }
            else if ((vx_enum)TIVX_PLATFORM_LOCK_LOG_RT==lock_id)
            {
                (void)gVdkHandleSpinlock(TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID, 0U, gVdkObj);
                (void)sem_post(&g_tivx_platform_info.semaphore_log_mem);
            }
            else if ((vx_enum)TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE==lock_id)
            {
                (void)gVdkHandleSpinlock(TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID, 0U, gVdkObj);
                (void)sem_post(&g_tivx_platform_info.semaphore);
            }
            else
            {
                /* do nothing */
            }
        }
        (void)tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
            (uint32_t)lock_id]);
    }
}

void ownPlatformGetObjDescTableInfo(tivx_obj_desc_table_info_t *table_info)
{
    if (NULL != table_info)
    {
        tivx_obj_desc_t *tmp_obj_desc = NULL;
        uint32_t i;

        if(NULL ==gVdkGetHostPtrFromSymbol)
        {
            table_info->table_base = gTivxObjDescShmEntry;
        }
        else
        {
            void *table_base = gVdkGetHostPtrFromSymbol("g_tiovx_obj_desc_mem", gVdkObj);
            table_info->table_base = (tivx_obj_desc_shm_entry_t *)table_base;
        }
        table_info->num_entries = TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST;

        /* Change this according available entries*/
        table_info->last_alloc_index = 0U;

        /* Initializing all desc to be available */
        for(i=0; i<table_info->num_entries; i++)
        {
            tmp_obj_desc = (tivx_obj_desc_t*)&table_info->table_base[i];
            tmp_obj_desc->type = (vx_enum)TIVX_OBJ_DESC_INVALID;
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
}

void ownPlatformTaskInit(void)
{
}

void ownPlatformPrintf(const char *format)
{
    char buf[4*1024];
    uint64_t cur_time = tivxPlatformGetTimeInUsecs();

    snprintf(buf, sizeof(buf), " %d.%ds: %s",
        (uint32_t)(cur_time/1000000U),
        (uint32_t)(cur_time%1000000U),
        format);
    if(NULL == gVdkPrintLog)
    {
        printf(buf);
    }
    else
    {
        gVdkPrintLog(buf, gVdkObj);
    }
}

void ownPlatformActivate()
{
}

void ownPlatformDeactivate()
{
}

void ownPlatformGetTargetPerfStats(uint32_t app_cpu_id, uint32_t target_values[TIVX_TARGET_RESOURCE_COUNT])
{
    uint32_t i;

    for (i = 0; i < TIVX_TARGET_RESOURCE_COUNT; i++)
    {
        target_values[i] = 0;
    }
}
