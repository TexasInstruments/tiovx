/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 * \file utility.h Utility APIs created as part of various tutorials
 */
#ifndef UTILITY_H
#define UTILITY_H

#include <VX/vx.h>

vx_image  create_image_from_file(vx_context context, char *filename, vx_bool convert_to_gray_scale);

vx_status load_image_from_file(vx_image image, char *filename, vx_bool convert_to_gray_scale);

vx_status save_image_to_file(char *filename, vx_image image);

vx_node create_generic_node(vx_graph graph,
                            vx_enum kernelenum,
                            vx_reference params[],
                            vx_uint32 num);

void show_image_attributes(vx_image image);
void show_graph_attributes(vx_graph graph);
void show_node_attributes(vx_node node);

#endif
