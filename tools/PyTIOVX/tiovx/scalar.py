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

## Scalar object (OpenVX equivalent = \ref vx_scalar)
#
# \par Example Usage: Create a Scalar object
#
# \code
# from tiovx import *
#
# my_scalar1 = Scalar(Type.UINT8, 8, name="myscalar")
# my_scalar2 = Scalar(Type.FLOAT32, 0.8)
# my_scalar3 = Scalar(Type.CHAR, 'c')
# \endcode
#
# \ingroup DATA
class Scalar (Reference) :
    ## Constructor used to create this object
    #
    # \see vxCreateScalar for more details about the parameters
    #
    # \param data_type [in] Data type. tiovx::enums::Type
    # \param value [in] Value of type 'data_type'
    # \param name [in] [optional] Name of the object
    def __init__(self, data_type, value="0", name="default") :
        Reference.__init__(self, Type.SCALAR, name)
        self.data_type = data_type
        self.value = value
        self.check_value()
        self.get_value_str()

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ' ]'

    def check_value(self) :
        if self.data_type == Type.CHAR :
            return
        if self.data_type == Type.INT8 :
            assert ( self.value > -127 and self.value <= 128)
            return
        if self.data_type == Type.UINT8 :
            assert ( self.value >= 0 and self.value <= 255)
            return

    def get_value_str(self):
        if self.data_type == Type.CHAR :
            return "'" + str(self.value) + "'"
        if self.data_type == Type.INT8 :
            return str(self.value)
        if self.data_type == Type.UINT8 :
            return str(self.value)
        if self.data_type == Type.INT16 :
            return str(self.value)
        if self.data_type == Type.UINT16 :
            return str(self.value)
        if self.data_type == Type.INT32 :
            return str(self.value)
        if self.data_type == Type.UINT32 :
            return str(self.value)
        if self.data_type == Type.INT64 :
            return str(self.value)
        if self.data_type == Type.UINT64 :
            return str(self.value)
        if self.data_type == Type.FLOAT32 :
            return str(self.value)
        if self.data_type == Type.FLOAT64 :
            return str(self.value)
        if self.data_type == Type.ENUM:
            return str(self.value.get_vx_enum_name())
        if self.data_type == Type.SIZE:
            return str(self.value)
        if self.data_type == Type.DF_IMAGE:
            return str(self.value.get_vx_enum_name())
        if self.data_type == Type.BOOL:
            return "vx_" + str(self.value).lower() + "_e"
