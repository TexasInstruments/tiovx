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
 * \file vx_tutorial_image_color_convert.c Convert image from RGB format to NV12 format
 * then extract luma channel
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context, OpenVX image data object and OpenVX virtual image data object
 * - How to read a BMP file and load the pixel values into the image data object
 * - How to query the image data object for attributes like width, height
 * - How to create OpenVX node and associate it with previously created graph
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
 * Follow the comments in the function vx_tutorial_image_color_convert()
 * to understand this tutorial
 *
 * As part of this tutorial, we create few utility functions as listed below.
 * These functions will be used in subsequent tutorials to display node and graph attributes.
 *  <TABLE frame="box" rules="all" cellspacing="0" width="50%" border="1" cellpadding="3">
 *      <TR bgcolor="lightgrey">
 *          <TH> Utility function </TH>
 *          <TH> Description </TH>
 *      </TR>
 *      <TR>
 *          <TD> show_graph_attributes() </TD>
 *          <TD> Displays attributes of a previously created graph.
 *              <b>NOTE:</b> This function though listed in this tutorial is used in later tutorials
 *           </TD>
 *      </TR>
 *      <TR>
 *          <TD> show_node_attributes() </TD>
 *          <TD> Displays attributes of a previously created node.
 *              <b>NOTE:</b> This function though listed in this tutorial is used in later tutorials
 *           </TD>
 *      </TR>
 *  </TABLE>
 *
 */

#include <stdio.h>
#include <VX/vx.h>
#include <TI/tivx.h>
#include <utility.h>

/** \brief Input file name */
#define IN_FILE_NAME       "${VX_TEST_DATA_PATH}/colors.bmp"

/** \brief Output file name */
#define OUT_FILE_NAME      "${VX_TEST_DATA_PATH}/vx_tutorial_image_color_convert_out.bmp"

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_image_color_convert()
{
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image rgb_image = NULL;
    vx_image nv12_image = NULL;
    vx_image y_image = NULL;
    vx_node node0 = NULL, node1 = NULL;
    vx_graph graph = NULL;
    /** \endcode */
    vx_uint32 width, height;
    vx_status status;

    printf(" vx_tutorial_image_color_convert: Tutorial Started !!! \n");

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
     * - Create OpenVX graph.
     *
     * \code
     */
    graph = vxCreateGraph(context);
    /** \endcode */
    vxSetReferenceName((vx_reference)graph, "MY_GRAPH");

    /**
     * - Create image object.
     *
     * Follow the comments in tivx_utils_create_vximage_from_bmpfile() to see
     * how a vx_image object is created and filled with RGB data from BMP file \ref IN_FILE_NAME
     * \code
     */
    rgb_image = tivx_utils_create_vximage_from_bmpfile(context, IN_FILE_NAME, (vx_bool)vx_false_e);
    /** \endcode */

    vxSetReferenceName((vx_reference)rgb_image, "RGB_IMAGE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(rgb_image);
    /** \endcode */

    vxQueryImage(rgb_image, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(rgb_image, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    /**
     * - Create OpenVX virtual image object.
     *
     * Creates an OpenVX virtual image object of 'width' x 'height' and having
     * data format 'df'.  Virtual images are allowed to have a value of 0 for
     * width and height.
     *
     * \code
     */
    nv12_image = vxCreateVirtualImage(graph, 0, 0, (vx_df_image)VX_DF_IMAGE_NV12);
    /** \endcode */
    vxSetReferenceName((vx_reference)nv12_image, "NV12_IMAGE");
    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(nv12_image);
    /** \endcode */

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    y_image = vxCreateImage(context, width, height, (vx_df_image)VX_DF_IMAGE_U8);
    /** \endcode */
    vxSetReferenceName((vx_reference)y_image, "Y_IMAGE");
    show_image_attributes(y_image);

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Color Convert node with rgb_image input
     * and nv12_image output.
     *
     * \code
     */
    node0 = vxColorConvertNode(graph, rgb_image, nv12_image);
    /** \endcode */
    vxSetReferenceName((vx_reference)node0, "COLOR_CONVERT");

    /**
     * - Create OpenVX node object.
     *
     * Creates an OpenVX Channel Combine node with nv12_image input
     * and y_image output. It extracts the channel VX_CHANNEL_Y from
     * input image.
     *
     * \code
     */
    node1 = vxChannelExtractNode(graph, nv12_image, (vx_enum)VX_CHANNEL_Y, y_image);
    /** \endcode */
    vxSetReferenceName((vx_reference)node1, "CHANNEL_EXTRACT");

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
    tivxExportGraphToDot(graph, ".", "vx_tutorial_image_color_convert");
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

    /**
     * - Show node attributes.
     *
     * Follow the comments in show_node_attributes() to see
     * how node attributes are queried and displayed.
     * \code
     */
    show_node_attributes(node0);
    show_node_attributes(node1);
    /** \endcode */

    if(status==(vx_status)VX_SUCCESS)
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

        /**
         * - Show node attributes.
         *
         * Follow the comments in show_node_attributes() to see
         * how node attributes are queried and displayed.
         * \code
         */
        show_node_attributes(node0);
        show_node_attributes(node1);
        /** \endcode */

        printf(" Saving to file %s ...\n", OUT_FILE_NAME);

        /**
         * - Save image object to bitmap file \ref OUT_FILE_NAME.
         *
         * Follow the comments in tivx_utils_save_vximage_to_bmpfile() to see
         * how data in vx_image object is accessed to store pixel values from the image object to
         * BMP file \ref OUT_FILE_NAME
         * \code
         */
        tivx_utils_save_vximage_to_bmpfile(OUT_FILE_NAME, y_image);
        /** \endcode */
    }

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&rgb_image);
    vxReleaseImage(&nv12_image);
    vxReleaseImage(&y_image);
    /** \endcode */

    /**
     * - Release node object.
     *
     * Since we are done with using this node object, release it
     * \code
     */
    vxReleaseNode(&node0);
    vxReleaseNode(&node1);
    /** \endcode */

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

    printf(" vx_tutorial_image_color_convert: Tutorial Done !!! \n");
    printf(" \n");
}

#define MAX_ATTRIBUTE_NAME (32u)

/**
 * \brief Show attributes of previously created graph
 *
 * This function queries the vx_graph graph for its number of nodes, number of parameters,
 * state, performance, reference name and reference count then prints this information.
 *
 * \param graph [in] Previouly created graph object
 *
 */
void show_graph_attributes(vx_graph graph)
{
    vx_uint32 num_nodes=0, num_params=0, ref_count=0;
    vx_enum state=0;
    vx_perf_t perf={0};
    vx_char *ref_name=NULL;
    char ref_name_invalid[MAX_ATTRIBUTE_NAME];
    char state_name[MAX_ATTRIBUTE_NAME];

    /** - Query graph attributes.
     *
     *  Queries graph for number of nodes, number of parameters, state and performance
     *
     * \code
     */
    vxQueryGraph(graph, (vx_enum)VX_GRAPH_NUMNODES, &num_nodes, sizeof(vx_uint32));
    vxQueryGraph(graph, (vx_enum)VX_GRAPH_NUMPARAMETERS, &num_params, sizeof(vx_uint32));
    vxQueryGraph(graph, (vx_enum)VX_GRAPH_STATE, &state, sizeof(vx_enum));
    vxQueryGraph(graph, (vx_enum)VX_GRAPH_PERFORMANCE, &perf, sizeof(vx_perf_t));
    /** \endcode */

    vxQueryReference((vx_reference)graph, (vx_enum)VX_REFERENCE_NAME, &ref_name, sizeof(vx_char*));
    vxQueryReference((vx_reference)graph, (vx_enum)VX_REFERENCE_COUNT, &ref_count, sizeof(vx_uint32));

    switch(state)
    {
        case (vx_enum)VX_GRAPH_STATE_UNVERIFIED:
            strncpy(state_name, "VX_GRAPH_STATE_UNVERIFIED", MAX_ATTRIBUTE_NAME);
            break;
        case (vx_enum)VX_GRAPH_STATE_VERIFIED:
            strncpy(state_name, "VX_GRAPH_STATE_VERIFIED", MAX_ATTRIBUTE_NAME);
            break;
        case (vx_enum)VX_GRAPH_STATE_RUNNING:
            strncpy(state_name, "VX_GRAPH_STATE_RUNNING", MAX_ATTRIBUTE_NAME);
            break;
        case (vx_enum)VX_GRAPH_STATE_ABANDONED:
            strncpy(state_name, "VX_GRAPH_STATE_ABANDONED", MAX_ATTRIBUTE_NAME);
            break;
        case (vx_enum)VX_GRAPH_STATE_COMPLETED:
            strncpy(state_name, "VX_GRAPH_STATE_COMPLETED", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(state_name, "VX_GRAPH_STATE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    if(ref_name==NULL)
    {
        strncpy(ref_name_invalid, "INVALID_REF_NAME", MAX_ATTRIBUTE_NAME);
        ref_name = &ref_name_invalid[0];
    }

    printf(" VX_TYPE_GRAPH: %s, %d nodes, %s, avg perf %9.6fs, %d parameters, %d refs\n",
        ref_name,
        num_nodes,
        state_name,
        perf.avg/1000000000.0,
        num_params,
        ref_count
        );
}

/**
 * \brief Show attributes of previously created node
 *
 * This function queries the vx_node node for its status, number of parameters,
 * performance, reference name and reference count then prints this information.
 *
 * \param node [in] Previouly created graph object
 *
 */
void show_node_attributes(vx_node node)
{
    vx_uint32 num_params=0, ref_count=0;
    vx_status status=(vx_status)VX_FAILURE;
    vx_perf_t perf={0};
    vx_char *ref_name=NULL;
    char ref_name_invalid[MAX_ATTRIBUTE_NAME];
    char status_name[MAX_ATTRIBUTE_NAME];

    /** - Query node attributes.
     *
     *  Queries node for number of status, number of parameters and performance
     *
     * \code
     */
    vxQueryNode(node, (vx_enum)VX_NODE_STATUS, &status, sizeof(vx_status));
    vxQueryNode(node, (vx_enum)VX_NODE_PARAMETERS, &num_params, sizeof(vx_uint32));
    vxQueryNode(node, (vx_enum)VX_NODE_PERFORMANCE, &perf, sizeof(vx_perf_t));
    /** \endcode */

    vxQueryReference((vx_reference)node, (vx_enum)VX_REFERENCE_NAME, &ref_name, sizeof(vx_char*));
    vxQueryReference((vx_reference)node, (vx_enum)VX_REFERENCE_COUNT, &ref_count, sizeof(vx_uint32));

    switch(status)
    {
        case (vx_status)VX_SUCCESS:
            strncpy(status_name, "VX_SUCCESS", MAX_ATTRIBUTE_NAME);
            break;
        case (vx_status)VX_FAILURE:
            strncpy(status_name, "VX_FAILURE", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(status_name, "VX_FAILURE_OTHER", MAX_ATTRIBUTE_NAME);
            break;
    }

    if(ref_name==NULL)
    {
        strncpy(ref_name_invalid, "INVALID_REF_NAME", MAX_ATTRIBUTE_NAME);
        ref_name = &ref_name_invalid[0];
    }

    printf(" VX_TYPE_NODE: %s, %d params, avg perf %9.6fs, %s, %d refs\n",
        ref_name,
        num_params,
        perf.avg/1000000000.0,
        status_name,
        ref_count
        );
}
