/******************************************************************************
 * Copyright (c) 2021, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/** \file dsp_load.h, dsp_load.c
    \brief For integrating DLOAD into PSDK C7x firmware

    Rest of the DLOAD sources are copied from OpenCL C7x development,
    in which case, DLOAD runs on Arm and loads dynamic executable for C7x.
    Here, DLOAD is running on C7x and loading dynamic executable for C7x.
    - hence the customizations in dsp_load.{h,c}
    - "dsp_syms.out" is the dummy firmware that exports firmware symbols
      and gets linked into the dynamic executable as a dependent
      executable/firmware
      - dummy firmware is built as a shared exectable (--dynamic=exe)
        named "dsp_syms.out",
      - "dsp_syms.out" is embedded as ".dsp_syms_out" section into the
        dynamic executable at build time
      - When DLOAD needs to load the dependent executable, it loads symbols
        from the ".dsp_syms_out" section.  Immediately after, DLOAD updates
        the symbols in the symbol table with real addresses in C7x firmware
    - appMem*() routines are from PSDK RTOS vision_apps
    - Once loaded, OpenVX TVM node can query and call functions in the
      dynamic executable
*/

#ifndef _DSP_LOAD_H_
#define _DSP_LOAD_H_

/** \brief Load dsp dynamic executable
 */
extern void *dspload_load_program(void *file_data, int file_size);
/** \brief Query symbol address
 */
extern void *dspload_query_symbol(void *dspload_handle, const char *sym_name);
/** \brief UnLoad dsp dynamic executable
 */
extern void  dspload_unload_program(void *dspload_handle);

#endif  /* _DSP_LOAD_H_ */
