'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ReferenceCode :
    def __init__(self, ref) :
        self.ref = ref

    def declare_var(self, code_gen) :
       code_gen.write_line('vx_reference %s;' % self.ref.name)
       code_gen.write_newline()

    def set_ref_name(self, code_gen) :
        code_gen.write_line('vxSetReferenceName( (vx_reference)usecase->%s, "%s");' % (self.ref.name, self.ref.name))

