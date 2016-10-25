'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Matrix (Reference) :
    def __init__(self, data_type, column, rows, name="default") :
        Reference.__init__(self, Type.MATRIX, name)
        self.data_type = data_type
        self.column = column
        self.rows = rows

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ':' + str(self.rows) + 'x' + str(self.column) + ' ]'
