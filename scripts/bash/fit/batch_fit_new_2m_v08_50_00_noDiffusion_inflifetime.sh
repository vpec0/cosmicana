#!/bin/bash

CUT=2m_track_cut

data_version=new_production
data_type=no_lt_diffusion_inf_e_lifetime_100_jobs

basedir=plots/v08_50_00/$data_type/elifetime/$CUT
outdir=$basedir
logdir=logs/$data_version/elifetime/$CUT

install -d $logdir
install -d $outdir

for batchno in 20005100 ; do
    {
	for part in {0..9} ; do
	    nice root -l -b -q "scripts/stdAna/fitLifeTime_langau_new_batch.C($batchno, $part, \"$outdir/new_batch_${batchno}_\", \"$basedir\")"
	done
    } &> $logdir/fitLifeTime_langau_new_batch_${batchno}.log &
done
