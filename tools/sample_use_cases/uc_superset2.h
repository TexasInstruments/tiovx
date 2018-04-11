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

#ifndef UC_SUPERSET2
#define UC_SUPERSET2

#include "VX/vx.h"
#include "TI/tivx.h"

typedef struct _uc_superset2_t *uc_superset2;

typedef struct _uc_superset2_t
{
    vx_context context;

    vx_graph graph_0;

    vx_image image_15;
    vx_scalar scalar_16;
    vx_scalar scalar_18;
    vx_scalar scalar_19;
    vx_scalar scalar_36;
    vx_scalar scalar_37;
    vx_array array_20;
    vx_scalar scalar_21;
    vx_image image_22;
    vx_pyramid pyramid_23;
    vx_image image_24;
    vx_scalar scalar_17;
    vx_scalar scalar_27;
    vx_array array_25;
    vx_scalar scalar_26;
    vx_pyramid pyramid_28;
    vx_array array_29;
    vx_scalar scalar_40;
    vx_scalar scalar_41;
    vx_scalar scalar_42;
    vx_scalar scalar_43;
    vx_scalar scalar_44;
    vx_image image_13;
    vx_image image_14;
    vx_image image_11;
    vx_distribution distribution_12;
    vx_image image_1;
    vx_scalar scalar_2;
    vx_scalar scalar_3;
    vx_image image_4;
    vx_scalar scalar_5;
    vx_scalar scalar_6;
    vx_array array_7;
    vx_array array_8;
    vx_scalar scalar_9;
    vx_scalar scalar_10;
    vx_image image_31;
    vx_pyramid pyramid_32;
    vx_image image_33;
    vx_image image_34;

    vx_node node_38;
    vx_node node_35;
    vx_node node_39;
    vx_node node_45;
    vx_node node_46;
    vx_node node_47;
    vx_node node_48;
    vx_node node_49;
    vx_node node_50;
    vx_node node_51;

} uc_superset2_t;

vx_status uc_superset2_data_create(uc_superset2 usecase);
vx_status uc_superset2_data_delete(uc_superset2 usecase);

vx_status uc_superset2_graph_0_create(uc_superset2 usecase);
vx_status uc_superset2_graph_0_delete(uc_superset2 usecase);
vx_status uc_superset2_graph_0_verify(uc_superset2 usecase);
vx_status uc_superset2_graph_0_run(uc_superset2 usecase);

vx_status uc_superset2_create(uc_superset2 usecase);
vx_status uc_superset2_delete(uc_superset2 usecase);
vx_status uc_superset2_verify(uc_superset2 usecase);
vx_status uc_superset2_run(uc_superset2 usecase);

#endif /* UC_SUPERSET2 */


