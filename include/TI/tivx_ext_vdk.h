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
#ifndef TIVX_EXT_VDK_H_
#define TIVX_EXT_VDK_H_

/*!
 * \file
 * \brief The TI VDK extension.
 */

/*!
 * \defgroup group_vdk VDK Extension APIs
 * \brief APIs required to integrating OpenVX target within VDK as a model
 * \ingroup group_tivx_ext_host
 */

#define TIVX_VDK  "tivx_vdk"

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

/* This value defines the upper bound of the count of possible mailbox clusters across every SoC. */
#define MAX_NUM_MBOX_CLUSTERS (8u * 12u)

typedef struct
{
    /* Bitmask of user interrupts that should trigger this model when asserted */
    uint16_t user_idx_bitmask[MAX_NUM_MBOX_CLUSTERS/4];
    /* Bitmask of mailbox fifo IDs that should be read by this model when the appropriate mbox interrupt is triggered */
    uint16_t fifo_id_bitmask[MAX_NUM_MBOX_CLUSTERS];

} tivx_mbox_config_t;

/*!
* \brief The user-defined print log function.
* \return void.
* \ingroup group_vdk
*/
typedef void  (*tivx_vdk_print_log_f)(const char * string, void * obj);

/*!
* \brief The user-defined function which returns the PC host pointer corresponding to a
* symbol in the loaded image.
* \return address pointer.
* \ingroup group_vdk
*/
typedef void * (*tivx_vdk_get_host_ptr_from_symbol_f)(const char * symbol, void * obj);

/*!
* \brief The user-defined function which returns the PC host pointer corresponding the
* associated physical address pointer from within the simulation environment.
* \return address pointer.
* \ingroup group_vdk
*/
typedef uint64_t(*tivx_vdk_get_host_ptr_from_phy_ptr_f)(uint64_t phy_ptr, void * obj);

/*!
* \brief The user-defined function which is meant to push a payload onto the appropriate
* mailbox for the given destination cpu id and port id.
* \return success is 0.
* \ingroup group_vdk
*/
typedef int32_t(*tivx_vdk_ipc_send_mbox_f)(uint32_t payload, uint32_t mailbox_fifo_id, void * obj);

/*!
* \brief The user-defined function which is meant to acquire or release a spinlock.
* \return success is 0.
* \ingroup group_vdk
*/
typedef uint32_t(*tivx_vdk_handle_spinlock_f)(uint32_t hw_lock_id, uint32_t is_acquire, void * obj);

/*! \brief Registers several callback functions with the OpenVX framework. This
 *         enables OpenVX framework target (such as a C7x core) to be run
 *         in the context of an SoC simulator environment, which has different
 *         memory pointer address spaces, potentially a different standard output
 *         for logging prints, and needs to send messages across a simulated MBOX interface.
 * \note This function should be called once during initialization of the application BEFORE app_init.
 * \param [in] print_log                 The pointer to <tt>\ref tivx_vdk_print_log_f</tt>.
 * \param [in] get_host_ptr_from_symbol  The pointer to <tt>\ref tivx_vdk_get_host_ptr_from_symbol_f</tt>.
 * \param [in] get_host_ptr_from_phy_ptr The pointer to <tt>\ref tivx_vdk_get_host_ptr_from_phy_ptr_f</tt>.
 * \param [in] ipc_send_mbox             The pointer to <tt>\ref tivx_vdk_ipc_send_mbox_f</tt>.
 * \param [in] handle_spinlock           The pointer to <tt>\ref tivx_vdk_handle_spinlock_f</tt>.
 * \param [in] obj                       The pointer to the application object.
 * \ingroup group_vdk
 * \return <tt>\ref vx_status</tt>.
 */
int32_t tivxVdkRegisterCallbacks(
                             tivx_vdk_print_log_f print_log,
                             tivx_vdk_get_host_ptr_from_symbol_f get_host_ptr_from_symbol,
                             tivx_vdk_get_host_ptr_from_phy_ptr_f get_host_ptr_from_phy_ptr,
                             tivx_vdk_ipc_send_mbox_f ipc_send_mbox,
                             tivx_vdk_handle_spinlock_f handle_spinlock,
                             void *obj);

/*! \brief Calls into OpenVX function to handle an IPC mailbox message.
 * \note This function is called each time a payload is read from the mbox.
 * \param [in] payload    payload containing target & object descriptor ID's.
 * \param [in] mailbox_fifo_id ID of origin mailbox fifo.
 * \ingroup group_vdk
 */
int32_t tivxVdkIpcRecvMbox(uint32_t payload, uint32_t mailbox_fifo_id);

/*! \brief Gets the mailbox configuration based on selected cpus.
* \note This function is called once (before or after tivxVdkRegisterCallbacks) during initialization of the application.
* \param [in] cpu_bitmask The list of cpus that this model represents; use bit offsets based on left shift of VDK_CoreID.
* \param [out] mbox_config The pointer to \ref tivx_mbox_config_t.
* \ingroup group_vdk
* \return success=0, fail=nonzero.
*/
int32_t tivxVdkGetMboxConfiguration(uint32_t cpu_bitmask, tivx_mbox_config_t *mbox_config);

/*! \brief Reports whether the VDK is enabled or not.
 * \ingroup group_vdk
 * \return 1 if VDK is enabled, 0 otherwise.
 */
uint32_t tivxVdkIsEnabled(void);

/*! \brief Gets the VDK wrapper object if it's enabled.
 * \ingroup group_vdk
 * \return void * to VDK wrapper object, NULL otherwise.
 */
void * tivxGetVdkObj(void);

/*! \brief OpenVX wrapper to get the VDK host pointer from a given symbol
 * \ingroup group_vdk
 * \param [in] symbol The symbol name to be searched for.
 * \return pointer to the host address of the symbol.
 */
void * tivxVdkGetHostPtrFromSymbol(const char * symbol);

/*! \brief OpenVX wrapper to get the VDK host pointer from a given phyiscal pointer
 * \ingroup group_vdk
 * \param [in] phy_ptr The physical pointer to be translated.
 * \return pointer to the host address of the physical pointer.
 */
uint64_t tivxVdkGetHostPtrFromPhyPtr(uint64_t phy_ptr);

/*! \brief OpenVX wrapper to retrieve a bitmask of emulated cores
 * \ingroup group_vdk
 * \return bitmask of emulated cores
 */
uint32_t tivxVdkGetEmulatedCores(void);

/*! \brief OpenVX wrapper to retrieve the TIOVX-mapped CPU ID
 * \ingroup group_vdk
 * \return TIOVX-mapped CPU ID on success, -1 on failure
 */
int16_t tivxVdkGetSelfOvxIpcCpuId(void);

/*! \brief OpenVX wrapper to retrieve the app_utils-mapped CPU ID
 * \ingroup group_vdk
 * \return app_utils-mapped CPU ID on success, -1 on failure
 */
int16_t tivxVdkGetSelfAppIpcCpuId(void);

/*! \brief OpenVX wrapper to retrieve the csl-mapped CPU ID
 * \ingroup group_vdk
 * \return csl-mapped CPU ID on success, -1 on failure
 */
int16_t tivxVdkGetSelfCslIpcCpuId(void);

#ifdef  __cplusplus
}
#endif

#endif
