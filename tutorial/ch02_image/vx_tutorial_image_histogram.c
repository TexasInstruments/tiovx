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

#define IN_FILE_NAME       "colors.bmp"
#define OUT_FILE_NAME      "vx_tutorial_image_histogram_out.bmp"


vx_image convert_distribution_to_image(vx_distribution distribution,
            uint32_t width, uint32_t height);

void vx_tutorial_image_histogram()
{
    vx_context context;
    vx_image in_image = NULL, out_image = NULL;
    vx_distribution histogram = NULL;
    uint32_t num_bins = 256;
    uint32_t histogram_image_width  = 256;
    uint32_t histogram_image_height = 128;
    vx_node node0 = NULL;
    vx_graph graph = NULL;
    vx_status status;

    printf(" vx_tutorial_image_histogram: Tutorial Started !!! \n");

    context = vxCreateContext();

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    graph = vxCreateGraph(context);
    vxSetReferenceName((vx_reference)graph, "MY_GRAPH");

    in_image = create_image_from_file(context, IN_FILE_NAME, vx_true_e);

    vxSetReferenceName((vx_reference)in_image, "INPUT");
    show_image_attributes(in_image);

    histogram = vxCreateDistribution(context, num_bins, 0, 256);
    vxSetReferenceName((vx_reference)histogram, "HISTOGRAM");

    {
        vx_reference refs[] = {(vx_reference)in_image, (vx_reference)histogram};

        /* below is equivalent of doing
         * node0 = vxHistogramNode(graph, in_image, histogram);
         */
        node0 = tivxCreateNodeByStructure(graph,
                    VX_KERNEL_HISTOGRAM,
                    refs, sizeof(refs)/sizeof(refs[0])
                    );
        vxSetReferenceName((vx_reference)node0, "HISTOGRAM");
    }


    status = vxVerifyGraph(graph);

    show_graph_attributes(graph);
    show_node_attributes(node0);

    if(status==VX_SUCCESS)
    {
        printf(" Executing graph ...\n");

        vxScheduleGraph(graph);
        vxWaitGraph(graph);

        printf(" Executing graph ... Done !!!\n");

        show_graph_attributes(graph);
        show_node_attributes(node0);

        out_image = convert_distribution_to_image(histogram,
                        histogram_image_width,
                        histogram_image_height);

        vxSetReferenceName((vx_reference)out_image, "HISTOGRAM_IMAGE");
        show_image_attributes(out_image);

        printf(" Saving to file %s ...\n", OUT_FILE_NAME);
        save_image_to_file(OUT_FILE_NAME, out_image);

        vxReleaseImage(&out_image);
    }

    vxReleaseImage(&in_image);
    vxReleaseDistribution(&histogram);
    vxReleaseNode(&node0);
    vxReleaseGraph(&graph);
    vxReleaseContext(&context);

    printf(" vx_tutorial_image_histogram: Tutorial Done !!! \n");
    printf(" \n");
}

#define MAX_BINS    (256u)

vx_image convert_distribution_to_image(vx_distribution distribution,
            uint32_t width, uint32_t height)
{
    vx_image image;
    vx_context context;
    vx_status status;

    context = vxGetContext((vx_reference)distribution);

    image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);

    if(vxGetStatus((vx_reference)image)==VX_SUCCESS)
    {
        vx_size num_bins;

        status = vxQueryDistribution(distribution, VX_DISTRIBUTION_BINS, &num_bins, sizeof(vx_size));

        if(status == VX_SUCCESS && num_bins <=  MAX_BINS)
        {
            uint32_t histogram_data[MAX_BINS] = {0};
            uint32_t i, max;

            vxCopyDistribution(distribution,
                (void**)&histogram_data[0],
                VX_READ_ONLY,
                VX_MEMORY_TYPE_HOST);

            /* normalize bins */
            max = 0;
            for(i=0; i<num_bins; i++)
            {
                if(histogram_data[i] > max )
                    max = histogram_data[i];
            }
            /* scale to image height */
            for(i=0; i<num_bins; i++)
            {
                histogram_data[i] = (uint32_t)(((float)histogram_data[i]/max)*height + 0.5);
                if(histogram_data[i]>height)
                    histogram_data[i] = height;
            }

            {
                uint8_t *data_ptr = NULL;
                vx_rectangle_t rect = { 0, 0, width, height};
                vx_map_id map_id;
                vx_imagepatch_addressing_t image_addr;

                status = vxMapImagePatch(image,
                            &rect,
                            0,
                            &map_id,
                            &image_addr,
                            (void**)&data_ptr,
                            VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);

                if(status==VX_SUCCESS && data_ptr!=NULL)
                {
                    uint32_t i, j, k, bin_width;
                    uint8_t *addr0 = NULL, *addr1 = NULL;

                    bin_width = width/num_bins;

                    for(i=0; i<num_bins; i++)
                    {
                        addr0 = data_ptr + i*bin_width;

                        for(j=0; j<height-histogram_data[i]; j++)
                        {
                            addr1 = addr0 + j*image_addr.stride_y;
                            for(k=0; k<bin_width; k++)
                            {
                                addr1[k] = 0xFF;
                            }
                        }

                        addr0 = data_ptr + j*image_addr.stride_y + i*bin_width;

                        for(j=0; j<histogram_data[i]; j++)
                        {
                            addr1 = addr0 + j*image_addr.stride_y;
                            for(k=0; k<bin_width; k++)
                            {
                                addr1[k] = 0x0;
                            }
                        }
                    }
                    vxUnmapImagePatch(image, map_id);
                }
            }
        }
        else
        {
            /* more bins than allocated memory */
            vxReleaseImage(&image);
        }
    }

    return image;
}
