#!/bin/bash

data_version=v08_34_00

outdir=plots/$data_version/elifetime
logdir=logs/$data_version/elifetime

for batchno in 2000{21..30}00 ; do
    {
	for part in {0..9} ; do
	    nice root -l -b -q "scripts/stdAna/fitLifeTime_langau_batch.C($batchno, $part, \"$outdir/batch_${batchno}_\")"
	done
    } &> $logdir/fitLifeTime_langau_batch_${batchno}.log &
done
