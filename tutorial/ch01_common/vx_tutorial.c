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
    "\n ==============="
    "\n OpenVX Tutorial"
    "\n ==============="
    "\n"
    "\n 1: Image"
    "\n 2: Graph"
    "\n 3: Target Kernel"
    "\n"
    "\n a: Run All"
    "\n"
    "\n x: Exit"
    "\n"
    "\n Enter Choice: "
};

void vx_tutorial_run_all()
{
    vx_tutorial_image_run_all();
    vx_tutorial_graph_run_all();
    vx_tutorial_target_kernel_run_all();
}

void vx_tutorial_run_interactive()
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
                vx_tutorial_image_run_interactive();
                break;
            case '2':
                vx_tutorial_graph_run_interactive();
                break;
            case '3':
                vx_tutorial_target_kernel_run_interactive();
                break;
            case 'a':
                vx_tutorial_run_all();
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
