# Copyright (C) 2013 Texas Instruments
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(OS),Windows_NT)
    HOST_OS=Windows_NT
	ifeq ($(TERM),cygwin)
        HOST_OS=CYGWIN
	endif
else
    OS=$(shell uname -s)
    ifeq ($(OS),Linux)
        HOST_OS=LINUX
        HOST_NUM_CORES := $(shell cat /proc/cpuinfo | grep processor | wc -l)
    endif
endif

# PATH_CONV and set HOST_COMPILER if not yet specified
ifeq ($(HOST_OS),Windows_NT)
    STRING_ESCAPE=$(subst \,\\,$(1))
    PATH_CONV=$(subst /,\,$(1))
    PATH_SEP=\\
    PATH_SEPD=$(strip \)
    HOST_COMPILER?=CL
else
    STRING_ESCAPE=$(1)
    PATH_CONV=$(1)
    PATH_SEP=/
    PATH_SEPD=/
    HOST_COMPILER?=GCC
endif

ifeq ($(HOST_OS),Windows_NT)
$(info ComSpec=$(ComSpec))
    SHELL:=$(ComSpec)
    .SHELLFLAGS=/C
else
    SHELL:=/bin/sh
endif

$(info SHELL=$(SHELL))