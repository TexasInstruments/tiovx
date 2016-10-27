'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

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
        if ref.type == Type.SCALAR :
            return ScalarCode(ref)
        if ref.type == Type.GRAPH :
            return GraphCode(ref)
        if ref.type == Type.NODE :
            return NodeCode(ref)
        if ref.type == Type.ARRAY :
            return ArrayCode(ref)
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
