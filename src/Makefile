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

LIB_DEST = /usr/local/lib
LIB_NAME = libresembla
MAJOR_VERSION = 1
MINOR_VERSION = 0
LIB_DIR = ../lib
LIB_FILE = $(LIB_NAME).so.$(MAJOR_VERSION).$(MINOR_VERSION)

INCLUDE_DEST = /usr/local/include
INCLUDE_HEADERS = *.hpp
INCLUDE_HEADERS_MEASURE = measure/*.hpp
INCLUDE_HEADERS_REGRESSION = regression/*.hpp
INCLUDE_HEADERS_EXTRACTOR = regression/extractor/*.hpp
INCLUDE_HEADERS_AGGREGATOR = regression/aggregator/*.hpp
INCLUDE_HEADERS_PREDICTOR = regression/predictor/*.hpp


lib: $(LIB_FILE)
all: lib executable
SUBDIR_OPTIONS =

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 -isystem../include -isystem../include/json -isystem../include/cmdline -isystem../include/paramset `pkg-config --cflags icu-uc icu-i18n` `mecab-config --cflags`
CXXLIBS := -lsvm `pkg-config --libs icu-uc icu-i18n` `mecab-config --libs`
CXXEXTRA :=
ifeq ($(UNAME_S),Darwin)
	CXXEXTRA := -Wl,-install_name,$(LIB_NAME).so
endif

debug: CXXFLAGS += -DDEBUG -g
debug: SUBDIR_OPTIONS += debug
debug: lib

SUBDIRS = measure regression executable


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

SUBDIR_SRCS = $(shell find measure -name "*.cpp")  $(shell find regression -name "*.cpp")
SUBDIR_OBJS = $(patsubst %.cpp,%.o,$(SUBDIR_SRCS))


$(SUBDIRS):
	$(MAKE) -C $@ $(SUBDIR_OPTIONS)

$(LIB_FILE): $(OBJS) measure regression
	$(CXX) $(CXXLIBS) -shared $(CXXEXTRA) -o $(LIB_FILE) $(OBJS) $(SUBDIR_OBJS)


install:
	install -d $(INCLUDE_DEST)/resembla/measure $(INCLUDE_DEST)/resembla/simstring
	install -d $(INCLUDE_DEST)/resembla/regression/extractor $(INCLUDE_DEST)/resembla/regression/aggregator $(INCLUDE_DEST)/resembla/regression/predictor
	install -p $(INCLUDE_HEADERS) $(INCLUDE_DEST)/resembla/
	install -p $(INCLUDE_HEADERS_MEASURE) $(INCLUDE_DEST)/resembla/measure/
	install -p $(INCLUDE_HEADERS_REGRESSION) $(INCLUDE_DEST)/resembla/regression/
	install -p $(INCLUDE_HEADERS_EXTRACTOR) $(INCLUDE_DEST)/resembla/regression/extractor/
	install -p $(INCLUDE_HEADERS_AGGREGATOR) $(INCLUDE_DEST)/resembla/regression/aggregator/
	install -p $(INCLUDE_HEADERS_PREDICTOR) $(INCLUDE_DEST)/resembla/regression/predictor/
	install -p ../include/simstring/* $(INCLUDE_DEST)/resembla/simstring/
	install -p $(LIB_FILE) $(LIB_DEST)/
	ln -fs $(LIB_DEST)/$(LIB_FILE) $(LIB_DEST)/$(LIB_NAME).so.$(MAJOR_VERSION)
	ln -fs $(LIB_DEST)/$(LIB_NAME).so.$(MAJOR_VERSION) $(LIB_DEST)/$(LIB_NAME).so


uninstall:
	$(RM) -rf $(INCLUDE_DEST)/resembla
	$(RM) -f $(LIB_DEST)/$(LIB_NAME).so $(LIB_DEST)/$(LIB_NAME).so.$(MAJOR_VERSION) $(LIB_DEST)/$(LIB_FILE)


clean:
	$(MAKE) -C measure clean
	$(MAKE) -C regression clean
	$(MAKE) -C executable clean
	$(RM) $(OBJS) $(DEPS) $(LIB_FILE)


.PHONY: clean lib all debug $(SUBDIRS)


.cpp.o:
	$(CXX) $(CXXFLAGS) -c -fPIC $<

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
