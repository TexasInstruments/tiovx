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
