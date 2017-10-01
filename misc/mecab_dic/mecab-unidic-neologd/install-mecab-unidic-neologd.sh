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

BASEDIR=$(cd $(dirname $0);pwd)

REPOSDIR=$BASEDIR/repos
DICTDIR=$BASEDIR/dic
if [ ! -e $REPOSDIR ]; then
    git clone --depth 1 https://github.com/neologd/mecab-unidic-neologd.git $REPOSDIR
fi
$REPOSDIR/bin/install-mecab-unidic-neologd -n -y -p $DICTDIR
sudo chown -R `whoami` $DICTDIR
rm -rf $REPOSDIR/build/*-neologd-*

echo "mecab-unidic-neologd has been installed to $DICTDIR"
echo "To use mecab-unidic-neologd from Resembla, add following MeCab options: -d $DICTDIR -Odump"
