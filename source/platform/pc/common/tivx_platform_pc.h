/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_PLATFORM_WINDOWS_H_
#define _TIVX_PLATFORM_WINDOWS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Windows Platform APIs
 */


/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_target.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS            (12u)


/*! \brief Maximum number obj descriptors that are present in shared memory
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST  (1024U)


/*! \brief Target ID for supported targets
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for CPU1 */
    TIVX_TARGET_ID_CPU1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0),

    /*! \brief target ID for CPU2 */
    TIVX_TARGET_ID_CPU2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 1),

    /*! \brief target ID for CPU2 */
    TIVX_TARGET_ID_CPU3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 2),

    /*! \brief target ID for CPU2 */
    TIVX_TARGET_ID_CPU4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 3),

} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_DSP2, TIVX_TARGET_ID_CPU2},                                   \
    {TIVX_TARGET_EVE1, TIVX_TARGET_ID_CPU1},                                   \
    {TIVX_TARGET_EVE2, TIVX_TARGET_ID_CPU2},                                   \
    {TIVX_TARGET_EVE3, TIVX_TARGET_ID_CPU3},                                   \
    {TIVX_TARGET_EVE4, TIVX_TARGET_ID_CPU4},                                   \
    {TIVX_TARGET_IPU1_0, TIVX_TARGET_ID_CPU1},                                 \
    {TIVX_TARGET_IPU1_1, TIVX_TARGET_ID_CPU2},                                 \
    {TIVX_TARGET_IPU2, TIVX_TARGET_ID_CPU3},                                   \
    {TIVX_TARGET_A15_0, TIVX_TARGET_ID_CPU4},                                  \
    {TIVX_TARGET_HOST, TIVX_TARGET_ID_CPU1}                                    \
}

#ifdef __cplusplus
}
#endif

#endif
