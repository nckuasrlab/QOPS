#!/bin/bash

# Outer loop values
outer=(3 4 5 6 7 8)

# Inner loop values
inner=(3 5)

# List of patterns to skip
skip_patterns=("24" "28" "29" "30" "31" "qknn" "qnn" "qaoa" "vqc")

q="32" # "24"

# Loop through circuit files
for circ in ./circuit/*.txt; do
    [ -e "$circ" ] || continue

    filename=$(basename "$circ")

    # Check for any matching pattern
    skip=false
    for pattern in "${skip_patterns[@]}"; do
        if [[ "$filename" == *$pattern* ]]; then
            echo "Skipping $filename (matched pattern: $pattern)"
            skip=true
            break
        fi
    done
    $skip && continue

    for o in "${outer[@]}"; do
        for i in "${inner[@]}"; do
            output_file="${o}_${i}_$(basename "$circ")"
            if [ -f "./tmp/fusion/${output_file}" ]; then
                echo "Skipping $output_file (already exists)"
                continue
            fi
            echo "./fusion ./circuit/${filename} ./tmp/fusion/${output_file} ${i} ${q} ${o}"
            DYNAMIC_COST_FILENAME=./log/gate_exe_time_aer.csv ./fusion ./circuit/${filename} ./tmp/fusion/${output_file} ${i} ${q} ${o} > fusion_dump.txt
        done
    done
done