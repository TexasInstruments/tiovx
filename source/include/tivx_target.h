/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_TARGET_H_
#define _TIVX_TARGET_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Target Object APIs
 */

/*!
 * \brief Max depth of queue associated with target
 * \ingroup group_tivx_target
 */
#define TIVX_TARGET_MAX_QUEUE_DEPTH         (32u)

/*! \brief Command ID for commands that can be send to a target
 * \ingroup group_tivx_target
 */
typedef enum _tivx_target_cmd_e {

    /*! \brief Command to create a node on target */
    TIVX_CMD_NODE_CREATE  = 0x00000001u,
    /*! \brief Command to delete a node on target */
    TIVX_CMD_NODE_DELETE  = 0x00000002u,
    /*! \brief Command to call a user callback, on node execution complete */
    TIVX_CMD_NODE_USER_CALLBACK  = 0x00000003u,

} tivx_target_cmd_e;

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_target
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = 0,

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = 1,

    /*! \brief target ID for EVE1 */
    TIVX_TARGET_ID_EVE1 = 2,

    /*! \brief target ID for EVE2 */
    TIVX_TARGET_ID_EVE2 = 3,

    /*! \brief target ID for EVE3 */
    TIVX_TARGET_ID_EVE3 = 4,

    /*! \brief target ID for EVE4 */
    TIVX_TARGET_ID_EVE4 = 5,

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = 6,

    /*! \brief target ID for IPU1-1 */
    TIVX_TARGET_ID_IPU1_1 = 7,

    /*! \brief target ID for IPU2 */
    TIVX_TARGET_ID_IPU2_0 = 8,

    /*! \brief target ID for A15-0 */
    TIVX_TARGET_ID_A15_0 = 9,

    /*! \brief target ID for invalid target */
    TIVX_TARGET_ID_INVALID = 0xFFFFFFFFu

} tivx_target_id_e;


/*! \brief Target Object */
typedef struct _tivx_target *tivx_target;

/*!
 * \brief Target object internal state
 *
 * \ingroup group_tivx_target
 */
typedef struct _tivx_target {

    /*! \brief ID of this target */
    tivx_target_id_e target_id;

    /*! \brief Handle to underlying task associate with this target */
    tivx_task task_handle;

    /*! \brief Pointer to task stack, if NULL then task stack
     *         is allcoated by OS and not supplied by user
     */
    uint32_t *task_stack_ptr;


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
    uint32_t task_pri;

    /*! \brief handle to job queue associated with this target */
    tivx_queue job_queue_handle;

    /*! \brief queue memory */
    uint32_t job_queue_memory[TIVX_TARGET_MAX_QUEUE_DEPTH];

    /*! \brief target main */
    tivx_task_main_f target_main;

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
    uint32_t *task_stack_ptr;

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

} tivx_target_create_params_t;

/*!
 * \brief Convert a target name or target class to a specific target ID
 *
 * \param target_name [in] Target name
 *
 * \return target ID
 *
 * \ingroup group_tivx_target
 */
tivx_target_id_e tivxGetTargetId(const char *target_name);

/*!
 * \brief Match a user specified target_string with kernel suported target name
 *
 * \param kernel_target_name [in] Kernel supported target name
 * \param target_string [in] user specified target string
 *
 * \return vx_true_e if match found, else vx_false_e
 *
 * \ingroup group_tivx_target
 */
vx_bool tivxTargetMatch(const char *kernel_target_name, const char *target_string);


/*!
 * \brief Get handle associated with this target
 *
 *        If handle is NULL then target does not exists on this CPU
 *        Use tivxTargetGetCpuId() to check if target exists on another CPU
 *
 * \param target_id [in] Target ID
 *
 * \return target handle is target is on the same CPU
 * \return NULL, if target does not exists on this CPU
 *
 * \ingroup group_tivx_target
 */
tivx_target tivxTargetGetHandle(tivx_target_id_e target_id);

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
uint32_t tivxTargetGetCpuId(tivx_target_id_e target_id);

/*!
 * \brief Queue object descriptor to a given target
 *
 * \param target [in] Target ID
 * \param obj_desc_id [in] object descriptor ID
 *
 * \return VX_SUCCESS on success
 * \return VX_ERROR_NO_RESOURCES if queue is full
 * \return VX_FAILURE on other errors
 *
 * \ingroup group_tivx_target
 */
vx_status tivxTargetQueueObjDesc(tivx_target target, uint16_t obj_desc_id);

/*!
 * \brief DeQueue object descriptor from a given target
 *
 * \param target [in] Target ID
 * \param obj_desc_id [out] object descriptor ID
 * \param timeout [in] Amount of time to wait until obj_desc_id is received
 *                     TIVX_EVENT_TIMEOUT_xxxx can used.
 *
 * \return VX_SUCCESS on success
 * \return VX_ERROR_NO_RESOURCES if queu is full
 * \return VX_FAILURE on other errors
 *
 * \ingroup group_tivx_target
 */
vx_status tivxTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout);

/*!
 * \brief Used to set default target create parameters in parameter structure
 *
 * \param params [out] Params to use for target creation
 *
 * \ingroup group_tivx_target
 */
void tivxTargetSetDefaultCreateParams(tivx_target_create_params_t *params);

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
vx_status tivxTargetCreate(tivx_target_id_e target_id, tivx_target_create_params_t *params);


/*!
 * \brief Delete a target object
 *
 *        During system deinit, this API needs to be called for all targets local to this CPU
 *
 * \param target_id [in] Target ID
 *
 * \ingroup group_tivx_target
 */
vx_status tivxTargetDelete(tivx_target_id_e target_id);

/*!
 * \brief Associate a target ID with given CPU ID
 *
 *        This API needs to be called for all targets both on local CPU as well as remote CPUs
 *        All CPUs MUST have the same Target ID mapping to CPU ID mapping
 *
 * \param target_id [in] Target ID
 * \param cpu_id [in] CPU ID
 *
 * \ingroup group_tivx_target
 */
vx_status tivxTargetRegisterCpuId(tivx_target_id_e target_id, uint32_t cpu_id);

#ifdef __cplusplus
}
#endif

#endif
