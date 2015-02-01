#!/bin/bash

function random0to200()
{
    local rand=$[100 + (RANDOM % 100)]$[1000 + (RANDOM % 1000)]
	echo "$[RANDOM % 200].${rand:1:2}${rand:4:3}"
}

function replaceMore()
{
    local result=1
    declare -a files=("${!1}")

    for topfile in "${files[@]}"
    do
        if sed -i "/$2/,\${s//$3/;:a;n;ba};\$q1" "multicast-sim-"$topfile"-percent_tosim.csc" ; then
            result=0
        fi
    done

    return $result
}

POSITION="XorYcoords"
SIMSPERSCENARIO=10

declare -a RESULTFILES=('raw' 'delay' 'summary');
declare -a SIMTOPOLOGIES=('25' '50' '75' '100');
declare -a ENGINECONF=('SMRF' 'BMRF' 'BMRF' 'BMRF'
					   'BMRF' 'BMRF' 'BMRF' 'BMRF'
					   'BMRF');
declare -a MODECONF=(   'MIXED' 'BROADCAST' 'MIXED' 'MIXED'
					   'MIXED' 'MIXED' 'MIXED' 'MIXED'
					   'UNICAST');
declare -a THRESHOLDCONF=(  '1' '1' '1' '2'
                            '3' '4' '5' '6'
                            '1');

for ((j=1; j <= $SIMSPERSCENARIO; j++))
do
    echo "Generating random positions"
    randomseed=$[RANDOM % 999999]
    for topologie in "${SIMTOPOLOGIES[@]}"
    do
        cp "multicast-sim-"$topologie"-percent.csc" "multicast-sim-"$topologie"-percent_tosim.csc"
        sed -i s/\#randomseed\#/$randomseed/ "multicast-sim-"$topologie"-percent_tosim.csc"
    done

    randompos=$(random0to200)
    while replaceMore SIMTOPOLOGIES[@] $POSITION $randompos
    do
        randompos=$(random0to200)
    done

    for ((i=0; i<${#ENGINECONF[@]}; ++i)); do
        echo "Loading project-conf"
        cp project-conf-sample.h project-conf.h
        sed -i s/\#engine\#/UIP_MCAST6_ENGINE_${ENGINECONF[i]}/ project-conf.h
        sed -i s/\#mode\#/BMRF_${MODECONF[i]}_MODE/ project-conf.h
        sed -i s/\#threshold\#/${THRESHOLDCONF[i]}/ project-conf.h

        for topologie in "${SIMTOPOLOGIES[@]}"
        do
            simfile="multicast-sim-"$topologie"-percent_tosim.csc"

            echo "Starting simulation of "$topologie"% topologie with "${ENGINECONF[i]}" engine, "${MODECONF[i]}" mode and threshold "${THRESHOLDCONF[i]}"."
            echo "Iteration number "$j
            make TARGET=cooja nogui=Simulation $simfile

            for filename in "${RESULTFILES[@]}"
            do
                mv $filename"_results.csv" $filename"_results_"$topologie"-percent_"${ENGINECONF[i]}"_"${MODECONF[i]}"_"${THRESHOLDCONF[i]}"_iteration-"$j".csv"
            done
        done

        rm project-conf.h
    done

    for topologie in "${SIMTOPOLOGIES[@]}"
    do
        rm "multicast-sim-"$topologie"-percent_tosim.csc"
    done
done