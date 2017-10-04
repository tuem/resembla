#!/usr/bin/env bash

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

set -eu

BASEDIR=$(cd $(dirname $0);pwd)


if [ $# -lt 2 ]; then
    echo 'usage: sh exec-diff.sh OLD_EVAL_DIR NEW_EVAL_DIR OUTPUT'
    exit 1
fi

OLD_DIR=$1
OLD_ID=$(basename $OLD_DIR)
if [[ $OLD_ID == eval.* ]]; then
    OLD_ID=${OLD_ID:5}
fi

NEW_DIR=$2
NEW_ID=$(basename $NEW_DIR)
if [[ $NEW_ID == eval.* ]]; then
    NEW_ID=${NEW_ID:5}
fi

if [ $# -ge 3 ]; then
    DIFF_DIR=$3
    DIFF_ID=$(basename $DIFF_DIR)
else
    DIFF_ID=${OLD_ID}.${NEW_ID}
    DIFF_DIR=$BASEDIR/result/diff.${DIFF_ID}
fi


OLD_OUTPUT=$OLD_DIR/result.$OLD_ID
NEW_OUTPUT=$NEW_DIR/result.$NEW_ID
DIFF_OUTPUT=$DIFF_DIR/diff.${DIFF_ID}
DIFF_SUMMARY=$DIFF_OUTPUT.summary
PY_DIR=$BASEDIR/py

mkdir -p $DIFF_DIR

#difference improved    degraded    changed
$PY_DIR/diff_raw.py $OLD_OUTPUT.raw.tsv $NEW_OUTPUT.raw.tsv > $DIFF_OUTPUT.raw.tsv
$PY_DIR/diff_count.py $OLD_OUTPUT.raw.tsv $NEW_OUTPUT.raw.tsv > $DIFF_OUTPUT.count
$PY_DIR/diff_summary.py $OLD_OUTPUT.summary $NEW_OUTPUT.summary > $DIFF_SUMMARY
echo '' >> $DIFF_SUMMARY
echo "# of improved	`tail -n +2 $DIFF_OUTPUT.count | awk '{print $2}'`" >> $DIFF_SUMMARY
echo "# of degraded	`tail -n +2 $DIFF_OUTPUT.count | awk '{print $3}'`" >> $DIFF_SUMMARY
echo "total increase	`tail -n +2 $DIFF_OUTPUT.count | awk '{print $1}'`" >> $DIFF_SUMMARY

cat $DIFF_OUTPUT.raw.tsv | $PY_DIR/tsv_to_mdtable.py > $DIFF_OUTPUT.raw.md
cat $DIFF_OUTPUT.count | $PY_DIR/tsv_to_mdtable.py > $DIFF_OUTPUT.count.md
cat $DIFF_SUMMARY | $PY_DIR/tsv_to_mdtable.py > $DIFF_SUMMARY.md

cat $DIFF_SUMMARY
echo ''
echo "all changes are stored in $DIFF_OUTPUT.raw.tsv"
