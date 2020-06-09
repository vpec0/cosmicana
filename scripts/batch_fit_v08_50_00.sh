#!/bin/bash

data_version=v08_50_00

outdir=plots/$data_version/elifetime
logdir=logs/$data_version/elifetime

for batchno in 2000{31..40}00 ; do
    {
	for part in {0..9} ; do
	    nice root -l -b -q "scripts/stdAna/fitLifeTime_langau_batch.C($batchno, $part, \"$outdir/batch_${batchno}_\", \"$data_version\")"
	done
    } &> $logdir/fitLifeTime_langau_batch_${batchno}.log &
done
