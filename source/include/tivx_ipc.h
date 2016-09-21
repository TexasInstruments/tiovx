/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_IPC_H_
#define _TIVX_IPC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Inter-processor Communication (IPC) APIs
 */

/*!
 * \brief Callback definition of handler for IPC message's
 *
 * \ingroup group_tivx_ipc
 */
typedef void (*tivx_ipc_handler_f)(uint32_t payload);

/*!
 * \brief Send payload to cpu_id via IPC
 *
 * \ingroup group_tivx_ipc
 */
vx_status tivxIpcSendMsg(vx_enum cpu_id, uint32_t payload);

/*!
 * \brief Register callback to call when IPC message is received
 *
 * \ingroup group_tivx_ipc
 */
void tivxIpcRegisterHandler(tivx_ipc_handler_f ipc_handler_func);

/*!
 * \brief Return CPU ID of the CPU on which this API is called
 *
 * \ingroup group_tivx_ipc
 */
vx_enum tivxIpcGetSelfCpuId();

/*!
 * \brief Init IPC module
 *
 * \ingroup group_tivx_ipc
 */
void tivxIpcInit();

/*!
 * \brief DeInit IPC module
 *
 * \ingroup group_tivx_ipc
 */
void tivxIpcDeInit();

#ifdef __cplusplus
}
#endif

#endif
