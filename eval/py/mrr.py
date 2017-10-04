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
    current = 0
    total_rr = 0
    print('%s\t%s\t%s\t%s\t%s' % ('rank', 'reciprocal_rank', 'mean_reciprocal_rank', 'best_score', 'true_score'))
    for line in sys.stdin:
        data = RawResult(line)
        current = current + 1
        if data.rank_correct > 0:
            rr = 1 / data.rank_correct
        else:
            rr = 0
        total_rr = total_rr + rr
        print('%d\t%f\t%f\t%f\t%f' % (data.rank_correct, rr, total_rr / current, data.score, data.score_correct))
