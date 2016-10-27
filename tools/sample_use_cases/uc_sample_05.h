/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#ifndef UC_SAMPLE_05
#define UC_SAMPLE_05

#include <VX/vx.h>
#include <TI/tivx.h>

typedef struct _uc_sample_05_t *uc_sample_05;

typedef struct _uc_sample_05_t
{
    vx_context context;
    
    vx_graph graph_0;
    
    vx_image image_1;
    vx_image image_2;
    vx_scalar scalar_5;
    vx_image image_4;
    vx_convolution convolution_8;
    vx_image image_7;
    vx_scalar scalar_10;
    vx_scalar scalar_11;
    vx_array array_12;
    vx_array array_13;
    vx_scalar scalar_14;
    vx_scalar scalar_15;
    vx_pyramid pyramid_17;
    vx_image image_19;
    vx_remap remap_21;
    vx_scalar scalar_26;
    vx_image image_22;
    vx_threshold threshold_28;
    vx_scalar scalar_30;
    vx_scalar scalar_31;
    vx_image image_29;
    vx_scalar scalar_35;
    vx_matrix matrix_34;
    vx_image image_33;
    vx_image image_37;
    vx_distribution distribution_39;
    vx_lut lut_42;
    vx_image image_41;
    vx_image image_44;
    
    vx_node node_3;
    vx_node node_6;
    vx_node node_9;
    vx_node node_16;
    vx_node node_18;
    vx_node node_20;
    vx_node node_27;
    vx_node node_32;
    vx_node node_36;
    vx_node node_38;
    vx_node node_40;
    vx_node node_43;
    vx_node node_45;
    
} uc_sample_05_t;

vx_status uc_sample_05_data_create(uc_sample_05 usecase);
vx_status uc_sample_05_data_delete(uc_sample_05 usecase);

vx_status uc_sample_05_graph_0_create(uc_sample_05 usecase);
vx_status uc_sample_05_graph_0_delete(uc_sample_05 usecase);
vx_status uc_sample_05_graph_0_verify(uc_sample_05 usecase);
vx_status uc_sample_05_graph_0_run(uc_sample_05 usecase);

vx_status uc_sample_05_create(uc_sample_05 usecase);
vx_status uc_sample_05_delete(uc_sample_05 usecase);
vx_status uc_sample_05_verify(uc_sample_05 usecase);
vx_status uc_sample_05_run(uc_sample_05 usecase);

#endif /* UC_SAMPLE_05 */


