'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *
    
class UsecaseCode :
    def __init__(self, context) :
        self.h_file = CodeGenerate(context.name + '.h')
        self.c_file = CodeGenerate(context.name + '.c')
        self.context = context
        self.context_code = ContextCode(context)

    def generate_h_code(self) :
        self.h_file.write_ifndef_define(self.context.name.upper())
        self.h_file.write_include("VX/vx.h")
        self.h_file.write_include("TI/tivx.h")
        self.h_file.write_newline()
        self.h_file.write_line('typedef struct _%s_t' % self.context.name)
        self.h_file.write_open_brace()
        self.context_code.declare_var(self.h_file)
        self.h_file.write_close_brace(' %s_t;' % self.context.name)
        self.h_file.write_newline()
        self.h_file.write_endif(self.context.name.upper())
        self.h_file.close()

    def generate_c_code(self) :
        self.c_file.write_include(self.context.name + '.h')
        self.c_file.close()

    def generate_code(self) :
        self.generate_h_code()
        self.generate_c_code()


