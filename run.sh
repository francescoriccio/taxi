#!/bin/bash - 
#===============================================================================
#
#          FILE: run.sh
# 
#         USAGE: ./run.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: Aijun Bai (), 
#  ORGANIZATION: 
#       CREATED: 01/15/2017 10:37
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

VERSION="release"
SIZE="5"
TRIALS="200"
EPISODES="100000"
PLT="plot.gnuplot"
OPT=""

declare -a ALGS=(
        "HAMQ-INT" 
        "HAMQ" 
        "MaxQ-Q" 
        "MaxQ-0" 
        "MaxQ-OP" 
    )

if [ $VERSION = "debug" ]; then
    OPT="--debug $OPT"
fi

ulimit -c unlimited

make cleanall
make $VERSION
mkdir -p data
cd data

cp ../${PLT} plot.gnuplot
cp ../plot.sh .

echo "set output \"reward.png\"" >> plot.gnuplot
echo "plot \\" >> plot.gnuplot

LS="1"
for alg in "${ALGS[@]}"; do
    time ../taxi $OPT --size $SIZE --trials $TRIALS --episodes $EPISODES \
        --train --multithreaded --$alg > ${alg}.out
    echo "'${alg}.out' u 1:2:3 t \"${alg}\" ls $LS with yerrorlines, \\" >> plot.gnuplot
    LS="`expr $LS + 1`"
done

./plot.sh

