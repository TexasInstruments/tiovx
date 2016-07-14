/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _VX_NODE_H_
#define _VX_NODE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Node object
 */


/*! \brief The internal representation of a node.
 * \ingroup group_vx_node
 */
typedef struct _vx_node {
    /*! \brief The internal reference object. */
    tivx_reference_t    base;
    /*! \brief The pointer to parent graph */
    vx_graph            graph;
    /*! \brief The pointer to the kernel structure */
    vx_kernel           kernel;
    /*! \brief The list of references which are the values to pass to the kernels */
    vx_reference        parameters[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief Node Object descriptor */
    tivx_obj_desc_node_t  *obj_desc;
    /*! \brief Node performance */
    vx_perf_t perf;
    /*! \brief parameter replicated flags */
    vx_bool replicated_flags[TIVX_KERNEL_MAX_PARAMS];
    /*! \brief reset valid rectangle */
    vx_bool valid_rect_reset;
    /*! \brief Node completeion callback */
    vx_nodecomplete_f completion_callback;
} tivx_node_t;

/**
 * \brief Set the target for node based on immediate mode target
 *        set in context
 *
 * \param [in] node The reference to the node
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_node
 */
vx_status ownSetNodeImmTarget(vx_node node);

/**
 * \brief Set the attribute VX_NODE_VALID_RECT_RESET in node
 *
 * \param [in] node The reference to the node
 * \param [in] is_reset The value to set for the attribute
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_node
 */
vx_status ownSetNodeAttributeValidRectReset(vx_node node, vx_bool is_reset);

/*! \brief Init user kernel or target kernel associated with this node
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelInit(vx_node node);


/*! \brief Schedule user kernel or target kernel associated with this node
 *         for execution
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelSchedule(vx_node node);

/*! \brief Wait for completion event
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelWaitEvent(vx_node node);

/*! \brief DeInit user kernel or target kernel associated with this node
 * \ingroup group_vx_node
 */
vx_status ownNodeKernelDeinit(vx_node node);

/*! \brief Reset node performance statistics
 * \ingroup group_vx_node
 */
vx_status ownResetNodePerf(vx_node node);

/*! \brief Called by graph each time after node execution
 * \ingroup group_vx_node
 */
vx_status ownUpdateNodePerf(vx_node node);

#ifdef __cplusplus
}
#endif

#endif
