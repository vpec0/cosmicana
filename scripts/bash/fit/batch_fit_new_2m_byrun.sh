#!/bin/bash

data_version=v08_50_00
[[ $# > 0 ]] && data_version=$1

range=$(echo 2000{31..40}00)
[[ $# > 1 ]] && range=${@:2}


CUT=2m_track_cut


base_dir=plots/$data_version/elifetime/$CUT/byrun
outdir=$base_dir
logdir=logs/$data_version/elifetime/$CUT/byrun

mkdir -p $outdir
mkdir -p $logdir


COMMAND="nice /usr/bin/time root -l -b -q scripts/stdAna/fitLifeTime_langau_new_batch.C(20003100, 0, \"test_\", \"\", \"MPV\", \"test_anahists.root\")"


pids=()
for batchno in $range ; do #2000{31..40}00 ; do
    for group in {0..9} ; do
	# echo "Parts $group0..$group9."
	# clear up if output files exist
	rm -f $outdir/new_batch_${batchno}_grp_${group}_MPV_fits.root
	{
	    for subpart in {0..9} ; do
		part=$[10*group + subpart]
		nice /usr/bin/time root -l -b -q "scripts/stdAna/fitLifeTime_langau_new_batch.C($batchno, $part, \"$outdir/new_batch_${batchno}_grp_${group}_\", \"$base_dir\", \"MPV\")"
	    done
	} &> $logdir/fitLifeTime_langau_new_batch_${batchno}_grp_${group}_MPV.log &
	pids[$group]=$!
    done
    # wait for all groups to finish before moving on to next batch
    for pid in ${pids[*]} ; do
	wait $pid
    done

    # Merge resulting trees
    echo "
auto tree = new TChain(\"elifetime\");
for (int i=0;i<10;++i) {
    tree->Add(Form(\"$outdir/new_batch_${batchno}_grp_%d_MPV_fits.root\", i));
}
auto outf = TFile::Open(\"$outdir/new_batch_${batchno}_tree_merged_MPV_fits.root\",\"recreate\");
tree->CloneTree(-1,\"fast\"); outf->Write(); outf->Close();" |root -l -b &> $logdir/merge_fitLifeTime_langau_new_batch_${batchno}_MPV.log

#    hadd -f $outdir/new_batch_${batchno}_merged_MPV_fits.root $outdir/new_batch_${batchno}_grp_{0..9}_MPV_fits.root &> $logdir/hadd_fitLifeTime_langau_new_batch_${batchno}_MPV.log
done
