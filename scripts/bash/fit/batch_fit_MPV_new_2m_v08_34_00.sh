#!/bin/bash

CUT=2m_track_cut

data_version=v08_34_00
base_dir=plots/$data_version/elifetime/$CUT
outdir=$base_dir
logdir=logs/$data_version/elifetime/$CUT

mkdir -p $outdir
mkdir -p $logdir

for batchno in 2000{21..30}00 ; do
    {
	for part in {0..9} ; do
	    nice /usr/bin/time root -l -b -q "scripts/stdAna/fitLifeTime_langau_MPV_new_batch.C($batchno, $part, \"$outdir/new_batch_${batchno}_\", \"$base_dir\")"
	done
    } &> $logdir/fitLifeTime_langau_MPV_new_batch_${batchno}.log &
done
