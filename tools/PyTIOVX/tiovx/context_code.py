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

class ContextCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)
        self.data_code_list = []
        self.graph_code_list = []
        self.node_code_list = []
        for data in self.ref.data_list :
            self.data_code_list.append( ContextCode.get_data_code_obj(data) )
        for graph in self.ref.graph_list :
            self.graph_code_list.append( GraphCode(graph) )
        for node in self.ref.node_list :
            self.node_code_list.append( NodeCode(node) )

    def get_data_code_obj(ref) :
        if ref.type == Type.IMAGE :
            return ImageCode(ref)
        if ref.type == Type.LUT :
            return LutCode(ref)
        if ref.type == Type.CONVOLUTION :
            return ConvolutionCode(ref)
        if ref.type == Type.DISTRIBUTION :
            return DistributionCode(ref)
        if ref.type == Type.MATRIX :
            return MatrixCode(ref)
        if ref.type == Type.REMAP :
            return RemapCode(ref)
        if ref.type == Type.THRESHOLD :
            return ThresholdCode(ref)
        if ref.type == Type.PYRAMID :
            return PyramidCode(ref)
        if ref.type == Type.OBJECT_ARRAY :
            return ObjectArrayCode(ref)
        if ref.type == Type.TENSOR :
            return TensorCode(ref)
        if ref.type == Type.USER_DATA_OBJECT :
            return UserDataObjectCode(ref)
        if ref.type == Type.SCALAR :
            return ScalarCode(ref)
        if ref.type == Type.GRAPH :
            return GraphCode(ref)
        if ref.type == Type.NODE :
            return NodeCode(ref)
        if ref.type == Type.ARRAY :
            return ArrayCode(ref)
        if ref.type == Type.NULL :
            return NullCode(ref)
        return None

    def declare_var(self, code_gen) :
        code_gen.write_line('vx_context context;')
        code_gen.write_newline()
        for graph_code in self.graph_code_list :
             graph_code.declare_var(code_gen)
        code_gen.write_newline()
        for data_code in self.data_code_list :
             data_code.declare_var(code_gen)
        code_gen.write_newline()
        for node_code in self.node_code_list :
             node_code.declare_var(code_gen)
        code_gen.write_newline()
