/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_NODES_H_
#define TIVX_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The list of supported nodes in the TIOVX.
 */

/*! \brief [Graph] Creates a Harris Corners Node.
 * \param [in] graph The reference to the graph.
 * \param [in] input The input <tt>\ref VX_DF_IMAGE_U8</tt> image.
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxHarrisCornersNode(vx_graph graph,
                            vx_image  input,
                            vx_uint32 scaling_factor,
                            vx_int32  nms_threshold,
                            vx_uint8  q_shift,
                            vx_uint8  win_size,
                            vx_uint8  score_method,
                            vx_uint8  suppression_method,
                            vx_array  corners,
                            vx_scalar num_corners);

#ifdef __cplusplus
}
#endif

#endif
