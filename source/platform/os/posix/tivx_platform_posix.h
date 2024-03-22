/*
*
* Copyright (c) 2023 - 2023 Texas Instruments Incorporated
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

#ifndef TIVX_PLATFORM_POSIX_H_
#define TIVX_PLATFORM_POSIX_H_

#include <vx_internal.h>
#include <pthread.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   /* for nanosleep */

int nanosleep(const struct timespec *req, struct timespec *rem);

#else
#include <unistd.h> /* for usleep */

extern int usleep (useconds_t __useconds);

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PRI_MAX  sched_get_priority_max(SCHED_FIFO)
#define PRI_MIN  sched_get_priority_min(SCHED_FIFO)

/*! \brief Max number of \ref tivx_event objects
 * \ingroup group_tivx_platform
 */
#define TIVX_EVENT_MAX_OBJECTS           (1024u)

/*! \brief Max number of \ref tivx_mutex objects
 * \ingroup group_tivx_platform
 */
#define TIVX_MUTEX_MAX_OBJECTS           (1024u)

/*! \brief Max number of \ref tivx_queue objects
 * \ingroup group_tivx_platform
 */
#define TIVX_QUEUE_MAX_OBJECTS           (1024u)

/*! \brief Max number of \ref tivx_task objects
 * \ingroup group_tivx_platform
 */
#define TIVX_TASK_MAX_OBJECTS            (1024u)

/*! \brief The posix type enumeration lists all posix types
 *         that can be allocated via \ref ownObjectPosixAlloc.
 * \ingroup group_tivx_platform
 */
enum tivx_posix_type_e {
    TIVX_POSIX_TYPE_INVALID   = 0x000,/*!< \brief An invalid type value. When passed an error must be returned. */
    TIVX_POSIX_TYPE_EVENT     = 0x001,/*!< \brief A <tt>\ref tivx_event</tt>. */
    TIVX_POSIX_TYPE_MUTEX     = 0x002,/*!< \brief A <tt>\ref tivx_mutex</tt>. */
    TIVX_POSIX_TYPE_QUEUE     = 0x003,/*!< \brief A <tt>\ref tivx_queue_context_t</tt>. */
    TIVX_POSIX_TYPE_TASK      = 0x004,/*!< \brief A <tt>\ref tivx_task_context_t</tt>. */
};

/*!
 * \brief Typedef for an event
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_event_t {

    uint16_t is_set;
    pthread_mutex_t lock;
    pthread_cond_t  cond;

} tivx_event_t;

/*!
 * \brief Typedef for a mutex
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_mutex_t {

  pthread_mutex_t lock;

} tivx_mutex_t;

/*! \brief Handle to queue context
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_queue_context *tivx_queue_context;

/*!
 * \brief Typedef for a queue context
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_queue_context {

  pthread_mutex_t lock;
  pthread_cond_t  condGet;
  pthread_cond_t  condPut;

} tivx_queue_context_t;

/*! \brief Handle to queue task
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_task_context *tivx_task_context;

/*!
 * \brief Typedef for a task context
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_task_context {

  pthread_t hndl;

} tivx_task_context_t;

/*!
 * \brief Structure to hold all posix platform objects
 *
 * \ingroup group_tivx_platform
 */
typedef struct tivx_vx_platform_posix_t
{
    tivx_event_t             event[TIVX_EVENT_MAX_OBJECTS];
    /**< Event Objects */
    vx_bool                  isEventUse[TIVX_EVENT_MAX_OBJECTS];
    /**< Flag indicating if event is in use or not */
    tivx_mutex_t             mutex[TIVX_MUTEX_MAX_OBJECTS];
    /**< Mutex Objects */
    vx_bool                  isMutexUse[TIVX_MUTEX_MAX_OBJECTS];
    /**< Flag indicating if mutex is in use or not */
    tivx_queue_context_t     queue[TIVX_QUEUE_MAX_OBJECTS];
    /**< Queue Objects */
    vx_bool                  isQueueUse[TIVX_QUEUE_MAX_OBJECTS];
    /**< Flag indicating if queue is in use or not */
    tivx_task_context_t      task[TIVX_TASK_MAX_OBJECTS];
    /**< Task Objects */
    vx_bool                  isTaskUse[TIVX_TASK_MAX_OBJECTS];
    /**< Flag indicating if task is in use or not */
} tivx_platform_posix_t;

/*! \brief Alloc memory for a reference of specified type
 * \param [in] type The reference type. See \ref tivx_posix_type_e
 * \return ref The posix object reference.
 * \ingroup group_tivx_platform
 */
uint8_t *ownPosixObjectAlloc(vx_enum type);

/*! \brief Free memory for a posix object
 * \param [in] ref The posix object reference.
 * \param [in] type The reference type. See \ref tivx_posix_type_e
 * \return VX_SUCCESS on success
 * \ingroup group_tivx_platform
 */
vx_status ownPosixObjectFree(uint8_t *obj, vx_enum type);

/*! \brief Initialize posix object module
 * \ingroup group_tivx_platform
 */
vx_status ownPosixObjectInit(void);

/*! \brief De-Initialize posix object module
 * \ingroup group_tivx_platform
 */
vx_status ownPosixObjectDeInit(void);

#ifdef __cplusplus
}
#endif

#endif

