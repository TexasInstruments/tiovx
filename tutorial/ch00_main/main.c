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


#include <TI/tivx.h>
#include <vx_tutorial.h>

static void show_usage(int argc, char* argv[])
{
    printf(" \n");
    printf(" OpenVX Tutorials, \n");
    printf(" (c) Texas Instruments 2017 - www.ti.com\n");
    printf(" \n");
    printf(" Usage: %s [options] \n", argv[0]);
    printf(" \n");
    printf(" Options:\n");
    printf("  -h, --help             Show this message\n");
    printf("  -i, --run_interactive  Run tutorials using console based interactive UI\n");
    printf("  --run_all              Run all tutorials\n");
    printf(" \n");
}

int main(int argc, char* argv[])
{
    tivxInit();

    if(argc <= 1)
    {
        show_usage(argc, argv);
        vx_tutorial_run_interactive();
    }
    else
    {
        uint32_t i;

        for(i=1; i < argc; i++)
        {
            if(strcmp(argv[i], "--run_all")==0)
            {
                vx_tutorial_run_all();
                break;
            }
            if(strcmp(argv[i], "--run_interactive")==0
                ||
               strcmp(argv[i], "-i")==0
                )
            {
                vx_tutorial_run_interactive();
                break;
            }
            if(strcmp(argv[i], "--help")==0
                ||
               strcmp(argv[i], "-h")==0
                )
            {
                show_usage(argc, argv);
                break;
            }
        }
    }
    tivxDeInit();
}
