/*
*
* Copyright (c) 2019 Texas Instruments Incorporated
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
#ifndef _TIVX_EXT_SUPER_NODE_H_
#define _TIVX_EXT_SUPER_NODE_H_

/*!
 * \file
 * \brief The TI Super Node extension.
 */

/*!
 * \defgroup group_super_node Super Node Framework Type APIs
 * \brief APIs creating and using super node framework type
 * \ingroup group_tivx_ext_host
 */

#define TIVX_SUPER_NODE  "tivx_super_node"

#include <VX/vx.h>
#include <TI/tivx.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*! \brief The object type enumeration for super nodes.
 * \ingroup group_super_node
 */
#define TIVX_TYPE_SUPER_NODE          0x818 /*!< \brief A <tt>\ref tivx_super_node</tt>. */

#define TIVX_KERNEL_SUPERNODE VX_KERNEL_BASE(VX_ID_TI, TIVX_LIBRARY_EXTENSION_BASE) + 0x0

/*! \brief The Super Node Framework Object. Super Node is a strongly-typed container for connected nodes.
 * \ingroup group_super_node
 */
typedef struct _tivx_super_node * tivx_super_node;

/*! \brief The super node attributes.
 * \ingroup group_super_node
 */
enum tivx_super_node_attribute_e {
    /*! \brief Queries the target this super-node is run on. Read-only. Use a <tt>\ref vx_char[]</tt> parameter of size
     *   TIVX_TARGET_MAX_NAME. */
    TIVX_SUPER_NODE_TARGET_STRING = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_SUPER_NODE) + 0x0,
    /*! \brief Queries the performance of the super node execution.
     * The accuracy of timing information is platform dependent and also depends on the graph
     * optimizations. Read-only.
     * \note Performance tracking must have been enabled. See <tt>\ref vx_directive_e</tt>.
     */
    TIVX_SUPER_NODE_PERFORMANCE = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_SUPER_NODE) + 0x1,
    /*! \brief Queries the status of the super node execution. Read-only. Use a <tt>\ref vx_status</tt> parameter. */
    TIVX_SUPER_NODE_STATUS = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_SUPER_NODE) + 0x2,
    /*! \brief Queries the number of nodes in a super node. Read-only. Use a <tt>\ref vx_uint32</tt> parameter.*/
    TIVX_SUPER_NODE_NUM_NODES = VX_ATTRIBUTE_BASE(VX_ID_TI, TIVX_TYPE_SUPER_NODE) + 0x3,
};

/*!
 * \brief Creates a reference to a super node object.
 * \details This API is used to identify a grouping of nodes for the
 *          purpose of optimizing data movement between the nodes.  The
 *          framework will attempt to parallelize the processing of all
 *          nodes in this group on the same processor such that each node
 *          operates on a block of memory at a time in a pipelined fashion
 *          so that intermediate data accesses can stay in local RAM instead
 *          of going to DDR.  This reduces the overall DDR bandwidth requirement
 *          as well as increases performance.
 *
 *          If any of the following conditions are violated, the object will
 *          not be created and an error will be registered:
 *
 *          - All nodes in this super node must not be included in a different
 *            super node
 *          - All nodes in this super node must be part of the 'graph' graph reference.
 *
 *          If any of the following coniditions are violated, the object may
 *          successfully be created, but may return an error durring graph
 *          verification:
 *
 *          - All nodes must map to the same processor core
 *          - All nodes must be connected via one or more edges
 *          - All nodes must have implementations which support this kind
 *            of node fusion (i.e. BAM if on C66x).
 *
 * \param [in] graph The graph reference.
 * \param [in] nodes List of nodes which should be grouped together
 * \param [in] num_nodes Number of nodes in nodes[] array
 *
 * \ingroup group_super_node
 *
 * \returns A super node reference <tt>\ref tivx_super_node</tt>. Any possible errors preventing a
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
 */
VX_API_ENTRY tivx_super_node VX_API_CALL tivxCreateSuperNode(vx_graph graph,
    vx_node nodes[], uint32_t num_nodes);

/*! \brief Releases a reference to a super node object.
 * The object may not be garbage collected until its total reference count is zero.
 * \param [in] super_node The pointer to the super node to release.
 * \ingroup group_super_node
 * \post After returning from this function the reference is zeroed.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_ERROR_INVALID_REFERENCE If super_node is not a <tt>\ref tivx_super_node</tt>.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxReleaseSuperNode(tivx_super_node *super_node);

/*!
 * \brief Queries the super node for some specific information.
 *
 * \param [in] super_node        The reference to the super node.
 * \param [in] attribute         The attribute to query. Use a <tt>\ref tivx_super_node_attribute_e</tt>.
 * \param [out] ptr              The location at which to store the resulting value.
 * \param [in] size              The size in bytes of the container to which \a ptr points.
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS                   No errors.
 * \retval VX_ERROR_INVALID_REFERENCE   If the \a super_node is not a <tt>\ref tivx_super_node</tt>.
 * \retval VX_ERROR_NOT_SUPPORTED       If the \a attribute is not a value supported on this implementation.
 * \retval VX_ERROR_INVALID_PARAMETERS  If any of the other parameters are incorrect.
 *
 * \ingroup group_super_node
 */
VX_API_ENTRY vx_status VX_API_CALL tivxQuerySuperNode (tivx_super_node super_node,
                                                      vx_enum attribute,
                                                      void *ptr,
                                                      vx_size size);

/*! \brief Sets the super node target to the provided value. A success invalidates the graph
 * that the super node belongs to (<tt>\ref vxVerifyGraph</tt> must be called before the next execution)
 * \param [in] super_node  The reference to the <tt>\ref tivx_super_node</tt> object.
 * \param [in] target_enum  The target enum to be set to the <tt>\ref tivx_super_node</tt> object.
 * Use a <tt>\ref vx_target_e</tt>.
 * \param [in] target_string  The target name ASCII string. This contains a valid value
 * when target_enum is set to <tt>\ref VX_TARGET_STRING</tt>, otherwise it is ignored.
 * \ingroup group_super_node
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS Super node target set.
 * \retval VX_ERROR_INVALID_REFERENCE If super_node is not a <tt>\ref tivx_super_node</tt> or if nodes within
 *                                    the super node is no a <tt>\ref vx_node</tt> .
 * \retval VX_ERROR_NOT_SUPPORTED If an included node kernel is not supported by the specified target.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetSuperNodeTarget(tivx_super_node super_node,
                                                          vx_enum target_enum,
                                                          const char* target_string);

/*! \brief Sets the tile size for a given supernode in a graph. This is only valid for
 * BAM-enabled kernels on C66 DSP.
 * \param [in] super_node  The reference to the <tt>\ref vx_node</tt> object.
 * \param [in] block_width The tile width in pixels.
 * \param [in] block_height The tile height in lines.
 * \ingroup group_super_node
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS if the tile size is set correctly.
 */
VX_API_ENTRY vx_status VX_API_CALL tivxSetSuperNodeTileSize(tivx_super_node super_node,
                                                            vx_uint32 block_width,
                                                            vx_uint32 block_height);

#ifdef  __cplusplus
}
#endif

#endif
