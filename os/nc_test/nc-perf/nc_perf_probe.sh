echo "Clear all counters - node 0"
./nc_perf fabric-loop-05.json -counter-clear 0 6
./nc_perf fabric-loop-05.json -counter-clear 0 7


echo "Select them - only node 0" 
./nc_perf fabric-loop-05.json -counter-select 0 6 7
./nc_perf fabric-loop-05.json -counter-select 0 7 7

echo "Mask node 0"
./nc_perf fabric-loop-05.json -counter-mask 0 6 7
./nc_perf fabric-loop-05.json -counter-mask 0 7 3

echo "read counter - node 0"
./nc_perf fabric-loop-05.json -counter-read 0 6
./nc_perf fabric-loop-05.json -counter-read 0 7         

