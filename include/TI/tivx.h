/*
*
* Copyright (c) 2017 - 2018 Texas Instruments Incorporated
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




#ifndef VX_EXT_TI_H_
#define VX_EXT_TI_H_

#include <VX/vx.h>
#include <TI/tivx_debug.h>
#include <TI/tivx_kernels.h>
#include <TI/tivx_nodes.h>
#include <TI/tivx_mem.h>
#include <TI/tivx_tensor.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI extension APIs
 */

/*! \brief Name for DSP target class, instance 1
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_DSP1        "DSP-1"

/*! \brief Name for DSP target class, instance 2
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_DSP2        "DSP-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_EVE1        "EVE-1"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_EVE2        "EVE-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_EVE3        "EVE-3"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_EVE4        "EVE-4"

/*! \brief Name for A15 target class, core 0
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_A15_0       "A15-0"

/*! \brief Name for IPU1 target class, core 0
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_IPU1_0      "IPU1-0"

/*! \brief Name for IPU1 target class, core 1
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_IPU1_1      "IPU1-1"

/*! \brief Name for IPU2 target class
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_IPU2        "IPU2"

#define TIVX_TARGET_RESV00      "RESV00"
#define TIVX_TARGET_RESV01      "RESV01"
#define TIVX_TARGET_RESV02      "RESV02"
#define TIVX_TARGET_RESV03      "RESV03"
#define TIVX_TARGET_RESV04      "RESV04"
#define TIVX_TARGET_RESV05      "RESV05"
#define TIVX_TARGET_RESV06      "RESV06"
#define TIVX_TARGET_RESV07      "RESV07"
#define TIVX_TARGET_RESV08      "RESV08"
#define TIVX_TARGET_RESV09      "RESV09"

/*!
 * \brief Max possible name of a target
 *
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_MAX_NAME            (16u)

/*! \brief String to name a OpenVX Host
 *
 *         Host is not a unique target on its own.
 *         At system config "HOST" will map to one
 *         of available targets
 *
 * \ingroup group_tivx_ext_host
 */
#define TIVX_TARGET_HOST        "HOST"

/*!
 * \brief Max size of macro
 *
 * \ingroup group_tivx_ext_host
 */
#define TIVX_RESOURCE_NAME_MAX (39u)

/*!
 * \brief Max size of config file path
 *
 * \ingroup group_tivx_ext_host
 */
#define TIVX_CONFIG_PATH_LENGTH   (512u)

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
 * \ingroup group_tivx_ext_host
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

/*! \brief Struct containing config parameters of given static value
 *
 * \ingroup group_tivx_ext_host
 */
typedef struct _tivx_resource_stats_t {

    /**< Maximum value of a macro */
    uint32_t  max_value;

    /**< Value currently being used by a statically allocated array */
    uint32_t  cur_used_value;

    /**< Highest value used at any point of the application */
    uint32_t  max_used_value;

    /**< Name of the macro */
    char name[TIVX_RESOURCE_NAME_MAX];

} tivx_resource_stats_t;

/*! \brief TI attribute extensions
 *
 *         TI attribute extensions to OpenVX
 *
 * \ingroup group_tivx_ext_host
 */
typedef enum _tivx_attribute_extensions_e {

    /*! \brief Returns the target string corresponding to the node */
    TIVX_NODE_TARGET_STRING = VX_ATTRIBUTE_BASE(VX_ID_TI, 0) + 0x0,

} tivx_attribute_extensions_e;


/*! \brief Based on the VX_DF_IMAGE definition.
 * \note Use <tt>\ref tivx_df_image</tt> to contain these values.
 * \ingroup group_tivx_ext_host
 */
typedef enum _tivx_df_image_e {

    /*! \brief A single plane of packed 12-bit data.
     * 
     * The stride_x for this data format must be set to 0 */
    TIVX_DF_IMAGE_P12 = VX_DF_IMAGE('P','0','1','2'),

} tivx_df_image_e;


/*!
 * \brief Function to initialize OpenVX framework
 *
 *        Should be called during system init on all OpenVX HOST
 *        as well as OpenVX target CPUs
 *
 *        NOTE: In Vision SDK, this function is called during VIsion SDK
 *              system init so user need not call this explicitly
 *
 * \ingroup group_tivx_ext_host
 */
void tivxInit(void);

/*!
 * \brief Function to de-initialize OpenVX framework
 *
 *        Should be called during system init on all OpenVX HOST
 *        as well as OpenVX target CPUs
 *
 *        NOTE: In Vision SDK, this function is called during VIsion SDK
 *              system init so user need not call this explicitly
 *
 * \ingroup group_tivx_ext_host
 */
void tivxDeInit(void);

/*!
 * \brief Function to initialize OpenVX HOST side functionality
 *
 *        Should be called during system init after tivxInit()
 *        on HOST CPUs only
 *
 *        NOTE: In Vision SDK, this function is called during VIsion SDK
 *              system init so user need not call this explicitly
 *
 * \ingroup group_tivx_ext_host
 */
void tivxHostInit(void);

/*!
 * \brief Function to de-initialize OpenVX HOST side functionality
 *
 *        Should be called during system init before tivxDeInit()
 *        on HOST CPUs only
 *
 *        NOTE: In Vision SDK, this function is called during VIsion SDK
 *              system init so user need not call this explicitly
 *
 * \ingroup group_tivx_ext_host
 */
void tivxHostDeInit(void);

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
 * \ingroup group_tivx_ext_host
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
 * \ingroup group_tivx_ext_host
 */
VX_API_ENTRY vx_status VX_API_CALL tivxRegisterModule(const char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish);

/*!
 * \brief UnRegister publish and unpublish functions if previously registered
 *
 * \ingroup group_tivx_ext_host
 */
VX_API_ENTRY vx_status VX_API_CALL tivxUnRegisterModule(char *name);

/*!
 * \brief Return CPU ID of the CPU on which this API is called
 *
 * \ingroup group_tivx_ext_host
 */
vx_enum tivxGetSelfCpuId(void);

/*!
 * \brief Query resource for resource stats
 *
 * \ingroup group_tivx_ext_host
 */
vx_status tivxQueryResourceStats(const char *resource_name, tivx_resource_stats_t *stat);

/*!
 * \brief Prints out resource stats
 *
 * \ingroup group_tivx_ext_host
 */
void tivxPrintAllResourceStats();

/*!
 * \brief Exports the max used values to a file
 *
 * \ingroup group_tivx_ext_host
 */
vx_status tivxExportAllResourceMaxUsedValueToFile();


/*! \brief Macro to find size of array
 * \ingroup group_tivx_ext_host
 */
#ifndef dimof
#define dimof(x) (sizeof(x)/sizeof(x[0]))
#endif

/*!
 * \brief Utility function to create a node given parameter references and kernel enum
 *
 * \ingroup group_tivx_ext_host
 */
vx_node tivxCreateNodeByKernelEnum(vx_graph graph,
                                vx_enum kernelenum,
                                vx_reference params[],
                                vx_uint32 num);

/*!
 * \brief Utility function to create a node given parameter references and kernel reference
 *
 * \ingroup group_tivx_ext_host
 */
vx_node tivxCreateNodeByKernelRef(vx_graph graph,
                                vx_kernel kernel,
                                vx_reference params[],
                                vx_uint32 num);

/*!
 * \brief Utility function to create a node given parameter references and kernel name
 *
 * \ingroup group_tivx_ext_host
 */
vx_node tivxCreateNodeByKernelName(vx_graph graph,
                                char *kernel_name,
                                vx_reference params[],
                                vx_uint32 num);

/*!
 * \brief Utility function to check if the given reference is virtual or not
 *
 * \ingroup group_tivx_ext_host
 */
vx_bool tivxIsReferenceVirtual(vx_reference ref);

/*!
 * \brief Utility function to know if target is enabled or not
 *
 * \ingroup group_tivx_ext_host
 */
vx_bool tivxIsTargetEnabled(char target_name[]);

/*!
 * \brief Get the time in micro seconds
 *
 * \ingroup group_tivx_ext_host
 */
uint64_t tivxPlatformGetTimeInUsecs(void);

/*!
 * \brief Export graph representation as DOT graph file
 *
 * Multiple representation of the graph are exported to different files
 * using 'output_file_prefix' as filename prefix.
 * The output files are stored at path 'output_file_path'
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxExportGraphToDot(vx_graph graph, char *output_file_path, char *output_file_prefix);


/*!
 * \brief Enable run-time logging of graph trace to 'stdout'
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxLogRtTrace(vx_graph graph);

/*!
 * \brief Set number of buffers to allocate at output of a node parameter
 *
 * - Graph to which the node belongs MUST be scheduled in VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO or
 *   VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL mode.
 * - The node parameter MUST be of type virtual or all references to the parameter reference
 * MUST be released before graph verify.
 * - The node parameter specified MUST be a output parameter.
 *
 * This API acts as a hint and framework may overide user specified settings
 * in case any of above conditions are not met.
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxSetNodeParameterNumBufByIndex(vx_node node, vx_uint32 index, vx_uint32 num_buf);

/*! \brief Indicates to the implementation the depth of the graph pipeline
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxSetGraphPipelineDepth(vx_graph graph, vx_uint32 pipeline_depth);


#ifdef __cplusplus
}
#endif

#endif
