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

    def call_create(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("usecase->%s = vxCreateScalar(context, %s, %s);" % (self.ref.name, Type.get_vx_name(self.ref.data_type), str(self.ref.value)));
        code_gen.write_line("if (usecase->%s == NULL)" % (self.ref.name));
        code_gen.write_open_brace()
        code_gen.write_line("status = VX_ERROR_NO_RESOURCES;");
        code_gen.write_close_brace()
        code_gen.write_close_brace()
