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

all: resembla.pb.o resembla.grpc.pb.o
debug: all

CXX := g++
CXXFLAGS := -w -O3 -std=c++11 -isystem../../include

SRCS = resembla.pb.cc resembla.grpc.pb.cc
OBJS = $(patsubst %.cc,%.o,$(SRCS))
DEPS = $(patsubst %.cc,%.d,$(SRCS))


clean:
	$(RM) *.o *.d


.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

%.d: %.cc
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
