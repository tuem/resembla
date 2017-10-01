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

import resembla_pb2

import sys
import grpc

_TIMEOUT_SECONDS = 10

def run(server_address, queries):
    channel = grpc.insecure_channel(server_address)
    resembla = resembla_pb2.ResemblaServiceStub(channel)

    for (query, candidates) in queries:
        print ('query=%s' % query)
        print ('candidates=%s' % ', '.join(candidates))
        responses = resembla.eval(resembla_pb2.ResemblaOnDemandRequest(query=query, candidates=candidates), _TIMEOUT_SECONDS).results
        for response in responses:
            print ('  response.text=%s' % response.text)
            print ('  response.score=%s' % response.score)

if __name__ == '__main__':
    queries = [
        ('サトー', ['サトウ', 'サイトウ', 'セト']),
        ('高田', ['高畑', '多田', '武田']),
        ('こんにちわん', ['こんにちは', 'こんばんは', 'ごはんちゃわん']),
    ]

    run('localhost:50051', queries)
