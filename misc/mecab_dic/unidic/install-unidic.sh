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

DEST=`dirname "${0}"`
if [ $# -ge 1 ]; then
    DEST=$1
fi

UNIDIC=unidic-mecab-2.1.2_bin
UNIDIC_URL=https://ja.osdn.net/dl/unidic/$UNIDIC.zip
TMP=./tmp.zip

wget -O $TMP $UNIDIC_URL
set +e
unzip $TMP -d $DEST
set -e
rm $TMP

UNIDIC_PATH=`cd $DEST/$UNIDIC && pwd`
echo ''
echo "UniDic installed to $UNIDIC_PATH"
echo "To use UniDic from Resembla, add following MeCab options: -d $UNIDIC_PATH -Odump"
