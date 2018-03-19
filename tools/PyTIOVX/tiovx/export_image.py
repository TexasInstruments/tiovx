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
import subprocess

## Export objects from context to JPG image file
#
# NOTE: This 'dot' tool to be installed.
#
# \par Example Usage: Export objects from context to JPG image file
#
# \code
#
# from tiovx import *
#
# my_context = Context("my_context")
#
# ...
#
# ExportImage(my_context).export()
# \endcode
#
# \ingroup FRAMEWORK
class ExportImage (Export) :
    ## Constructor used to create this object
    #
    # \param context [in] Context object. tiovx::context::Context
    def __init__(self, context) :
        Export.__init__(self, context)
        self.filename_prefix = context.name
        self.filename = context.name + "_img.txt"
        self.filenameJpg = context.name + ".jpg"
        self.file = None

    def getDataColor(self, ref) :
        return "GhostWhite"

    def getTargetColor(self, target) :
        if target == Target.DSP1 :
            return "palegreen"
        if target == Target.DSP2 :
            return "darkturquoise"
        if target == Target.EVE1 :
            return "yellow"
        if target == Target.EVE2 :
            return "gold"
        if target == Target.EVE3 :
            return "orange"
        if target == Target.EVE4 :
            return "goldenrod4"
        if target == Target.A15_0 :
            return "lightblue"
        if target == Target.IPU1_0 :
            return "grey"
        if target == Target.IPU1_1 :
            return "LightSalmon"
        if target == Target.IPU2 :
            return "MediumOrchid"
        return "white"

    def __str__(self) :
        file = open(self.filename,'r')
        str = 'Export Image [' + self.filename + ' ]\n'
        str += file.read()
        file.close()
        return str

    def outputTarget(self, target) :
        self.file.write('        <TR><TD bgcolor="%s">%s</TD></TR>' % (self.getTargetColor(target), target.name))

    def outputTargetList(self) :
        self.file.write('  ColorScheme [shape=none, margin=0, label=<\n')
        self.file.write('        <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n')
        for target in self.context.target_list :
            self.outputTarget(target)
        self.file.write('        </TABLE>>];\n')
        self.file.write('\n')
        self.file.write('\n')

    def outputData(self, data) :
        self.file.write('  %s [color=%s, style=filled]\n' % (data.name, self.getDataColor(data)))

    def outputDataList(self) :
        self.file.write('\n')
        self.file.write('  /* DATA OBJECTS */\n')
        for ref in self.context.data_list :
            self.outputData(ref)
        self.file.write('\n')

    def outputNode(self, node) :
        self.file.write('  %s [label=\"%s (%s)\", color=%s, style=filled]\n' % (node.name, node.name, node.kernel, self.getTargetColor(node.target)) )

    def outputNodeList(self) :
        self.file.write('\n')
        self.file.write('  /* NODE OBJECTS */\n')
        for ref in self.context.node_list :
            self.outputNode(ref)
        self.file.write('\n')

    def outputNodeConnection(self, node) :
        idx = 0
        for dir in node.param_dir :
            if dir == Direction.INPUT :
                self.file.write('  %s -> %s [taillabel=%d, labeldistance=3]\n' % (node.ref[idx].name, node.name, idx))
            else :
                self.file.write('  %s -> %s [headlabel=%d, labeldistance=3]\n' % (node.name, node.ref[idx].name, idx))
            idx = idx + 1

    def outputNodeConnectionList(self) :
        self.file.write('\n')
        self.file.write('  /* NODE CONNECTIONS */\n')
        for node in self.context.node_list :
            self.outputNodeConnection(node)
        self.file.write('\n')

    ## Export object as C source code
    #
    def export(self) :
        print ('Generating image from OpenVX context ...')
        print ('dot tool input  file is [%s] ...' % self.filename)
        print ('dot tool output file is [%s] ...' % self.filenameJpg)

        self.file = open(self.filename, 'w')
        self.file.write('digraph %s {\n' % self.context.name)
        self.file.write('\n')
        self.file.write('  label = \"%s\"\n' % self.context.name)
        self.outputTargetList()
        self.outputDataList()
        self.outputNodeList()
        self.outputNodeConnectionList()
        self.file.write('\n')
        self.file.write('}\n')
        self.file.close()

        try :
            command_str = 'dot %s -Tjpg -o%s' % (self.filename, self.filenameJpg)
            command_args = ['dot', self.filename, '-Tjpg','-o%s' % self.filenameJpg] 
            print('Executing dot tool command ... [' + command_str + ']')
            subprocess.call(command_args)
            print ('Generating image from OpenVX context ... DONE !!!')
        except FileNotFoundError:
            print('ERROR: \'dot\' tool not found. Make sure \'graphviz\' is installed and \'dot\' command is added to system PATH !!!')
            print('ERROR: Cannot generate .jpg file !!!')

