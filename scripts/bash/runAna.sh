#!/bin/bash


for i in `ls data/2000*/*_reco.root` ; do
    echo $i
    OUTFNAME=$(echo $i | sed 's/_reco.root/_reco_ana.root/g')
    LOGNAME=$(echo $OUTFNAME | sed 's|data/|logs/|g' | sed 's/.root/.log/g')
    echo $OUTFNAME $LOGNAME
    lar -c standard_ana_dune10kt.fcl $i -T $OUTFNAME -o /dev/null -n -1 2>&1 | tee $LOGNAME
done
