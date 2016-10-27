'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ArrayCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)

    def declare_var(self, code_gen) :
       code_gen.write_line('vx_array %s;' % self.ref.name)


    def call_create(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("usecase->%s = vxCreateArray(context, %s, %d);" % (self.ref.name, Type.getvx_name(ref.item_type), ref.capacity));
        code_gen.write_line("if (usecase->%s == NULL)" % (self.ref.name));
        code_gen.write_open_brace()
        code_gen.write_line("status = VX_ERROR_NO_RESOURCES;");
        code_gen.write_close_brace()
        code_gen.write_close_brace()

