'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ScalarCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)

    def declare_var(self, code_gen) :
        code_gen.write_line('vx_scalar %s;' % self.ref.name)
