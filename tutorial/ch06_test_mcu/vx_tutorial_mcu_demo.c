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

#include <stdio.h>
#include <VX/vx.h>
#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <utility.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>
#include "vx_tutorial_mcu_demo.h"

#if defined(SOC_AM62A)
void vx_tutorial_mcu_demo(vx_bool use_mcu1_core)
#else
void vx_tutorial_mcu_demo(vx_bool use_mcu3_core)
#endif
{
    vx_graph graph;
    vx_context context;
    vx_uint8  scalar_val = 0;
    vx_scalar scalar, scalar_out;    
    vx_uint8 output = 0;
    vx_uint32 num_streams = 0, golden_output = 0;
    vx_node node0, node1;
    vx_status status;

    printf(" vx_tutorial_mcu_demo: Tutorial Started !!! \n");
    /**
     * - Create OpenVX context.
     *
     * This MUST be done first before any OpenVX API call.
     * The context that is returned is used as input for subsequent OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    tivxTestKernelsLoadKernels(context);

    /**
     * - Create OpenVX graph.
     *
     * \code
     */
    graph = vxCreateGraph(context);
    /** \endcode */

    scalar = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val);
    scalar_out = vxCreateScalar(context, VX_TYPE_UINT8, &scalar_val);

    /**
     * - Create OpenVX node object.
     *
     * \code
     */
    node0 = tivxScalarSourceNode(graph, scalar);
    vxSetReferenceName((vx_reference)node0, "Scalar_Source 1");
    #if defined(SOC_AM62A)
    if (vx_true_e == use_mcu1_core)
    {
        vxSetNodeTarget(node0, VX_TARGET_STRING, TIVX_TARGET_MCU1_0);
    }
    #else
    if (vx_true_e == use_mcu3_core)
    {
        if(tivxIsTargetEnabled(TIVX_TARGET_MCU3_0))
        {
            vxSetNodeTarget(node0, VX_TARGET_STRING, TIVX_TARGET_MCU3_0);
        }
        else
        {
            vxSetNodeTarget(node0, VX_TARGET_STRING, TIVX_TARGET_MCU3_1);
        }
    }
    #endif
    #if defined(SOC_J784S4)
    else
    {
        if(tivxIsTargetEnabled(TIVX_TARGET_MCU4_0))
        {
            vxSetNodeTarget(node0, VX_TARGET_STRING, TIVX_TARGET_MCU4_0);
        }
        else
        {
            vxSetNodeTarget(node0, VX_TARGET_STRING, TIVX_TARGET_MCU4_1);
        }
    }
    #endif

    node1 = tivxScalarIntermediateNode(graph, scalar, scalar_out);
    vxSetReferenceName((vx_reference)node1, "Scalar_Source 2");
    #if defined(SOC_AM62A)
    if (vx_true_e == use_mcu1_core)
    {
        vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_MCU1_0);
    }
    #else
    if (vx_true_e == use_mcu3_core)
    {
        if(tivxIsTargetEnabled(TIVX_TARGET_MCU3_0))
        {
            vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_MCU3_0);
        }
        else
        {
            vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_MCU3_1);
        }
    }
    #endif
    #if defined(SOC_J784S4)
    else
    {
        if(tivxIsTargetEnabled(TIVX_TARGET_MCU4_0))
        {
            vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_MCU4_0);
        }
        else
        {
            vxSetNodeTarget(node1, VX_TARGET_STRING, TIVX_TARGET_MCU4_1);
        }
    }
    #endif
    /** \endcode */


    status = vxEnableGraphStreaming(graph, node0);
    /**
     * - Verify graph object.
     *
     * Verifies that all parameters of graph object are valid.
     *
     * \code
     */
    status = vxVerifyGraph(graph);
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

    printf("Input scalar value = %d\n", scalar_val);

    if(status==(vx_status)VX_SUCCESS)
    {
        printf(" Executing graph ...\n");

        vxStartGraphStreaming(graph);

        tivxTaskWaitMsecs(50);

        vxStopGraphStreaming(graph);

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

         vxQueryGraph(graph, TIVX_GRAPH_STREAM_EXECUTIONS, &num_streams, sizeof(num_streams));

        if (num_streams != 0)
        {
            golden_output = num_streams % 256;
            vxCopyScalar(scalar_out, &output, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
            printf("Output scalar value = %d\n", output);            
            if(output!=golden_output)
            {
                printf("ERROR: %d != %d\n", output, golden_output);
            }
            else
            {
                printf("RUN SUCCESS!!!\n");
            }
        }
    }

    /**
     * - Release object.
     *
     * Since we are done with using the objects, release it
     * \code
     */
    vxReleaseScalar(&scalar);
    vxReleaseScalar(&scalar_out);
    vxReleaseNode(&node0);
    vxReleaseNode(&node1);

    /**
     * - Release graph object.
     *
     * Since we are done with using this graph object, release it
     * \code
     */
    vxReleaseGraph(&graph);
    /** \endcode */

    tivxTestKernelsUnLoadKernels(context);

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

    printf(" vx_tutorial_mcu_demo: Tutorial Done !!! \n");
    printf(" \n");
}