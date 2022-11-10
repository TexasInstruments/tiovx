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




#ifndef TIVX_TARGET_H_
#define TIVX_TARGET_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <TI/tivx_config.h>

/*!
 * \file
 * \brief Implementation of Target Object APIs
 */

/*! \brief Target ID bit mask
 *  \details Set assuming max 256 Targets
 * \ingroup group_tivx_target
 */
#define TIVX_TARGET_INST_MASK (0xFFu)

/*! \brief Target ID bit mask
 *  \details Target ID is in lowest position, so shift of 0
 * \ingroup group_tivx_target
 */
#define TIVX_TARGET_INST_SHIFT (0x0u)

/*! \brief CPU ID bit mask
 *  \details This is CPU from the perspective of different OS.  For example
 *  if there are dual core A72 running 1 instance of HLOS, then this counts
 *  as 1 CPU.
 *  Set assuming max 16 CPUs
 * \ingroup group_tivx_target
 */
#define TIVX_CPU_ID_MASK    (0xFu)

/*! \brief CPU ID bit shift
 *  \details CPU ID is in higher position, so shift of 8 to get above TIVX_TARGET_INST_MASK
 * \ingroup group_tivx_target
 */
#define TIVX_CPU_ID_SHIFT   (0x8)

/*! \brief Target ID bit mask
 * \ingroup group_tivx_target
 */
#define TIVX_TARGET_ID_MASK    (((uint32_t)TIVX_CPU_ID_MASK << (uint32_t)TIVX_CPU_ID_SHIFT) | \
                                ((uint32_t)TIVX_TARGET_INST_MASK << (uint32_t)TIVX_TARGET_INST_SHIFT))

/*! \brief Target ID bit shift
 * \ingroup group_tivx_target
 */
#define TIVX_TARGET_ID_SHIFT    (TIVX_TARGET_INST_SHIFT)

/*! \brief Make target based on CPU ID
 *
 *         Currently it assumes there are max 16 CPUs in system
 *         and max 256 targets on a given CPU in system
 *         (PC emulation is worst case as all targets are on x86)
 *
 * \ingroup group_tivx_target
 */
#define TIVX_MAKE_TARGET_ID(cpu, target_inst)  ((((((uint32_t)cpu) & (uint32_t)TIVX_CPU_ID_MASK) << (uint32_t)TIVX_CPU_ID_SHIFT) |((((uint32_t)target_inst)&TIVX_TARGET_INST_MASK) << (uint32_t)TIVX_TARGET_INST_SHIFT)))

/*! \brief Get CPU Id from given target
 *
 *         CPU Id is stored in upper nibble of the lower 8bits of the target
 *
 * \ingroup group_tivx_target
 */
#define TIVX_GET_CPU_ID(target)                ((((uint32_t)(target) >> (uint32_t)TIVX_CPU_ID_SHIFT) & (uint32_t)TIVX_CPU_ID_MASK))

/*! \brief Get the target instances from target
 *
 *         Target Id is stored in lower nibble of 8bit target
 *
 * \ingroup group_tivx_target
 */
#define TIVX_GET_TARGET_INST(target)           ((uint16_t)(((uint16_t)(target) >> TIVX_TARGET_INST_SHIFT)) & TIVX_TARGET_INST_MASK)


/*! \brief Const to denote invalid ID's
 * \ingroup group_tivx_target
 */
#define TIVX_TARGET_ID_INVALID      (0xFFFFFFFFU)

/*! \brief Const to denote invalid ID's
 * \ingroup group_tivx_target
 */
#define TIVX_CPU_ID_INVALID         (0xFFFFFFFFu)


/*! \brief Max size of target name string */
#define TIVX_TARGET_MAX_TASK_NAME   (12u)

/*! \brief Command ID for commands that can be send to a target
 * \ingroup group_tivx_target
 */
typedef enum _tivx_target_cmd_e {

    /*! \brief Command to create a node on target */
    TIVX_CMD_NODE_CREATE  = 0x00000001u,
    /*! \brief Command to delete a node on target */
    TIVX_CMD_NODE_DELETE  = 0x00000002u,
    /*! \brief Command to control a node on target */
    TIVX_CMD_NODE_CONTROL  = 0x00000003u,
    /*! \brief Command to call a user callback, on node execution complete */
    TIVX_CMD_NODE_USER_CALLBACK  = 0x00000004u,
    /*! \brief Command to inform host that a ref is consumed and is available for user dequeue */
    TIVX_CMD_DATA_REF_CONSUMED = 0x00000005u,
} tivx_target_cmd_e;

/*! \brief Target Object */
typedef struct _tivx_target *tivx_target;

/*!
 * \brief Target object internal state
 *
 * \ingroup group_tivx_target
 */
typedef struct _tivx_target {

    /*! \brief ID of this target */
    vx_enum target_id;

    /*! \brief Handle to underlying task associate with this target */
    tivx_task task_handle;

    /*! \brief Parameters of task associated with this target
     */
    tivx_task_create_params_t task_params;

    /*! \brief handle to job queue associated with this target */
    tivx_queue job_queue_handle;

    /*! \brief queue memory */
    uintptr_t job_queue_memory[TIVX_TARGET_MAX_JOB_QUEUE_DEPTH];

    /*! \brief target main */
    tivx_task_main_f target_main;

    /*! \brief Flag to request target to exit main so that it can be deleted */
    vx_bool targetExitRequest;

    /*! \brief Flag to indicate target exited its processing loop and is about to exit
     *   its main function
     */
    vx_bool targetExitDone;
} tivx_target_t;

/*!
 * \brief Parameters that can be set during target creation
 *
 * \ingroup group_tivx_target
 */
typedef struct _tivx_target_create_params
{
    /*! \brief Pointer to task stack, if NULL then task stack
     *         is allcoated by OS and not supplied by user
     */
    uint8_t *task_stack_ptr;

    /*! \brief Task stack size, if 0, OS allocates stack with default size
     */
    uint32_t task_stack_size;

    /*! \brief If task runs on a SMP CPU then this value tells the affinity
     *         of task to a given core,
     *         Valid values are 0 .. max cores in the SMP CPU.
     *         when TIVX_TASK_AFFINITY_ANY is used OS decides the task affinity.
     */
    uint32_t task_core_affinity;

    /*! \brief task priority for task associated with this target
     *         TIVX_TASK_PRI_HIGHEST is highest priority,
     *         TIVX_TASK_PRI_LOWEST is lowest priority
     */
    uint32_t task_priority;

    /*! \brief Task name associated with this target */
    char     task_name[TIVX_TARGET_MAX_TASK_NAME];

} tivx_target_create_params_t;

/*!
 * \brief Get CPU ID associated with given target ID
 *
 * \param target_id [in] Target ID
 *
 * \return CPU ID if target found on some CPU
 * \return TIVX_CPU_ID_INVALID if target not found any CPU
 *
 * \ingroup group_tivx_target
 */
vx_enum ownTargetGetCpuId(vx_enum target_id);

/*!
 * \brief Queue object descriptor to a given target
 *
 * \param target_id [in] Target ID
 * \param obj_desc_id [in] object descriptor ID
 *
 * \return VX_SUCCESS on success
 * \return VX_ERROR_NO_RESOURCES if queue is full
 * \return VX_FAILURE on other errors
 *
 * \ingroup group_tivx_target
 */
vx_status ownTargetQueueObjDesc(vx_enum target_id, uint16_t obj_desc_id);

/*! \brief Trigger execution of a node obj desc
 *
 * \ingroup group_tivx_target
 */
void ownTargetTriggerNode(uint16_t node_obj_desc_id);


/*! \brief Acquire parameters for node execution
 *
 * \ingroup group_tivx_target
 */
void ownTargetNodeDescAcquireAllParameters(tivx_obj_desc_node_t *node_obj_desc,
            uint16_t prm_obj_desc_id[], vx_bool *is_node_blocked);


/*! \brief Acquire parameters for node execution during pipeup phase
 *
 * \ingroup group_tivx_target
 */
void ownTargetNodeDescAcquireAllParametersForPipeup(tivx_obj_desc_node_t *node_obj_desc,
            uint16_t prm_obj_desc_id[]);

/*! \brief Release parameters that were previously acquired
 *
 * \ingroup group_tivx_target
 */
void ownTargetNodeDescReleaseAllParameters(tivx_obj_desc_node_t *node_obj_desc, uint16_t prm_obj_desc_id[]);

/*!
 * \brief Used to set default target create parameters in parameter structure
 *
 * \param params [out] Params to use for target creation
 *
 * \ingroup group_tivx_target
 */
void ownTargetSetDefaultCreateParams(tivx_target_create_params_t *params);

/*!
 * \brief Create a target object and associate with given target ID
 *
 *        During system init, this API needs to be called for all targets local to this CPU
 *        Make sure two different CPUs do not associate against the same target ID
 *
 * \param target_id [in] Target ID
 * \param params [in] Params to use for target creation
 *
 * \ingroup group_tivx_target
 */
vx_status ownTargetCreate(vx_enum target_id, const tivx_target_create_params_t *params);


/*!
 * \brief Delete a target object
 *
 *        During system deinit, this API needs to be called for all targets local to this CPU
 *
 * \param target_id [in] Target ID
 *
 * \ingroup group_tivx_target
 */
vx_status ownTargetDelete(vx_enum target_id);

/*!
 * \brief Init global state of target module
 *
 *        Also inits target kernel, target kernel instance
 *
 * \ingroup group_tivx_target
 */
void ownTargetInit(void);

/*!
 * \brief De-Init global state of target module
 *
 *        Also de-inits target kernel, target kernel instance
 *
 * \ingroup group_tivx_target
 */
void ownTargetDeInit(void);


/*!
 * \brief Propagates time stamp of input parameters of a kernel to output parameters
 *
 * \ingroup group_tivx_target
 */
void ownTargetSetTimestamp(
    const tivx_obj_desc_node_t *node_obj_desc, tivx_obj_desc_t *obj_desc[]);

#ifdef __cplusplus
}
#endif

#endif
