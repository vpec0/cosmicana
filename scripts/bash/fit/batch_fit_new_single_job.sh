#!/bin/bash

[[ $# < 3 ]] && echo "Give at least 3 arguments" && exit -1

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup root v6_22_08d -q "e20:p392:prof"

DEVDIR=/home/pec/dune/cosmics/cosmicana

INFILE=$1
BATCH=$2
OUTPREF=$3
STARTPART=0
[ -n "$4" ] && STARTPART=$4

SCRIPT_NAME=$DEVDIR/scripts/stdAna/fitLifeTime_langau_new_batch.C

OUTDIR=$(dirname $OUTPREF)

LOGDIR=$OUTDIR/logs

mkdir -p $OUTDIR
mkdir -p $LOGDIR

LOGFNAME=${LOGDIR}/$(basename $OUTPREF)MPV_fits.log


nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}($BATCH, $STARTPART, \"$OUTPREF\", \"\", \"MPV\", \"$INFILE\")" |& tee $LOGFNAME

# # Merge resulting trees
# TOMERGE=${OUTPREF_TEMPLATE/BATCHNO/$batchno}MPV_fits.root
# TOMERGE=${TOMERGE/GROUPNO/%d}
# MERGEOUT=${TOMERGE/group%d/tree_merged}
# LOGFILE=${MERGEOUT/plots/logs}
# LOGFILE=${LOGFILE/.root/.log}
# echo $TOMERGE $MERGEOUT $LOGFILE
# echo "
# auto tree = new TChain(\"elifetime\");
# for (int i=0;i<$NGROUPS;++i) {
#     tree->Add(Form(\"$TOMERGE\", i));
# }
# auto outf = TFile::Open(\"$MERGEOUT\",\"recreate\");
# tree->CloneTree(-1,\"fast\"); outf->Write(); outf->Close();" |root -l -b &> $LOGFILE
