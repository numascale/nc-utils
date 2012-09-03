echo "START (Clear, select and mask) all counters"
./nc_perf fabric-loop-05.json -counter-start all 0 1 6
./nc_perf fabric-loop-05.json -counter-start all 1 1 5
./nc_perf fabric-loop-05.json -counter-start all 2 6 3
./nc_perf fabric-loop-05.json -counter-start all 3 6 2

echo "Read all nodes all counters"
./nc_perf fabric-loop-05.json -counter-read all 0 
./nc_perf fabric-loop-05.json -counter-read all 1         
./nc_perf fabric-loop-05.json -counter-read all 2 
./nc_perf fabric-loop-05.json -counter-read all 3 
