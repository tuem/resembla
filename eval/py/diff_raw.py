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

def load_data(path):
    datum = []
    first = True
    for line in open(path, 'r'):
        if first:
            first = False
            continue
        datum.append(RawResult(line))
    return datum


def check_data(datum_old, datum_new):
    if len(datum_old) != len(datum_new):
        print('line counts are different')
        quit(1)
    elif len(datum_new) == 0:
        quit(0)


if __name__ == '__main__':
    datum_old = load_data(sys.argv[1])
    datum_new = load_data(sys.argv[2])
    check_data(datum_old, datum_new)

    diffs = []
    for i in range(len(datum_new)):
        d_new = datum_new[i]
        d_old = datum_old[i]
        if d_old.pred == d_new.pred:
            continue
        elif d_new.pred == d_new.true:
            label = 'IMPROVED'
        elif d_old.pred == d_old.true:
            label = 'DEGRADED'
        else:
            continue
        diffs.append('%d\t%s\t%d\t%s\t%s\t%s\t%s\t%f\t%f\t%f\t%f\t%d\t%d' % (i + 1, label, d_new.freq, d_new.input, d_old.pred, d_new.pred, d_new.true, d_old.score, d_new.score, d_old.score_correct, d_new.score_correct, d_old.rank_correct, d_new.rank_correct))
    if len(diffs) == 0:
        quit(0)

    print('#\ttype\tfreq\tinput\told\tnew\ttrue\tscore_old\tscore_new\tscore_correct_old\tscore_correct_new\trank_correct_old\trank_correct_new')
    for diff in diffs:
        print(diff)
