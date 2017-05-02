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
    "\n ================"
    "\n Tutorial : Graph"
    "\n ================"
    "\n"
    "\n 1: Graph with multiple targets"
    "\n 2: Graph generated with PyTIOVX tool"
    "\n 3: Graph with user kernels"
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
