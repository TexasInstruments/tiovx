/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef VX_TUTORIAL_GRAPH_H
#define VX_TUTORIAL_GRAPH_H

#include <VX/vx.h>

/*
 * \brief Tutorials showing more complex graph operations
 *
 *        Aim of these tutorials is to show
 *        - How to associate targets with graph for improving performance
 *        - How to implement user kernels and mix them with OpenVX kernels
 *        - How to use the PyTIVX tool to generate OpenVX code for complex graphs
 */

/*
 * \brief vxNodeSetTarget usage
 *
 *        Load a image from BMP file
 *        Compute gradients, magnitude, phase
 *        Save output to BMP file
 */
void vx_tutorial_graph_image_gradients();

/*
 * \brief vxNodeSetTarget usage
 *
 *        Load a image from BMP file
 *        Same graph as vx_tutorial_graph_image_gradients()
 *        Save output to BMP file
 */
void vx_tutorial_graph_image_gradients_pytiovx();

/*
 * \brief User kernel usage
 *
 *        Load a 8b phase image from BMP file
 *        Add post processing user kernel to the graph
 *        Save output to BMP file
 */
void vx_tutorial_graph_user_kernel(vx_bool add_as_target_kernel);

/*
 * \brief PyTIOVX usage
 *
 *        Load a image from BMP file
 *        Compute gradients, magnitude, phase, user kernel
 *        Generate OpenVX graph code using PyTIOVX tool
 *        Save output to BMP file
 */
void vx_tutorial_graph_user_kernel_pytiovx(vx_bool add_as_target_kernel);

/*
 * \brief Run all tutorials in this module
 */
void vx_tutorial_graph_run_all();

/*
 * \brief Interactive execution of tutorials using console IO
 */
void vx_tutorial_graph_run_interactive();

#endif
