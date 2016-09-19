/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_PLATFORM_VISION_SDK_H_
#define _TIVX_PLATFORM_VISION_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Vision SDK Platform APIs
 */


/*! \brief Maximum number of targets and thus targetid supported
 */
#define TIVX_MAX_TARGETS            (12u)


/*! \brief CPU ID for supported CPUs
 *
 *         CPU ID is defined in platform module since
 *         depending on platform the CPUs could be different
 *
 *         Current CPU IDs are defined assuming TDA2x/3x/2Ex
 *         family of SoCs
 *
 *         Caution: This enum is used as index into the array
 *         #gPlatformCpuIdMap, so change in this enum will require
 *         change in this array as well.
 *
 *
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_cpu_id_e {

    /*! \brief CPU ID for DSP1 */
    TIVX_CPU_ID_DSP1 = 0,

    /*! \brief CPU ID for DSP2 */
    TIVX_CPU_ID_DSP2 = 1,

    /*! \brief CPU ID for EVE1 */
    TIVX_CPU_ID_EVE1 = 2,

    /*! \brief CPU ID for EVE2 */
    TIVX_CPU_ID_EVE2 = 3,

    /*! \brief CPU ID for EVE3 */
    TIVX_CPU_ID_EVE3 = 4,

    /*! \brief CPU ID for EVE4 */
    TIVX_CPU_ID_EVE4 = 5,

    /*! \brief CPU ID for IPU1-0 */
    TIVX_CPU_ID_IPU1_0 = 6,

    /*! \brief CPU ID for IPU1-1 */
    TIVX_CPU_ID_IPU1_1 = 7,

    /*! \brief CPU ID for IPU2 */
    TIVX_CPU_ID_IPU2_0 = 8,

    /*! \brief CPU ID for A15-0 */
    TIVX_CPU_ID_A15_0 = 9,

    /*! \brief CPU ID for A15-0 */
    TIVX_CPU_ID_A15_1 = 10,

    /*! \brief Invalid CPU ID */
    TIVX_INVALID_CPU_ID = 0xFF

} tivx_cpu_id_e;

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_target
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0),

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP2, 0),

    /*! \brief target ID for EVE1 */
    TIVX_TARGET_ID_EVE1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE1, 0),

    /*! \brief target ID for EVE2 */
    TIVX_TARGET_ID_EVE2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE2, 0),

    /*! \brief target ID for EVE3 */
    TIVX_TARGET_ID_EVE3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE3, 0),

    /*! \brief target ID for EVE4 */
    TIVX_TARGET_ID_EVE4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_EVE4, 0),

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 0),

    /*! \brief target ID for IPU1-1 */
    TIVX_TARGET_ID_IPU1_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_1, 0),

    /*! \brief target ID for IPU2 */
    TIVX_TARGET_ID_IPU2_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU2_0, 0),

    /*! \brief target ID for A15-0 */
    TIVX_TARGET_ID_A15_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A15_0, 0),

} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_target
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
    {TIVX_TARGET_HOST, TIVX_TARGET_ID_IPU1_0}                                  \
}

#ifdef __cplusplus
}
#endif

#endif
