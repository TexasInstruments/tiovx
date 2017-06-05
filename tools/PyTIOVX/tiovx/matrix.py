'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Matrix object (OpenVX equivalent = vx_matrix)
#
#
# \par Example Usage: Create a matrix object of size 10x10
#
# \code
# from tiovx import *
#
# my_mat1 = Matrix(Type.UINT16, 10, 10, name="mymat")
# my_mat2 = Matrix(Type.UINT16, 10, 10)
# \endcode
#
# \ingroup DATA
class Matrix (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateMatrix for more details about the parameters
    #
    # \param data_type [in] Data type. tiovx::enums::Type
    # \param coloumns [in] Coloumns
    # \param rows [in] Rows
    # \param name [in] [optional] Name of the object
    def __init__(self, data_type, column, rows, name="default") :
        Reference.__init__(self, Type.MATRIX, name)
        self.data_type = data_type
        self.column = column
        self.rows = rows

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.data_type.name + ':' + str(self.rows) + 'x' + str(self.column) + ' ]'
