/*
 *
 * Copyright (c) 2017-2021 Texas Instruments Incorporated
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

#ifndef TIVX_HWA_HOST_PRIV_H_
#define TIVX_HWA_HOST_PRIV_H_

#include <TI/tivx.h>

#ifdef BUILD_CAPTURE
vx_status tivxAddKernelCapture(vx_context context);
vx_status tivxRemoveKernelCapture(vx_context context);
#endif

#ifdef BUILD_CSITX
vx_status tivxAddKernelCsitx(vx_context context);
vx_status tivxRemoveKernelCsitx(vx_context context);
#endif

#ifdef BUILD_DISPLAY
vx_status tivxAddKernelDisplay(vx_context context);
vx_status tivxAddKernelDisplayM2M(vx_context context);
vx_status tivxRemoveKernelDisplay(vx_context context);
vx_status tivxRemoveKernelDisplayM2M(vx_context context);
#endif

#ifdef BUILD_VPAC_VISS
vx_status tivxAddKernelVpacViss(vx_context context);
vx_status tivxRemoveKernelVpacViss(vx_context context);
#endif

#ifdef BUILD_VPAC_LDC
vx_status tivxAddKernelVpacLdc(vx_context context);
vx_status tivxRemoveKernelVpacLdc(vx_context context);
#endif

#ifdef BUILD_VPAC_MSC
vx_status tivxAddKernelVpacMscMultiScale(vx_context context);
vx_status tivxAddKernelVpacMscPyramid(vx_context context);
vx_status tivxRemoveKernelVpacMscMultiScale(vx_context context);
vx_status tivxRemoveKernelVpacMscPyramid(vx_context context);
#endif

#ifdef BUILD_VPAC_NF
vx_status tivxAddKernelVpacNfGeneric(vx_context context);
vx_status tivxAddKernelVpacNfBilateral(vx_context context);
vx_status tivxRemoveKernelVpacNfGeneric(vx_context context);
vx_status tivxRemoveKernelVpacNfBilateral(vx_context context);
#endif

#ifdef BUILD_DMPAC_DOF
vx_status tivxAddKernelDmpacDof(vx_context context);
vx_status tivxAddKernelDofVisualize(vx_context context);
vx_status tivxRemoveKernelDmpacDof(vx_context context);
vx_status tivxRemoveKernelDofVisualize(vx_context context);
#endif

#ifdef BUILD_DMPAC_SDE
vx_status tivxAddKernelDmpacSde(vx_context context);
vx_status tivxRemoveKernelDmpacSde(vx_context context);
#endif

vx_status tivxAddKernelObjArraySplit(vx_context context);
vx_status tivxRemoveKernelObjArraySplit(vx_context context);

#endif /* TIVX_HWA_HOST_PRIV_H_ */
