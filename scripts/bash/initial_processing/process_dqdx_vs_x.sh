#!/bin/bash

data_version=v08_34_00
noRuns=10

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x.C
OUTPUTDIR=plots/${data_version}/elifetime
LOGDIR=logs/$data_version

for batch in 2000{22..30}00 ; do
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    install -d $newdir
    install -d $newlogdir
    pids=()
    for part in {0..9} ; do
	LOGFILE=$newlogdir/process_dqdx_vs_x_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	root -l -b -q "${SCRIPT_NAME}(\"\",\"$newdir/batch_${batch}_part${part}_\", ${batch}, $noRuns, $START_RUN, \"$data_version\")" &> $LOGFILE &
	pids[$part]=$!
    done

    # wait for all parts to finish before moving on to next batch
    for pid in ${pids[*]} ; do
	wait $pid
    done
done
