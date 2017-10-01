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

function md5cmd(){
    _MD5_CMD='md5sum'
    if [ "$(uname)" = 'Darwin' ]; then
        _MD5_CMD='md5 -r'
    fi
    $_MD5_CMD $1 | awk '{print $1}'
}

BASEDIR=$(cd $(dirname $0);pwd)
DATETIME=`date "+%Y%m%d-%H%M%S"`

COMPRESS_CMD=gzip
MD5_CMD=md5cmd
GIT_ID_CMD='git show -s --format=%H'
EVALUATION_BIN=$BASEDIR/src/eval_resembla
PYDIR=$BASEDIR/py


if [ $# -lt 3 ]; then
    echo 'usage: ./exec-eval.sh CONF MEASURE INPUT [OUTPUT [other Resembla options]]'
    exit 1
fi

CONF_ORG=$1
MEASURE=$2
if [ $MEASURE = 'default' ]; then
    MEASURE_OPTION=''
else
    MEASURE_OPTION="-m $MEASURE"
fi
INPUT_ORG=$(cd $(dirname $3) && pwd)/$(basename $3)
shift 3

if [ $# -ge 1 ]; then
    DIR=$1
    shift 1
else
    DIR='default'
fi
if [ $DIR = 'default' ]; then
    DIR=$BASEDIR/result/eval.$DATETIME
    ID=$DATETIME
else
    ID=$(basename $DIR)
    if [[ $ID == eval.* ]]; then
        ID=${ID:5}
    fi
fi


INPUT=$DIR/input/input.$ID
CONF=$DIR/input/config.json
OUTPUT=$DIR/result.$ID
CSV=$DIR/csv/result.$ID
MD=$DIR/md/result.$ID

mkdir -p $DIR/input $DIR/csv $DIR/md
cp -p $INPUT_ORG $INPUT.tsv
cp -p $CONF_ORG $CONF

EVAL_CMD="$EVALUATION_BIN -c $CONF $MEASURE_OPTION $@ $INPUT.tsv > $OUTPUT.raw.tsv"
(time -p (eval $EVAL_CMD) 2>&1) | tee $OUTPUT.log
TIME="$(cat $OUTPUT.log | tail -3 | head -1 | awk '{print $2}')"

tail -n +2 $OUTPUT.raw.tsv | $PYDIR/count.py > $OUTPUT.count
cat $OUTPUT.raw.tsv | $PYDIR/sort.py score > $OUTPUT.raw.score.tsv
cat $OUTPUT.raw.tsv | $PYDIR/sort.py freq > $OUTPUT.raw.freq.tsv
cat $OUTPUT.raw.tsv | $PYDIR/sort.py rank_correct > $OUTPUT.raw.rank_correct.tsv
tail -n +2 $OUTPUT.raw.score.tsv | $PYDIR/rpf.py > $OUTPUT.rpf.tsv
tail -n +2 $OUTPUT.raw.score.tsv | $PYDIR/best.py > $OUTPUT.best
tail -n +2 $OUTPUT.raw.score.tsv | $PYDIR/roc.py > $OUTPUT.roc.tsv
tail -n +2 $OUTPUT.raw.score.tsv | $PYDIR/mrr.py > $OUTPUT.mrr.tsv

SUMMARY=$OUTPUT.summary
echo "Evaluation date	$DATETIME" > $SUMMARY
echo "Git commit ID	`$GIT_ID_CMD`" >> $SUMMARY
echo "Original input file	$INPUT_ORG" >> $SUMMARY
echo "Input data MD5	`$MD5_CMD $INPUT_ORG`" >> $SUMMARY
echo "Used measure	$MEASURE" >> $SUMMARY
echo "Executed command	$EVAL_CMD" >> $SUMMARY
echo "Computation time	$TIME" >> $SUMMARY
echo "corpus size	`tail -n +2 $INPUT.tsv | grep -c ''`" >> $SUMMARY
echo "# of input	`tail -n +2 $OUTPUT.raw.tsv | grep -c ''`" >> $SUMMARY
echo "# of correct	`tail -n +2 $OUTPUT.count | awk '{print $2}'`" >> $SUMMARY
echo "AUC	`tail -1 $OUTPUT.roc.tsv | awk '{print $4}'`" >> $SUMMARY
echo "Mean reciprocal rank	`tail -1 $OUTPUT.mrr.tsv | awk '{print $3}'`" >> $SUMMARY
echo "Recall	`tail -1 $OUTPUT.rpf.tsv | awk '{print $2}'`" >> $SUMMARY
echo "Precision	`tail -1 $OUTPUT.rpf.tsv | awk '{print $3}'`" >> $SUMMARY
echo "F-measure	`tail -1 $OUTPUT.rpf.tsv | awk '{print $4}'`" >> $SUMMARY
echo "Score(best)	`tail -1 $OUTPUT.best | awk '{print $1}'`" >> $SUMMARY
echo "Recall(best)	`tail -1 $OUTPUT.best | awk '{print $2}'`" >> $SUMMARY
echo "Precision(best)	`tail -1 $OUTPUT.best | awk '{print $3}'`" >> $SUMMARY
echo "F-measure(best)	`tail -1 $OUTPUT.best | awk '{print $4}'`" >> $SUMMARY

cat $INPUT.tsv | $PYDIR/tsv_to_mdtable.py > $DIR/md/input.$ID.csv
cat $SUMMARY | $PYDIR/tsv_to_mdtable.py > $MD.summary.md
cat $OUTPUT.raw.tsv | $PYDIR/tsv_to_mdtable.py > $MD.raw.md
cat $OUTPUT.raw.score.tsv | $PYDIR/tsv_to_mdtable.py > $MD.raw.score.md
cat $OUTPUT.raw.freq.tsv | $PYDIR/tsv_to_mdtable.py > $MD.raw.freq.md
cat $OUTPUT.raw.rank_correct.tsv | $PYDIR/tsv_to_mdtable.py > $MD.raw.rank_correct.md
cat $OUTPUT.rpf.tsv | $PYDIR/tsv_to_mdtable.py > $MD.rpf.md
cat $OUTPUT.roc.tsv | $PYDIR/tsv_to_mdtable.py > $MD.roc.md
cat $OUTPUT.mrr.tsv | $PYDIR/tsv_to_mdtable.py > $MD.mrr.md

cat $INPUT.tsv | sed 's/	/,/g' | nkf -s -Lw > $DIR/csv/input.$ID.csv
cat $SUMMARY | sed 's/	/,/g' | nkf -s -Lw > $CSV.summary.csv
cat $OUTPUT.raw.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.raw.csv
cat $OUTPUT.raw.score.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.raw.score.csv
cat $OUTPUT.raw.freq.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.raw.freq.csv
cat $OUTPUT.raw.rank_correct.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.raw.rank_correct.csv
cat $OUTPUT.rpf.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.rpf.csv
cat $OUTPUT.roc.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.roc.csv
cat $OUTPUT.mrr.tsv | sed 's/ /,/g' | nkf -s -Lw > $CSV.mrr.csv

cat $SUMMARY
echo ''
echo "whole results are stored in $DIR"

rm $INPUT.tsv.*
$COMPRESS_CMD $INPUT.tsv
