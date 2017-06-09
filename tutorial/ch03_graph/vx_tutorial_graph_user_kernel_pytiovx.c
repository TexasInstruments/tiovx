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
#include <ch03_graph/vx_tutorial_graph_user_kernel_pytiovx_uc.h>
#include <ch03_graph/phase_rgb_user_kernel.h>

#define IN_FILE_NAME                "colors.bmp"
#define OUT_USER_KERNEL_FILE_NAME   "vx_tutorial_graph_user_kernel_pytiovx_out.bmp"
#define OUT_TARGET_KERNEL_FILE_NAME "vx_tutorial_graph_target_kernel_pytiovx_out.bmp"

void vx_tutorial_graph_user_kernel_pytiovx(vx_bool add_as_target_kernel)
{
    vx_status status;
    vx_tutorial_graph_user_kernel_pytiovx_uc_t uc;
    vx_context context = NULL;
    char *out_file = OUT_USER_KERNEL_FILE_NAME;

    if(add_as_target_kernel)
    {
        out_file = OUT_TARGET_KERNEL_FILE_NAME;
    }
    printf(" vx_tutorial_graph_user_kernel_pytiovx: Tutorial Started !!! \n");

    context = vxCreateContext();

    status = phase_rgb_user_kernel_add(context, add_as_target_kernel);
    if(status!=VX_SUCCESS)
    {
        printf(" vx_tutorial_graph_user_kernel_pytiovx: ERROR: unable to add user kernel !!!\n");
    }
    if(status==VX_SUCCESS)
    {
        vx_tutorial_graph_user_kernel_pytiovx_uc_create(&uc);

        printf(" Loading file %s ...\n", IN_FILE_NAME);
        status = load_image_from_file(uc.input, IN_FILE_NAME, vx_true_e);

        show_image_attributes(uc.input);
        show_image_attributes(uc.grad_x);
        show_image_attributes(uc.grad_y);
        show_image_attributes(uc.phase);
        show_image_attributes(uc.phase_rgb);

        vx_tutorial_graph_user_kernel_pytiovx_uc_verify(&uc);
        show_graph_attributes(uc.graph_0);

        if(status==VX_SUCCESS)
        {
            printf(" Executing graph ...\n");

            vx_tutorial_graph_user_kernel_pytiovx_uc_run(&uc);

            printf(" Executing graph ... Done !!!\n");
            show_graph_attributes(uc.graph_0);

            printf(" Saving to file %s ...\n", out_file);
            save_image_to_file(out_file, uc.phase_rgb);
        }

        vx_tutorial_graph_user_kernel_pytiovx_uc_delete(&uc);

        phase_rgb_user_kernel_remove(context);
    }

    vxReleaseContext(&context);

    printf(" vx_tutorial_graph_user_kernel_pytiovx: Tutorial Done !!! \n");
    printf(" \n");
}
