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

## \defgroup PYTIOVX_API PyTIOVX APIs
# \brief TI Python APIs for autogenerating OpenVX application and kernel plugin code.
#

## \defgroup ENUMS 1: Enum APIs
# \brief Classes corresponding to OpenVX equvalent = \ref vx_enum_e
# \ingroup PYTIOVX_API

## \defgroup ATTRIBUTES 2: Attribute APIs
# \brief Classes corresponding to attribute enumerations of the data types.
# \ingroup PYTIOVX_API

## \defgroup DATA 3: Data Object APIs
# \brief Classes corresponding to OpenVX data object references.
# \ingroup PYTIOVX_API

## \defgroup FRAMEWORK 4: Framework Object APIs
# \brief Classes corresponding to OpenVX framework object references.
# \ingroup PYTIOVX_API

## \defgroup KERNEL_CODE 5: Kernel Code APIs
# \brief Classes corresponding to generating target kernel plugins for a custom kernel.
# \ingroup PYTIOVX_API

## \defgroup KERNEL 6: Kernel APIs
# \brief Classes corresponding to configuring a custom kernel for code generation.
# \ingroup PYTIOVX_API

## \defgroup NODE 7: Node APIs
# \brief Classes corresponding to OpenVX standard Vision Kernel Node APIs.
# \ingroup PYTIOVX_API

# Reference object (OpenVX equivalent = vx_reference)
#
# NOT used by user directly.
# Other objects inherit from this class
#
# \ingroup FRAMEWORK
class Reference :
    id = 0

    def make_default_name(type) :
        name = type.name.lower() + '_' + str(Reference.id)
        Reference.id = Reference.id + 1
        return name

    def setName(self, name) :
        if ( name == "default" ) :
            self.name = Reference.make_default_name(self.type)
        else :
            self.name = name

    def __init__(self, type, name) :
        self.type = type
        self.setName(name)

    def __str__(self):
        return self.name + ' ( ' + self.type.name + ' )'

