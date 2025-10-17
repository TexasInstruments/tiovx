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




#ifndef TIVX_PLATFORM_H_
#define TIVX_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <tivx_obj_desc_priv.h>
#include <TI/tivx_log_stats.h>

/*!
 * \file
 * \brief Platform APIs
 */

/*!
 * \brief Types of system level locks
 *
 * \ingroup group_tivx_platform
 */
typedef enum {

    /*! \brief Lock the shared object descriptor table during
     *         object descriptor slot alloc and dealloc */
    TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE = 0,

    /*! \brief Lock the context during context create and delete */
    TIVX_PLATFORM_LOCK_CONTEXT,

    /*! \brief Lock the data reference queue during enqueue and dequeue */
    TIVX_PLATFORM_LOCK_DATA_REF_QUEUE,

    /*! \brief Lock the index information of run-time logger */
    TIVX_PLATFORM_LOCK_LOG_RT_INDEX,

    /*! \brief Lock the run-time logger */
    TIVX_PLATFORM_LOCK_LOG_RT,

    /*! \brief Max number of locks */
    TIVX_PLATFORM_LOCK_MAX

} tivx_platform_lock_type_e;

/*!
 * \brief Get size of TIOVX object descriptor memory carveout
 *
 * \param shm_size [in] Address of unsigned int to hold size of shared memory
 *
 * \return 0 for success status
 *
 * \ingroup group_tivx_platform
 */
int32_t tivxPlatformGetShmSize (uint32_t * shm_size);

/*! \brief Struct containing object descriptor allocation counts. Allows
 *         for a log to be kept of the object descriptors used throughout
 *         runtime.
 *
 * \ingroup group_tivx_platform
 */
typedef struct _tivx_shm_obj_count_t {

    uint32_t  value; /**< Count of times the resource is currently allocated */

    uint32_t  max_value; /**< Highest resource count during current runtime */

    char      name[TIVX_RESOURCE_NAME_MAX]; /**< Name of the object descriptor */

    vx_enum   object_type; /**< Current resource's enumeration */

    vx_bool   is_configured; /**< Flag to indicate a resource's max exists in tivx_config.h*/

} tivx_shm_obj_count_t;

/*! \brief Struct containing sizes of different OS object types. Allows
 *         for logging of these OS objects throughout runtime.
 *
 * \ingroup group_tivx_platform
 */
typedef struct _tivx_os_obj_sizes_t {

    const uint32_t  event_size; /**< Size of an event object */

    const uint32_t  mutex_size; /**< Size of a mutex object */

    const uint32_t  queue_size; /**< Size of a queue object */

    const uint32_t  task_size;  /**< Size of a task object */

} tivx_os_obj_sizes_t;

/*!
 * \brief Increment count of given resource
 *
 * \param resource_name [in] Enum of resource whose count should be updated
 *
 * \ingroup group_tivx_platform
 */
void ownTableIncrementValue(vx_enum resource_name);

/*!
 * \brief Decrement count of given resource
 *
 * \param resource_name [in] Enum of resource whose count should be updated
 *
 * \ingroup group_tivx_platform
 */
void ownTableDecrementValue(vx_enum resource_name);

/*!
 * \brief Convert a target name to a specific target ID
 *
 * \param target_name [in] Target name
 *
 * \return target ID
 *
 * \ingroup group_tivx_platform
 */
vx_enum ownPlatformGetTargetId(const char *target_name);

/*!
 * \brief Convert a specific target ID to a target name
 *
 * \param target_id [in] Target ID
 * \param target_name [out] Target name
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformGetTargetName(vx_enum target_id, char *target_name);

/*!
 * \brief Return shared memory info which holds the object descriptors
 *
 *        This is platform APIs since method of specifying shared memory,
 *        number of object descriptors is platform dependant
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformGetObjDescTableInfo(tivx_obj_desc_table_info_t *table_info);

/*!
 * \brief Return shared memory info which holds the run-time logger buffer
 *
 *        This is platform APIs since method of specifying shared memory,
 *        is platform dependant
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformGetLogRtShmInfo(void **shm_base, uint32_t *shm_size);

/*!
 * \brief Take a system level lock
 *
 *        This locks is taken across all targets to mutual exclusion
 *        across targets
 *
 *        WARNING: No OS calls shall be made after calling this function
 *        until the \ref ownPlatformSystemUnlock has been called due to the
 *        chance that interrupts may be disabled during this duration.
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformSystemLock(vx_enum lock_id);

/*!
 * \brief Release system level lock
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformSystemUnlock(vx_enum lock_id);

/*!
 * \brief Init Platform module
 *
 * \ingroup group_tivx_platform
 */
vx_status ownPlatformInit(void);

/*!
 * \brief DeInit Platform module
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformDeInit(void);

/*!
 * \brief Print given string
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformPrintf(const char *format);

/*!
 * \brief Function to set the target configuration
 *
 *        It creates target and adds it to the list of targets supported
 *        on each core.
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformCreateTargets(void);

/*!
 * \brief Function to destroy created targets
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformDeleteTargets(void);

/*!
 * \brief Utility function to enable Platform specific things
 *        Currently Used for EVE algorithm to enable EDMA
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformActivate(void);

/*!
 * \brief Utility function to disable Platform specific things
 *        Currently Used for EVE algorithm to disable EDMA
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformDeactivate(void);

/*!
 * \brief Utility function to call OS-specific task init functions
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformTaskInit(void);

/*! \brief Function to query targets for their TIOVX resource statistics
 *
 * \ingroup group_tivx_platform
 */
void ownPlatformGetTargetPerfStats(uint32_t app_cpu_id, uint32_t target_values[TIVX_TARGET_RESOURCE_COUNT]);

#ifdef __cplusplus
}
#endif

#endif
