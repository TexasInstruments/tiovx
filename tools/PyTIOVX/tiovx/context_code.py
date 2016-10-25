'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ContextCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)
        self.data_code_list = []
        for data in self.ref.data_list :
            self.data_code_list.append( ContextCode.create(data) )

    def create(ref) :
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
        if ref.type == Type.OBJECTARRAY :
            return ObjectArrayCode(ref)
        return None

    def declare_var(self, code_gen) :
        code_gen.write_line('vx_context %s;' % self.ref.name)
        code_gen.write_newline()
        for data_code in self.data_code_list :
             data_code.declare_var(code_gen)
        code_gen.write_newline()
