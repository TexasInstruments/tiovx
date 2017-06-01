'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## \defgroup PYTIOVX_API PyTIOVX APIs
#

## \defgroup ENUMS 1: Enum APIs
#
# \ingroup PYTIOVX_API

## \defgroup DATA 2: Data Object APIs
#
# \ingroup PYTIOVX_API

## \defgroup FRAMEWORK 3: Framework Object APIs
#
# \ingroup PYTIOVX_API

## \defgroup KERNELS 4: Kernel / Node APIs
#
# \ingroup PYTIOVX_API

## Reference object (OpenVX equivalent = vx_reference)
#
# NOT used by user directly.
# Other objects inherit from this class
#
# \ingroup FRAMEWORK
class Reference :
    id = 0

    def make_default_name(type) :
        name = type.name.lower() + '_' + str(Reference.id)
        Reference.id = Reference.id + 1
        return name

    def setName(self, name) :
        if ( name == "default" ) :
            self.name = Reference.make_default_name(self.type)
        else :
            self.name = name

    def __init__(self, type, name) :
        self.type = type
        self.setName(name)

    def __str__(self):
        return self.name + ' ( ' + self.type.name + ' )'

