#!/usr/bin/env bash

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

set -eu

BASE_DIR=$(cd $(dirname $0);pwd)
PROGRAM_NAME=generate_grpc_templates
ECHO_PREFIX="[${PROGRAM_NAME}] :"

PROJECT_NAME=resembla
PROTO_DIR=$BASE_DIR/protos
CPP_DIR=$BASE_DIR/src/grpc
HEADER_DIR=$BASE_DIR/include
PYTHON_DIR=$BASE_DIR/python
RUBY_DIR=$BASE_DIR/ruby

echo "${ECHO_PREFIX} Generate C++ gRPC server templete"
protoc -I $PROTO_DIR $PROTO_DIR/$PROJECT_NAME.proto --grpc_out=$CPP_DIR --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
protoc -I $PROTO_DIR $PROTO_DIR/$PROJECT_NAME.proto --cpp_out=$CPP_DIR --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
echo "${ECHO_PREFIX} Output C++ source to ${CPP_DIR}"
mv $CPP_DIR/${PROJECT_NAME}*.h $HEADER_DIR/
echo "${ECHO_PREFIX} Output C++ header to ${HEADER_DIR}"

echo "${ECHO_PREFIX} Generate Python gRPC client templete"
python -m grpc_tools.protoc -I$PROTO_DIR --python_out=$PYTHON_DIR --grpc_python_out=$PYTHON_DIR $PROTO_DIR/$PROJECT_NAME.proto
echo "${ECHO_PREFIX} Output Python client to ${HEADER_DIR}"

echo "${ECHO_PREFIX} Generate Ruby gRPC client templete"
protoc -I $PROTO_DIR $PROTO_DIR/$PROJECT_NAME.proto --ruby_out=$RUBY_DIR --grpc_out=$RUBY_DIR --plugin=protoc-gen-grpc=`which grpc_ruby_plugin`
echo "${ECHO_PREFIX} Output Ruby client to ${RUBY_DIR}"
