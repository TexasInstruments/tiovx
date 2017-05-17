/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_tutorial.h>

static char menu[] = {
    "\n"
    "\n ========================"
    "\n Tutorial : Target Kernel"
    "\n ========================"
    "\n"
    "\n 1: Target Kernel on DSP"
    "\n 2: Target Kernel with BAM framework"
    "\n 3: Target Kernel with multiple BAM kernels"
    "\n"
    "\n a: Run All Tutorial"
    "\n"
    "\n x: Exit Menu"
    "\n"
    "\n Enter Choice: "
};

void vx_tutorial_target_kernel_run_all()
{
}

void vx_tutorial_target_kernel_run_interactive()
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
            case 'a':
                vx_tutorial_target_kernel_run_all();
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

/**
 \page CH04_TARGET_KERNEL Chatper 4: Understanding TI OpenVX target kernels

 In TI OpenVX implementation, extension APIs are provided which allow user
 to execute kernels, on TI specific native targets like DSP. These tutorials
 show examples of how to use the TI target kernel API.

    <TABLE>
        <TR bgcolor="lightgrey">
            <TH> Tutorial file </TH>
            <TH> Purpose </TH>
        </TR>
        <TR>
            <TD>  </TD>
            <TD>  </TD>
        </TR>
   </TABLE>

 */
