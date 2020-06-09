#!/bin/bash

data_version=new_production_diffusion_2
#data_version=v08_50_00_noDiffusion

outdir=plots/$data_version/elifetime
logdir=logs/$data_version/elifetime

install -d $logdir

for batchno in 20005100 ; do
    {
	for part in {0..9} ; do
	    nice root -l -b -q "scripts/stdAna/fitLifeTime_langau_batch.C($batchno, $part, \"$outdir/batch_${batchno}_\", \"$data_version\")"
	done
    } &> $logdir/fitLifeTime_langau_batch_${batchno}.log &
done
