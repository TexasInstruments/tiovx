/*
* util.h
*
* Definition of some useful string comparison routines (not
* not provided on all platforms) and a few generic macros.
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

#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>

#if defined(_MSC_VER)

/*****************************************************************************/
/* STRCASECMP() - Case-insensitive strcmp.                                   */
/*****************************************************************************/
static int strcasecmp(const char* s1, const char* s2)
{
   char c1, c2;
   do { c1 = *s1++; c2 = *s2++; }
   while (c1 && c2 && (tolower(c1) == tolower(c2)));

   return tolower(c1) - tolower(c2);
}

/*****************************************************************************/
/* STRNCASECMP() - Case-insensitive strncmp.                                 */
/*****************************************************************************/
static int strncasecmp(const char* s1, const char* s2, size_t n)
{
   char c1, c2;

   if (!n) return 0;
   
   do { c1 = *s1++; c2 = *s2++; }
   while (--n && c1 && c2 && (tolower(c1) == tolower(c2)));

   return tolower(c1) - tolower(c2);
}

#endif

/*****************************************************************************/
/* Define MIN and MAX macros.                                                */
/*****************************************************************************/
#define MIN(x,y)	(((x) > (y)) ? (y) : (x))
#define MAX(x,y)	(((x) >= (y)) ? (x) : (y))

/*****************************************************************************/
/* C implementation of 'bool' type.                                          */
/*****************************************************************************/
typedef int BOOL;
#undef  TRUE
#define TRUE    1
#undef  FALSE
#define FALSE   0

#endif
