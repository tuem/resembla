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

BINS = eval_resembla benchmark_eliminator
all: $(BINS)

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 -isystem../../include -isystem../../include/json -isystem../../include/cmdline -isystem../../include/paramset -I../../src `mecab-config --cflags`
CXXLIBS := -lresembla -lsvm `pkg-config --libs icu-uc icu-i18n` `mecab-config --libs`

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

eval_resembla: eval_resembla.o history.o
	$(CXX) -o $@ eval_resembla.o history.o $(CXXLIBS)

benchmark_eliminator: benchmark_eliminator.o history.o
	$(CXX) -o $@ benchmark_eliminator.o history.o $(CXXLIBS)


.PHONY: clean all


clean:
	$(RM) $(OBJS) $(DEPS) $(BINS)


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's|\($*\)\.o[ :]*|\1.o $@ : |g' > ./$@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
