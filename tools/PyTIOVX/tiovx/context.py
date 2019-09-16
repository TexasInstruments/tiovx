#
# Copyright (c) 2017 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

from . import *

## Context object (OpenVX equivalent = \ref vx_context)
#
#
# \par Example Usage: Create a context
#
# \code
#
# from tiovx import *
#
# my_context = Context("my_context_name")
# my_context.add(my_graph)
# my_context.add(my_node)
# my_context.add(my_data)
# \endcode
#
# \ingroup FRAMEWORK
class Context (Reference) :
    ## Constructor used to create this object
    #
    # \see vxCreateContext for more details about the parameters
    #
    # \param name [in] Name to assign to this context
    def __init__(self, name="default") :
        Reference.__init__(self, Type.CONTEXT, name)
        self.ref = []
        self.node_list = []
        self.data_list = []
        self.graph_list = []
        self.target_list = []
        self.is_target_present = {}
        for target in Target :
            self.is_target_present[target.name] = False

    def isDuplicate(self, ref, list) :
        for l in list :
            if l == ref :
                return True
        return False

    ## Add graph or node or data object to context
    #
    # \param ref [in] Object of type Graph, Node or Data Object
    def add(self,ref) :
        if self.isDuplicate(ref, self.ref) == False :
            self.ref.append(ref)
            # add to node, data, graph list
            if( ref.type == Type.GRAPH) :
                self.addGraph(ref)
            elif( ref.type == Type.NODE) :
                self.addNode(ref)
            else :
                self.addData(ref)

    def __str__(self):
        print_str = Reference.__str__(self)
        for ref in self.ref :
            print_str = print_str + '\n' + str(ref)
        return print_str

    def addGraph(self, ref) :
        ref.__class__ = Graph
        if self.isDuplicate(ref, self.graph_list) == False :
            self.graph_list.append(ref)
            for node in ref.ref :
                self.addNode(node)

    def addData(self, ref) :
        if self.isDuplicate(ref, self.data_list) == False :
            self.data_list.append(ref)

    def addNode(self, ref) :
        ref.__class__ = Node
        if self.isDuplicate(ref, self.node_list) == False :
            self.node_list.append(ref)
            self.addTarget(ref.target)
            for data in ref.ref :
                self.addData(data)

    def addTarget(self, target) :
        # if not already added, then add it
        if self.is_target_present[target.name] == False :
            self.target_list.append(target)
            self.is_target_present[target.name] = True
