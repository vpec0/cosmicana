#!/bin/bash

CUT=2m_track_cut

data_version=new_production_diffusion_2
basedir=plots/$data_version/elifetime/$CUT
outdir=$basedir
logdir=logs/$data_version/elifetime/$CUT

install -d $logdir
install -d $outdir

for batchno in 2000{41..43}00 ; do
    {
	for part in {0..9} ; do
	    nice root -l -b -q "scripts/stdAna/fitLifeTime_langau_new_batch.C($batchno, $part, \"$outdir/new_batch_${batchno}_\", \"$basedir\")"
	done
    } &> $logdir/fitLifeTime_langau_new_batch_${batchno}.log &
done
