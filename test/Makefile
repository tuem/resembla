# Resembla
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

BIN = test_all
all: $(BIN)

test_debug: CXXFLAGS += -DDEBUG -g
test_debug: all

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 `pkg-config --cflags icu-uc` `mecab-config --cflags` -I../src -isystem../include -isystem../include/Catch -isystem../include/json -isystem../include/cmdline -isystem../include/paramset
CXXLIBS := -lsvm `pkg-config --libs icu-uc icu-i18n` `mecab-config --libs`


SRCS = $(wildcard test_*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))


SRC_DIR = ../src

RESEMBLA_COMMON_SRCS = $(SRC_DIR)/string_util.cpp $(SRC_DIR)/symbol_normalizer.cpp $(SRC_DIR)/resembla_util.cpp $(SRC_DIR)/string_normalizer.cpp $(SRC_DIR)/mecab_util.cpp
RESEMBLA_COMMON_OBJS = $(patsubst %.cpp,%.o,$(RESEMBLA_COMMON_SRCS))
RESEMBLA_COMMON_OBJ_FILENAMES = $(patsubst $(SRC_DIR)/%,%,$(RESEMBLA_COMMON_OBJS))

RESEMBLA_MEASURE_SRCS = $(wildcard $(SRC_DIR)/measure/*.cpp)
RESEMBLA_MEASURE_OBJS = $(patsubst %.cpp,%.o,$(RESEMBLA_MEASURE_SRCS))
RESEMBLA_MEASURE_OBJ_FILENAMES = $(patsubst $(SRC_DIR)/measure/%,%,$(RESEMBLA_MEASURE_OBJS))

RESEMBLA_REGRESSION_SRCS = $(wildcard $(SRC_DIR)/regression/*.cpp) $(wildcard $(SRC_DIR)/regression/extractor/*.cpp) $(wildcard $(SRC_DIR)/regression/aggregator/*.cpp) $(wildcard $(SRC_DIR)/regression/predictor/*.cpp)
RESEMBLA_REGRESSION_OBJS = $(patsubst %.cpp,%.o,$(SUBDIR_SRCS))

ALL_SRCS = $(SRCS) $(RESEMBLA_COMMON_SRCS) $(RESEMBLA_MEASURE_SRCS) $(RESEMBLA_REGRESSION_SRCS)
ALL_OBJS = $(patsubst %.cpp,%.o,$(ALL_SRCS))
ALL_DEPS = $(patsubst %.cpp,%.d,$(ALL_SRCS))


resembla_common:
	$(MAKE) -C ../src $(RESEMBLA_COMMON_OBJ_FILENAMES)

resembla_measure:
	$(MAKE) -C ../src/measure $(RESEMBLA_MEASURE_OBJ_FILENAMES)

resembla_regression:
	$(MAKE) -C ../src/regression

test_all: resembla_common resembla_measure resembla_regression $(OBJS)
	$(CXX) -o test_all $(ALL_OBJS) $(CXXLIBS)
	./test_all


clean:
	$(MAKE) -C ../src clean
	$(RM) $(OBJS) $(DEPS) $(BIN)

.PHONY: clean resembla_common resembla_measure test_all test_debug


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's|\($*\)\.o[ :]*|\1.o $@ : |g' > ./$@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
