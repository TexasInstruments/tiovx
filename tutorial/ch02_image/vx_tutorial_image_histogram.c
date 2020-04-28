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
 * \file vx_tutorial_image_histogram.c Create a distribution from an image then convert the
 * to an image.
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context, OpenVX image data object and OpenVX distribution data object
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
 * To include TI OpenVX extensions include below file
 * \code
 * #include <TI/tivx.h>
 * \endcode
 *
 * Follow the comments in the function vx_tutorial_image_histogram()
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
 *          <TD> convert_distribution_to_image() </TD>
 *          <TD> Converts a distribution data object to an image data object </TD>
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
#define OUT_FILE_NAME      "${VX_TEST_DATA_PATH}/vx_tutorial_image_histogram_out.bmp"


vx_image convert_distribution_to_image(vx_distribution distribution,
            uint32_t width, uint32_t height);

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_image_histogram()
{
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image in_image = NULL, out_image = NULL;
    vx_distribution histogram = NULL;
    /** \endcode */
    uint32_t num_bins = 256;
    uint32_t histogram_image_width  = 256;
    uint32_t histogram_image_height = 128;
    vx_node node0 = NULL;
    vx_graph graph = NULL;
    vx_status status;

    printf(" vx_tutorial_image_histogram: Tutorial Started !!! \n");

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
    in_image = tivx_utils_create_vximage_from_bmpfile(context, IN_FILE_NAME, (vx_bool)vx_true_e);
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

    /**
     * - Create OpenVX distribution.
     *
     * Creates distribution with parameters num_bins (256), offset (0) and range (256)
     * \code
     */
    histogram = vxCreateDistribution(context, num_bins, 0, 256);
    /** \endcode */
    vxSetReferenceName((vx_reference)histogram, "HISTOGRAM");

    {
        vx_reference refs[] = {(vx_reference)in_image, (vx_reference)histogram};

       /**
         * Below is equivalent of doing
         * node0 = vxHistogramNode(graph, in_image, histogram);
         * \code
         */
        node0 = tivxCreateNodeByKernelEnum(graph,
                    (vx_enum)VX_KERNEL_HISTOGRAM,
                    refs, sizeof(refs)/sizeof(refs[0])
                    );
        /** \endcode */
        vxSetReferenceName((vx_reference)node0, "HISTOGRAM");
    }

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
    tivxExportGraphToDot(graph, ".", "vx_tutorial_image_histogram");
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
        /** \endcode */

        out_image = convert_distribution_to_image(histogram,
                        histogram_image_width,
                        histogram_image_height);

        vxSetReferenceName((vx_reference)out_image, "HISTOGRAM_IMAGE");
        /**
         * - Show image attributes.
         *
         * Follow the comments in show_image_attributes() to see
         * how image attributes are queried and displayed.
         * \code
         */
        show_image_attributes(out_image);
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
        tivx_utils_save_vximage_to_bmpfile(OUT_FILE_NAME, out_image);
        /** \endcode */

        /**
         * - Release image object.
         *
         * Since we are done with using this image object, release it
         * \code
         */
        vxReleaseImage(&out_image);
        /** \endcode */
    }

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&in_image);
    /** \endcode */

    /**
     * - Release distribution object.
     *
     * Since we are done with using this distribution object, release it
     * \code
     */
    vxReleaseDistribution(&histogram);
    /** \endcode */

    /**
     * - Release node object.
     *
     * Since we are done with using this node object, release it
     * \code
     */
    vxReleaseNode(&node0);
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

    printf(" vx_tutorial_image_histogram: Tutorial Done !!! \n");
    printf(" \n");
}

#define MAX_BINS    (256u)

/**
 * \brief Convert distribution given as input to image
 *
 * \param distribution [in] Distribution to be converted to image
 * \param width [in] Width of distribution
 * \param height [in] Height of distribution
 *
 * \return Image data object converted from distribution. \n
 */
vx_image convert_distribution_to_image(vx_distribution distribution,
            uint32_t width, uint32_t height)
{
    vx_image image;
    vx_context context;
    vx_status status;

    context = vxGetContext((vx_reference)distribution);

    /**
     * - Create OpenVX image object.
     *
     * Creates an OpenVX image object of 'width' x 'height' and having
     * data format 'df'.
     * \code
     */
    image = vxCreateImage(context, width, height, (vx_df_image)VX_DF_IMAGE_U8);
    /** \endcode */

    if(vxGetStatus((vx_reference)image)==(vx_status)VX_SUCCESS)
    {
        vx_size num_bins;

        /** - Query distribution attributes.
         *
         *  Queries distribution for number of bins
         *
         * \code
         */
        status = vxQueryDistribution(distribution, (vx_enum)VX_DISTRIBUTION_BINS, &num_bins, sizeof(vx_size));
        /** \endcode */

        if(status == (vx_status)VX_SUCCESS && num_bins <=  MAX_BINS)
        {
            uint32_t histogram_data[MAX_BINS] = {0};
            uint32_t i, max;

            /** - Copy distribution.
             *
             *  Copy distribution to array
             *
             * \code
             */
            vxCopyDistribution(distribution,
                (void**)&histogram_data[0],
                (vx_enum)VX_READ_ONLY,
                (vx_enum)VX_MEMORY_TYPE_HOST);
            /** \endcode */

            /* normalize bins */
            max = 0;
            for(i=0; i<num_bins; i++)
            {
                if(histogram_data[i] > max )
                    max = histogram_data[i];
            }
            /* scale to image height */
            for(i=0; i<num_bins; i++)
            {
                histogram_data[i] = (uint32_t)(((float)histogram_data[i]/max)*height + 0.5);
                if(histogram_data[i]>height)
                    histogram_data[i] = height;
            }

            {
                uint8_t *data_ptr = NULL;
                vx_rectangle_t rect = { 0, 0, width, height};
                vx_map_id map_id;
                vx_imagepatch_addressing_t image_addr;

                /** - Map image patch.
                 *
                 *  Allows direct access to rectangular patch of image object plane
                 *
                 * \code
                 */
                status = vxMapImagePatch(image,
                            &rect,
                            0,
                            &map_id,
                            &image_addr,
                            (void**)&data_ptr,
                            (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_NOGAP_X);
                /** \endcode */


                if(status==(vx_status)VX_SUCCESS && data_ptr!=NULL)
                {
                    uint32_t i, j, k, bin_width;
                    uint8_t *addr0 = NULL, *addr1 = NULL;

                    bin_width = width/num_bins;

                    for(i=0; i<num_bins; i++)
                    {
                        addr0 = data_ptr + i*bin_width;

                        for(j=0; j<height-histogram_data[i]; j++)
                        {
                            addr1 = addr0 + j*image_addr.stride_y;
                            for(k=0; k<bin_width; k++)
                            {
                                addr1[k] = 0xFF;
                            }
                        }

                        addr0 = data_ptr + j*image_addr.stride_y + i*bin_width;

                        for(j=0; j<histogram_data[i]; j++)
                        {
                            addr1 = addr0 + j*image_addr.stride_y;
                            for(k=0; k<bin_width; k++)
                            {
                                addr1[k] = 0x0;
                            }
                        }
                    }
                    /** - Unmap image patch.
                      *
                      * \code
                      */
                    vxUnmapImagePatch(image, map_id);
                    /** \endcode */
                }
            }
        }
        else
        {
            /* more bins than allocated memory */
            /**
              * - Release image object.
              *
              * Since we are done with using this image object, release it
              * \code
              */
            vxReleaseImage(&image);
            /** \endcode */
        }
    }

    return image;
}
