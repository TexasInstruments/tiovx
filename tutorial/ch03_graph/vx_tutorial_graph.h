/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
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
