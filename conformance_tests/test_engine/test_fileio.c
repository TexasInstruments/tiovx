/*

 * Copyright (c) 2025 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdarg.h>
#include <test_utils.h>

FILE *ct_fopen(const char *filename, const char *mode)
{
    return fopen(filename, mode);
}

int ct_fclose(FILE *stream)
{
    return fclose(stream);
}

size_t ct_fread(void * ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread(ptr, size, nmemb, stream);
}

size_t ct_fwrite(const void * ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

int ct_fseek(FILE *stream, long int offset, int whence)
{
    return fseek(stream, offset, whence);
}

long int ct_ftell(FILE *stream)
{
    return ftell(stream);
}

int ct_fprintf(FILE *stream, const char *format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = vfprintf(stream, format, args);
    va_end(args);

    return ret;
}
