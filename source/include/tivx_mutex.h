/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_MUTEX_H_
#define _TIVX_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Mutex APIs
 */

/*!
 * \brief Typedef for a mutex
 *
 * \ingroup group_tivx_mutex
 */
typedef struct _tivx_mutex_t *tivx_mutex;


/*!
 * \brief Create a mutex
 *
 * \param mutex [out] Pointer to muter object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_mutex
 */
vx_status tivxMutexCreate(tivx_mutex *mutex);

/*!
 * \brief Delete a mutex
 *
 * \param mutex [in] Pointer to mutex object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_mutex
 */
vx_status tivxMutexDelete(tivx_mutex *mutex);

/*!
 * \brief Lock a mutex
 *
 * \param mutex [in] mutex object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_mutex
 */
vx_status tivxMutexLock(tivx_mutex mutex);

/*!
 * \brief UnLock a mutex
 *
 * \param mutex [in] mutex object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_mutex
 */
vx_status tivxMutexUnlock(tivx_mutex mutex);


#ifdef __cplusplus
}
#endif

#endif
