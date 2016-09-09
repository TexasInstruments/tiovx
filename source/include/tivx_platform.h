/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_PLATFORM_H_
#define _TIVX_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <tivx_obj_desc_priv.h>

/*!
 * \file
 * \brief Platform APIs
 */

/*!
 * \brief Callback definition of handler for IPC message's
 *
 * \ingroup group_tivx_platform
 */
typedef void (*tivx_platform_ipc_handler_f)(uint32_t payload);

/*!
 * \brief Types of system level locks
 *
 * \ingroup group_tivx_platform
 */
typedef enum {

    /*! \brief Lock the shared object descriptor table */
    TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE = 0,

    /*! \brief Max number of locks */
    TIVX_PLATFORM_LOCK_MAX

} tivx_platform_lock_type_e;

/*!
 * \brief Convert a target name to a specific target ID
 *
 * \param target_name [in] Target name
 *
 * \return target ID
 *
 * \ingroup group_tivx_platform
 */
vx_enum tivxPlatformGetTargetId(const char *target_name);

/*!
 * \brief Match a user specified target_string with kernel suported target name
 *
 * \param kernel_target_name [in] Kernel supported target name
 * \param target_string [in] user specified target string
 *
 * \return vx_true_e if match found, else vx_false_e
 *
 * \ingroup group_tivx_platform
 */
vx_bool tivxPlatformTargetMatch(const char *kernel_target_name, const char *target_string);


/*!
 * \brief Get current time in units of micro-secs
 *
 * \ingroup group_tivx_platform
 */
uint32_t tivxPlatformGetTimeInUsecs();

/*!
 * \brief Wait for user specified number of milli-secs
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformWaitMsecs();

/*!
 * \brief Return shared memory info which holds the object descriptors
 *
 *        This is platform APIs since method of specifying shared memory,
 *        number of object descriptors is platform dependant
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformGetObjDescTableInfo(tivx_obj_desc_table_info_t *table_info);

/*!
 * \brief Take a system level lock
 *
 *        This locks is taken across all targets to mutual exclusion
 *        across targets
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformSystemLock(vx_enum lock_id);

/*!
 * \brief Release system level lock
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformSystemUnlock(vx_enum lock_id);

/*!
 * \brief Send payload to cpu_id via IPC
 *
 * \ingroup group_tivx_platform
 */
vx_status tivxPlatformSendIpcMsg(vx_enum cpu_id, uint32_t payload);

/*!
 * \brief Register callback to call when IPC message is received
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformRegisterIpcHandler(tivx_platform_ipc_handler_f ipc_handler_func);

/*!
 * \brief Return CPU ID of the CPU on which this API is called
 *
 * \ingroup group_tivx_platform
 */
vx_enum tivxPlatformGetSelfCpuId();

/*!
 * \brief Init Platform module
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformInit();

#ifdef __cplusplus
}
#endif

#endif
