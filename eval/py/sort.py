#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import sys

from raw_result import RawResult

if __name__ == '__main__':
    key = '4' # score
    if len(sys.argv) > 1:
        key = sys.argv[1]
    rev = key in ('0', '4', '5', '6', 'freq', 'score', 'score_correct', 'rank_correct')

    datum = []
    first = True
    for line in sys.stdin:
        if first:
            print(line, end='')
            first = False
        else:
            datum.append(RawResult(line))

    datum.sort(key=lambda i: i.sort_key(key), reverse=rev)
    for data in datum:
        print(data.raw_data, end='')
