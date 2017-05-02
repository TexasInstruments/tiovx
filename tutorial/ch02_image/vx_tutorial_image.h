/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef VX_TUTORIAL_IMAGE_H
#define VX_TUTORIAL_IMAGE_H

#include <VX/vx.h>

/*
 * \brief Tutorials showing basic image operations
 *
 *        Aim of these tutorials is to show
 *        - How to load, save images using basic image create APIs
 *        - How to query images and print image attributes
 *        - How to use advanced image create APIs
 *        - How to operate on images using vxu node create APIs
 *        - How to operate on images using graphs and OpenVX node create APIs
 *        - How to use create nodes using graphs and generic node create APIs
 */

/*
 * \brief vxCreateImage usage
 *
 *        Loads a image from a BMP file
 *        Save the resulting image to BMP file
 */
void vx_tutorial_image_load_save();

/*
 * \brief vxQueryImage usage
 *
 *        Loads a image from a BMP file
 *        Print image information to console
 */
void vx_tutorial_image_query();

/*
 * \brief vxCreateImageFromHandle, vxCreateImageFromROI usage
 *
 *        Loads a image from a BMP file
 *        Crops the image
 *        Save the resulting image to BMP file
 */
void vx_tutorial_image_crop_roi();

/*
 * \brief vxu usage
 *
 *        Loads a image from a BMP file using
 *        Extract R, G, B channel
 *        Save as BMP with B and R swapped
 */
void vx_tutorial_image_extract_channel();

/*
 * \brief Graph and Node usage
 *
 *        Loads a image from a BMP file using
 *        Convert to NV12
 *        Extract Y channel
 *        Save as BMP with Y channel replcate for R, G, B channels
 */
void vx_tutorial_image_color_convert();

/*
 * \brief vxCreateGenericNode usage
 *
 *        Loads a "green" channel of image from a BMP file
 *        Compute histogram
 *        Plot histogram as an image
 *        Save histogram image as BMP
 */
void vx_tutorial_image_histogram();

/*
 * \brief Run all tutorials in this module
 */
void vx_tutorial_image_run_all();

/*
 * \brief Interactive execution of tutorials using console IO
 */
void vx_tutorial_image_run_interactive();

#endif
