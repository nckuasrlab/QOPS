#!/bin/bash
# sudo apt install cpufrequtils
NUM_CORES=$(nproc)
for ((i=0; i<NUM_CORES; i++)); do
    sudo cpufreq-set -c $i -f 2700000
    #sudo cpufreq-set -c $i -g ondemand
    #sudo cpufreq-set -c $i -g performance
done
