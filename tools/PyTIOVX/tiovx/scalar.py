'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Scalar (Reference) :
    def __init__(self, data_type, name="default") :
        Reference.__init__(self, Type.SCALAR, name)
        self.data_type = data_type

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ' ]'
