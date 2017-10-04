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
from diff_raw import load_data, check_data

if __name__ == '__main__':
    datum_old = load_data(sys.argv[1])
    datum_new = load_data(sys.argv[2])
    check_data(datum_old, datum_new)

    improved = 0
    degraded = 0
    for i in range(len(datum_old)):
        d_old = datum_old[i]
        d_new = datum_new[i]
        if d_old.pred == d_new.pred:
            continue
        elif d_new.pred == d_new.true:
            improved = improved + 1
        elif d_old.pred == d_old.true:
            degraded = degraded + 1
        else:
            continue

    print('%s\t%s\t%s\t%s' % ('difference', 'improved', 'degraded', 'changed'))
    print('%d\t%d\t%d\t%d' % (improved - degraded, improved, degraded, improved + degraded))
