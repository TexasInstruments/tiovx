'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Export objects from context to C source code
#
#
# \par Example Usage: Export objects from context to C source code
#
# \code
#
# from tiovx import *
#
# my_context = Context("my_context")
#
# ...
#
# ExportCode(my_context).export()
# \endcode
#
# \ingroup FRAMEWORK
class ExportCode (Export) :
    ## Constructor used to create this object
    #
    # \param context [in] Context object. tiovx::context::Context
    def __init__(self, context) :
        Export.__init__(self, context)
        self.usecase_code = UsecaseCode(context)

    ## Export object as C source code
    def export(self) :
        print ('Generating C code from OpenVX context ...')
        print ('Files [%s] and [%s]' % (self.context.name + '.h', self.context.name + '.c'))
        self.usecase_code.generate_code()
        print ('Generating C code from OpenVX context ... DONE !!!')

