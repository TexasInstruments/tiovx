/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
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
        show_node_attributes(uc.node_6);
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
