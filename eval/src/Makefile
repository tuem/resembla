# Resembla: Word-based Japanese similar sentence search library
# https://github.com/tuem/resembla
#
# Copyright 2017 Takashi Uemura
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

all: eval_resembla

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 `pkg-config --cflags icu-uc icu-i18n` `mecab-config --cflags` -I../../src -isystem../../include -isystem../../include/json -isystem../../include/cmdline -isystem../../include/paramset
CXXLIBS := `pkg-config --libs icu-uc icu-i18n` `mecab-config --libs`


SRCS = eval_resembla.cpp
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

SRC_DIR := ../../src
RESEMBLA_COMMON_SRCS = $(SRC_DIR)/string_util.cpp $(SRC_DIR)/resembla_util.cpp $(SRC_DIR)/resembla_response.cpp $(SRC_DIR)/resembla_ensemble.cpp
RESEMBLA_COMMON_OBJS = $(patsubst %.cpp,%.o,$(RESEMBLA_COMMON_SRCS))
RESEMBLA_COMMON_OBJ_FILENAMES = $(patsubst ../../src/%,%,$(RESEMBLA_COMMON_OBJS))
RESEMBLA_MEASURE_SRCS = $(wildcard $(SRC_DIR)/measure/*.cpp)
RESEMBLA_MEASURE_OBJS = $(patsubst %.cpp,%.o,$(RESEMBLA_MEASURE_SRCS))
RESEMBLA_MEASURE_OBJ_FILENAMES = $(patsubst ../../src/measure/%,%,$(RESEMBLA_MEASURE_OBJS))

ALL_SRCS = $(RESEMBLA_COMMON_SRCS) $(RESEMBLA_MEASURE_SRCS) $(SRCS)
ALL_OBJS = $(patsubst %.cpp,%.o,$(ALL_SRCS))


resembla_common:
	$(MAKE) -C ../../src/ $(RESEMBLA_COMMON_OBJ_FILENAMES)

resembla_measure:
	$(MAKE) -C ../../src/measure/ $(RESEMBLA_MEASURE_OBJ_FILENAMES)

eval_resembla: resembla_common resembla_measure $(OBJS)
	$(CXX) $(CXXLIBS) -o ../eval_resembla $(ALL_OBJS)


.PHONY: clean all resembla_common resembla_measure


clean:
	$(MAKE) -C ../../src/ clean
	$(RM) $(OBJS) $(DEPS) ../eval_resembla


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's|\($*\)\.o[ :]*|\1.o $@ : |g' > ./$@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
