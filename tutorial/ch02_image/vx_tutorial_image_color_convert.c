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
#include <utility.h>

#define IN_FILE_NAME       "colors.bmp"
#define OUT_FILE_NAME      "vx_tutorial_image_color_convert_out.bmp"

void vx_tutorial_image_color_convert()
{
    vx_context context;
    vx_image rgb_image = NULL;
    vx_image nv12_image = NULL;
    vx_image y_image = NULL;
    vx_node node0 = NULL, node1 = NULL;
    vx_graph graph = NULL;
    vx_uint32 width, height;
    vx_status status;

    printf(" vx_tutorial_image_color_convert: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    graph = vxCreateGraph(context);
    vxSetReferenceName((vx_reference)graph, "MY_GRAPH");

    rgb_image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);

    vxSetReferenceName((vx_reference)rgb_image, "RGB_IMAGE");
    show_image_attributes(rgb_image);

    vxQueryImage(rgb_image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(rgb_image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    nv12_image = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_NV12);
    vxSetReferenceName((vx_reference)nv12_image, "NV12_IMAGE");
    show_image_attributes(nv12_image);

    y_image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vxSetReferenceName((vx_reference)y_image, "Y_IMAGE");
    show_image_attributes(y_image);

    node0 = vxColorConvertNode(graph, rgb_image, nv12_image);
    vxSetReferenceName((vx_reference)node0, "COLOR_CONVERT");

    node1 = vxChannelExtractNode(graph, nv12_image, VX_CHANNEL_Y, y_image);
    vxSetReferenceName((vx_reference)node1, "CHANNEL_EXTRACT");

    status = vxVerifyGraph(graph);

    show_graph_attributes(graph);
    show_node_attributes(node0);
    show_node_attributes(node1);

    if(status==VX_SUCCESS)
    {
        printf(" Executing graph ...\n");

        vxScheduleGraph(graph);
        vxWaitGraph(graph);

        printf(" Executing graph ... Done !!!\n");

        show_graph_attributes(graph);
        show_node_attributes(node0);
        show_node_attributes(node1);

        printf(" Saving to file %s ...\n", OUT_FILE_NAME);

        save_image_to_file(OUT_FILE_NAME, y_image);
    }

    vxReleaseImage(&rgb_image);
    vxReleaseImage(&nv12_image);
    vxReleaseImage(&y_image);
    vxReleaseNode(&node0);
    vxReleaseNode(&node1);
    vxReleaseGraph(&graph);
    vxReleaseContext(&context);

    printf(" vx_tutorial_image_color_convert: Tutorial Done !!! \n");
    printf(" \n");
}

#define MAX_ATTRIBUTE_NAME (32u)

void show_graph_attributes(vx_graph graph)
{
    vx_uint32 num_nodes=0, num_params=0, ref_count=0;
    vx_enum state=0;
    vx_perf_t perf={0};
    vx_char *ref_name=NULL;
    char ref_name_invalid[MAX_ATTRIBUTE_NAME];
    char state_name[MAX_ATTRIBUTE_NAME];

    vxQueryGraph(graph, VX_GRAPH_NUMNODES, &num_nodes, sizeof(vx_uint32));
    vxQueryGraph(graph, VX_GRAPH_NUMPARAMETERS, &num_params, sizeof(vx_uint32));
    vxQueryGraph(graph, VX_GRAPH_STATE, &state, sizeof(vx_enum));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf, sizeof(vx_perf_t));

    vxQueryReference((vx_reference)graph, VX_REFERENCE_NAME, &ref_name, sizeof(vx_char*));
    vxQueryReference((vx_reference)graph, VX_REFERENCE_COUNT, &ref_count, sizeof(vx_uint32));

    switch(state)
    {
        case VX_GRAPH_STATE_UNVERIFIED:
            strncpy(state_name, "VX_GRAPH_STATE_UNVERIFIED", MAX_ATTRIBUTE_NAME);
            break;
        case VX_GRAPH_STATE_VERIFIED:
            strncpy(state_name, "VX_GRAPH_STATE_VERIFIED", MAX_ATTRIBUTE_NAME);
            break;
        case VX_GRAPH_STATE_RUNNING:
            strncpy(state_name, "VX_GRAPH_STATE_RUNNING", MAX_ATTRIBUTE_NAME);
            break;
        case VX_GRAPH_STATE_ABANDONED:
            strncpy(state_name, "VX_GRAPH_STATE_ABANDONED", MAX_ATTRIBUTE_NAME);
            break;
        case VX_GRAPH_STATE_COMPLETED:
            strncpy(state_name, "VX_GRAPH_STATE_COMPLETED", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(state_name, "VX_GRAPH_STATE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    if(ref_name==NULL)
    {
        strncpy(ref_name_invalid, "INVALID_REF_NAME", MAX_ATTRIBUTE_NAME);
        ref_name = &ref_name_invalid[0];
    }

    printf(" VX_TYPE_GRAPH: %s, %d nodes, %s, avg perf %9.6fs, %d parameters, %d refs\n",
        ref_name,
        num_nodes,
        state_name,
        perf.avg/1000000000.0,
        num_params,
        ref_count
        );
}

void show_node_attributes(vx_node node)
{
    vx_uint32 num_params=0, ref_count=0;
    vx_status status=VX_FAILURE;
    vx_perf_t perf={0};
    vx_char *ref_name=NULL;
    char ref_name_invalid[MAX_ATTRIBUTE_NAME];
    char status_name[MAX_ATTRIBUTE_NAME];

    vxQueryNode(node, VX_NODE_STATUS, &status, sizeof(vx_status));
    vxQueryNode(node, VX_NODE_PARAMETERS, &num_params, sizeof(vx_uint32));
    vxQueryNode(node, VX_NODE_PERFORMANCE, &perf, sizeof(vx_perf_t));

    vxQueryReference((vx_reference)node, VX_REFERENCE_NAME, &ref_name, sizeof(vx_char*));
    vxQueryReference((vx_reference)node, VX_REFERENCE_COUNT, &ref_count, sizeof(vx_uint32));

    switch(status)
    {
        case VX_SUCCESS:
            strncpy(status_name, "VX_SUCCESS", MAX_ATTRIBUTE_NAME);
            break;
        case VX_FAILURE:
            strncpy(status_name, "VX_FAILURE", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(status_name, "VX_FAILURE_OTHER", MAX_ATTRIBUTE_NAME);
            break;
    }

    if(ref_name==NULL)
    {
        strncpy(ref_name_invalid, "INVALID_REF_NAME", MAX_ATTRIBUTE_NAME);
        ref_name = &ref_name_invalid[0];
    }

    printf(" VX_TYPE_NODE: %s, %d params, avg perf %9.6fs, %s, %d refs\n",
        ref_name,
        num_params,
        perf.avg/1000000000.0,
        status_name,
        ref_count
        );
}
