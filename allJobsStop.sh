#!/bin/bash

stop(){
    ./jobCommander stop "$1"
}

# Get the queued jobs from the poll command
queued=$(./jobCommander poll queued)

skip_first=1

# Extract job IDs from queued jobs output and stop them
IFS=$'\n'
for job in $queued; do
    if [ $skip_first -eq 1 ]; then
        skip_first=0
        continue
    fi
    job_id=$(echo "$job" | cut -d ',' -f 1)
    stop "$job_id"
done

# Get the running jobs from the poll command
running=$(./jobCommander poll running)

skip_first=1

# Extract job IDs from running jobs output and stop them
IFS=$'\n'
for job in $running; do
    if [ $skip_first -eq 1 ]; then
        skip_first=0
        continue
    fi
    job_id=$(echo "$job" | cut -d ',' -f 1)
    stop "$job_id"
done
