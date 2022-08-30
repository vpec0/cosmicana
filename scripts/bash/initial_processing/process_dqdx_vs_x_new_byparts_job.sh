#!/bin/bash

[[ $# < 6 ]] && echo "Give at least 6 arguments" && exit -1

TOPDIR=$1
DATA_VERSION=$2
BATCH_LIST=$3
OUTDIR=$4
NPARTS=$5
RUNS_PER_PART=$6
STARTRUN=0
[ -n "$7" ] && STARTRUN=$7
STARTPART=0
[ -n "$8" ] && STARTPART=$8

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup root v6_22_08d -q "e20:p392:prof"

data_type=""


DEVDIR=/home/pec/dune/cosmics/cosmicana

SCRIPT_NAME=$DEVDIR/scripts/stdAna/process_dqdx_vs_x_new.C

LOGDIR=$OUTDIR/logs

for batch in $BATCH_LIST ; do
    newdir=$OUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    mkdir -p $newdir
    mkdir -p $newlogdir

    START_RUN=$STARTRUN
    for part in $(seq $STARTPART $[NPARTS-1]) ; do
	OUTPREFIX=$newdir/new_batch_${batch}_singlerun_part${part}_
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_singlerun_part${part}.log
	echo "Running batch $batch part $part: 	nice /usr/bin/time root -l -b -q \"${SCRIPT_NAME}(\\\"\\\",\\\"${OUTPREFIX}\\\", ${batch}, $RUNS_PER_PART, $START_RUN, \\\"\\\", \\\"\\\", \\\"$TOPDIR\\\")\" &> $LOGFILE"
	nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $RUNS_PER_PART, $START_RUN, \"\", \"\", \"$TOPDIR\")" &> $LOGFILE
	START_RUN=$[ START_RUN + RUNS_PER_PART ]
    done
done
