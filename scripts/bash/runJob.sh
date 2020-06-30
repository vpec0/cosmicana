#!/bin/bash

RUNNO=$1
GENFHICL=prodMUSUN_dune10kt_${RUNNO}.fcl

OUTFNAME=data/$RUNNO/MUSUN_dunefd_${RUNNO}
LOGNAME=logs/$RUNNO/MUSUN_dunefd_${RUNNO}
install -d data/$RUNNO
install -d logs/$RUNNO


lar -c ./fcl/$GENFHICL -n500 -o ${OUTFNAME}_gen.root &> ${LOGNAME}_gen.log

lar -n -1  -c standard_g4_dune10kt.fcl -o ${OUTFNAME}_gen_g4.root ${OUTFNAME}_gen.root &> ${LOGNAME}_gen_g4.log

lar -n -1  -c standard_detsim_dune10kt.fcl -o ${OUTFNAME}_gen_g4_detsim.root ${OUTFNAME}_gen_g4.root &> ${LOGNAME}_gen_g4_detsim.log

lar -n -1  -c standard_reco_dune10kt.fcl -o ${OUTFNAME}_gen_g4_detsim_reco.root ${OUTFNAME}_gen_g4_detsim.root &> ${LOGNAME}_gen_g4_detsim_reco.log
