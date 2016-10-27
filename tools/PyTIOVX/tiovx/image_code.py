'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ImageCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)

    def declare_var(self, code_gen) :
        code_gen.write_line('vx_image %s;' % self.ref.name)

    def call_create(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("status = vxCreateImage(context, %d, %d, %s);" % (ref.width, ref.height, DfImage.get_vx_name(ref.df_format)));
        code_gen.write_close_brace();
