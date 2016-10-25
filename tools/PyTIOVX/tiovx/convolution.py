'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Convolution (Reference) :
    def __init__(self, columns, rows, name="default") :
        Reference.__init__(self, Type.CONVOLUTION, name)
        self.columns = columns
        self.rows = rows

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.columns) + ', ' + str(self.rows) + ' ]'

