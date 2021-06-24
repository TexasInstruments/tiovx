/*
* version.h
*
* Dynamic Loader source version identifictaion.
*
* Copyright (C) 2009-2014 Texas Instruments Incorporated - http://www.ti.com/
*
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the
* distribution.
*
* Neither the name of Texas Instruments Incorporated nor the names of
* its contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef _VERSION_H_
#define _VERSION_H_

/*****************************************************************************/
/* VERSION NUMBER COMPONENTS - ALWAYS INCREASING!!                           */
/* Initial version ID is 1.0.0. Successive version ID's will be incremented  */
/* by automated processes during release port.                               */
/*****************************************************************************/
#define VERSION_MAJOR     1
#define VERSION_MINOR     0
#define VERSION_PATCH     0

/******************************************************************************/
/* Macros used to convert version macros into strings.                        */
/******************************************************************************/
#define MKCSTR(_str) #_str
#define MKMSTR(_str) MKCSTR(_str)

/******************************************************************************/
/* VERSION string construction macros.                                        */
/******************************************************************************/
#define VERSTR  MKMSTR(VERSION_MAJOR) "." MKMSTR(VERSION_MINOR) "." MKMSTR(VERSION_PATCH)
#define VERSION "Texas Instruments Dynamic Loader API/Core v"VERSTR

#endif
