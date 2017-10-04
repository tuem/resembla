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

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 -isystem../../include -isystem../../include/cmdline -isystem../../include/json -isystem../../include/paramset

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

SUBDIRS = extractor aggregator predictor

all: $(OBJS) $(SUBDIRS)
debug: all

-include $(DEPS)
SUBDIR_OPTIONS =

debug: CXXFLAGS += -DDEBUG -g
debug: SUBDIR_OPTIONS += debug
debug: all


$(SUBDIRS):
	$(MAKE) -C $@ $(SUBDIR_OPTIONS)


clean:
	$(MAKE) -C extractor clean
	$(MAKE) -C aggregator clean
	$(MAKE) -C predictor clean
	$(RM) $(OBJS) $(DEPS)


.PHONY: clean all $(SUBDIRS)


.cpp.o:
	$(CXX) $(CXXFLAGS) -c -fPIC $<

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
