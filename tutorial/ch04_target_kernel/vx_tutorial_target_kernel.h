/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef VX_TUTORIAL_TARGET_KERNEL_H
#define VX_TUTORIAL_TARGET_KERNEL_H

#include <VX/vx.h>

/*
 * \brief Tutorials showing target kernel integration
 *
 *        Aim of these tutorials is to show
 *        - How to implement a target kernel on DSP and use it from host side
 *        - How to integrate a target kernel with BAM framework
 *        - How to group multiple BAM kernel to make a larger target kernel
 */

/*
 * \brief Run all tutorials in this module
 */
void vx_tutorial_target_kernel_run_all();

/*
 * \brief Interactive execution of tutorials using console IO
 */
void vx_tutorial_target_kernel_run_interactive();

#endif
