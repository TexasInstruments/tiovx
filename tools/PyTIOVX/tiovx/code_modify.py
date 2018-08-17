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
import os, re

from . import *

class CodeModify :

    def file_append(self, filename, insert) :
        self.file = open(filename,'a')
        self.file.write(insert)
        self.file.close()

    def file_search(self, in_filename, search) :
        if os.path.exists(in_filename):
            self.status = False
            with open(in_filename) as rd_file:
                for line in rd_file:
                    if search in line :
                        return True
            return False

    def block_search(self, in_filename, start, end, search) :
        if os.path.exists(in_filename):
            self.status = False
            with open(in_filename) as rd_file:
                for line in rd_file:
                    if start in line :
                        self.multiline = line
                        self.status = True
                    elif self.status :
                        self.multiline = self.multiline + line
                        if end in line :
                            self.status = False
                            searchObj = re.findall(search, self.multiline)
                            return (searchObj[-1])

    def block_insert(self, in_filename, start, end, find, search, searchNoEsc, insert, overrideFind=False) :
        if os.path.exists(in_filename):
            self.status = False
            self.found = False
            self.include_customer_kernels_code = CodeGenerate(in_filename + ".tmp", header=False)
            self.multiline = ""
            with open(in_filename) as rd_file:
                for line in rd_file:
                    if self.status==False and start in line :
                        self.include_customer_kernels_code.write_line(self.multiline, new_line=False)
                        self.multiline = line
                        self.status = True
                        self.found = True
                    elif self.status :
                        self.multiline = self.multiline + line
                        if end in line:
                            self.status = False
                            if not find in self.multiline or overrideFind:
                                self.multiline = re.sub(search,insert + searchNoEsc, self.multiline)
                            self.include_customer_kernels_code.write_line(self.multiline, new_line=False)
                    else :
                        self.include_customer_kernels_code.write_line(line, new_line=False)
            if self.status==True and self.found==True:
                if not find in self.multiline :
                    self.multiline = re.sub(search,insert + searchNoEsc, self.multiline)
                self.include_customer_kernels_code.write_line(self.multiline, new_line=False)

            self.include_customer_kernels_code.close(new_line=False)
            os.rename(in_filename + ".tmp", in_filename)


