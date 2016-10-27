/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#ifndef UC_SAMPLE_03
#define UC_SAMPLE_03

#include <VX/vx.h>
#include <TI/tivx.h>

typedef struct _uc_sample_03_t *uc_sample_03;

typedef struct _uc_sample_03_t
{
    vx_context context;
    
    vx_graph graph_0;
    
    vx_image image_1;
    vx_image image_2;
    vx_image image_3;
    
    vx_node node_4;
    
} uc_sample_03_t;

vx_status uc_sample_03_data_create(uc_sample_03 usecase);
vx_status uc_sample_03_data_delete(uc_sample_03 usecase);

vx_status uc_sample_03_graph_0_create(uc_sample_03 usecase);
vx_status uc_sample_03_graph_0_delete(uc_sample_03 usecase);
vx_status uc_sample_03_graph_0_verify(uc_sample_03 usecase);
vx_status uc_sample_03_graph_0_run(uc_sample_03 usecase);

vx_status uc_sample_03_create(uc_sample_03 usecase);
vx_status uc_sample_03_delete(uc_sample_03 usecase);
vx_status uc_sample_03_verify(uc_sample_03 usecase);
vx_status uc_sample_03_run(uc_sample_03 usecase);

#endif /* UC_SAMPLE_03 */


