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
