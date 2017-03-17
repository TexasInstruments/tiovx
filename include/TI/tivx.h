/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef VX_EXT_TI_H_
#define VX_EXT_TI_H_

#include <VX/vx.h>
#include <TI/tivx_kernels.h>
#include <TI/tivx_nodes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI extension APIs
 */


/*! \brief Name for TI OpenVX kernel module
 * \ingroup group_tivx_ext
 */
#define TIVX_MODULE_NAME0    "openvx-core"
/*! \brief Name for TI OpenVX kernel module
 * \ingroup group_tivx_ext
 */
#define TIVX_MODULE_NAME1    "ivsion"

/*! \brief Name for DSP target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP1        "DSP-1"

/*! \brief Name for DSP target class, instance 2
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_DSP2        "DSP-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE1        "EVE-1"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE2        "EVE-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE3        "EVE-3"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_EVE4        "EVE-4"

/*! \brief Name for A15 target class, core 0
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_A15_0       "A15-0"

/*! \brief Name for IPU1 target class, core 0
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU1_0      "IPU1-0"

/*! \brief Name for IPU1 target class, core 1
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU1_1      "IPU1-1"

/*! \brief Name for IPU2 target class
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_IPU2        "IPU2"

/*!
 * \brief Max possible name of a target
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_MAX_NAME            (16u)

/*! \brief String to name a OpenVX Host
 *
 *         Host is not a unique target on its own.
 *         At system config "HOST" will map to one
 *         of available targets
 *
 * \ingroup group_tivx_ext
 */
#define TIVX_TARGET_HOST        "HOST"


/*! \brief CPU ID for supported CPUs
 *
 *         CPU ID is defined in platform module since
 *         depending on platform the CPUs could be different
 *
 *         Current CPU IDs are defined assuming TDA2x/3x/2Ex
 *         family of SoCs
 *
 *         Caution: This enum is used as index into the array
 *         #g_ipc_cpu_id_map, so change in this enum will require
 *         change in this array as well.
 *
 *
 * \ingroup group_tivx_ext
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

    /*! \brief Max value of CPU ID  */
    TIVX_CPU_ID_MAX = 11,

    /*! \brief Invalid CPU ID */
    TIVX_INVALID_CPU_ID = 0xFF

} tivx_cpu_id_e;


/*!
 * \brief Function to initialize host
 *
 * \ingroup group_tivx_ext
 */
void tivxInit(void);

/*!
 * \brief Function to de-initialize host
 *
 * \ingroup group_tivx_ext
 */
void tivxDeInit(void);

/*!
 * \brief Associate a target with a kernel
 *
 *        Call multiple times for each supported target
 *
 *        If given target is not valid on current platform then
 *        error VX_ERROR_NOT_SUPPORTED is returned.
 *
 *        Typically VX_ERROR_NOT_SUPPORTED error should be ignored for this API,
 *        since this code is typically kept same across platforms
 *
 *        During graph verify however if user asks to run the kernel
 *        on a target not supported by this platform it results in a
 *        error and graph cannot execute.
 *
 *
 * \ingroup group_tivx_ext
 */
VX_API_ENTRY vx_status VX_API_CALL tivxAddKernelTarget(vx_kernel kernel, char *target_name);


/*!
 * \brief Register publish and unpublish functions against a module name.
 *
 *        These functions are invoked when vxLoadKernels is called with
 *        the registered name.
 *
 *        This is alternative instead of dynamically loading kernels during vxLoadKernels
 *
 *        Duplicate module names not checked by this API.
 *
 *        API is not reentrant, user is recommended to call all tivxRegisterModule
 *        during system init before vxCreateContext() from a single thread.
 *
 *        Modules registered against TIVX_MODULE_NAME are called during vxCreateContext
 *        so user MUST ensure tivxRegisterModule() is called for TIVX_MODULE_NAME module
 *
 * \ingroup group_tivx_ext
 */
VX_API_ENTRY vx_status VX_API_CALL tivxRegisterModule(const char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish);

/*!
 * \brief UnRegister publish and unpublish functions if previously registered
 *
 * \ingroup group_tivx_ext
 */
VX_API_ENTRY vx_status VX_API_CALL tivxUnRegisterModule(char *name);

/*!
 * \brief Return CPU ID of the CPU on which this API is called
 *
 * \ingroup group_tivx_ext
 */
vx_enum tivxGetSelfCpuId(void);


/*!
 * \brief Utility function to create a node given parameter references
 *
 * \ingroup group_tivx_ext
 */
vx_node tivxCreateNodeByStructure(vx_graph graph,
                                vx_enum kernelenum,
                                vx_reference params[],
                                vx_uint32 num);

/*!
 * \brief Utility function to check if the given reference is virtual or not
 *
 * \ingroup group_tivx_ext
 */
vx_bool tivxIsReferenceVirtual(vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif
