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


#include <vx_tutorial.h>

static char menu[] = {
    "\n"
    "\n ================"
    "\n Tutorial : Graph"
    "\n ================"
    "\n"
    "\n 1: Graph with multiple targets"
    "\n 2: Graph generated with PyTIOVX tool"
    "\n 3: Graph with user kernels"
    "\n 4: Graph with user kernels and generated with PyTIOVX tool"
    "\n 5: Graph with target kernels"
    "\n 6: Graph with target kernels and generated with PyTIOVX tool"
    "\n"
    "\n a: Run All"
    "\n"
    "\n x: Exit Menu"
    "\n"
    "\n Enter Choice: "
};

void vx_tutorial_graph_run_all()
{
    vx_tutorial_graph_image_gradients();
    vx_tutorial_graph_image_gradients_pytiovx();
    vx_tutorial_graph_user_kernel(vx_false_e);
    vx_tutorial_graph_user_kernel_pytiovx(vx_false_e);
    vx_tutorial_graph_user_kernel(vx_true_e);
    vx_tutorial_graph_user_kernel_pytiovx(vx_true_e);
}

void vx_tutorial_graph_run_interactive()
{
    char ch;
    vx_bool done = vx_false_e;

    while(!done)
    {
        printf(menu);
        ch = vx_tutorial_get_char();
        printf("\n");

        switch(ch)
        {
            case '1':
                vx_tutorial_graph_image_gradients();
                break;
            case '2':
                vx_tutorial_graph_image_gradients_pytiovx();
                break;
            case '3':
                vx_tutorial_graph_user_kernel(vx_false_e);
                break;
            case '4':
                vx_tutorial_graph_user_kernel_pytiovx(vx_false_e);
                break;
            case '5':
                vx_tutorial_graph_user_kernel(vx_true_e);
                break;
            case '6':
                vx_tutorial_graph_user_kernel_pytiovx(vx_true_e);
                break;
            case 'a':
                vx_tutorial_graph_run_all();
                break;
            case 'x':
                done = vx_true_e;
                break;
            case '\n':
                break;
            default:
                printf("\n Invalid option !!!");
                break;
        }
    }
}

