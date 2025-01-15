#!/bin/bash

for i in "$@"; do
    echo "Processing file: $i"
    while IFS= read -r line; do
        echo " "
        echo "Executing command: $line"
        $line
        # sleep 1
    done < "$i"
done


