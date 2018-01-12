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
 * \file vx_tutorial_graph_image_gradients.c Performs a Sobel filter followed by a magnitude node
 * and phase node on the X and Y outputs of the Sobel filter.
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context, OpenVX image data object and OpenVX virtual image data object
 * - How to read a BMP file and load the pixel values into the image data object
 * - How to query the image data object for attributes like width, height
 * - How to create OpenVX node and associate it with previously created graph
 * - How to choose a CPU core for an OpenVX to execute on
 * - How to schedule OpenVX graph for execution then execute the graph
 * - How to query the node data object for attributes like width, height
 * - How to query the graph data object for attributes like number of nodes and parameters
 * - How to read pixel values from an image data object and save it as a BMP file
 * - How to cleanup all created resources and exit the OpenVX application
 *
 * To include OpenVX interfaces include below file
 * \code
 * #include <VX/vx.h>
 * \endcode
 *
 * Follow the comments in the function vx_tutorial_graph_image_gradients()
 * to understand this tutorial
 *
 */


#include <stdio.h>
#include <VX/vx.h>
#include <TI/tivx.h>
#include <utility.h>

/** \brief Input file name */
#define IN_FILE_NAME         "colors.bmp"

/** \brief Phase file name */
#define PHASE_FILE_NAME      "vx_tutorial_graph_image_gradients_phase_out.bmp"

/** \brief Magnitude file name */
#define MAGNITUDE_FILE_NAME  "vx_tutorial_graph_image_gradients_magnitude_out.bmp"

/** \brief Gradient X file name */
#define GRAD_X_FILE_NAME     "vx_tutorial_graph_image_gradients_grad_x_out.bmp"

/** \brief Gradient Y file name */
#define GRAD_Y_FILE_NAME     "vx_tutorial_graph_image_gradients_grad_y_out.bmp"

#define NUM_NODES    (6u)

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_graph_image_gradients()
{
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image in_image = NULL;
    vx_image grad_x = NULL;
    vx_image grad_y = NULL;
    vx_image magnitude = NULL;
    vx_image phase = NULL;
    vx_image magnitude_image = NULL;
    vx_image grad_x_image = NULL;
    vx_image grad_y_image = NULL;
    vx_graph graph = NULL;
    vx_scalar shift = NULL;
    vx_node node[NUM_NODES] = {NULL};
    /** \endcode */
    int32_t shift_value = 0;
    vx_uint32 width, height;
    vx_status status;
    uint32_t i;

    printf(" vx_tutorial_graph_image_gradients: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context.
     *
     * This MUST be done first before any OpenVX API call.
     * The context that is returned is used as input for subsequent OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    /**
     * - Create image object.
     *
     * Follow the comments in create_image_from_file() to see
     * how a vx_image object is created and filled with RGB data from BMP file \ref IN_FILE_NAME
     * \code
     */
    in_image = create_image_from_file(context, IN_FILE_NAME, vx_true_e);
    /** \endcode */

    vxSetReferenceName((vx_reference)in_image, "INPUT");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(in_image);
    /** \endcode */

    vxQueryImage(in_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(in_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    grad_x = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    /** \endcode */

    vxSetReferenceName((vx_reference)grad_x, "GRAD_X");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(grad_x);
    /** \endcode */

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    grad_y = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    /** \endcode */

    vxSetReferenceName((vx_reference)grad_y, "GRAD_Y");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(grad_y);
    /** \endcode */

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    magnitude = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    /** \endcode */

    vxSetReferenceName((vx_reference)magnitude, "MAGNITUDE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(magnitude);
    /** \endcode */

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    magnitude_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    /** \endcode */

    vxSetReferenceName((vx_reference)magnitude_image, "MAGNITUDE_IMAGE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(magnitude_image);
    /** \endcode */

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    grad_x_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    /** \endcode */

    vxSetReferenceName((vx_reference)grad_x_image, "GRAD_X_IMAGE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(grad_x_image);
    /** \endcode */

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    grad_y_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    /** \endcode */

    vxSetReferenceName((vx_reference)grad_y_image, "GRAD_Y_IMAGE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(grad_y_image);
    /** \endcode */

    shift = vxCreateScalar(context, VX_TYPE_INT32, &shift_value);

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    phase = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    /** \endcode */
    vxSetReferenceName((vx_reference)phase, "PHASE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(phase);
    /** \endcode */

    /**
     * - Create OpenVX graph.
     *
     * \code
     */
    graph = vxCreateGraph(context);
    /** \endcode */
    i = 0;

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Sobel 3x3 node with in_image input
     * and grad_x and grad_y output.
     *
     * \code
     */
    node[i] = vxSobel3x3Node(graph, in_image, grad_x, grad_y);
    /** \endcode */

    vxSetReferenceName((vx_reference)node[i], "SOBEL3x3");

    /**
     * - Set node target CPU.
     *
     * Sets target CPU for node[i] to DSP1
     * \code
     */
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    /** \endcode */
    i++;

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Magnitude node with grad_x and grad_y input
     * and magnitude output.
     *
     * \code
     */
    node[i] = vxMagnitudeNode(graph, grad_x, grad_y, magnitude);
    /** \endcode */

    vxSetReferenceName((vx_reference)node[i], "MAGNITUDE");

    /**
     * - Set node target CPU.
     *
     * Sets target CPU for node[i] to DSP1
     * \code
     */
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    /** \endcode */
    i++;

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Phase node with grad_x and grad_y input
     * and phase output.
     *
     * \code
     */
    node[i] = vxPhaseNode(graph, grad_x, grad_y, phase);
    /** \endcode */

    vxSetReferenceName((vx_reference)node[i], "PHASE");
    /**
     * - Set node target CPU.
     *
     * Sets target CPU for node[i] to DSP2
     * \code
     */
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DSP2))
    {
        vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP2);
    }
    else /* DSP2 is not present on some platforms, so changing target to DSP1 */
    {
        vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    /** \endcode */
    i++;

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Convert Depth node with magnitude input
     * and magnitude_image output. The conversion policy is Saturate
     * and the shift value is shift.
     *
     * \code
     */
    node[i] = vxConvertDepthNode(graph,
                    magnitude, magnitude_image,
                    VX_CONVERT_POLICY_SATURATE,
                    shift);
    /** \endcode */

    vxSetReferenceName((vx_reference)node[i], "MAGNITUDE_IMAGE");

    /**
     * - Set node target CPU.
     *
     * Sets target CPU for node[i] to DSP1
     * \code
     */
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    /** \endcode */

    i++;

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Convert Depth node with grad_x input
     * and grad_x_image output. The conversion policy is Saturate
     * and the shift value is shift.
     *
     * \code
     */
    node[i] = vxConvertDepthNode(graph,
                    grad_x, grad_x_image,
                    VX_CONVERT_POLICY_SATURATE,
                    shift);
    /** \endcode */

    vxSetReferenceName((vx_reference)node[i], "GRAD_X_IMAGE");

    /**
     * - Set node target CPU.
     *
     * Sets target CPU for node[i] to DSP1
     * \code
     */
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    /** \endcode */

    i++;

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Convert Depth node with grad_y input
     * and grad_y_image output. The conversion policy is Saturate
     * and the shift value is shift.
     *
     * \code
     */
    node[i] = vxConvertDepthNode(graph,
                    grad_y, grad_y_image,
                    VX_CONVERT_POLICY_SATURATE,
                    shift);
    vxSetReferenceName((vx_reference)node[i], "GRAD_Y_IMAGE");

    /**
     * - Set node target CPU.
     *
     * Sets target CPU for node[i] to DSP2
     * \code
     */
    if (vx_true_e == tivxIsTargetEnabled(TIVX_TARGET_DSP2))
    {
        vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP2);
    }
    else /* DSP2 is not present on some platforms, so changing target to DSP1 */
    {
        vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    }
    /** \endcode */

    i++;

    /**
     * - Verify graph object.
     *
     * Verifies that all parameters of graph object are valid.
     *
     * \code
     */
    status = vxVerifyGraph(graph);
    /** \endcode */

    /** export graph to dot file, which can be coverted to jpg using dot tool
     * \code
     */
    tivxExportGraphToDot(graph, ".", "vx_tutorial_graph_image_gradients");
    /** \endcode */

    /**
     * - Show graph attributes.
     *
     * Follow the comments in show_graph_attributes() to see
     * how graph attributes are queried and displayed.
     * \code
     */
    show_graph_attributes(graph);
    /** \endcode */

    for(i=0; i<sizeof(node)/sizeof(node[0]); i++)
    {
        if(node[i])
        {
           /**
             * - Show node attributes.
             *
             * Follow the comments in show_node_attributes() to see
             * how node attributes are queried and displayed.
             * \code
             */
            show_node_attributes(node[i]);
            /** \endcode */
        }
    }

    if(status==VX_SUCCESS)
    {
        printf(" Executing graph ...\n");

        /**
         * - Schedule graph.
         *
         * Schedules graph for future execution. vxVerifyGraph must return VX_SUCCESS
         * before this function will pass.
         *
         * \code
         */
        vxScheduleGraph(graph);
        /** \endcode */
        /**
         * - Wait graph.
         *
         * Waits for graph to complete.
         *
         * \code
         */
        vxWaitGraph(graph);
        /** \endcode */

        printf(" Executing graph ... Done !!!\n");

       /**
         * - Show graph attributes.
         *
         * Follow the comments in show_graph_attributes() to see
         * how graph attributes are queried and displayed.
         * \code
         */
        show_graph_attributes(graph);
        /** \endcode */
        for(i=0; i<sizeof(node)/sizeof(node[0]); i++)
        {
            if(node[i])
            {
               /**
                 * - Show node attributes.
                 *
                 * Follow the comments in show_node_attributes() to see
                 * how node attributes are queried and displayed.
                 * \code
                 */
                show_node_attributes(node[i]);
                /** \endcode */
            }
        }

        printf(" Saving to file %s ...\n", PHASE_FILE_NAME);
        /**
         * - Save image object to bitmap file \ref OUT_FILE_NAME.
         *
         * Follow the comments in save_image_to_file() to see
         * how data in vx_image object is accessed to store pixel values from the image object to
         * BMP file \ref OUT_FILE_NAME
         * \code
         */
        save_image_to_file(PHASE_FILE_NAME, phase);
        /** \endcode */

        printf(" Saving to file %s ...\n", MAGNITUDE_FILE_NAME);
        /**
         * - Save image object to bitmap file \ref OUT_FILE_NAME.
         *
         * Follow the comments in save_image_to_file() to see
         * how data in vx_image object is accessed to store pixel values from the image object to
         * BMP file \ref OUT_FILE_NAME
         * \code
         */
        save_image_to_file(MAGNITUDE_FILE_NAME, magnitude_image);
        /** \endcode */

        printf(" Saving to file %s ...\n", GRAD_X_FILE_NAME);
        /**
         * - Save image object to bitmap file \ref OUT_FILE_NAME.
         *
         * Follow the comments in save_image_to_file() to see
         * how data in vx_image object is accessed to store pixel values from the image object to
         * BMP file \ref OUT_FILE_NAME
         * \code
         */
        save_image_to_file(GRAD_X_FILE_NAME, grad_x_image);
        /** \endcode */

        printf(" Saving to file %s ...\n", GRAD_Y_FILE_NAME);
        /**
         * - Save image object to bitmap file \ref OUT_FILE_NAME.
         *
         * Follow the comments in save_image_to_file() to see
         * how data in vx_image object is accessed to store pixel values from the image object to
         * BMP file \ref OUT_FILE_NAME
         * \code
         */
        save_image_to_file(GRAD_Y_FILE_NAME, grad_y_image);
        /** \endcode */
    }

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&in_image);
    vxReleaseImage(&grad_x);
    vxReleaseImage(&grad_y);
    vxReleaseImage(&grad_x_image);
    vxReleaseImage(&grad_y_image);
    vxReleaseImage(&phase);
    vxReleaseImage(&magnitude);
    vxReleaseImage(&magnitude_image);
    /** \endcode */

    /**
     * - Release scalar object.
     *
     * Since we are done with using this scalar object, release it
     * \code
     */
    vxReleaseScalar(&shift);
    /** \endcode */

    for(i=0; i<sizeof(node)/sizeof(node[0]); i++)
    {
        if(node[i])
        {
           /**
             * - Release node object.
             *
             * Since we are done with using this node object, release it
             * \code
             */
            vxReleaseNode(&node[i]);
            /** \endcode */
        }
    }

    /**
     * - Release graph object.
     *
     * Since we are done with using this graph object, release it
     * \code
     */
    vxReleaseGraph(&graph);
    /** \endcode */

    /**
     * - Release context object.
     *
     * Since we are done using OpenVX context, release it.
     * No further OpenVX API calls should be done, until a context is again created using
     * vxCreateContext()
     * \code
     */
    vxReleaseContext(&context);
    /** \endcode */

    printf(" vx_tutorial_graph_image_gradients: Tutorial Done !!! \n");
    printf(" \n");
}
