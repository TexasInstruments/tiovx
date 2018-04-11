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

#ifndef UC_SUPERSET1
#define UC_SUPERSET1

#include "VX/vx.h"
#include "TI/tivx.h"

typedef struct _uc_superset1_t *uc_superset1;

typedef struct _uc_superset1_t
{
    vx_context context;

    vx_graph graph_0;

    vx_image image_1;
    vx_image image_2;
    vx_image image_3;
    vx_image image_4;
    vx_scalar scalar_59;
    vx_image image_5;
    vx_image image_6;
    vx_scalar scalar_61;
    vx_image image_7;
    vx_image image_8;
    vx_image image_9;
    vx_image image_10;
    vx_image image_11;
    vx_image image_12;
    vx_image image_13;
    vx_image image_14;
    vx_image image_15;
    vx_convolution convolution_16;
    vx_image image_17;
    vx_image image_18;
    vx_image image_19;
    vx_scalar scalar_71;
    vx_matrix matrix_20;
    vx_image image_21;
    vx_image image_22;
    vx_scalar scalar_23;
    vx_scalar scalar_73;
    vx_scalar scalar_74;
    vx_image image_24;
    vx_image image_25;
    vx_scalar scalar_76;
    vx_scalar scalar_28;
    vx_image image_26;
    vx_image image_27;
    vx_image image_29;
    vx_image image_30;
    vx_image image_31;
    vx_image image_32;
    vx_image image_33;
    vx_image image_34;
    vx_scalar scalar_81;
    vx_image image_35;
    vx_image image_36;
    vx_lut lut_37;
    vx_image image_38;
    vx_image image_39;
    vx_image image_40;
    vx_image image_41;
    vx_image image_42;
    vx_scalar scalar_87;
    vx_matrix matrix_44;
    vx_scalar scalar_89;
    vx_image image_43;
    vx_matrix matrix_46;
    vx_scalar scalar_91;
    vx_image image_45;
    vx_threshold threshold_47;
    vx_image image_48;
    vx_remap remap_49;
    vx_scalar scalar_94;
    vx_image image_50;
    vx_image image_51;
    vx_scalar scalar_96;
    vx_threshold threshold_52;
    vx_scalar scalar_98;
    vx_scalar scalar_99;
    vx_image image_53;
    vx_image image_54;
    vx_pyramid pyramid_55;
    vx_image image_56;
    vx_image image_57;

    vx_node node_58;
    vx_node node_60;
    vx_node node_62;
    vx_node node_63;
    vx_node node_64;
    vx_node node_65;
    vx_node node_66;
    vx_node node_67;
    vx_node node_68;
    vx_node node_69;
    vx_node node_70;
    vx_node node_72;
    vx_node node_75;
    vx_node node_77;
    vx_node node_78;
    vx_node node_79;
    vx_node node_80;
    vx_node node_82;
    vx_node node_83;
    vx_node node_84;
    vx_node node_85;
    vx_node node_86;
    vx_node node_88;
    vx_node node_90;
    vx_node node_92;
    vx_node node_93;
    vx_node node_95;
    vx_node node_97;
    vx_node node_100;
    vx_node node_101;
    vx_node node_102;

} uc_superset1_t;

vx_status uc_superset1_data_create(uc_superset1 usecase);
vx_status uc_superset1_data_delete(uc_superset1 usecase);

vx_status uc_superset1_graph_0_create(uc_superset1 usecase);
vx_status uc_superset1_graph_0_delete(uc_superset1 usecase);
vx_status uc_superset1_graph_0_verify(uc_superset1 usecase);
vx_status uc_superset1_graph_0_run(uc_superset1 usecase);

vx_status uc_superset1_create(uc_superset1 usecase);
vx_status uc_superset1_delete(uc_superset1 usecase);
vx_status uc_superset1_verify(uc_superset1 usecase);
vx_status uc_superset1_run(uc_superset1 usecase);

#endif /* UC_SUPERSET1 */


