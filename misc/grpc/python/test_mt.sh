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

echo "started at `date "+%Y%m%d-%H%M%S"`"
NUM_THREAD=$1
NUM_REPEAT=$2
shift 2
for((i=0;i<$NUM_THREAD;i++))
do
    $BASE_DIR/test_st.py $i $NUM_REPEAT $@ &
done
wait
echo "finished at `date "+%Y%m%d-%H%M%S"`"
