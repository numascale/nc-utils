echo "Read four counters all nodes"
./nc_perf fabric-loop-05.json -counter-read all 0 
./nc_perf fabric-loop-05.json -counter-read all 1         
./nc_perf fabric-loop-05.json -counter-read all 2 
./nc_perf fabric-loop-05.json -counter-read all 3 
