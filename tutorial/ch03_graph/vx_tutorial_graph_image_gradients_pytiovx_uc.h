/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#ifndef VX_TUTORIAL_GRAPH_IMAGE_GRADIENTS_PYTIOVX_UC
#define VX_TUTORIAL_GRAPH_IMAGE_GRADIENTS_PYTIOVX_UC

#include "VX/vx.h"
#include "TI/tivx.h"

typedef struct _vx_tutorial_graph_image_gradients_pytiovx_uc_t *vx_tutorial_graph_image_gradients_pytiovx_uc;

typedef struct _vx_tutorial_graph_image_gradients_pytiovx_uc_t
{
    vx_context context;
    
    vx_graph graph_0;
    
    vx_image input;
    vx_image grad_x;
    vx_image grad_y;
    vx_image magnitude;
    vx_image phase;
    vx_image magnitude_img;
    vx_scalar scalar_4;
    vx_scalar shift;
    vx_image grad_x_img;
    vx_scalar scalar_6;
    vx_image grad_y_img;
    vx_scalar scalar_8;
    
    vx_node node_1;
    vx_node node_2;
    vx_node node_3;
    vx_node node_5;
    vx_node node_7;
    vx_node node_9;
    
} vx_tutorial_graph_image_gradients_pytiovx_uc_t;

vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_data_create(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_data_delete(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);

vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_graph_0_create(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_graph_0_delete(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_graph_0_verify(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_graph_0_run(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);

vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_create(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_delete(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_verify(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);
vx_status vx_tutorial_graph_image_gradients_pytiovx_uc_run(vx_tutorial_graph_image_gradients_pytiovx_uc usecase);

#endif /* VX_TUTORIAL_GRAPH_IMAGE_GRADIENTS_PYTIOVX_UC */


