echo "Stop four counters all nodes"
./nc_perf fabric-loop-05.json -counter-stop all 0 
./nc_perf fabric-loop-05.json -counter-stop all 1         
./nc_perf fabric-loop-05.json -counter-stop all 2 
./nc_perf fabric-loop-05.json -counter-stop all 3 
