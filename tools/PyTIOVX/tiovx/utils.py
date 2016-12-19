'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

def toCamelCase(input_str):
    output = ''.join(x for x in input_str.title() if x.isalpha())
    return output[0].title() + output[1:]
