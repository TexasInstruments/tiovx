'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class PyramidCode :
    def __init__(self, ref) :
        self.ref = ref

    def declare_var(self, code_gen) :
       code_gen.write_line('vx_pyramid %s;' % self.ref.name)

    def call_create(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("usecase->%s = vxCreatePyramid(context, %d, %d, %d, %d, %s);"
            % (self.ref.name, self.ref.num_levels, self.ref.scale, self.ref.width, self.ref.height, DfImage.get_vx_enum_name(self.ref.format)));
        code_gen.write_line("if (usecase->%s == NULL)" % (self.ref.name));
        code_gen.write_open_brace()
        code_gen.write_line("status = VX_ERROR_NO_RESOURCES;");
        code_gen.write_close_brace()
        code_gen.write_close_brace()

