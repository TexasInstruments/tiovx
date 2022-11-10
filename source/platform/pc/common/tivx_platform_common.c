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
#include <tivx_platform_pc.h>

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

tivx_obj_desc_shm_entry_t gTivxObjDescShmEntry
    [TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST];

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

    tivxPlatformGetTimeInUsecs();

    return (status);
}


void ownPlatformDeInit(void)
{
    uint32_t i;

    ownIpcDeInit();

    for (i = 0; i < (uint32_t)(vx_enum)TIVX_PLATFORM_LOCK_MAX; i ++)
    {
        if (NULL != g_tivx_platform_info.g_platform_lock[i])
        {
            tivxMutexDelete(&g_tivx_platform_info.g_platform_lock[i]);
        }
    }
}

void ownPlatformSystemLock(vx_enum lock_id)
{
    if ((int32_t)lock_id < (int32_t)(vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        tivxMutexLock(g_tivx_platform_info.g_platform_lock[(uint32_t)lock_id]);
    }
}

void ownPlatformSystemUnlock(vx_enum lock_id)
{
    if ((int32_t)lock_id < (int32_t)(vx_enum)TIVX_PLATFORM_LOCK_MAX)
    {
        tivxMutexUnlock(g_tivx_platform_info.g_platform_lock[
            (uint32_t)lock_id]);
    }
}

vx_enum ownPlatformGetTargetId(const char *target_name)
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

void ownPlatformGetTargetName(vx_enum target_id, char *target_name)
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

void ownPlatformGetObjDescTableInfo(tivx_obj_desc_table_info_t *table_info)
{
    if (NULL != table_info)
    {
        tivx_obj_desc_t *tmp_obj_desc = NULL;
        uint32_t i;

        table_info->table_base = gTivxObjDescShmEntry;
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

void ownPlatformPrintf(const char *format)
{
    char buf[4*1024];
    uint64_t cur_time = tivxPlatformGetTimeInUsecs();

    snprintf(buf, sizeof(buf), " %d.%ds: %s",
        (uint32_t)(cur_time/1000000U),
        (uint32_t)(cur_time%1000000U),
        format);
    printf(buf);
}

void ownPlatformActivate()
{
}

void ownPlatformDeactivate()
{
}

void *tivxPlatformGetDmaObj()
{
  return NULL;
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
}

void ownPlatformTaskInit()
{
}
