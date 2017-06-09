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



#ifndef TIVX_TASK_H_
#define TIVX_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to Task APIs
 */

/*!
 * \brief Constant to define highest priority for a task
 *
 * \ingroup group_tivx_task
 */
#define TIVX_TASK_PRI_HIGHEST   (0u)

/*!
 * \brief Constant to define lowest priority for a task
 *
 * \ingroup group_tivx_task
 */
#define TIVX_TASK_PRI_LOWEST    (15u)

/*!
 * \brief constant to indicate task affinity can be decided by OS on a SMP CPU
 *
 * \ingroup group_tivx_task
 */
#define TIVX_TASK_AFFINITY_ANY      (0xFFFFu)

/*!
 * \brief Max Task Name Size
 *
 * \ingroup group_tivx_task
 */
#define TIVX_MAX_TASK_NAME         (20)

/*! \brief Entry point of task
 *
 *  \param target [in] target handle
 *
 * \ingroup group_tivx_target
 */
typedef void (VX_CALLBACK *tivx_task_main_f)(void *app_var);


/*!
 * \brief Typedef for a task
 *
 * \ingroup group_tivx_task
 */
typedef struct _tivx_task_t
{
    /*! \brief Handle to the task created
     */
    void *tsk_handle;

    /*! \brief Pointer to task stack, if NULL then task stack
     *         is allcoated by OS and not supplied by user
     */
    uint8_t *stack_ptr;

    /*! \brief Task stack size, if 0, OS allocates stack with default size
     */
    uint32_t stack_size;

    /*! \brief If task runs on a SMP CPU then this value tells the affinity
     *         of task to a given core,
     *         Valid values are 0 .. max cores in the SMP CPU.
     *         when TIVX_TASK_AFFINITY_ANY is used OS decides the task affinity.
     */
    uint32_t core_affinity;

    /*! \brief task priority for task associated with this target
     *         TIVX_TASK_PRI_HIGHEST is highest priority,
     *         TIVX_TASK_PRI_LOWEST is lowest priority
     */
    uint32_t priority;

    /*! \brief Entry point for task */
    tivx_task_main_f task_func;

    /*! \brief private app object */
    void *app_var;
} tivx_task;



/*!
 * \brief Parameters that can be set during task creation
 *
 * \ingroup group_tivx_task
 */
typedef struct _tivx_task_create_params
{
    /*! \brief Pointer to task stack, if NULL then task stack
     *         is allcoated by OS and not supplied by user
     */
    uint8_t *stack_ptr;

    /*! \brief Task stack size, if 0, OS allocates stack with default size
     */
    uint32_t stack_size;

    /*! \brief If task runs on a SMP CPU then this value tells the affinity
     *         of task to a given core,
     *         Valid values are 0 .. max cores in the SMP CPU.
     *         when TIVX_TASK_AFFINITY_ANY is used OS decides the task affinity.
     */
    uint32_t core_affinity;

    /*! \brief task priority for task associated with this target
     *         TIVX_TASK_PRI_HIGHEST is highest priority,
     *         TIVX_TASK_PRI_LOWEST is lowest priority
     */
    uint32_t priority;

    /*! \brief Entry point for task */
    tivx_task_main_f task_main;

    /*! \brief private app object */
    void *app_var;


    /*! \brief Task Name */
    char task_name[TIVX_MAX_TASK_NAME];
} tivx_task_create_params_t;




/*!
 * \brief Used to set default task create parameters in parameter structure
 *
 * \param params [out] Params to use for task creation
 *
 * \ingroup group_tivx_task
 */
void tivxTaskSetDefaultCreateParams(tivx_task_create_params_t *params);

/*!
 * \brief Create a task
 *
 * \param task [out] Pointer to task object
 * \param params [in] Task create parameters
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_task
 */
vx_status tivxTaskCreate(tivx_task *task, tivx_task_create_params_t *params);

/*!
 * \brief Delete a task
 *
 * \param task [in] Pointer to task object
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_task
 */
vx_status tivxTaskDelete(tivx_task *task);

/*!
 * \brief waits/sleeps for given milliseconds
 *
 * \param msec [in] amount of milliseconds to sleep
 *
 * \ingroup group_tivx_task
 */
void tivxTaskWaitMsecs(uint32_t msec);

#ifdef __cplusplus
}
#endif

#endif
