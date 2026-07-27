#!/bin/bash

# Target file
OUTPUT_FILE="memcpuperf.txt"

echo "Monitoring started... Press [CTRL+C] to stop."

while true; do
    # 1. Get current timestamp
    TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

    # 2. Calculate CPU usage (User + System)
    CPU_USAGE=$(top -bn1 | grep "Cpu(s)" | awk '{print $2 + $4}')

    # 3. Calculate Memory usage percentage
    MEM_USAGE=$(free | awk '/Mem/ {printf("%.2f%%"), $3/$2*100.0}')

    # 4. Log it to the file
    echo "$TIMESTAMP | CPU: ${CPU_USAGE}% | MEM: $MEM_USAGE" >> "$OUTPUT_FILE"

    # Wait 1 second before the next check
    sleep 1
done
