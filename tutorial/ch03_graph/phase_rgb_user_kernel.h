/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef PHASE_RGB_USER_KERNEL
#define PHASE_RGB_USER_KERNEL

#include <VX/vx.h>

vx_status phase_rgb_user_kernel_add(vx_context context, vx_bool add_as_target_kernel);
vx_status phase_rgb_user_kernel_remove(vx_context context);
vx_node phase_rgb_user_kernel_node(vx_graph graph, vx_image in, vx_image out);

#endif
