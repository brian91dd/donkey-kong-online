#!/bin/bash



ipcs -s | awk 'NR > 3 {print "ipcrm -s", $2}' | sh
`rm /dev/shm/sem.*`