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
 * \file vx_tutorial_graph_pipeline_two_nodes.c Pipeline a graph with two nodes
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX graph with two nodes and pipeline across two different target CPUs
 *
 * To include OpenVX graph pipeline include below files
 * \code
 * #include <VX/vx_khr_pipelining.h>
 * #include <TI/tivx.h>
 * \endcode
 *
 * Follow the comments in the function vx_tutorial_graph_pipeline_two_nodes()
 * to understand this tutorial
 *
 */

#include <stdio.h>
#include <VX/vx_khr_pipelining.h>
#include <TI/tivx.h>

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
static void add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);

    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

static void print_performance(vx_perf_t performance, uint32_t numPixels, const char* testName)
{
    printf("[ %c%c ] Execution time for %9d pixels (avg = %4.6f ms, min = %4.6f ms, max = %4.6f ms)\n",
        testName[0], testName[1],
        numPixels,
        performance.avg/1000000.0,
        performance.min/1000000.0,
        performance.max/1000000.0
        );
}

static void print_graph_pipeline_performance(vx_graph graph,
            vx_node nodes[], uint32_t num_nodes,
            uint64_t exe_time, uint32_t loop_cnt, uint32_t numPixels)
{
    vx_perf_t perf_ref;
    char ref_name[8];
    uint32_t i;
    uint64_t avg_exe_time;

    avg_exe_time = exe_time / loop_cnt;

    for(i=0; i<num_nodes; i++)
    {
         vxQueryNode(nodes[i], (vx_enum)VX_NODE_PERFORMANCE, &perf_ref, sizeof(perf_ref));
         snprintf(ref_name, 8, "N%d ", i);
         print_performance(perf_ref, numPixels, ref_name);
    }

    printf("[ SYS ] Execution time (avg = %4d.%03d ms, sum = %4d.%03d ms, num = %d)\n",
        (uint32_t)(avg_exe_time/1000u), (uint32_t)(avg_exe_time%1000u),
        (uint32_t)(exe_time/1000u), (uint32_t)(exe_time%1000u),
        loop_cnt
        );
}

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_graph_pipeline_two_nodes()
{
/* max number of buffers to allocate for input and output */
#define MAX_NUM_BUF   (2u)

    /**
     * - Define objects that we wish to create in the OpenVX application.
     * - in_img[] are the input images that we will pass to the graph
     * - out_img[] are the output images we will get back from the graph
     * - tmp is the intermediate image, we will not access this within the application
     * - n0 and n1 are the two NOT nodes running on two different target CPUs
     * - graph_parameters_queue_params_list[] are the graph parameters info that
     *   we will pipeline and associate with the graph
     *
     * \code
     */
    vx_context context;
    vx_image in_img[MAX_NUM_BUF], tmp, out_img[MAX_NUM_BUF];
    vx_node n0, n1;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[2];
    vx_graph graph;
    uint32_t width, height, num_buf, pipeline_depth, buf_id, loop_id, loop_cnt, exe_time;
    /** \endcode */

    printf(" vx_tutorial_graph_pipeline_two_nodes: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context.
     *
     * This MUST be done first before any OpenVX API call.
     * The context that is returned is used as input for subsequent OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    width = 64;
    height = 64;
    num_buf = MAX_NUM_BUF;
    pipeline_depth = MAX_NUM_BUF;
    loop_cnt = 10 - num_buf; /* runs the graph 10 times */

    graph = vxCreateGraph(context);

    /**
     * - Allocate Input and Output images, multiple refs created to allow pipelining of graph.
     * - Allocate intermediate image, application creates a single intermediate vx_image.
     *   It is converted to multiple vx_images internally using tivxSetNodeParameterNumBufByIndex()
     *   later.
     * \code
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        in_img[buf_id]    = vxCreateImage(context, width, height, (vx_df_image)VX_DF_IMAGE_U8);
        out_img[buf_id]   = vxCreateImage(context, width, height, (vx_df_image)VX_DF_IMAGE_U8);
    }
    tmp    = vxCreateImage(context, width, height, (vx_df_image)VX_DF_IMAGE_U8);
    /** \endcode */

    /**
     * - Construct the graph and set node targets such that each runs
     *    on a different CPU target
     * \code
     */
    n0    = vxNotNode(graph, in_img[0], tmp);
    n1    = vxNotNode(graph, tmp, out_img[0]);

    vxSetNodeTarget(n0, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_DSP1);
    #if defined(SOC_AM62A)
    vxSetNodeTarget(n1, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_DSP1);
    #else
    vxSetNodeTarget(n1, (vx_enum)VX_TARGET_STRING, TIVX_TARGET_DSP2);
    #endif
    /** \endcode */

    /**
     * - The position where input and output images are made as graph
     *   parameters. This is because for pipelining, one can only enqueue/dequeue
     *   into a graph parameter
     * \code 
     */
    /* input @ n0 index 0, becomes graph parameter 0 */
    add_graph_parameter_by_node_index(graph, n0, 0);
    /* output @ n1 index 1, becomes graph parameter 1 */
    add_graph_parameter_by_node_index(graph, n1, 1);
    /** \endcode */

    /**
     * - Set graph scehdule policy to make it pipelined.
     *   - This is done by providing a list of graph parameters that user wants to
     *     enqueue or dequeue. This is the input and output graph parameters in this case.
     *   - The number of buffers that can be enqueued before its internal queue becomes
     *     full is also specified via graph_parameters_queue_params_list[0].refs_list_size.
     *   - The list of buffers that could be every enqueued is specified via,
     *     graph_parameters_queue_params_list[0].refs_list
     * \code
     */
    /* set graph schedule config such that graph parameter @ index 0 and 1 are enqueuable */
    graph_parameters_queue_params_list[0].graph_parameter_index = 0;
    graph_parameters_queue_params_list[0].refs_list_size = num_buf;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&in_img[0];

    graph_parameters_queue_params_list[1].graph_parameter_index = 1;
    graph_parameters_queue_params_list[1].refs_list_size = num_buf;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&out_img[0];

    /* Schedule mode auto is used, here we dont need to call vxScheduleGraph
     * Graph gets scheduled automatically as refs are enqueued to it
     */
    vxSetGraphScheduleConfig(graph,
            (vx_enum)VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            2,
            graph_parameters_queue_params_list
            );
    /** \endcode */

    /**
     * - Set graph pipeline depth. This has to be done explicitly. Default is 1.
     * - Set number of intermediate buffers. This has to be done explicitly. Default is 1.
     * \code
     */
    /* explicitly set graph pipeline depth */
    tivxSetGraphPipelineDepth(graph, pipeline_depth);

    /* make the 'tmp' reference which is output of n0 @ index 1 a "multi-buffer" */
    tivxSetNodeParameterNumBufByIndex(n0, 1, num_buf);
    /** \endcode */

    /**
     * - Verify the graph
     * - Optionally export the graph as graphviz dot graph to visualize the graph and
     *   its pipelined structure
     * - Optionally enable real-time logging to get a waveform like trace of graph execution
     * \code
     */
    vxVerifyGraph(graph);

    #if 0
    tivxExportGraphToDot(graph, ".", "vx_tutorial_graph_pipeline_two_nodes");
    #endif
    /** \endcode */

    exe_time = tivxPlatformGetTimeInUsecs();

    /**
     * - Enqueue input and output references,
     * - input and output can be enqueued in any order
     * - The moment something is enqueued at graph parameter 0, the graph execution begins
     * \code
     */
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        /* reset output */
        vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&out_img[buf_id], 1);
        /* load input */
        vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&in_img[buf_id], 1);
    }
    /** \endcode */

    buf_id = 0;

    /**
     * - Wait for graph instances to complete,
     * - Compare output and recycle data buffers, schedule again
     * \code
     */
    for(loop_id=0; loop_id<(loop_cnt+num_buf); loop_id++)
    {
        vx_image cur_out_img, cur_in_img;
        uint32_t num_refs;

        /* Get output reference, waits until a reference is available */
        vxGraphParameterDequeueDoneRef(graph, 1, (vx_reference*)&cur_out_img, 1, &num_refs);

        /* Get consumed input reference, waits until a reference is available
         */
        vxGraphParameterDequeueDoneRef(graph, 0, (vx_reference*)&cur_in_img, 1, &num_refs);

        /* A graph execution completed, since we dequeued both input and output refs */

        /* use output */

        buf_id = (buf_id+1)%num_buf;

        /* recycles dequeued input and output refs 'loop_cnt' times */
        if(loop_id<loop_cnt)
        {
            /* input and output can be enqueued in any order */
            vxGraphParameterEnqueueReadyRef(graph, 1, (vx_reference*)&cur_out_img, 1);
            vxGraphParameterEnqueueReadyRef(graph, 0, (vx_reference*)&cur_in_img, 1);
        }
    }
    /* ensure all graph processing is complete */
    vxWaitGraph(graph);
    /** \endcode */

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    {
        vx_node nodes[] = { n0, n1 };

        print_graph_pipeline_performance(graph, nodes, 2, exe_time, loop_cnt+num_buf, width*height);
    }

    /**
     * - Release all perviously allocated resources
     * \code
     */
    vxReleaseNode(&n0);
    vxReleaseNode(&n1);
    for(buf_id=0; buf_id<num_buf; buf_id++)
    {
        vxReleaseImage(&in_img[buf_id]);
        vxReleaseImage(&out_img[buf_id]);
    }
    vxReleaseImage(&tmp);
    vxReleaseGraph(&graph);
    vxReleaseContext(&context);
    /** \endcode */

    printf(" vx_tutorial_graph_pipeline_two_nodes: Tutorial Done !!! \n");
    printf(" \n");
}
