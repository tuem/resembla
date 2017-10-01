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
    datum = []
    for line in sys.stdin:
        datum.append(RawResult(line))

    total = len(datum)
    if total == 0:
        quit()
    correct = 0
    current = 0
    best_f = -1
    best_r = -1
    best_p = -1
    best_score = -1
    for data in datum:
        current = current + 1
        if data.pred == data.true:
            correct = correct + 1
        r = correct / total
        p = correct / current if current > 0 else 0
        f = 2 * r * p / (r + p) if r + p > 0 else 0
        if best_f < f:
            best_f = f
            best_r = r
            best_p = p
            best_score = data.score

    print('%s\t%s\t%s\t%s' % ('score', 'recall', 'precision', 'F-measure'))
    print('%f\t%f\t%f\t%f' % (best_score, best_r, best_p, best_f))
