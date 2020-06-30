# run as
# $ ./generate_gen_fhicls.sh FROM TO

for i in $(seq $1 $2) ; do
    sed "s/20000031/$[20000031 + i]/g" fcl/prodMUSUN_dune10kt.fcl > fcl/prodMUSUN_dune10kt_$[20000031 + i].fcl
done
