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
#include <bmp_rd_wr.h>
#include <utility.h>

#define IN_FILE_NAME       "colors.bmp"

void vx_tutorial_image_query()
{
    vx_context context;
    vx_image image;

    printf(" vx_tutorial_image_query: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);

    vxSetReferenceName((vx_reference)image, "MY_IMAGE");

    show_image_attributes(image);

    vxReleaseImage(&image);
    vxReleaseContext(&context);

    printf(" vx_tutorial_image_query: Tutorial Done !!! \n");
    printf(" \n");
}

#define MAX_ATTRIBUTE_NAME (32u)

void show_image_attributes(vx_image image)
{
    vx_uint32 width=0, height=0, ref_count=0;
    vx_df_image df=0;
    vx_size num_planes=0, size=0;
    vx_enum color_space=0, channel_range=0, memory_type=0;
    vx_char *ref_name=NULL;
    char df_name[MAX_ATTRIBUTE_NAME];
    char color_space_name[MAX_ATTRIBUTE_NAME];
    char channel_range_name[MAX_ATTRIBUTE_NAME];
    char memory_type_name[MAX_ATTRIBUTE_NAME];
    char ref_name_invalid[MAX_ATTRIBUTE_NAME];

    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
    vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(vx_size));
    vxQueryImage(image, VX_IMAGE_SIZE, &size, sizeof(vx_size));
    vxQueryImage(image, VX_IMAGE_SPACE, &color_space, sizeof(vx_enum));
    vxQueryImage(image, VX_IMAGE_RANGE, &channel_range, sizeof(vx_enum));
    vxQueryImage(image, VX_IMAGE_MEMORY_TYPE, &memory_type, sizeof(vx_enum));

    vxQueryReference((vx_reference)image, VX_REFERENCE_NAME, &ref_name, sizeof(vx_char*));
    vxQueryReference((vx_reference)image, VX_REFERENCE_COUNT, &ref_count, sizeof(vx_uint32));

    switch(df)
    {
        case VX_DF_IMAGE_VIRT:
            strncpy(df_name, "VX_DF_IMAGE_VIRT", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_RGB:
            strncpy(df_name, "VX_DF_IMAGE_RGB", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_RGBX:
            strncpy(df_name, "VX_DF_IMAGE_RGBX", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_NV12:
            strncpy(df_name, "VX_DF_IMAGE_NV12", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_NV21:
            strncpy(df_name, "VX_DF_IMAGE_NV21", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_UYVY:
            strncpy(df_name, "VX_DF_IMAGE_UYVY", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_YUYV:
            strncpy(df_name, "VX_DF_IMAGE_YUYV", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_IYUV:
            strncpy(df_name, "VX_DF_IMAGE_IYUV", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_YUV4:
            strncpy(df_name, "VX_DF_IMAGE_YUV4", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_U8:
            strncpy(df_name, "VX_DF_IMAGE_U8", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_U16:
            strncpy(df_name, "VX_DF_IMAGE_U16", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_S16:
            strncpy(df_name, "VX_DF_IMAGE_S16", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_U32:
            strncpy(df_name, "VX_DF_IMAGE_U32", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_S32:
            strncpy(df_name, "VX_DF_IMAGE_S32", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(df_name, "VX_DF_IMAGE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    switch(color_space)
    {
        case VX_COLOR_SPACE_NONE:
            strncpy(color_space_name, "VX_COLOR_SPACE_NONE", MAX_ATTRIBUTE_NAME);
            break;
        case VX_COLOR_SPACE_BT601_525:
            strncpy(color_space_name, "VX_COLOR_SPACE_BT601_525", MAX_ATTRIBUTE_NAME);
            break;
        case VX_COLOR_SPACE_BT601_625:
            strncpy(color_space_name, "VX_COLOR_SPACE_BT601_625", MAX_ATTRIBUTE_NAME);
            break;
        case VX_COLOR_SPACE_BT709:
            strncpy(color_space_name, "VX_COLOR_SPACE_BT709", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(color_space_name, "VX_COLOR_SPACE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    switch(channel_range)
    {
        case VX_CHANNEL_RANGE_FULL:
            strncpy(channel_range_name, "VX_CHANNEL_RANGE_FULL", MAX_ATTRIBUTE_NAME);
            break;
        case VX_CHANNEL_RANGE_RESTRICTED:
            strncpy(channel_range_name, "VX_CHANNEL_RANGE_RESTRICTED", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(channel_range_name, "VX_CHANNEL_RANGE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    switch(memory_type)
    {
        case VX_MEMORY_TYPE_NONE:
            strncpy(memory_type_name, "VX_MEMORY_TYPE_NONE", MAX_ATTRIBUTE_NAME);
            break;
        case VX_MEMORY_TYPE_HOST:
            strncpy(memory_type_name, "VX_MEMORY_TYPE_HOST", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(memory_type_name, "VX_MEMORY_TYPE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    if(ref_name==NULL)
    {
        strncpy(ref_name_invalid, "INVALID_REF_NAME", MAX_ATTRIBUTE_NAME);
        ref_name = &ref_name_invalid[0];
    }

    printf(" VX_TYPE_IMAGE: %s, %d x %d, %d plane(s), %d B, %s %s %s %s, %d refs\n",
        ref_name,
        width,
        height,
        num_planes,
        size,
        df_name,
        color_space_name,
        channel_range_name,
        memory_type_name,
        ref_count
        );
}
