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
#include <VX/vxu.h>
#include <utility.h>

#define IN_FILE_NAME       "colors.bmp"
#define OUT_FILE_NAME      "vx_tutorial_image_extract_channel_out.bmp"

void vx_tutorial_image_extract_channel()
{
    vx_context context;
    vx_image in_image = NULL;
    vx_image r_channel = NULL;
    vx_image g_channel = NULL;
    vx_image b_channel = NULL;
    vx_image out_image = NULL;
    vx_uint32 width, height;

    printf(" vx_tutorial_image_extract_channel: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    in_image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);

    vxSetReferenceName((vx_reference)in_image, "INPUT");
    show_image_attributes(in_image);

    vxQueryImage(in_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(in_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    r_channel = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)r_channel, "R_CHANNEL");
    show_image_attributes(r_channel);

    g_channel = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)g_channel, "G_CHANNEL");
    show_image_attributes(g_channel);

    b_channel = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)b_channel, "B_CHANNEL");
    show_image_attributes(b_channel);

    out_image = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);
    vxSetReferenceName((vx_reference)out_image, "OUTPUT");
    show_image_attributes(out_image);

    vxuChannelExtract(context, in_image, VX_CHANNEL_R, r_channel);
    vxuChannelExtract(context, in_image, VX_CHANNEL_G, g_channel);
    vxuChannelExtract(context, in_image, VX_CHANNEL_B, b_channel);
    vxuChannelCombine(context, b_channel, g_channel, r_channel, NULL, out_image);

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    save_image_to_file(OUT_FILE_NAME, out_image);

    vxReleaseImage(&in_image);
    vxReleaseImage(&r_channel);
    vxReleaseImage(&g_channel);
    vxReleaseImage(&b_channel);
    vxReleaseImage(&out_image);
    vxReleaseContext(&context);

    printf(" vx_tutorial_image_extract_channel: Tutorial Done !!! \n");
    printf(" \n");
}
