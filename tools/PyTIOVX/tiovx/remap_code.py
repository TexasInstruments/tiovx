'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class RemapCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)

    def declare_var(self, code_gen) :
       code_gen.write_line('vx_remap %s;' % self.ref.name)

    def call_create(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("usecase->%s = vxCreateRemap(context, %d, %d, %d, %d);" % (self.ref.name, self.ref.src_width, self.ref.src_height, self.ref.dst_width, self.ref.dst_height));
        code_gen.write_line("if (usecase->%s == NULL)" % (self.ref.name));
        code_gen.write_open_brace()
        code_gen.write_line("status = VX_ERROR_NO_RESOURCES;");
        code_gen.write_close_brace()
        code_gen.write_close_brace()

