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

class RawResult:
    # When comparing RawResults by using rank_correct, this value is used if rank_correct=-1.
    DEFAULT_MAX_RANK = 1000000

    def __init__(self, line, delimiter='\t'):
        self.raw_data = line
        self.delimiter = delimiter
        self.values = line.rstrip().split(delimiter)
        self.values[0] = int(self.values[0])
        self.values[4] = float(self.values[4])
        self.values[5] = float(self.values[5])
        self.values[6] = int(self.values[6])

        self.freq = self.values[0]
        self.input = self.values[1]
        self.pred = self.values[2]
        self.true = self.values[3]
        self.score = self.values[4]
        self.score_correct = self.values[5]
        self.rank_correct = self.values[6]

    # To sort correctly you must pass enough large max_rank if rank_correct could be larger than DEFAULT_MAX_RANK.
    def sort_key(self, k, max_rank=None):
        max_rank = max_rank if max_rank is not None else self.DEFAULT_MAX_RANK
        names = ['freq', 'input', 'pred', 'true', 'score', 'score_correct', 'rank_correct']
        if isinstance(k, str) and k.isdigit():
            return self.sort_key(names[int(k)])
        elif isinstance(k, int) and k >= 0 and k < len(self.values):
            return self.sort_key(names[k])
        else:
            for i in range(len(names)):
                if k == names[i]:
                    if names[i] == 'rank_correct' and self.values[i] == -1:
                        return max_rank
                    else:
                        return self.values[i]
        return self.raw_data
