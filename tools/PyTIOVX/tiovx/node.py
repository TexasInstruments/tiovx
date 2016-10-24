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
    
    def setNumInOut(self, num_in, num_out) :
        self.num_in = num_in
        self.num_out = num_out

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
        self.setNumInOut(2, 1)
        self.setTarget(target)

