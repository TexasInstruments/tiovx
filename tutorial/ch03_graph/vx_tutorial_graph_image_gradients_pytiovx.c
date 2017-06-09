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
#include <utility.h>
#include <ch03_graph/vx_tutorial_graph_image_gradients_pytiovx_uc.h>

#define IN_FILE_NAME         "colors.bmp"
#define PHASE_FILE_NAME      "vx_tutorial_graph_image_gradients_pytiovx_phase_out.bmp"
#define MAGNITUDE_FILE_NAME  "vx_tutorial_graph_image_gradients_pytiovx_magnitude_out.bmp"
#define GRAD_X_FILE_NAME     "vx_tutorial_graph_image_gradients_pytiovx_grad_x_out.bmp"
#define GRAD_Y_FILE_NAME     "vx_tutorial_graph_image_gradients_pytiovx_grad_y_out.bmp"

void vx_tutorial_graph_image_gradients_pytiovx()
{
    vx_status status;
    vx_tutorial_graph_image_gradients_pytiovx_uc_t uc;

    printf(" vx_tutorial_graph_image_gradients_pytiovx: Tutorial Started !!! \n");

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    vx_tutorial_graph_image_gradients_pytiovx_uc_create(&uc);

    status = load_image_from_file(uc.input, IN_FILE_NAME, vx_true_e);

    show_image_attributes(uc.input);
    show_image_attributes(uc.grad_x);
    show_image_attributes(uc.grad_y);
    show_image_attributes(uc.magnitude);
    show_image_attributes(uc.phase);
    show_image_attributes(uc.grad_x_img);
    show_image_attributes(uc.grad_x_img);
    show_image_attributes(uc.grad_x_img);

    vx_tutorial_graph_image_gradients_pytiovx_uc_verify(&uc);
    show_graph_attributes(uc.graph_0);

    if(status==VX_SUCCESS)
    {
        printf(" Executing graph ...\n");

        vx_tutorial_graph_image_gradients_pytiovx_uc_run(&uc);

        printf(" Executing graph ... Done !!!\n");

        show_graph_attributes(uc.graph_0);
        show_node_attributes(uc.node_1);
        show_node_attributes(uc.node_3);
        show_node_attributes(uc.node_5);
        show_node_attributes(uc.node_7);
        show_node_attributes(uc.node_9);

        printf(" Saving to file %s ...\n", PHASE_FILE_NAME);
        save_image_to_file(PHASE_FILE_NAME, uc.phase);

        printf(" Saving to file %s ...\n", MAGNITUDE_FILE_NAME);
        save_image_to_file(MAGNITUDE_FILE_NAME, uc.magnitude_img);

        printf(" Saving to file %s ...\n", GRAD_X_FILE_NAME);
        save_image_to_file(GRAD_X_FILE_NAME, uc.grad_x_img);

        printf(" Saving to file %s ...\n", GRAD_Y_FILE_NAME);
        save_image_to_file(GRAD_Y_FILE_NAME, uc.grad_y_img);
    }

    vx_tutorial_graph_image_gradients_pytiovx_uc_delete(&uc);

    printf(" vx_tutorial_graph_image_gradients_pytiovx: Tutorial Done !!! \n");
    printf(" \n");
}
