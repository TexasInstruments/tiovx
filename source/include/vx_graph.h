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


#ifndef _VX_GRAPH_H_
#define _VX_GRAPH_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Graph object
 */


/*! \brief Max possible head nodes in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_HEAD_NODES          (8u)

/*! \brief Max possible leaf nodes in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_LEAF_NODES          (8u)

/*! \brief Max possible parameters in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_PARAMS              (8u)

/*! \brief Max possible delays in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_DELAYS              (8u)

/*! \brief The top level data for graph object
 * \ingroup group_vx_graph
 */
typedef struct _vx_graph {

    /*! \brief The base reference object */
    tivx_reference_t      base;

    /*! \brief Flag to maintain state of graph verification */
    vx_bool verified;

    /*! \brief Flag to maintain state of graph re-verification */
    vx_bool reverify;

    /*! \brief Nodes in a graph */
    vx_node nodes[TIVX_GRAPH_MAX_NODES];

    /* \brief Number of nodes in a graph */
    uint32_t num_nodes;

    /*! \brief Head Nodes in a graph */
    vx_node head_nodes[TIVX_GRAPH_MAX_HEAD_NODES];

    /* \brief Number of head nodes in a graph */
    uint32_t num_head_nodes;

    /*! \brief Leaf Nodes in a graph */
    vx_node leaf_nodes[TIVX_GRAPH_MAX_HEAD_NODES];

    /* \brief Number of leaf nodes in a graph */
    uint32_t num_leaf_nodes;

    /*! \brief The list of graph parameters. */
    struct {
        /*! \brief The reference to the node which has the parameter */
        vx_node node;
        /*! \brief The index to the parameter on the node. */
        uint32_t  index;
    } parameters[TIVX_GRAPH_MAX_PARAMS];
    /*! \brief The number of graph parameters. */
    uint32_t      num_params;

    /*! \brief The state of the graph (vx_graph_state_e) */
    vx_enum        state;

    /*! \brief The performance logging variable. */
    vx_perf_t      perf;

    /*! \brief The array of all delays in this graph */
    vx_delay       delays[TIVX_GRAPH_MAX_DELAYS];

    /*! \brief valid rect information for input image's */
    vx_rectangle_t in_valid_rect[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief valid rect information for input image's */
    vx_rectangle_t *in_valid_rect_ptr[TIVX_KERNEL_MAX_PARAMS];


    /*! \brief valid rect information for output image's */
    vx_rectangle_t out_valid_rect[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    /*! \brief valid rect information for output image's */
    vx_rectangle_t *out_valid_rect_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

} tivx_graph_t;



/*! \brief Get next free node entry in graph
 *
 * \param graph [in] graph object
 *
 * \return 0 or more value, index of node in graph
 * \return -1, all nodes in graph are used and free node not found
 *
 * \ingroup group_vx_graph
 */
int32_t ownGraphGetFreeNodeIndex(vx_graph graph);

/*! \brief Add's a node to a graph
 *
 *         'index' should be the one that is returned via
 *         ownGraphGetFreeNodeIndex()
 *
 * \param graph [in] graph object
 * \param node  [in] the node to add
 * \param index [in] the index in graph at which to add the node
 *
 * \return VX_SUCCESS, on sucess
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphAddNode(vx_graph graph, vx_node node, int32_t index);

/*! \brief Remove a node from a graph
 *
 * \param graph [in] graph object
 * \param node  [in] the node to remove
 *
 * \return VX_SUCCESS, on sucess
 *
 * \ingroup group_vx_graph
 */
vx_status ownGraphRemoveNode(vx_graph graph, vx_node node);


/*! \brief Perform topological sort of graph nodes
 *
 * \param context   [in] context to use while sorting
 * \param nodes     [in/out] IN: Unosrted node, OUT: Sorted nodes
 * \param num_nodes [in] Number of nodes
 * \param has_cycles [out] vx_true_e: Graph has cycles and cannot be sorted
 *                         vx_false_e: Graph is acyclic and nodes[] contains the sorted nodes
 *
 * \ingroup group_vx_graph
 */
void ownGraphTopologicalSort(tivx_graph_sort_context *context, vx_node *nodes, uint32_t num_nodes, vx_bool *has_cycle);


/*! \brief Mark graph to be reverified
 * \ingroup group_vx_graph
 */
void ownGraphSetReverify(vx_graph graph);

#ifdef __cplusplus
}
#endif

#endif
