/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/



/**
 * \file vx_tutorial_image_load_save.c Load and save data from OpenVX image objects
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context and OpenVX image data object
 * - How to read a BMP file and load the pixel values into the image data object
 * - How to query the image data object for attributes like width, height
 * - How to read pixel values from an image data object and save it as a BMP file
 * - How to cleanup all created resources and exit the OpenVX application
 *
 * To include OpenVX interfaces include below file
 * \code
 * #include <VX/vx.h>
 * \endcode
 *
 * To include utility APIs to read and write BMP file include below file
 * \code
 * #include <bmp_rd_wr.h>
 * \endcode
 *
 * Follow the comments in the function vx_tutorial_image_load_save()
 * to understand this tutorial
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <VX/vx.h>
#include <utility.h>

/* in some systems file IO is not present, so skip it using below flag */
/* #define SKIP_FILEIO */

/** \brief Input file name */
#define IN_FILE_NAME       "${VX_TEST_DATA_PATH}/colors.bmp"

/** \brief Output file name */
#define OUT_FILE_NAME      "${VX_TEST_DATA_PATH}/vx_tutorial_image_load_save_out.bmp"

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_image_load_save()
{
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image image;
    /** \endcode */
    vx_uint32 width, height;

    printf(" vx_tutorial_image_load_save: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context.
     *
     * This MUST be done first before any OpenVX API call.
     * The context that is returned is used as input for subsequent OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    /**
     * - Create image object.
     *
     * Follow the comments in tivx_utils_create_vximage_from_bmpfile() to see
     * how a vx_image object is created and filled with RGB data from BMP file \ref IN_FILE_NAME
     * \code
     */
    image = tivx_utils_create_vximage_from_bmpfile(context, IN_FILE_NAME, (vx_bool)vx_false_e);
    /** \endcode */

    /**
     * - Query image object.
     *
     * Here we print the image dimensions.
     * \code
     */
    vxQueryImage(image, (vx_enum)VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, (vx_enum)VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    /** \endcode */

    printf(" Loaded image of dimensions %d x %d\n", width, height);

    printf(" Saving to file %s ...\n", OUT_FILE_NAME);

    /**
     * - Save image object to bitmap file \ref OUT_FILE_NAME.
     *
     * Follow the comments in tivx_utils_save_vximage_to_bmpfile() to see
     * how data in vx_image object is accessed to store pixel values from the image object to
     * BMP file \ref OUT_FILE_NAME
     * \code
     */
    tivx_utils_save_vximage_to_bmpfile(OUT_FILE_NAME, image);
    /** \endcode */

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&image);
    /** \endcode */

    /**
     * - Release context object.
     *
     * Since we are done using OpenVX context, release it.
     * No further OpenVX API calls should be done, until a context is again created using
     * vxCreateContext()
     * \code
     */
    vxReleaseContext(&context);
    /** \endcode */

    printf(" vx_tutorial_image_load_save: Tutorial Done !!! \n");
    printf(" \n");
}

void ascii_file_read(char *filename, int num_elements, void* buffer, vx_enum data_type){
    FILE* ptr_file;

    // Initialize buffer for reading each line of file
    int nbytes = sizeof(float);
    char* buff = malloc(nbytes);

    if (NULL == buff)
    {
        printf("FAILED ALLOCATING MEMORY\n");
        return;
    }

    // Initialize variables for tokenizing each line based on delimiter values
    char* token;
    char* delim = " \n\t,";
    float val;

    // Initialize different data type arrays
    vx_uint8* u8;
    vx_int32* i32;
    vx_float32* f32;
    uint8_t val_u8;
    int32_t val_int32;

    // Try reading in file
    ptr_file = fopen(filename, "r");
    if(!ptr_file){
        printf("FAILED READING FILE\n");
        free(buff);
        return;
    }

    // If file read is successful, try to populate matrix
    int i = 0;
    while(fgets(buff, nbytes, ptr_file) > 0){
        // Split string into tokens at whitespaces, commas, newlines, or tabs
        for(token = strtok(buff, delim); token != NULL; token = strtok(NULL, delim)){
            val = strtod(token, NULL);

            switch(data_type){
                case (vx_enum)VX_TYPE_FLOAT32:
                    f32 = buffer;
                    f32[i] = val;
                    break;

                case (vx_enum)VX_TYPE_UINT8:
                    u8 = buffer;
                    val_u8 = val;
                    u8[i] = val_u8;
                    break;

                case (vx_enum)VX_TYPE_INT32:
                    i32 = buffer;
                    val_int32 = val;
                    i32[i] = val_int32;
                    break;

                default:
                    break;
            }

            i++;
            if (i >= num_elements){
                break;
            }
        }
        // Need to break out of both loops when this condition is met
        if (i >= num_elements){
            break;
        }
    }
    fclose(ptr_file);
    free(buff);
    //printf("val of first element: %d\n", u8[0]);
}

vx_matrix create_matrix_from_file(vx_context context, vx_enum data_type, int cols, int rows, char *filename){
    // Create vx_float32 buffer object
    vx_float32 mat[rows*cols];

    // Call generic function that will populate data object
    ascii_file_read(filename, rows*cols, mat, data_type);

    // Create vx_matrix object
    vx_matrix matrix = vxCreateMatrix(context, data_type, cols, rows);

    // Set pointer reference between vx_float32 data object and
    vxCopyMatrix(matrix, mat, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);

    return matrix;
}

vx_status save_pyramid_to_file(char *filename, vx_pyramid pyr, vx_size levels){
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 index;
    vx_image image;

    printf("Saving pyramid object...\n");
    for (index=0; index<levels; index++){
        // Create buffer for filename
        char img_filename[1000] = "";
        strcat(img_filename, filename);

        // Create new filename for each level
        char img_index[16] = "";
        sprintf(img_index, "%d.bmp", index);
        strcat(img_filename, img_index);
        printf("New filename: %s", img_filename);

        // Save each level
        image = vxGetPyramidLevel(pyr, index);
        status = tivx_utils_save_vximage_to_bmpfile(img_filename, image);
        status = vxReleaseImage(&image);
    }

    return status;
}
