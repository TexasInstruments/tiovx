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
