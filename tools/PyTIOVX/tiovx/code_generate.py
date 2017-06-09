'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class CodeGenerate :
    def __init__(self, filename) :
        self.indent_level = 0
        self.indent = "    "
        self.filename = filename
        self.open()

    def open(self) :
        self.file = open(self.filename,'w')
        self.write_header()

    def close(self) :
        self.write_newline()
        self.file.close()

    def write_header(self) :
        self.write_line('/*')
        self.write_line(' *')
        self.write_line(' * Copyright (c) 2017 Texas Instruments Incorporated')
        self.write_line(' *')
        self.write_line(' * All rights reserved not granted herein.')
        self.write_line(' *')
        self.write_line(' * Limited License.')
        self.write_line(' *')
        self.write_line(' * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive')
        self.write_line(' * license under copyrights and patents it now or hereafter owns or controls to make,')
        self.write_line(' * have made, use, import, offer to sell and sell ("Utilize") this software subject to the')
        self.write_line(' * terms herein.  With respect to the foregoing patent license, such license is granted')
        self.write_line(' * solely to the extent that any such patent is necessary to Utilize the software alone.')
        self.write_line(' * The patent license shall not apply to any combinations which include this software,')
        self.write_line(' * other than combinations with devices manufactured by or for TI ("TI Devices").')
        self.write_line(' * No hardware patent is licensed hereunder.')
        self.write_line(' *')
        self.write_line(' * Redistributions must preserve existing copyright notices and reproduce this license')
        self.write_line(' * (including the above copyright notice and the disclaimer and (if applicable) source')
        self.write_line(' * code license limitations below) in the documentation and/or other materials provided')
        self.write_line(' * with the distribution')
        self.write_line(' *')
        self.write_line(' * Redistribution and use in binary form, without modification, are permitted provided')
        self.write_line(' * that the following conditions are met:')
        self.write_line(' *')
        self.write_line(' * *       No reverse engineering, decompilation, or disassembly of this software is')
        self.write_line(' * permitted with respect to any software provided in binary form.')
        self.write_line(' *')
        self.write_line(' * *       any redistribution and use are licensed by TI for use only with TI Devices.')
        self.write_line(' *')
        self.write_line(' * *       Nothing shall obligate TI to provide you with source code for the software')
        self.write_line(' * licensed and provided to you in object code.')
        self.write_line(' *')
        self.write_line(' * If software source code is provided to you, modification and redistribution of the')
        self.write_line(' * source code are permitted provided that the following conditions are met:')
        self.write_line(' *')
        self.write_line(' * *       any redistribution and use of the source code, including any resulting derivative')
        self.write_line(' * works, are licensed by TI for use only with TI Devices.')
        self.write_line(' *')
        self.write_line(' * *       any redistribution and use of any object code compiled from the source code')
        self.write_line(' * and any resulting derivative works, are licensed by TI for use only with TI Devices.')
        self.write_line(' *')
        self.write_line(' * Neither the name of Texas Instruments Incorporated nor the names of its suppliers')
        self.write_line(' *')
        self.write_line(' * may be used to endorse or promote products derived from this software without')
        self.write_line(' * specific prior written permission.')
        self.write_line(' *')
        self.write_line(' * DISCLAIMER.')
        self.write_line(' *')
        self.write_line(' * THIS SOFTWARE IS PROVIDED BY TI AND TI\'S LICENSORS "AS IS" AND ANY EXPRESS')
        self.write_line(' * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES')
        self.write_line(' * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.')
        self.write_line(' * IN NO EVENT SHALL TI AND TI\'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,')
        self.write_line(' * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,')
        self.write_line(' * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,')
        self.write_line(' * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY')
        self.write_line(' * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE')
        self.write_line(' * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED')
        self.write_line(' * OF THE POSSIBILITY OF SUCH DAMAGE.')
        self.write_line(' *')
        self.write_line(' */')
        self.write_newline()

    def write_comment_line(self, text_line) :
        self.write_line('/* %s */' % text_line)

    def write_line(self, text_line) :
        for i in range(0, self.indent_level) :
            self.file.write(self.indent)
        self.file.write(text_line)
        self.file.write('\n')

    def write_open_brace(self) :
        self.write_line('{')
        self.indent_level = self.indent_level+1

    def write_close_brace(self, text="") :
        self.indent_level = self.indent_level-1
        self.write_line('}%s' % text)

    def write_include(self, include_file_name) :
        self.write_line('#include "%s"' % include_file_name)

    def write_ifndef_define(self, text) :
        self.write_line('#ifndef %s' % text)
        self.write_line('#define %s' % text)
        self.write_newline()

    def write_endif(self, text) :
        self.write_line('#endif /* %s */' % text)
        self.write_newline()

    def write_newline(self) :
        self.write_line('')

    def write_define_status(self) :
        self.write_line("vx_status status = VX_SUCCESS;")

    def write_if_status(self) :
        self.write_line("if (status == VX_SUCCESS)");


