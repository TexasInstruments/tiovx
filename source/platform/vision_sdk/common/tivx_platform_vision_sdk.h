/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_PLATFORM_VISION_SDK_H_
#define TIVX_PLATFORM_VISION_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Vision SDK Platform APIs
 */


/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_config.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS            (12u)


/*! \brief Maximum number obj descriptors that are present in shared memory
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST  (1024u)


/*! \brief Default buffer allocation alignment
 * \ingroup group_tivx_platform
 */
#define TIVX_MEM_BUFFER_ALLOC_ALIGN             (16U)

/*! \brief Macro to check the alignment of the size of
 *         the shared memory entry
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN      (8U)

/*! \brief HW spinlock ID to use for locking data ref queue
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID    (255u)

/*! \brief Macros for build time check
 * \ingroup group_tivx_platform
 */
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define BUILD_ASSERT(e) \
     enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0u),

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP2, 0u),

    /*! \brief target ID for EVE1 */
    TIVX_TARGET_ID_EVE1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE1, 0u),

    /*! \brief target ID for EVE2 */
    TIVX_TARGET_ID_EVE2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE2, 0u),

    /*! \brief target ID for EVE3 */
    TIVX_TARGET_ID_EVE3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE3, 0u),

    /*! \brief target ID for EVE4 */
    TIVX_TARGET_ID_EVE4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE4, 0u),

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 0u),

    /*! \brief target ID for IPU1-1 */
    TIVX_TARGET_ID_IPU1_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_1, 0u),

    /*! \brief target ID for IPU2 */
    TIVX_TARGET_ID_IPU2_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU2_0, 0u),

    /*! \brief target ID for A15-0 */
    TIVX_TARGET_ID_A15_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A15_0, 0u),

} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, TIVX_TARGET_ID_DSP1},                                   \
    {TIVX_TARGET_DSP2, TIVX_TARGET_ID_DSP2},                                   \
    {TIVX_TARGET_EVE1, TIVX_TARGET_ID_EVE1},                                   \
    {TIVX_TARGET_EVE2, TIVX_TARGET_ID_EVE2},                                   \
    {TIVX_TARGET_EVE3, TIVX_TARGET_ID_EVE3},                                   \
    {TIVX_TARGET_EVE4, TIVX_TARGET_ID_EVE4},                                   \
    {TIVX_TARGET_IPU1_0, TIVX_TARGET_ID_IPU1_0},                               \
    {TIVX_TARGET_IPU1_1, TIVX_TARGET_ID_IPU1_1},                               \
    {TIVX_TARGET_IPU2, TIVX_TARGET_ID_IPU2_0},                                 \
    {TIVX_TARGET_A15_0, TIVX_TARGET_ID_A15_0},                                 \
    /* TIVX_TARGET_HOST will be filled later during tivxHostInit()             \
     * by calling function tivxPlatformSetHostTargetId                         \
     */                                                                        \
    {TIVX_TARGET_HOST, TIVX_TARGET_ID_INVALID}                                 \
}

/*! \brief Set target ID for HOST.
 *
 *         Called during tivxHostInit()
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformSetHostTargetId(tivx_target_id_e host_targe_id);

#ifdef __cplusplus
}
#endif

#endif
