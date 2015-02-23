#!/bin/bash

POSITION="XorYcoords"
MOTE_IDENT="moteIdentifier"
SIMSPERSCENARIO=4

declare -a RESULTFILES=('raw' 'delay' 'summary' 'duplicates' 'duplicates_raw');
declare -a PERCENTTOPOLOGIES=('25' '50' '75' '100');
declare -a ENGINECONF=('SMRF' 'BMRF' 'BMRF' 'BMRF'
					   'BMRF' 'BMRF' 'BMRF' 'BMRF'
					   'BMRF');
declare -a MODECONF=(  'MIXED' 'BROADCAST' 'MIXED' 'MIXED'
					   'MIXED' 'MIXED' 'MIXED' 'MIXED'
					   'UNICAST');
declare -a THRESHOLDCONF=(  '1' '1' '1' '2'
                            '3' '4' '5' '6'
                            '1');

declare -a RANDOMTOPOLOGIES=('random_topology_A' 'random_topology_B')

declare -a remaining_motes=()
declare -a remaining_motes_aux=()
declare -a motes_names=()
declare -a motes_names_aux=()

for topology in "${RANDOMTOPOLOGIES[@]}"
do
    mkdir $topology
    for ((j=1; j <= $SIMSPERSCENARIO; j++))
    do
        echo "Generating random topologies"
        for percenttop in "${PERCENTTOPOLOGIES[@]}"
        do
            randomseed=$[RANDOM % 999999]
            subscribed_number=$((50*$percenttop/100))
            remaining_motes=(1 $((50-$subscribed_number)) $subscribed_number)
            motes_names=('sky1' 'sky2' 'sky3')
            cp $topology".csc" "multicast-sim-"$percenttop"-percent_tosim.csc"
            sed -i s/\#randomseed\#/$randomseed/ "multicast-sim-"$percenttop"-percent_tosim.csc"

            rand_mote="moteIdentifier"
            while sed -i "/$MOTE_IDENT/,\${s//$rand_mote/;:a;n;ba};\$q1" "multicast-sim-"$percenttop"-percent_tosim.csc"
            do
                motes_names_aux=()
                remaining_motes_aux=()
                for ((h=0; h <${#remaining_motes[@]}; h++))
                do
                    if [ "${remaining_motes[$h]}" != "0" ]; then
                        motes_names_aux+=(${motes_names[$h]})
                        remaining_motes_aux+=(${remaining_motes[$h]})
                    fi
                done
                if [[ "${#motes_names[@]}" == "1" ]]; then
                    random_number=0
                else
                    random_number=$[RANDOM % ((${#motes_names[@]}-1))]
                fi
                remaining_motes_aux[$random_number]=$((${remaining_motes_aux[$random_number]}-1))
                remaining_motes=("${remaining_motes_aux[@]}")
                motes_names=("${motes_names_aux[@]}")
                rand_mote=${motes_names[$random_number]}
            done
        done

        for ((i=0; i<${#ENGINECONF[@]}; ++i)); do
            echo "Loading project-conf"
            cp project-conf-sample.h project-conf.h
            sed -i s/\#engine\#/UIP_MCAST6_ENGINE_${ENGINECONF[i]}/ project-conf.h
            sed -i s/\#mode\#/BMRF_${MODECONF[i]}_MODE/ project-conf.h
            sed -i s/\#threshold\#/${THRESHOLDCONF[i]}/ project-conf.h

            for percenttop in "${PERCENTTOPOLOGIES[@]}"
            do
                simfile="multicast-sim-"$percenttop"-percent_tosim.csc"

                echo "Starting simulation of "$percenttop"% topologie with "${ENGINECONF[i]}" engine, "${MODECONF[i]}" mode and threshold "${THRESHOLDCONF[i]}"."
                echo "Iteration number "$j
                make TARGET=cooja nogui=Simulation $simfile

                for filename in "${RESULTFILES[@]}"
                do
                    mv $filename"_results.csv" $topology"/"$filename"_results_"$percenttop"-percent_"${ENGINECONF[i]}"_"${MODECONF[i]}"_"${THRESHOLDCONF[i]}"_iteration-"$j".csv"
                done
            done

            rm project-conf.h
        done

        for topologie in "${SIMTOPOLOGIES[@]}"
        do
            rm "multicast-sim-"$topologie"-percent_tosim.csc"
        done
    done
done