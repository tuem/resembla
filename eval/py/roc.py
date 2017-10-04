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
    total_tp = 0
    total_fp = 0
    for line in sys.stdin:
        data = RawResult(line)
        if data.pred == data.true:
            total_tp = total_tp + 1
        else:
            total_fp = total_fp + 1
        datum.append(data)

    tp = 0
    fp = 0
    auc = 0 if total_fp > 0 else 1
    print('%s\t%s\t%s\t%s' % ('score', 'tp', 'fp', 'auc'))
    for data in datum:
        if data.pred == data.true:
            tp = tp + 1 / (total_tp + total_fp)
        else:
            fp = fp + 1 / (total_fp + total_fp)
            auc = auc + tp / total_fp
        print('%f\t%f\t%f\t%f' % (data.score, tp, fp, auc))
