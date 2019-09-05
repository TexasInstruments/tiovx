#
# Copyright (c) 2017 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

from enum import Enum
from . import *



## Extension of Target class
#
# Used as parameter when creating a node
# \ingroup ENUMS
#
class TargetExtended(Enum) :
    ## Below are J7 targets
    ## TIOVX equivalent = TIVX_TARGET_VPAC_NF
    VPAC_NF      = 12
    ## TIOVX equivalent = TIVX_TARGET_VPAC_LDC1
    VPAC_LDC1    = 13
    ## TIOVX equivalent = TIVX_TARGET_VPAC_LDC2
    VPAC_LDC2    = 14
    ## TIOVX equivalent = TIVX_TARGET_VPAC_MSC1
    VPAC_MSC1    = 15
    ## TIOVX equivalent = TIVX_TARGET_VPAC_MSC2
    VPAC_MSC2    = 16
    ## TIOVX equivalent = TIVX_TARGET_DMPAC_SDE
    DMPAC_SDE    = 17
    ## TIOVX equivalent = TIVX_TARGET_DMPAC_DOF
    DMPAC_DOF    = 18
    ## TIOVX equivalent = TIVX_TARGET_VPAC_VISS1
    VPAC_VISS1   = 19
    ## TIOVX equivalent = TIVX_TARGET_VDEC
    VDEC         = 20
    ## Above are J7 targets

    def get_vx_enum_name(type) :
        return "TIVX_TARGET_" + type.name

    def get_target_folder_name(type) :
        if target == TargetExtended.VPAC_NF :
            return "vpac_nf"
        if target == TargetExtended.VPAC_LDC1 :
            return "vpac_ldc"
        if target == TargetExtended.VPAC_LDC2 :
            return "vpac_ldc"
        if target == TargetExtended.VPAC_MSC1 :
            return "vpac_msc"
        if target == TargetExtended.VPAC_MSC2 :
            return "vpac_msc"
        if target == TargetExtended.DMPAC_SDE :
            return "dmpac_sde"
        if target == TargetExtended.DMPAC_DOF :
            return "dmpac_sde"
        if target == TargetExtended.VPAC_VISS1 :
            return "vpac_viss"
        if target == TargetExtended.VDEC :
            return "vdec"
        return None

    def is_j6_target(target) :
        return False

    def get_cpu(target) :
        if target == TargetExtended.VPAC_NF :
            return Cpu.IPU1_0
        if target == TargetExtended.VPAC_LDC1 :
            return Cpu.IPU1_0
        if target == TargetExtended.VPAC_LDC2 :
            return Cpu.IPU1_0
        if target == TargetExtended.VPAC_MSC1 :
            return Cpu.IPU1_0
        if target == TargetExtended.VPAC_MSC2 :
            return Cpu.IPU1_0
        if target == TargetExtended.DMPAC_SDE :
            return Cpu.IPU1_0
        if target == TargetExtended.DMPAC_DOF :
            return Cpu.IPU1_0
        if target == TargetExtended.VPAC_VISS1 :
            return Cpu.IPU1_0
        if target == TargetExtended.VDEC :
            return Cpu.IPU1_0

        return CpuExtended.INVALID

class CpuExtended(Enum) :
    ## Below are J7 targets
    ## TIOVX equivalent = TIVX_TARGET_R5F
    DEFAULT      = 1
    ## Above are J7 targets

    def get_vx_enum_name(type) :
        return "TIVX_CPU_ID_" + type.name
