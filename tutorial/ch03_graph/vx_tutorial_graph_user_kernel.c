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


/**
 * \file vx_tutorial_graph_user_kernel.c Show example usage of developing and invoking a user kernel
 *
 * OpenVX allows users to define their own nodes called <b> user kernels </b> and
 * provides an API to implement and invoke these user kernels.
 * Once defined and implemented, user kernels can be added as any other node in an OpenVX graph
 *
 * OpenVX user kernels need to run on the "HOST" CPU, i.e CPU where OpenVX APIs are invoked.
 * This can be limiting for users who want
 * to exploit performnance of specialized targets, like DSP, to execute their own functions.
 * For this purpose, TIOVX specifies an API
 * to define and implement <b> "target kernels" </b>. Like user kernels, once a target kernel is
 * defined and implemented on a target (ex, DSP), it can be invoked as a normal node in an OpenVX graph
 * using standard OpenVX APIs.
 *
 * Most of the host side logic remains same for both user kernel and target kernel.
 * For target kernel, additional target specific implementation is required on the required on target.
 *
 * In this tutorial we learn below concepts,
 * - Firstly, implement an OpenVX user kernel on HOST CPU
 * - Invoke it as a single node OpenVX graph to test the user kernel in standalone mode
 * - Next, implement target kernel on DSP for the same user kernel function
 * - Modify the HOST side logic to use target kernel instead of user kernel
 * - Invoke the same single node graph but this time with the target kernel
 *
 * The custom kernel (user or target) we define in this tutorial does the below,
 * - Take 16b signed phase image as input
 * - Generate a 24b RGB image with different colors representing different phase values
 *
 * <b>NOTE: </b>
 * Make sure to run \ref vx_tutorial_graph_image_gradients.c before running this tutorial, since output file from
 * \ref vx_tutorial_graph_image_gradients.c is used as input by this tutorial
 *
 *
 * \par Implementing a User Kernel
 * - Follow comments in file \ref phase_rgb_user_kernel.c to understand how
 *   to implement a user kernel
 * - Follow comments in the function vx_tutorial_graph_user_kernel() to understand how
 *   to register the implemented user kernel and invoke it as a user graph.
 *
 * \par Implementing a Target Kernel on C66x DSP
 * - For target kernel, there are two parts to the implementation, HOST side implementation
 *   and target side implementation
 * - Target side implementation
 *   - Follow comments in \ref phase_rgb_target_kernel.c for target side implementation
 * - HOST side implementation
 *   - Follow comments in file \ref phase_rgb_user_kernel.c to understand how
 *     to implement the HOST side portion of target kernel
 *   - Follow comments in the function vx_tutorial_graph_user_kernel() to understand how
 *     to register the implemented target kernel and invoke it as a user graph.
 *
 *   <b> NOTE: </b> \n
 *   The implementation on HOST side for user kernel and target kernel is largely the same. Any
 *   difference in implementation between user kernel and target kernel is shown by using
 *   using boolen variable 'add_as_target_kernel'.
 *   - When 'add_as_target_kernel' is 'vx_false_e', it is the
 *   implementation for user kernel.
 *   - When 'add_as_target_kernel' is 'vx_true_e', it is the implementation
 *   for target kernel.
 *
 * Include below file to use the HOST callable interface for the user/target kernel
 * \code
 * #include <ch03_graph/phase_rgb_user_kernel.h>
 * \endcode
 */

#include <stdio.h>
#include <VX/vx.h>
#include <TI/tivx.h>
#include <utility.h>
#include <ch03_graph/phase_rgb_user_kernel.h>

/** \brief Input file name */
#define IN_FILE_NAME      "${VX_TEST_DATA_PATH}/vx_tutorial_graph_image_gradients_phase_out.bmp"

/** \brief Output file name when tutorial is run with user kernel */
#define OUT_USER_KERNEL_FILE_NAME   "${VX_TEST_DATA_PATH}/vx_tutorial_graph_user_kernel_out.bmp"

/** \brief Output file name when tutorial is run with target kernel */
#define OUT_TARGET_KERNEL_FILE_NAME "${VX_TEST_DATA_PATH}/vx_tutorial_graph_target_kernel_out.bmp"

/**
 * \brief Tutorial Entry Point
 *
 * \param add_as_target_kernel [in] 'vx_false_e', run this tutorial with custom kernel running as user kernel \n
 *                                  'vx_true_e', run this tutorial with custom kernel running as target kernel \n
 */
void vx_tutorial_graph_user_kernel(vx_bool add_as_target_kernel)
{
    /**
     * - Define OpenVX object references required to invoke the user/target kernel
     *
     * We need an OpenVX context, a graph to execute the node, a node to invoke the user target/kernel,
     * and input/output image for the node.
     * \code
     */
    vx_context context;
    vx_image in_image = NULL;
    vx_image out_image = NULL;
    vx_node node = NULL;
    vx_graph graph = NULL;
    /** \endcode */
    vx_uint32 width, height;
    vx_status status;

    /**
     * - Specify the output file name
     *
     * In order to compare and confirm that the user kernel and target kernel generate the
     * same output we specify a different output file for user and target kernel
     *
     * \code
     */
    char *out_file = OUT_USER_KERNEL_FILE_NAME;

    if(add_as_target_kernel)
    {
        out_file = OUT_TARGET_KERNEL_FILE_NAME;
    }
    /** \endcode */

    printf(" vx_tutorial_graph_user_kernel: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context
     *
     * \code
     */
    context = vxCreateContext();
    /** \endcode */


    /**
     * - Register user or target kernel into the OpenVX context
     *
     * This is required so that the user kernel or target can be invoked as
     * any other node within an OpenVX graph.
     * 'add_as_target_kernel' is used specify whether to register kernel
     * as user kernel or target kernel.
     *
     * This function is implemented in \ref phase_rgb_user_kernel.c
     *
     * <b> Rest of the implementation post this is same for both user kernel and
     * target kernel. </b>
     * \code
     */
    status = phase_rgb_user_kernel_add(context, add_as_target_kernel);
    if(status!=(vx_status)VX_SUCCESS)
    {
        printf(" vx_tutorial_graph_user_kernel: ERROR: unable to add user kernel !!!\n");
    }
    /** \endcode */

    if(status==(vx_status)VX_SUCCESS)
    {
        printf(" Loading file %s ...\n", IN_FILE_NAME);

        /**
         * - Create input image and load input data into it
         *
         * A name is given to the input image object via vxSetReferenceName.
         * Image attributes are shown for informative purpose and width x height
         * queried for later use.
         *
         * \code
         */
        in_image = tivx_utils_create_vximage_from_bmpfile(context, IN_FILE_NAME, (vx_bool)vx_true_e);
        vxSetReferenceName((vx_reference)in_image, "INPUT");
        show_image_attributes(in_image);

        vxQueryImage(in_image, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(in_image, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
        /** \endcode */

        /**
         * - Create output image
         *
         * Output image dimensions as same as input image.
         * \code
         */
        out_image = vxCreateImage(context, width, height, (vx_df_image)VX_DF_IMAGE_RGB);
        vxSetReferenceName((vx_reference)out_image, "OUTPUT");
        show_image_attributes(out_image);
        /** \endcode */

        /**
         * - Create a graph object to execute the user/target kernel node
         *
         * \code
         */
        graph = vxCreateGraph(context);
        /** \endcode */

        /**
         * - Create a node for the registered user/target kernel and insert into the graph
         *
         * This function is implemented in \ref phase_rgb_user_kernel.c
         * \code
         */
        node = phase_rgb_user_kernel_node(graph, in_image, out_image);
        /** \endcode */

        /**
         * - Verify the graph using standard OpenVX API
         * \code
         */
        status = vxVerifyGraph(graph);

        show_graph_attributes(graph);
        show_node_attributes(node);
        /** \endcode */

        /** export graph to dot file, which can be coverted to jpg using dot tool
         * \code
         */
        tivxExportGraphToDot(graph, ".", "vx_tutorial_graph_user_kernel");
        /** \endcode */

        /**
         * - Excute the graph using standard OpenVX APIs
         *
         * Ouptut image is saved to specified output file.
         * \code
         */
        if(status==(vx_status)VX_SUCCESS)
        {
            printf(" Executing graph ...\n");

            vxScheduleGraph(graph);
            vxWaitGraph(graph);

            printf(" Executing graph ... Done !!!\n");

            show_graph_attributes(graph);
            show_node_attributes(node);

            printf(" Saving to file %s ...\n", out_file);
            tivx_utils_save_vximage_to_bmpfile(out_file, out_image);
        }
        /** \endcode */

        /**
         * - Release image, node, graph objects
         *
         * \code
         */
        vxReleaseImage(&in_image);
        vxReleaseImage(&out_image);
        vxReleaseNode(&node);
        vxReleaseGraph(&graph);
        /** \endcode */

        /**
         * - Unregister user/kernel from the context
         *
         * This function is implemented in \ref phase_rgb_user_kernel.c
         * \code
         */
        phase_rgb_user_kernel_remove(context);
        /** \endcode */
    }
    /**
     * - Finally release the OpenVX context
     *
     * \code
     */
    vxReleaseContext(&context);
    /** \endcode */

    printf(" vx_tutorial_graph_user_kernel: Tutorial Done !!! \n");
    printf(" \n");
}

