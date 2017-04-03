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

#define VXLIB_MAX_EDGES 10

/*! \brief Graph handle
 *
 *         Handle associated with an instance of a BAM graph.
 *
 *         The structure this handle represents is internally defined
 *         and not needed by the user.
 *
 *         The user is given this handle when calling \ref tivxBamCreateHandle,
 *         and can destroy it by calling \ref tivxBamDestroyHandle.
 *
 * \ingroup group_tivx_ext
 */
typedef void *tivx_bam_graph_handle;


/*! \brief Frame Parameters structure
 *
 *         Frame level parameters for each buffer in the kernel.
 *
 *         This includes frame level dimention properties for each buffer
 *         that the kernel needs, as well as kernel info that the user can
 *         obtain by calling the kernel sepecific getKernelInfo function.
 *
 *         This structure should be populated as an input to the
 *         \ref tivxBamCreateHandle function.
 *
 * \ingroup group_tivx_ext
 */
typedef struct _tivx_bam_frame_params
{
    VXLIB_bufParams2D_t *buf_params[VXLIB_MAX_EDGES];
    BAM_KernelInfo    kernel_info;
}tivx_bam_frame_params_t;

typedef struct _tivx_bam_frame_params2
{
    VXLIB_bufParams2D_t *buf_params[VXLIB_MAX_EDGES];
    BAM_KernelInfo    kernel_info[VXLIB_MAX_EDGES];
}tivx_bam_frame_params2_t;

/*!
 * \brief BAM Create Graph Handle for Single Node
 *
 *        This function will create a BAM graph of 1 node, given the
 *        kernel_id, frame_params, and computation kernel parameters
 *        pointer. compute_kernel_params may be set to NULL if there are
 *        none for the kernel.
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
                                        tivx_bam_frame_params_t *frame_params,
                                        void *compute_kernel_params,
                                        tivx_bam_graph_handle *graph_handle);

/*!
 * \brief BAM Create Graph Handle for Multiple Nodes
 *
 *        This function will create a BAM graph of multiple nodes, given the
 *        node list, edge list, frame_params, and list of computation kernel
 *        parameters. compute_kernel_params may be set to NULL if there are
 *        none for the graph, and each pointer in the list may be set to NULL
 *        if there are no parameters for the particular node.
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
                                       tivx_bam_frame_params2_t *frame_params,
                                       void *compute_kernel_params[10],
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
