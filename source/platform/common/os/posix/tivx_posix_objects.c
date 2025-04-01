/*
*
* Copyright (c) 2017-2024 Texas Instruments Incorporated
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

#include <tivx_platform_posix.h>
#include <tivx_objects.h>

static tivx_platform_posix_t g_tivx_posix_objects;
static tivx_mutex g_tivx_objects_lock = NULL;

static uint8_t *ownAllocPosixObject(
    uint8_t *obj_start_ptr, vx_bool inUse[], uint32_t max_objects,
    uint32_t size, char *resource_name)
{
    uint32_t i;
    uint8_t *obj_ptr = NULL;

    for (i = 0;i < max_objects; i ++)
    {
        if ((vx_bool)vx_false_e == inUse[i])
        {
            obj_ptr = obj_start_ptr;
            inUse[i] = (vx_bool)vx_true_e;
            break;
        }
        obj_start_ptr += size;
    }

    if (obj_ptr == NULL)
    {
        VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of %s in tiovx/source/platform/os/posix/tivx_platform_posix.h\n", resource_name);
    }

    return (obj_ptr);
}

static vx_status ownFreePosixObject(
    const uint8_t *obj_ptr, const uint8_t *obj_start_ptr, vx_bool inUse[],
    uint32_t max_objects, uint32_t size)
{
    uint32_t i;
    vx_status status = (vx_status)VX_FAILURE;

    for (i = 0;i < max_objects; i ++)
    {
        if ((obj_ptr == obj_start_ptr) && ((vx_bool)vx_true_e == inUse[i]))
        {
            inUse[i] = (vx_bool)vx_false_e;
            status = (vx_status)VX_SUCCESS;
            break;
        }
        obj_start_ptr += size;
    }

    return (status);
}

uint8_t *ownPosixObjectAlloc(vx_enum type)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t *obj = NULL;

    status = tivxMutexLock(g_tivx_objects_lock);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_POSIX_OBJECTS_UBR001
<justification end> */
    if ((vx_status)VX_SUCCESS == status)
    {
        switch(type)
        {
            case (vx_enum)TIVX_POSIX_TYPE_EVENT:
                obj = ownAllocPosixObject(
                    (uint8_t *)g_tivx_posix_objects.event, g_tivx_posix_objects.isEventUse,
                    TIVX_EVENT_MAX_OBJECTS, (uint32_t)sizeof(tivx_event_t), "TIVX_EVENT_MAX_OBJECTS");
                break;
            case (vx_enum)TIVX_POSIX_TYPE_MUTEX:
                obj = ownAllocPosixObject(
                    (uint8_t *)g_tivx_posix_objects.mutex, g_tivx_posix_objects.isMutexUse,
                    TIVX_MUTEX_MAX_OBJECTS, (uint32_t)sizeof(tivx_mutex_t), "TIVX_MUTEX_MAX_OBJECTS");
                break;
            case (vx_enum)TIVX_POSIX_TYPE_QUEUE:
                obj = ownAllocPosixObject(
                    (uint8_t *)g_tivx_posix_objects.queue, g_tivx_posix_objects.isQueueUse,
                    TIVX_QUEUE_MAX_OBJECTS, (uint32_t)sizeof(tivx_queue_context_t), "TIVX_QUEUE_MAX_OBJECTS");
                break;
            case (vx_enum)TIVX_POSIX_TYPE_TASK:
                obj = ownAllocPosixObject(
                    (uint8_t *)g_tivx_posix_objects.task, g_tivx_posix_objects.isTaskUse,
                    TIVX_TASK_MAX_OBJECTS, (uint32_t)sizeof(tivx_task_context_t), "TIVX_TASK_MAX_OBJECTS");
                break;
            default:
                obj = NULL;
                break;
        }
/* LDRA_JUSTIFY_END */
        (void)tivxMutexUnlock(g_tivx_objects_lock);
    }

    return (obj);
}

vx_status ownPosixObjectFree(uint8_t *obj, vx_enum type)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != obj)
    {
        status = tivxMutexLock(g_tivx_objects_lock);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_POSIX_OBJECTS_UBR002
<justification end> */
        if ((vx_status)VX_SUCCESS == status)
        {
            switch(type)
            {
                case (vx_enum)TIVX_POSIX_TYPE_EVENT:
                    status = ownFreePosixObject(obj,
                                (uint8_t *)g_tivx_posix_objects.event,
                                g_tivx_posix_objects.isEventUse,
                                TIVX_EVENT_MAX_OBJECTS,
                                (uint32_t)sizeof(tivx_event_t));
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Free parameter object failed\n");
                    }
                    break;
                case (vx_enum)TIVX_POSIX_TYPE_MUTEX:
                    status = ownFreePosixObject(obj,
                                (uint8_t *)g_tivx_posix_objects.mutex,
                                g_tivx_posix_objects.isMutexUse,
                                TIVX_MUTEX_MAX_OBJECTS,
                                (uint32_t)sizeof(tivx_mutex_t));
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Free parameter object failed\n");
                    }
                    break;
                case (vx_enum)TIVX_POSIX_TYPE_QUEUE:
                    status = ownFreePosixObject(obj,
                                (uint8_t *)g_tivx_posix_objects.queue,
                                g_tivx_posix_objects.isQueueUse,
                                TIVX_QUEUE_MAX_OBJECTS,
                                (uint32_t)sizeof(tivx_queue_context_t));
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Free parameter object failed\n");
                    }
                    break;
                case (vx_enum)TIVX_POSIX_TYPE_TASK:
                    status = ownFreePosixObject(obj,
                                (uint8_t *)g_tivx_posix_objects.task,
                                g_tivx_posix_objects.isTaskUse,
                                TIVX_TASK_MAX_OBJECTS,
                                (uint32_t)sizeof(tivx_task_context_t));
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Free parameter object failed\n");
                    }
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid reference type\n");
                    status = (vx_status)VX_ERROR_INVALID_REFERENCE;
                    break;
            }

            (void)tivxMutexUnlock(g_tivx_objects_lock);
        }
/* LDRA_JUSTIFY_END */
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Posix Object is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

vx_status ownPosixObjectInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    ownInitUseFlag(g_tivx_posix_objects.isEventUse,
        TIVX_EVENT_MAX_OBJECTS);
    ownInitUseFlag(g_tivx_posix_objects.isMutexUse,
        TIVX_MUTEX_MAX_OBJECTS);
    ownInitUseFlag(g_tivx_posix_objects.isQueueUse,
        TIVX_QUEUE_MAX_OBJECTS);
    ownInitUseFlag(g_tivx_posix_objects.isTaskUse,
        TIVX_TASK_MAX_OBJECTS);

    g_tivx_objects_lock = (tivx_mutex)ownAllocPosixObject(
                    (uint8_t *)g_tivx_posix_objects.mutex, g_tivx_posix_objects.isMutexUse,
                    TIVX_MUTEX_MAX_OBJECTS, (uint32_t)sizeof(tivx_mutex_t), "TIVX_MUTEX_MAX_OBJECTS");

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_POSIX_OBJECTS_UM001
<justification end> */
    if (NULL == g_tivx_objects_lock)
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "Error creating g_tivx_objects_lock\n");
    }
/* LDRA_JUSTIFY_END */

    return status;
}

vx_status ownPosixObjectDeInit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t error_index;

    status = ownFreePosixObject((uint8_t *)g_tivx_objects_lock,
                                (uint8_t *)g_tivx_posix_objects.mutex,
                                g_tivx_posix_objects.isMutexUse,
                                TIVX_MUTEX_MAX_OBJECTS,
                                (uint32_t)sizeof(tivx_mutex_t));

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_POSIX_OBJECTS_UBR003
<justification end> */
    if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
    {
        g_tivx_objects_lock = NULL;

        status = ownCheckUseFlag(g_tivx_posix_objects.isEventUse,
            TIVX_EVENT_MAX_OBJECTS, &error_index);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_POSIX_OBJECTS_UM003
<justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Deiniting event at index: %d failed\n", error_index);
        }
/* LDRA_JUSTIFY_END */
        status = ownCheckUseFlag(g_tivx_posix_objects.isQueueUse,
            TIVX_QUEUE_MAX_OBJECTS, &error_index);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_POSIX_OBJECTS_UM004
<justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Deiniting queue at index: %d failed\n", error_index);
        }
/* LDRA_JUSTIFY_END */
        status = ownCheckUseFlag(g_tivx_posix_objects.isTaskUse,
            TIVX_TASK_MAX_OBJECTS, &error_index);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_POSIX_OBJECTS_UM002
<justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Deiniting task at index: %d failed\n", error_index);
        }
/* LDRA_JUSTIFY_END */
        status = ownCheckUseFlag(g_tivx_posix_objects.isMutexUse,
            TIVX_MUTEX_MAX_OBJECTS, &error_index);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_POSIX_OBJECTS_UM005
<justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Deiniting mutex at index: %d failed\n", error_index);
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

