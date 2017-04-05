/*==========================================================================*/
/*      Copyright (C) 2017 Texas Instruments Incorporated.                  */
/*                      All Rights Reserved                                 */
/*==========================================================================*/

/**
 *  @file       bam_kernel_wrapper.h
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
#include <VX/vx.h>
#include "vx_bam_kernel_database.h"
#include "bam_common.h"

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
 * \ingroup group_tivx_ext
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
 * \ingroup group_tivx_ext
 */
typedef struct _tivx_bam_kernel_details
{
    BAM_KernelInfo    kernel_info;
    void *compute_kernel_params;

}tivx_bam_kernel_details_t;


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
 * \ingroup group_tivx_ext
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
 * \ingroup group_tivx_ext
 */
vx_status tivxBamCreateHandleMultiNode(BAM_NodeParams node_list[],
                                       BAM_EdgeParams edge_list[],
                                       VXLIB_bufParams2D_t *buf_params[],
                                       tivx_bam_kernel_details_t kernel_details[],
                                       tivx_bam_graph_handle *graph_handle);

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
 * \ingroup group_tivx_ext
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
 * \ingroup group_tivx_ext
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
 * \ingroup group_tivx_ext
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
 * \ingroup group_tivx_ext
 */
void tivxBamDestroyHandle(tivx_bam_graph_handle graph_handle);


#endif
