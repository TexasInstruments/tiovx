/*
*
* Copyright (c) 2017 - 2019 Texas Instruments Incorporated
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
#include <VX/vx_khr_pipelining.h>
#include <VX/vx_khr_user_data_object.h>
#include <TI/tivx_debug.h>
#include <TI/tivx_kernels.h>
#include <TI/tivx_nodes.h>
#include <TI/tivx_mem.h>
#include <TI/tivx_tensor.h>
#include <TI/tivx_ext_raw_image.h>
#include <TI/tivx_ext_super_node.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to TI extension APIs
 */

/*! \brief Name for DSP target class, instance 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP1        "DSP-1"

/*! \brief Name for DSP target class, instance 2
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_DSP2        "DSP-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_EVE1        "EVE-1"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_EVE2        "EVE-2"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_EVE3        "EVE-3"

/*! \brief Name for EVE target class, instance 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_EVE4        "EVE-4"

/*! \brief Name for A15 target class, core 0
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_A15_0       "A15-0"

/*! \brief Name for IPU1 target class, core 0
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_IPU1_0      "IPU1-0"

/*! \brief Name for IPU1 target class, core 1
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_IPU1_1      "IPU1-1"

/*! \brief Name for IPU2 target class
 * \ingroup group_tivx_ext_targets
 */
#define TIVX_TARGET_IPU2        "IPU2"


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



/*!
 * \brief Flag used with tivxGraphParameterEnqueueReadyRef to
 *        indicate that this enqueue call if for pipeup phase
 *
 * Typically used only with source nodes which have a pipeup
 * requirement
 *
 * \ingroup group_tivx_ext_host
 */
#define TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP (0x00000001u)

/*!
 * \brief When sending a control command to a replicated node,
 *        this can be used to send control command to all replicated node.
 *
 * Currently used as parameter in tivxNodeSendCommand API
 *
 * \ingroup group_tivx_ext_host
 */
#define TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES ((uint32_t)-1)


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
    TIVX_CPU_ID_INVALID = 0xFF

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
    TIVX_NODE_TARGET_STRING = VX_ATTRIBUTE_BASE(VX_ID_TI, (vx_enum)0) + 0x0,

    /*! \brief Sets the valid data size within the user data object */
    TIVX_USER_DATA_OBJECT_VALID_SIZE = VX_ATTRIBUTE_BASE(VX_ID_TI, (vx_enum)0) + 0x1,

    /*! \brief Returns the timestamp in micro-seconds corresponding to the given reference */
    TIVX_REFERENCE_TIMESTAMP = VX_ATTRIBUTE_BASE(VX_ID_TI, (vx_enum)0) + 0x2,

    /*! \brief Returns a boolean indicating whether or not a reference is invalid */
    TIVX_REFERENCE_INVALID = VX_ATTRIBUTE_BASE(VX_ID_TI, (vx_enum)0) + 0x3
} tivx_attribute_extensions_e;


/*! \brief Based on the VX_DF_IMAGE definition.
 * \note Use <tt>\ref vx_df_image</tt> to contain these values.
 * \ingroup group_tivx_ext_host
 */
typedef enum _tivx_df_image_e {

    /*! \brief A single plane of packed 12-bit data.
     *
     * The stride_x for this data format must be set to 0 */
    TIVX_DF_IMAGE_P12 = VX_DF_IMAGE('P','0','1','2'),

    /*! \brief a NV12 frame of packed 12-bit data.
     *
     * The stride_x for this data format must be set to 0 */
    TIVX_DF_IMAGE_NV12_P12 = VX_DF_IMAGE('N','1','2','P'),

    /*! \brief A single plane of packed 16-bit RGB565 data.
     *
     * The stride_x for this data format must be set to 2 */
    TIVX_DF_IMAGE_RGB565 = VX_DF_IMAGE('R','5','6','5'),

} tivx_df_image_e;

/*! \brief Based on the vx_graph_attribute_e definition.
 * \ingroup group_tivx_ext_host
 */
typedef enum _tivx_graph_attribute_e {

    /*! \brief Returns the graph stream executions. Read-only. Use a <tt>\ref vx_uint32</tt> parameter. */
    TIVX_GRAPH_STREAM_EXECUTIONS = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_GRAPH) + 0x5,

} tivx_graph_attribute_e;

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
VX_API_ENTRY vx_status VX_API_CALL tivxAddKernelTarget(vx_kernel kernel, const char *target_name);

/*!
 * \brief Set the number of sink buffers
 *
 *
 * \ingroup group_tivx_ext_host
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetKernelSinkDepth(vx_kernel kernel, uint32_t num_sink_bufs);

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
VX_API_ENTRY vx_status VX_API_CALL tivxUnRegisterModule(const char *name);

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
void tivxPrintAllResourceStats(void);

/*!
 * \brief Exports the max used values to a file
 *
 *        Note 1: For PC emulation mode, the exported files will not have a legend with the correct target
 *        Note 2: For target mode, only the .txt files will be generated.  These can be converted to JPEG's using the following command:
 *
 *        dot -Tjpg -o<output file path>/<jpg name>.jpg <text file name>
 *
 *        For example, to generate a JPEG named "graph.jpg" for the file "graph.txt" in the current directory, the following command would be used.
 *
 *        dot -Tjpg -o./graph.jpg graph.txt
 *
 * \ingroup group_tivx_ext_host
 */
vx_status tivxExportAllResourceMaxUsedValueToFile(void);


/*! \brief Macro to find size of array
 * \ingroup group_tivx_ext_host
 */
#ifndef dimof
#define dimof(x) (sizeof(x)/sizeof(x[0]))
#endif

/*!
 * \brief Utility function to create a node given parameter references and kernel enum
 *
 * \param [in] graph Graph reference
 * \param [in] kernelenum Enumeration specifying a specific kernel
 * \param [in] params List of parameter references corresponding to the enumerated kernel
 * \param [in] num Number of parameter references in params list
 *
 * \returns A node reference <tt>\ref vx_node</tt>. Any possible errors preventing a
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
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
 * \param [in] graph Graph reference
 * \param [in] kernel Kernel reference
 * \param [in] params List of parameter references corresponding to the kernel reference
 * \param [in] num Number of parameter references in params list
 *
 * \returns A node reference <tt>\ref vx_node</tt>. Any possible errors preventing a
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
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
 * \param [in] graph Graph reference
 * \param [in] kernel_name String corresponding to the kernel name
 * \param [in] params List of parameter references corresponding to the kernel name
 * \param [in] num Number of parameter references in params list
 *
 * \returns A node reference <tt>\ref vx_node</tt>. Any possible errors preventing a
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
 *
 * \ingroup group_tivx_ext_host
 */
vx_node tivxCreateNodeByKernelName(vx_graph graph,
                                const char *kernel_name,
                                vx_reference params[],
                                vx_uint32 num);

/*!
 * \brief Utility function to check if the given reference is virtual or not
 *
 * \param [in] ref Reference to query
 *
 * \return A <tt>\ref vx_bool</tt> value.
 * \retval vx_true_e The reference is virtual.
 * \retval vx_false_e The reference is not virtual.
 *
 * \ingroup group_tivx_ext_host
 */
vx_bool tivxIsReferenceVirtual(vx_reference ref);

/*!
 * \brief Utility function to extract the parent reference from an element
 *
 * \param [in] child_ref     Child reference to query
 * \return vx_reference in case of success, else NULL
 *
 * \ingroup group_tivx_ext_host
 */
vx_reference tivxGetReferenceParent(vx_reference child_ref);

/*!
 * \brief Utility function to know if target is enabled or not
 *
 * \param [in] target_name String specifying the target name
 *
 * \return A <tt>\ref vx_bool</tt> value.
 * \retval vx_true_e The target is enabled
 * \retval vx_false_e The target is not enabled
 *
 * \ingroup group_tivx_ext_host
 */
vx_bool tivxIsTargetEnabled(char target_name[]);

/*!
 * \brief Get the time in micro seconds
 *
 * \return Time in micro seconds.
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
 * Note: This must be called after vxVerifyGraph()
 *
 * \param [in] graph Graph reference
 * \param [in] output_file_path String specifying the ouput file path
 * \param [in] output_file_prefix String specifying the filename prefix of the output file
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxExportGraphToDot(vx_graph graph, char *output_file_path, char *output_file_prefix);


/*!
 * \brief Enable run-time logging of graph trace to 'stdout'
 *
 * \param [in] graph Graph reference
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxLogRtTrace(vx_graph graph);

/*!
 * \brief Set number of buffers to allocate at output of a node parameter
 *
 * - Graph to which the node belongs MUST be scheduled in VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO or
 *   VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL mode.
 * - The node parameter specified MUST be a output parameter.
 * - The node parameter specified MUST NOT be an enqueueable graph parameter.
 *
 * This API acts as a hint and framework may overide user specified settings
 * in case any of above conditions are not met.
 *
 * \param [in] node Node reference
 * \param [in] index Node parameter index
 * \param [in] num_buf Number of buffers to allocate
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxSetNodeParameterNumBufByIndex(vx_node node, vx_uint32 index, vx_uint32 num_buf);

/*! \brief Indicates to the implementation the depth of the graph pipeline
 *
 * \param [in] graph Graph reference
 * \param [in] pipeline_depth Pipeline depth; Max value is (TIVX_GRAPH_MAX_PIPELINE_DEPTH-1)
 *                            else it will return an error
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxSetGraphPipelineDepth(vx_graph graph, vx_uint32 pipeline_depth);

/*! \brief Same as vxGraphParameterEnqueueReadyRef except that it take an
 *         additional TIOVX specific flag parameter
 *
 *  \details For valid values of flag see
 *  - \ref TIVX_GRAPH_PARAMETER_ENQUEUE_FLAG_PIPEUP
 *           The tivxGraphParameterEnqueueReadyRef API is needed if explicitly
 *           enqueueing and dequeueing from a capture node graph parameter
 *           that has set the VX_KERNEL_PIPEUP_OUTPUT_DEPTH using the
 *           vxSetKernelAttribute API.  The tivxGraphParameterEnqueueReadyRef
 *           must be called for the num_bufs given as the
 *           VX_KERNEL_PIPEUP_OUTPUT_DEPTH.  This API differs from
 *           vxGraphParameterEnqueueReadyRef in that it does not additionally
 *           schedule a graph execution.  In the case that this API is not used
 *           at the capture node graph parameter, the teardown of the graph will
 *           pend as it will be waiting on graph executions that have not completed.
 *
 *  For more detailed description of vxGraphParameterEnqueueReadyRef see
 *  \ref vxGraphParameterEnqueueReadyRef
 *
 * \param [in] graph Graph reference
 * \param [in] graph_parameter_index Graph parameter index
 * \param [in] refs The array of references to enqueue into the graph parameter
 * \param [in] num_refs Number of references to enqueue
 * \param [in] flags Flag to control behavior of the operation
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE graph is not a valid reference OR reference is not a valid reference
 * \retval VX_ERROR_INVALID_PARAMETERS graph_parameter_index is NOT a valid graph parameter index
 * \retval VX_FAILURE Reference could not be enqueued.
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxGraphParameterEnqueueReadyRef(vx_graph graph,
                vx_uint32 graph_parameter_index,
                vx_reference *refs,
                vx_uint32 num_refs,
                vx_uint32 flags);

/*!
 * \brief Send node specific Control command
 * \details This API is used to send specific control command to the node.
 *          Refer to Node documentation for specific control command.
 *          Note that this API is blocking and does not return untill
 *          command is executed by the node.
 *          This API is thread safe, ie multi commands can be sent to
 *          same or different nodes from different threads
 *
 * \param [in] node Reference of the node to which this command is to be sent.
 * \param [in] replicate_nodex_idx: In case of a non-replicated node this
 *             should be 0, For a replicated node this is the index
 *             of the replicated node to which the command is targeted.
 *             To send same command to all replicated nodes use
 *             TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES
 * \param [in] node_cmd_id Node specific control command id, refer to node
 *             specific interface file
 * \param [in,out] ref[] Node parameter,
 *             This is an array of references, required as parameters for
 *             this control command.
 *             They can be any OpenVX object, created using create API.
 *             It is bidirectional parameters, can be used for INPUT,
 *             OUTPUT or both.
 *             These parameters are list of references,
 *             which are required for the control command on the given node.
 *             Refer to node documentation to get details about the parameters
 *             required for given control command.
 *             Caller of this API should explicitely release these refs after
 *             their usage is completed.
 * \param [in] num_refs Number of valid entries/references in ref[] array
 *
 *
 * \ingroup group_tivx_ext_host
 */
vx_status VX_API_CALL tivxNodeSendCommand(vx_node node,
    uint32_t replicate_nodex_idx, uint32_t node_cmd_id,
    vx_reference ref[], uint32_t num_refs);


/*!
 * \brief This API is used to get a reference to a node within a graph at a given index within the graph
 *
 *        Use vxQueryGraph with attribute VX_GRAPH_NUMNODES to get number of nodes within a graph.
 *        This API can be called only on a verified graph.
 *        Node returned by this API is not reference counted and should not be released by the user.
 *
 * \param [in] graph graph handle
 * \param [in] index  node index, value from 0 .. value returned by vxQueryGraph(VX_GRAPH_NUMNODES) - 1
 *
 * \return vx_node in case of success, else NULL
 *
 * \ingroup group_tivx_ext_host
 *
 */
vx_node tivxGraphGetNode(vx_graph graph, uint32_t index);

/*!
 * \brief Sets attributes on the user data object
 *
 * \param [in] user_data_object  The reference to the user data object.
 * \param [in] attribute         The attribute to modify. Use a <tt>\ref vx_user_data_object_attribute_e</tt>.
 * \param [in] ptr               The pointer to the value to which to set the attribute.
 * \param [in] size              The size in bytes of the container to which \a ptr points.
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS                   No errors.
 * \retval VX_ERROR_INVALID_REFERENCE   If the \a user_data_object is not a <tt>\ref vx_user_data_object</tt>.
 * \retval VX_ERROR_NOT_SUPPORTED       If the \a attribute is not a value supported on this implementation.
 * \retval VX_ERROR_INVALID_PARAMETERS  If any of the other parameters are incorrect.
 *
 * \ingroup group_tivx_ext_host
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetUserDataObjectAttribute(vx_user_data_object user_data_object, vx_enum attribute, const void *ptr, vx_size size);

/*! \brief Sets the tile size for a given node in a graph. This is only valid for
 * BAM-enabled kernels on C66 DSP.
 * \param [in] node  The reference to the <tt>\ref vx_node</tt> object.
 * \param [in] block_width The tile width in pixels.
 * \param [in] block_height The tile height in lines.
 * \ingroup group_tivx_ext_host
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS if the tile size is set correctly.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetNodeTileSize(vx_node node, vx_uint32 block_width, vx_uint32 block_height);


/*! \brief Sets reference attributes for the below enums:
 *         TIVX_REFERENCE_TIMESTAMP
 * \param [in] ref       The reference object.
 * \param [in] attribute The attribute of the reference to be set.
 * \param [in] ptr       The value of the reference attribute to be set.
 * \param [in] size      The size of the value of the reference attribute to be set.
 * \ingroup group_tivx_ext_host
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS if the attribute was set correctly.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetReferenceAttribute(vx_reference ref, vx_enum attribute, const void *ptr, vx_size size);

#ifdef __cplusplus
}
#endif

#endif
