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

#ifndef UC_SAMPLE_05
#define UC_SAMPLE_05

#include "VX/vx.h"
#include "TI/tivx.h"

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


