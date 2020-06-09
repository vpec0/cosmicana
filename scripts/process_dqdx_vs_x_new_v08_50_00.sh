#!/bin/bash

data_version=v08_50_00
noRuns=100

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new.C
OUTPUTDIR=plots/${data_version}/elifetime
LOGDIR=logs/$data_version

for batch in 2000{31..33}00 ; do
    for part in 0 ; do
	OUTPREFIX=$OUTPUTDIR/new_batch_${batch}_part${part}_
	LOGFILE=$LOGDIR/process_dqdx_vs_x_new_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $noRuns, $START_RUN, \"$data_version\")" &> $LOGFILE &
    done
done
