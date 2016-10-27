'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ExportCode (Export) :
    def __init__(self, context) :
        Export.__init__(self, context)
        self.usecase_code = UsecaseCode(context)

    def export(self) :
        print ('Generating C code from OpenVX context ...')
        print ('Files [%s] and [%s]' % (self.context.name + '.h', self.context.name + '.c'))
        self.usecase_code.generate_code()
        print ('Generating C code from OpenVX context ... DONE !!!')

