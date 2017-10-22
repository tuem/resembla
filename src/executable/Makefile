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

BIN_DEST = /usr/local/bin
BINS = resembla_index resembla_cli


all: $(BINS)


CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 -isystem../../include -isystem../../include/json -isystem../../include/cmdline -isystem../../include/paramset -I.. `mecab-config --cflags`
CXXLIBS := -lresembla -lsvm `pkg-config --libs icu-uc icu-i18n` `mecab-config --libs`

debug: CXXFLAGS += -DDEBUG -g
debug: all


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

RESEMBLA_INDEX_OBJS = resembla_index.o
RESEMBLA_CLI_OBJS = resembla_cli.o


resembla_index: $(RESEMBLA_INDEX_OBJS)
	$(CXX) -o $@ $(RESEMBLA_INDEX_OBJS) $(CXXLIBS)

resembla_cli: $(RESEMBLA_CLI_OBJS)
	$(CXX) -o $@ $(RESEMBLA_CLI_OBJS) $(CXXLIBS)


install:
	install -p $(BINS) $(BIN_DEST)


uninstall:
	cd $(BIN_DEST) && $(RM) $(BINS)


clean:
	$(RM) $(OBJS) $(DEPS) $(BINS)


.PHONY: clean all


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
