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
import time
import datetime

_TIMEOUT_SECONDS = 10

def time_mill():
    return time.time() * 1000

def run(server_address, thread_id, count, queries):
    channel = grpc.insecure_channel(server_address)
    resembla = resembla_pb2.ResemblaServiceStub(channel)

    results = []
    t_start = time_mill()
    for i in range(count):
        for query in queries:
            t0 = time_mill()
            response = resembla.find(resembla_pb2.ResemblaRequest(query=query), _TIMEOUT_SECONDS)
            t1 = time_mill()
            results.append((thread_id, 'query/' + query, str(t1 - t0)))
    t_end = time_mill()
    t_sum = 0
    for r in results:
        t_sum = t_sum + float(r[2])
    t_average = t_sum / len(results)
    results.append((thread_id, 'total', str(t_end - t_start)))
    results.append((thread_id, 'average', str(t_average)))
    for r in results:
        print('\t'.join(r))

if __name__ == '__main__':
  thread_id = sys.argv[1]
  count= int(sys.argv[2])
  queries = sys.argv[3:]

  run('localhost:50051', thread_id, count, queries)
