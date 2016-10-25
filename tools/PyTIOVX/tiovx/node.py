'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class Node (Reference) :
    def __init__(self, kernel, *args) :
        Reference.__init__(self, Type.NODE, "default")
        self.kernel = kernel
        self.ref = []
        self.target = Target.DEFAULT
        for arg in args :
            self.ref.append(arg)
        self.num_in = 0
        self.num_out = 0

    def checkParams(self, *param_type_args) :
        assert (len(param_type_args) == (self.num_in + self.num_out)), 'Expected %d arguments but %d provided' % (len(param_type_args), (self.num_in + self.num_out))
        for i in range(0, len(param_type_args)) :
            assert (self.ref[i].type == param_type_args[i]), 'Parameter %d: Expected %s but %s is provided' % (i, param_type_args[i], self.ref[i].type)

    def setParams(self, num_in, num_out, *param_type_args) :
        self.num_in = num_in
        self.num_out = num_out
        self.checkParams(*param_type_args)

    def setTarget(self, target):
        self.target = target

    def __str__(self):
        print_str = Reference.__str__(self) + ' [ ' + self.kernel + ' ] '
        idx = 0
        for ref in self.ref :
            print_str = print_str + '\n' + str(idx) + ': ' + str(ref)
            idx = idx + 1
        return print_str

class NodeAbsDiff (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.absdiff", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Inputs and Output MUST have same image data format"

