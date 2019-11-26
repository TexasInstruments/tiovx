/*
*
* Copyright (c) 2017-2019 Texas Instruments Incorporated
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


/**
 *  @file       tivx_bam_kernel_wrapper.h
 *
 *  @brief      This file provides a BAM wrapper interface around each VXLIB kernel
 *               to be included in TIOVX. Using this BAM wrapper will accelerate
 *               the memory movement portion of the kernel by leveraging a hardware
 *               DMA (Direct Memory Access) unit to bring blocks of the image from
 *               external DRAM into fast L2 SRAM.  BAM manages the sequencing of
 *               the DMA such that the DSP can process one block in parallel to the DMA
 *               writing the previous block to DDR and reading the next block into
 *               L2 SRAM.  For most kernels, this is typically faster than relying
 *               on the DSP cache alone.
 */

#ifndef BAM_KERNEL_WRAPPER_H
#define BAM_KERNEL_WRAPPER_H

#include <TI/tivx.h>
#include <TI/tivx_target_kernel.h>
#include <VX/vx.h>
#include "tivx_bam_kernel_database.h"
#include "bam_common.h"
#include "tivx_target_kernel_instance.h"


/*! \brief Max number of nodes in a BAM graph
 * \ingroup group_tivx_ext_bam
 */
#define TIVX_BAM_MAX_NODES  (16u)

/*! \brief Max number of edges in a BAM graph
 * \ingroup group_tivx_ext_bam
 */
#define TIVX_BAM_MAX_EDGES  (32u)

/*! \brief Max number of buf params in a BAM graph
 * \ingroup group_tivx_ext_bam
 */
#define TIVX_MAX_BUF_PARAMS  (32u)

/*!
 * \brief indicates if a plane is not connected
 * \ingroup group_tivx_ext_bam
 */
#define TIVX_IMAGE_NULL_PLANE        (255)

/*! \brief Graph handle
 *
 *         Handle associated with an instance of a BAM graph.
 *
 *         The structure this handle represents is internally defined
 *         and not needed by the user.
 *
 *         The user is given this handle when calling \ref tivxBamCreateHandleSingleNode,
 *         or \ref tivxBamCreateHandleMultiNode, and can destroy it by
 *         calling \ref tivxBamDestroyHandle.
 *
 * \ingroup group_tivx_ext_bam
 */
typedef void *tivx_bam_graph_handle;


/*! \brief Kernel Details  structure
 *
 *         Kernel details for each kernel in the graph.
 *
 *         This includes kernel info that the user can obtain from the
 *         kernel by calling the kernel sepecific getKernelInfo function.
 *
 *         This also includes a pointer to the kernel specific parameter
 *         list that the user should fill out.  If the kernel doesn't
 *         have a parameter list, then compute_kernel_params should be
 *         set to NULL.
 *
 *         This structure should be populated as an input to the
 *         \ref tivxBamCreateHandleSingleNode function, or an array of
 *         this structure should be filled as an input to the
 *         \ref tivxBamCreateHandleMultiNode function.
 *
 * \ingroup group_tivx_ext_bam
 */
typedef struct _tivx_bam_kernel_details
{
    BAM_KernelInfo    kernel_info;
    void *compute_kernel_params;
    uint32_t block_width;
    uint32_t block_height;
}tivx_bam_kernel_details_t;


/*! \brief BAM plugin definition
 *
 *         Used for registering a BAM plugin with OpenVX on the DSPs.
 *
 * \ingroup group_tivx_ext_bam
 */
typedef struct _tivx_bam_plugin_def
{
    const BAM_KernelInfo *kernelInfo;                 /**< Pointer to the kernel's contextual information structure BAM_KernelInfo */
    const BAM_KernelHelperFuncDef *kernelHelperFunc;  /**< Pointer to the structure BAM_KernelHelperFuncDef that list the helper functions */
    const BAM_KernelExecFuncDef *kernelExecFunc;      /**< Pointer to the structure BAM_KernelExecFuncDef that list the execution functions  */
    const char *name;                                 /**< Unique name of the kernel plugin */

}tivx_bam_plugin_def;

/*!
 * \brief Register BAM plugin with OpenVX framework on DSP
 *
 *        This function is used to register a BAM plugin with the DSP
 *        target framework so that the kernel can be included in BAM
 *        graphs.
 *
 *        The plugin is expected to have a unique "name" string.  If a
 *        plugin with the same name has already been registered, then
 *        the kernelId of the previously registered plugin will be returned
 *        without registering the kernel again.
 *
 *        This function should be called before any calls to
 *        tivxBamCreateHandleSingleNode or tivxBamCreateHandleMultiNode, and
 *        is recommended to be called during the kernel registration function
 *        where tivxAddTargetKernel() is called, or as part of a kernel library
 *        registration function.
 *
 * \param [in] plugin    Pointer to a plugin definition
 * \param [out] kernelId The kernel id given to the passed plugin.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_PARAMETERS If one of the parameters are NULL.
 * \retval VX_ERROR_NO_MEMORY If there is not enough memory to register new plugins.
 *         The total number of user plugins can be incremented by changing the
 *         TIVX_MAX_DSP_BAM_USER_PLUGINS define in the TI/tivx_config.h file.
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamRegisterPlugin(tivx_bam_plugin_def *plugin, BAM_KernelId *kernelId);

/*!
 * \brief Returns the kernelID associated with a plugin name
 *
 *        This function is used to return the kernelID of a BAM plugin
 *        which has already been registered on the DSP target framework.
 *
 *        If the name is not found in the BAM kernel database, the
 *        BAM_TI_KERNELID_UNDEFINED will be returned as the kernelId.
 *
 * \param [in] name      Name of the BAM plugin
 * \param [out] kernelId The kernel id given to the passed plugin.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_PARAMETERS If one of the parameters are NULL.
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamGetKernelIdFromName(const char *name, BAM_KernelId *kernelId);

/*!
 * \brief Initialize memory block for BAM usage
 *
 *        This function is used by the application to assign blocks of
 *        internal memory for use by BAM.  This memory is typically
 *        on-chip RAM (L2SRAM, L1DRAM, or OCM RAM).
 *
 *        BAM uses the ibuf memory to temporarily store the input block
 *        buffers from the DMA source nodes, the intermediate block buffers
 *        within kernels of the graph, and the output block buffers to the
 *        DMA sink nodes.
 *
 *        BAM uses the wbuf memory to temporarily store internal scratch
 *        memory requested by the kernels of the graph.
 *
 *        All of this memory is considered scratch memory, meaning that the BAM
 *        does not use it for persistent memory, and it can be overwritten
 *        by the application when in between calls to tivxBamProcessGraph.
 *
 *        This function should be called before any calls to
 *        tivxBamCreateHandleSingleNode or tivxBamCreateHandleMultiNode.
 *
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamMemInit(void *ibuf_mem, uint32_t ibuf_size,
                          void *wbuf_mem, uint32_t wbuf_size);

/*!
 * \brief BAM Create Graph Handle for Single Node
 *
 *        This function will create a BAM graph of 1 node, given the
 *        kernel_id, array of pointers to buf_params, and computation
 *        kernel_details, containing kernel info and parameters.
 *
 *        Upon success, a non-NULL graph_handle will be returned and
 *        vx_status will be VX_SUCCESS.
 *
 *        If there is not enough memory to allocate the handle for
 *        the given kernel and block size, then the VX_ERROR_NO_MEMORY
 *        error will be returned and the graph_handle will be set to NULL.
 *
 *        Any other failure will return a VX_FAILURE and the graph_handle
 *        will be set to NULL.
 *
 *        The user should pass the graph_handle to other functions in this
 *        wrapper related to BAM graph wrappers.
 *
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamCreateHandleSingleNode(BAM_TI_KernelID kernel_id,
                                        VXLIB_bufParams2D_t *buf_params[],
                                        tivx_bam_kernel_details_t *kernel_details,
                                        tivx_bam_graph_handle *graph_handle);

/*!
 * \brief BAM Create Graph Handle for Multiple Nodes
 *
 *        This function will create a BAM graph of multiple nodes, given the
 *        node list, edge list, array of pointers to buf_params, and array
 *        of the computation kernel_details, containing kernel info and parameters.
 *        This function expects the array size of kernel_details to be equal
 *        to the full number of nodes in the node_list, including SOURCE and
 *        SINK nodes.
 *
 *        Upon success, a non-NULL graph_handle will be returned and
 *        vx_status will be VX_SUCCESS.
 *
 *        If there is not enough memory to allocate the handle for
 *        the given kernel and block size, then the VX_ERROR_NO_MEMORY
 *        error will be returned and the graph_handle will be set to NULL.
 *
 *        Any other failure will return a VX_FAILURE and the graph_handle
 *        will be set to NULL.
 *
 *        The user should pass the graph_handle to other functions in this
 *        wrapper related to BAM graph wrappers.
 *
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamCreateHandleMultiNode(BAM_NodeParams node_list[],
                                       uint32_t max_nodes,
                                       BAM_EdgeParams edge_list[],
                                       uint32_t max_edges,
                                       VXLIB_bufParams2D_t *buf_params[],
                                       tivx_bam_kernel_details_t kernel_details[],
                                       tivx_bam_graph_handle *g_handle);

/*!
 * \brief BAM Update Pointers
 *
 *        This function will update the external memory pointers for the
 *        associated graph_handle.  This is typically called before
 *        processing each frame in case the pointer values have changed.
 *
 *        If the pointer values have not changed since the previous time
 *        they were set, there is no reason to call this function.
 *
 *        Upon success, vx_status will be VX_SUCCESS.
 *        Upon failure, vx_status will be VX_FAILURE.
 *
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamUpdatePointers(tivx_bam_graph_handle graph_handle,
                                uint32_t num_inputs,
                                uint32_t num_outputs,
                                void  *ptrs[]);

/*!
 * \brief BAM Node Control
 *
 *        Some kernel have a control interface to set information or
 *        get information back from the kernel context.
 *
 *        This API allows the application to send control commands to a
 *        specific node.  The commands and control interface is defined
 *        in the bam_plugin for each kernel.
 *
 *        In the case of single node handle, node_id is ignored.
 *
 *        Upon success, vx_status will be VX_SUCCESS.
 *        Upon failure, vx_status will be VX_FAILURE.
 *
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamControlNode(tivx_bam_graph_handle graph_handle,
                             uint32_t node_id,
                             uint32_t cmd,
                             void  *payload);


/*!
 * \brief BAM Process Graph
 *
 *        This function will execute the kernel within the BAM graph.
 *
 *        This is called once per frame.  The BAM wrapper internally
 *        will manage the sequencing of DMA fetches and DSP processing
 *        for each block during this call.
 *
 *        Upon success, vx_status will be VX_SUCCESS.
 *        Upon failure, vx_status will be VX_FAILURE.
 *
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamProcessGraph(tivx_bam_graph_handle graph_handle);

/*!
 * \brief BAM Destroy Handle
 *
 *        This function will destroy the BAM graph handle, effectivly
 *        releasing the memory to the system.
 *
 *        This is typically called if the user releases a graph or makes
 *        changes to the graph that require a reverification of the graph.
 *
 * \ingroup group_tivx_ext_bam
 */
void tivxBamDestroyHandle(tivx_bam_graph_handle graph_handle);

/*!
 * \brief BAM Initialize kernel details structure array
 *
 *        This function should be called before setting any parameters
 *        within the kernel_details structure since it sets all parameters
 *        to default values.
 *
 *        This is typically called in the "create" target kernel callback
 *        for bam enabled kernels.
 *
 * \param [out] kernel_details Pointer to array of kernel_details to be initialized
 * \param [in] num_bam_nodes Number of bam nodes in the kernel_details array
 * \param [in] kernel Kernel instance input
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_PARAMETERS If kernel_details is NULL or if num_bam_nodes is 0.
 * \ingroup group_tivx_ext_bam
 */
vx_status tivxBamInitKernelDetails(tivx_bam_kernel_details_t *kernel_details,
                                   uint32_t num_bam_nodes,
                                   tivx_target_kernel_instance kernel);

/*!
* \brief The "create in bam graph" target kernel callback
*
* \param [in] kernel The kernel for which the callback is called
* \param [in] obj_desc Object descriptor array passed as input to this callback
* \param [in] num_params[] Number of parameters in the obj_desc[] array
* \param [in] priv_arg Private argument
* \param [in,out] node_list Pointer to array of nodes to be updated by callback
* \param [in,out] kernel_details Pointer to array of kernel_details to be updated by callback
* \param [in,out] bam_node_cnt Number of bam nodes to be updated by callback
* \param [in] scratch Pointer to scratch memory requirement from node
*
* \ingroup group_tivx_ext_bam_supernode
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_create_in_bam_graph_f)(tivx_target_kernel_instance kernel,
                                                               tivx_obj_desc_t *obj_desc[],
                                                               uint16_t num_params,
                                                               void *priv_arg,
                                                               BAM_NodeParams node_list[],
                                                               tivx_bam_kernel_details_t kernel_details[],
                                                               int32_t * bam_node_cnt,
                                                               void * scratch,
                                                               int32_t * size);

/*!
* \brief The "get node port" target kernel callback
*
* \param [in] kernel The kernel for which the callback is called
* \param [in] ovx_port OpenVX node port number
* \param [out] bam_node BAM node number corresponding to the OpenVX node
* \param [out] bam_port BAM kernel port number corresponding to the OpenVX node number
*
* \ingroup group_tivx_ext_bam_supernode
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_get_node_port_f)(tivx_target_kernel_instance kernel,
                                                               uint8_t ovx_port,
                                                               uint8_t plane,
                                                               uint8_t *bam_node,
                                                               uint8_t *bam_port);
/*!
* \brief The "append internal edges" target kernel callback
*        This callback is optional and only needs to be implemented if the node has more
*        than one BAM kernel inside it.  In this case, the node needs to append the internal
*        edges to the overal BAM graph edge list.
*
* \param [in] kernel The kernel for which the callback is called
* \param [in,out] edge_list BAM graph edge list.
* \param [in,out] bam_edge_cnt Number of edges in the edge list
*
* \ingroup group_tivx_ext_bam_supernode
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_append_internal_edges_f)(tivx_target_kernel_instance kernel,
                                                               BAM_EdgeParams edge_list[],
                                                               int32_t * bam_edge_cnt);
/*!
* \brief The "create in bam graph" target kernel callback
*
* \param [in] kernel The kernel for which the callback is called
* \param [in] obj_desc Object descriptor array passed as input to this callback
* \param [in] num_params[] Number of parameters in the obj_desc[] array
* \param [in,out] graph handle from the supernode
* \param [in] priv_arg private argument
*
* \ingroup group_tivx_ext_bam_supernode
*/
typedef vx_status(VX_CALLBACK *tivx_target_kernel_pre_post_process_f)(tivx_target_kernel_instance kernel,
                                                               tivx_obj_desc_t *obj_desc[],
                                                               uint16_t num_params,
                                                               tivx_bam_graph_handle *graph_handle,
                                                               void *priv_arg);
/*!
 * \brief Allows users to support kernel as part of super node
 *
 *         This is intended to be run after adding the target kernel.
 *
 * \param [in] target_kernel              The kernel for which the callbacks are associated with
 * \param [in] create_in_bam_func         Callback for giving information needed by supernode to create BAM graph.
 * \param [in] get_node_port_func         Callback for translating OpenVX node ports to BAM ports.
 * \param [in] append_internal_edges_func (optional) Callback for appending internal edges to BAM edge list (if there are any)
 * \param [in] preprocess_func            (optional) Callback for performing any processing before the BAM graph is called for each frame (if there is any)
 * \param [in] postprocess_func           (optional) Callback for performing any processing after the BAM graph is called for each frame (if there is any)
 * \param [in] kernel_params_size         Size in bytes of the kernel_params structure
 * \param [in] priv_arg                   (optional) Private arguments to pass to the create callback
 *
 * \ingroup group_tivx_ext_bam_supernode
 *
 */
VX_API_ENTRY vx_status VX_API_CALL tivxEnableKernelForSuperNode(
                             tivx_target_kernel target_kernel,
                             tivx_target_kernel_create_in_bam_graph_f   create_in_bam_func,
                             tivx_target_kernel_get_node_port_f         get_node_port_func,
                             tivx_target_kernel_append_internal_edges_f append_internal_edges_func,
                             tivx_target_kernel_pre_post_process_f      preprocess_func,
                             tivx_target_kernel_pre_post_process_f      postprocess_func,
                             int32_t                                    kernel_params_size,
                             void *priv_arg);


#endif
