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

#define IN_FILE_NAME         "colors.bmp"
#define PHASE_FILE_NAME      "vx_tutorial_graph_image_gradients_phase_out.bmp"
#define MAGNITUDE_FILE_NAME  "vx_tutorial_graph_image_gradients_magnitude_out.bmp"
#define GRAD_X_FILE_NAME     "vx_tutorial_graph_image_gradients_grad_x_out.bmp"
#define GRAD_Y_FILE_NAME     "vx_tutorial_graph_image_gradients_grad_y_out.bmp"

#define NUM_NODES    (6u)

void vx_tutorial_graph_image_gradients()
{
    vx_context context;
    vx_image in_image = NULL;
    vx_image grad_x = NULL;
    vx_image grad_y = NULL;
    vx_image magnitude = NULL;
    vx_image phase = NULL;
    vx_image magnitude_image = NULL;
    vx_image grad_x_image = NULL;
    vx_image grad_y_image = NULL;
    vx_graph graph = NULL;
    vx_scalar shift = NULL;
    int32_t shift_value = 0;
    vx_node node[NUM_NODES] = {NULL};
    vx_uint32 width, height;
    vx_status status;
    uint32_t i;

    printf(" vx_tutorial_graph_image_gradients: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    in_image = create_image_from_file(context, IN_FILE_NAME, vx_true_e);

    vxSetReferenceName((vx_reference)in_image, "INPUT");
    show_image_attributes(in_image);

    vxQueryImage(in_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(in_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    grad_x = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    vxSetReferenceName((vx_reference)grad_x, "GRAD_X");
    show_image_attributes(grad_x);

    grad_y = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    vxSetReferenceName((vx_reference)grad_y, "GRAD_Y");
    show_image_attributes(grad_y);

    magnitude = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    vxSetReferenceName((vx_reference)magnitude, "MAGNITUDE");
    show_image_attributes(magnitude);

    magnitude_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)magnitude_image, "MAGNITUDE_IMAGE");
    show_image_attributes(magnitude_image);

    grad_x_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)grad_x_image, "GRAD_X_IMAGE");
    show_image_attributes(grad_x_image);

    grad_y_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)grad_y_image, "GRAD_Y_IMAGE");
    show_image_attributes(grad_y_image);

    shift = vxCreateScalar(context, VX_TYPE_INT32, &shift_value);

    phase = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)phase, "PHASE");
    show_image_attributes(phase);

    graph = vxCreateGraph(context);
    i = 0;

    node[i] = vxSobel3x3Node(graph, in_image, grad_x, grad_y);
    vxSetReferenceName((vx_reference)node[i], "SOBEL3x3");
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    i++;

    node[i] = vxMagnitudeNode(graph, grad_x, grad_y, magnitude);
    vxSetReferenceName((vx_reference)node[i], "MAGNITUDE");
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    i++;

    node[i] = vxPhaseNode(graph, grad_x, grad_y, phase);
    vxSetReferenceName((vx_reference)node[i], "PHASE");
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP2);
    i++;

    node[i] = vxConvertDepthNode(graph,
                    magnitude, magnitude_image,
                    VX_CONVERT_POLICY_SATURATE,
                    shift);
    vxSetReferenceName((vx_reference)node[i], "MAGNITUDE_IMAGE");
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    i++;

    node[i] = vxConvertDepthNode(graph,
                    grad_x, grad_x_image,
                    VX_CONVERT_POLICY_SATURATE,
                    shift);
    vxSetReferenceName((vx_reference)node[i], "GRAD_X_IMAGE");
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP1);
    i++;

    node[i] = vxConvertDepthNode(graph,
                    grad_y, grad_y_image,
                    VX_CONVERT_POLICY_SATURATE,
                    shift);
    vxSetReferenceName((vx_reference)node[i], "GRAD_Y_IMAGE");
    vxSetNodeTarget(node[i], VX_TARGET_STRING, TIVX_TARGET_DSP2);
    i++;

    status = vxVerifyGraph(graph);

    show_graph_attributes(graph);
    for(i=0; i<sizeof(node)/sizeof(node[0]); i++)
    {
        if(node[i])
        {
            show_node_attributes(node[i]);
        }
    }

    if(status==VX_SUCCESS)
    {
        printf(" Executing graph ...\n");

        vxScheduleGraph(graph);
        vxWaitGraph(graph);

        printf(" Executing graph ... Done !!!\n");

        show_graph_attributes(graph);
        for(i=0; i<sizeof(node)/sizeof(node[0]); i++)
        {
            if(node[i])
            {
                show_node_attributes(node[i]);
            }
        }

        printf(" Saving to file %s ...\n", PHASE_FILE_NAME);
        save_image_to_file(PHASE_FILE_NAME, phase);

        printf(" Saving to file %s ...\n", MAGNITUDE_FILE_NAME);
        save_image_to_file(MAGNITUDE_FILE_NAME, magnitude_image);

        printf(" Saving to file %s ...\n", GRAD_X_FILE_NAME);
        save_image_to_file(GRAD_X_FILE_NAME, grad_x_image);

        printf(" Saving to file %s ...\n", GRAD_Y_FILE_NAME);
        save_image_to_file(GRAD_Y_FILE_NAME, grad_y_image);
    }

    vxReleaseImage(&in_image);
    vxReleaseImage(&grad_x);
    vxReleaseImage(&grad_y);
    vxReleaseImage(&grad_x_image);
    vxReleaseImage(&grad_y_image);
    vxReleaseImage(&phase);
    vxReleaseImage(&magnitude);
    vxReleaseImage(&magnitude_image);
    vxReleaseScalar(&shift);
    for(i=0; i<sizeof(node)/sizeof(node[0]); i++)
    {
        if(node[i])
        {
            vxReleaseNode(&node[i]);
        }
    }
    vxReleaseGraph(&graph);
    vxReleaseContext(&context);

    printf(" vx_tutorial_graph_image_gradients: Tutorial Done !!! \n");
    printf(" \n");
}
