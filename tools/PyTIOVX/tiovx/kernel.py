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

from . import *

class KernelParams :
    def __init__(self, index, type, direction, state, name, data_types=[],  do_map=True, do_unmap=True, do_map_unmap_all_planes=False):
        self.index = index
        self.type = type
        self.direction = direction
        self.state = state
        self.name_upper = name.upper()
        self.name_lower = name.lower()
        self.name_camel = toCamelCase(name)
        self.data_types = data_types
        self.do_map = do_map
        self.do_unmap = do_unmap
        self.do_map_unmap_all_planes = do_map_unmap_all_planes
        if Type.is_scalar_type(type) :
            self.do_map = False;
            self.do_unmap = False;

    def __str__(self):
        return "Param " + str(self.index) + ": " + self.name_upper + " " + Type.get_vx_enum_name(self.type) + " " + Direction.get_vx_enum_name(self.direction) + " " + ParamState.get_vx_enum_name(self.state)

class KernelParamRelationship :
    def __init__(self, prm_list, attribute_list, type, state):
        self.attribute_list = attribute_list
        self.prm_list = prm_list
        self.type = type
        self.state = state

    def __str__(self):
        return "Attribute " + self.attribute_list + ": " + self.prm_list + " " + type + " " + state

class Kernel  :
    def __init__(self, name="default") :
        self.name_lower = name.lower()
        self.name_upper = name.upper()
        self.name_camel = toCamelCase(name)
        self.index = 0;
        self.params = []
        self.name_str_prefix = "com.ti."
        self.enum_str_prefix = "TIVX_KERNEL_"
        self.targets = []
        self.relationship_list = []
        self.relationship_list_index = 0

    def setKernelPrefix(self, name_str_prefix, enum_str_prefix) :
        self.name_str_prefix = name_str_prefix
        self.enum_str_prefix = enum_str_prefix

    def setTarget(self, target) :
        self.targets.append(target)

    def __str__(self) :
        kernel_str = "Kernel: " + self.name_lower + " "+ self.name_upper + " "+ self.name_camel + "\n"
        kernel_str += "Targets: "
        for target in self.targets :
            kernel_str += Target.get_vx_enum_name(target) + "(CPU: " + Cpu.get_vx_enum_name(Target.get_cpu(target)) + ") "
        kernel_str += "\n"
        for prm in self.params :
            kernel_str += str(prm) + "\n"
        return kernel_str

    def setParameter(self, type, direction, state, name, data_types=[], do_map=True, do_unmap=True, do_map_unmap_all_planes=False):
        params = KernelParams(self.index, type, direction, state, name, data_types, do_map, do_unmap, do_map_unmap_all_planes);
        self.params.append(params)
        self.index = self.index + 1

    def getNumImages(self) :
        num_images = 0
        for prm in self.params :
            if prm.type == Type.IMAGE :
                num_images += 1
        return num_images

    def getNumScalars(self) :
        num_scalars = 0
        for prm in self.params :
            if Type.is_scalar_type(prm.type) is True :
                num_scalars += 1
        return num_scalars

    def getNumInputImages(self) :
        num_input_images = 0
        for prm in self.params :
            if prm.type == Type.IMAGE and prm.direction == Direction.INPUT:
                num_input_images += 1
        return num_input_images

    def getNumOutputImages(self) :
        num_output_images = 0
        for prm in self.params :
            if prm.type == Type.IMAGE and prm.direction == Direction.OUTPUT:
                num_output_images += 1
        return num_output_images

    def setParameterRelationship(self, name_list=[], attribute_list=["all"], type="equal") :
        assert len(name_list) > 1, "There should be more than 1 parameter in name_list"
        prm_list = []
        required = 0
        optional = 0

        # Get params from names
        for name in name_list :
            found = False
            for prm in self.params :
                if name.lower() == prm.name_lower :
                    prm_list.append(prm)
                    if prm.state == ParamState.REQUIRED :
                        required += 1
                    elif prm.state == ParamState.OPTIONAL :
                        optional += 1
                    found = True
                    break
            assert found == True, "'%s' was not found in parameter list" % name

        # Divide list into multiple lists depending on if some of the parameters are optional or not
        first_required = 0
        if required > 1 :
            if optional == 0 :
                relationship = KernelParamRelationship(prm_list, attribute_list, type, ParamState.REQUIRED)
            else :
                sub_prm_list = []
                for prm in prm_list :
                    if prm.state == ParamState.REQUIRED :
                        if first_required == 0 :
                            first_required = prm
                        sub_prm_list.append(prm)
                        prm_list.remove(prm)
                relationship = KernelParamRelationship(sub_prm_list, attribute_list, type, ParamState.REQUIRED)
            self.relationship_list.append(relationship)
            self.relationship_list_index = self.relationship_list_index + 1
        if required > 0 :
            while optional > 0 :
                optional -= 1
                sub_prm_list = []
                for prm in prm_list :
                    if prm.state == ParamState.OPTIONAL :
                        sub_prm_list.append(prm)
                        sub_prm_list.append(first_required)
                        relationship = KernelParamRelationship(sub_prm_list, attribute_list, type, ParamState.OPTIONAL)
                        self.relationship_list.append(relationship)
                        self.relationship_list_index = self.relationship_list_index + 1
                        prm_list.remove(prm)
                        break
