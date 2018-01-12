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
 * \file vx_tutorial_graph_user_kernel_pytiovx.c Show example usage of using user kernel in larger graph.
 *
 * In the tutorial we learn below concepts,
 * - Use the user/target kernel defined in tutorial \ref vx_tutorial_graph_user_kernel.c
 *   in a larger graph
 * - Specify the user defined kernel in .py file in using PyTIOVX APIs
 * - Generate the graph code using the user defined kernel and other built-in kernels using the PyTIOVX APIs.
 *
 * When using PyTIOVX tool, first create a .py file which describes the OpenVX graph.
 * Follow comments in \ref vx_tutorial_graph_user_kernel_pytiovx_uc.py to understand
 * the graph description basic API. In this example, we also use the PyTIOVX API to define
 * a user defined kernel. This allows user to add their own kernels to the tool without
 * having to modify deep within the tool.
 *
 * Next PyTIOVX tool is run to generate .c/.h file for the graph described in .py file.
 * A .jpg file which shows the graph visually is also generated in the process.
 *
 * Follow steps mentioned in PyTIOVX user guide <a href="../pytiovx_guide/index.html" target="_blank">[HTML]</a>
 * to install and run the PyTIOVX tool. See also additional APIs provided to describe a
 * graph via PyTIOVX.
 *
 * For this example, run below command to generate the code and image file
 * \code
 * python vx_tutorial_graph_user_kernel_pytiovx_uc.py
 * \endcode
 *
 * Include the generated C code API header file
 * \code
 * #include <ch03_graph/vx_tutorial_graph_user_kernel_pytiovx_uc.h>
 * \endcode
 * Note, that the generated file name and APIs use as prefix the string name passed
 * as input during context create in the .py file.
 *
 * Follow the comments in the function vx_tutorial_graph_user_kernel_pytiovx()
 * to complete the rest of tutorial code to invoke the generated OpenVX graph.
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
#include <ch03_graph/vx_tutorial_graph_user_kernel_pytiovx_uc.h>
#include <ch03_graph/phase_rgb_user_kernel.h>

/** \brief Input file name */
#define IN_FILE_NAME                "colors.bmp"

/** \brief Output file name when tutorial is run with user kernel */
#define OUT_USER_KERNEL_FILE_NAME   "vx_tutorial_graph_user_kernel_pytiovx_out.bmp"

/** \brief Output file name when tutorial is run with target kernel */
#define OUT_TARGET_KERNEL_FILE_NAME "vx_tutorial_graph_target_kernel_pytiovx_out.bmp"

/**
 * \brief Tutorial Entry Point
 *
 * \param add_as_target_kernel [in] 'vx_false_e', run this tutorial with custom kernel running as user kernel \n
 *                                  'vx_true_e', run this tutorial with custom kernel running as target kernel \n
 */
void vx_tutorial_graph_user_kernel_pytiovx(vx_bool add_as_target_kernel)
{
    vx_status status;
    /**
     * - Define the data structure representing the generated OpenVX use-case code.
     *
     * This structure includes the context, data object, node, graph handles
     * for the generated code.
     * \code
     */
    vx_tutorial_graph_user_kernel_pytiovx_uc_t uc;
    /** \endcode */

    /**
     * - Define the OpenVX context
     *
     * \code
     */
    vx_context context = NULL;
    /** \endcode */

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

    printf(" vx_tutorial_graph_user_kernel_pytiovx: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context
     *
     * Even though the generated use-case creates OpenVX context later, we create it
     * here, so as to allow us to register the user/target kernel, before creating
     * the OpenVX use-case
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
    if(status!=VX_SUCCESS)
    {
        printf(" vx_tutorial_graph_user_kernel_pytiovx: ERROR: unable to add user kernel !!!\n");
    }
    /** \endcode */

    if(status==VX_SUCCESS)
    {
        /**
         * - Create the OpenVX use-case using the generated create API.
         *
         * This creates the OpenVX context, data objects, nodes and graph for this use-case. \n
         * NOTE: graph verify is not yet called. \n
         * NOTE: Any customization like changing parameter values, loading data to data objects
         *       should be done next, before calling graph verify
         * \code
         */
        vx_tutorial_graph_user_kernel_pytiovx_uc_create(&uc);
        /** \endcode */

        /**
         * - Load input into input data reference
         *
         * NOTE: the data reference names in the use-case structure, match the name specified via name="xyz"
         *       in the .py file
         * \code
         */
        printf(" Loading file %s ...\n", IN_FILE_NAME);
        status = load_image_from_file(uc.input, IN_FILE_NAME, vx_true_e);
        /** \endcode */

        /**
         * - Print data object info for debug purposes
         * \code
         */
        show_image_attributes(uc.input);
        show_image_attributes(uc.grad_x);
        show_image_attributes(uc.grad_y);
        show_image_attributes(uc.phase);
        show_image_attributes(uc.phase_rgb);
        /** \endcode */

        /**
         * - Call generated API to verify graphs present in this use-case
         *
         * Also prints graph info for debug purposes
         * \code
         */
        vx_tutorial_graph_user_kernel_pytiovx_uc_verify(&uc);
        show_graph_attributes(uc.graph_0);
        /** \endcode */

        /** export graph to dot file, which can be coverted to jpg using dot tool
         * \code
         */
        tivxExportGraphToDot(uc.graph_0, ".", "vx_tutorial_graph_user_kernel_pytiovx");
        /** \endcode */

        if(status==VX_SUCCESS)
        {
            printf(" Executing graph ...\n");

            /**
             * - Call generated API to run graphs present in this use-case
             * \code
             */
            vx_tutorial_graph_user_kernel_pytiovx_uc_run(&uc);
            /** \endcode */

            printf(" Executing graph ... Done !!!\n");

            /**
             * - Print graph execution info, save output data to file
             * \code
             */
            show_graph_attributes(uc.graph_0);

            printf(" Saving to file %s ...\n", out_file);
            save_image_to_file(out_file, uc.phase_rgb);
            /** \endcode */
        }

        /**
         * - Call generated API to delete this use-case
         *
         * This releases the data objects, nodes, graphs and context associated with this use-case
         * \code
         */
        vx_tutorial_graph_user_kernel_pytiovx_uc_delete(&uc);
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

    printf(" vx_tutorial_graph_user_kernel_pytiovx: Tutorial Done !!! \n");
    printf(" \n");
}
