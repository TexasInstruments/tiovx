/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_PLATFORM_PRIV_H_
#define _TIVX_PLATFORM_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Vision SDK Platform internal APIs
 */


/*!
 * \brief Platform IPC handler
 *
 * \param payload [in] payload
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformIpcHandler(uint32_t payload);


#ifdef __cplusplus
}
#endif

#endif
