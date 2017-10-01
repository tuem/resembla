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

def load_data(path):
    datum = []
    first = True
    for line in open(path, 'r'):
        if first:
            first = False
            continue
        (key, value) = line.rstrip().split('\t')[:2]
        if key in ['corpus size', '# of input', '# of correct']:
            value = int(value)
        elif key in ['AUC', 'Mean reciprocal rank', 'Recall', 'Precision', 'F-measure', 'Score(best)', 'Recall(best)', 'Precision(best)', 'F-measure(best)']:
            value = float(value)
        datum.append((key, value))
    return datum


def check_data(datum_old, datum_new):
    if datum_old[1][1] != datum_new[1][1]:
        print('warning: input corpora are different', file=sys.stderr)


if __name__ == '__main__':
    datum_old = load_data(sys.argv[1])
    datum_new = load_data(sys.argv[2])
    check_data(datum_old, datum_new)

    for i in range(len(datum_new)):
        (key, new_val) = datum_new[i]
        old_val = datum_old[i][1]

        label = ''
        if key in ['# of correct', 'AUC', 'Mean reciprocal rank', 'Recall', 'Precision', 'F-measure', 'Recall(best)', 'Precision(best)', 'F-measure(best)']:
            if old_val < new_val:
                label = 'IMPROVED'
            elif old_val > new_val:
                label = 'DEGRADED'

        print('%s\t%s\t%s\t%s' % (key, str(old_val), str(new_val), label))
