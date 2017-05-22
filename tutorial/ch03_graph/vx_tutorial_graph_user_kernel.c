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
#include <ch03_graph/phase_rgb_user_kernel.h>

#define IN_FILE_NAME      "vx_tutorial_graph_image_gradients_phase_out.bmp"
#define OUT_USER_KERNEL_FILE_NAME   "vx_tutorial_graph_user_kernel_out.bmp"
#define OUT_TARGET_KERNEL_FILE_NAME "vx_tutorial_graph_target_kernel_out.bmp"


void vx_tutorial_graph_user_kernel(vx_bool add_as_target_kernel)
{
    vx_context context;
    vx_image in_image = NULL;
    vx_image out_image = NULL;
    vx_node node = NULL;
    vx_graph graph = NULL;
    vx_uint32 width, height;
    vx_status status;
    char *out_file = OUT_USER_KERNEL_FILE_NAME;

    if(add_as_target_kernel)
    {
        out_file = OUT_TARGET_KERNEL_FILE_NAME;
    }

    printf(" vx_tutorial_graph_user_kernel: Tutorial Started !!! \n");

    context = vxCreateContext();

    status = phase_rgb_user_kernel_add(context, add_as_target_kernel);
    if(status!=VX_SUCCESS)
    {
        printf(" vx_tutorial_graph_user_kernel: ERROR: unable to add user kernel !!!\n");
    }

    if(status==VX_SUCCESS)
    {
        printf(" Loading file %s ...\n", IN_FILE_NAME);

        in_image = create_image_from_file(context, IN_FILE_NAME, vx_true_e);

        vxSetReferenceName((vx_reference)in_image, "INPUT");
        show_image_attributes(in_image);

        vxQueryImage(in_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
        vxQueryImage(in_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

        out_image = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);
        vxSetReferenceName((vx_reference)out_image, "GRAD_X");
        show_image_attributes(out_image);

        graph = vxCreateGraph(context);

        node = phase_rgb_user_kernel_node(graph, in_image, out_image);

        status = vxVerifyGraph(graph);

        show_graph_attributes(graph);
        show_node_attributes(node);

        if(status==VX_SUCCESS)
        {
            printf(" Executing graph ...\n");

            vxScheduleGraph(graph);
            vxWaitGraph(graph);

            printf(" Executing graph ... Done !!!\n");

            show_graph_attributes(graph);
            show_node_attributes(node);

            printf(" Saving to file %s ...\n", out_file);
            save_image_to_file(out_file, out_image);
        }

        vxReleaseImage(&in_image);
        vxReleaseImage(&out_image);
        vxReleaseNode(&node);
        vxReleaseGraph(&graph);
        phase_rgb_user_kernel_remove(context);
    }
    vxReleaseContext(&context);

    printf(" vx_tutorial_graph_user_kernel: Tutorial Done !!! \n");
    printf(" \n");
}

