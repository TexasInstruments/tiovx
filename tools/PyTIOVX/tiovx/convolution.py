'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Convolution object (OpenVX equivalent = vx_convolution)
#
#
# \par Example Usage: Create a convolution object of 10 rows, 15 coloumns
#
# \code
# from tiovx import *
#
# my_conv1 = Convolution(10, 15, name="myconvolution")
# my_conv2 = Convolution(10, 15)
# \endcode
#
# \ingroup DATA
class Convolution (Reference) :
    ## Constructor used to create this object
    #
    # \param coloumns [in] Number of coloumns
    # \param rows [in] Number of rows
    # \param name [in] [optional] Name of the object
    def __init__(self, columns, rows, name="default") :
        Reference.__init__(self, Type.CONVOLUTION, name)
        self.columns = columns
        self.rows = rows

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.columns) + ', ' + str(self.rows) + ' ]'

