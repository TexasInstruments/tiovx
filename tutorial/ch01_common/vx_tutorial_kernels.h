/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef VX_TUTORIAL_KERNELS_H
#define VX_TUTORIAL_KERNELS_H

#include <VX/vx.h>
#include <TI/tivx.h>

/*!
 * \brief The list of kernels supported in this tutorial
 *
 */
enum tivx_kernel_tutorial_e {
    /*! \brief Converts Phase output to RGB image
     */
    TIVX_TUTORIAL_KERNEL_PHASE_RGB = VX_KERNEL_BASE(VX_ID_TI, TIVX_LIBRARY_TUTORIAL_BASE) + 0x0,
};


#endif
