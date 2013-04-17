echo "Clear all counters - all nodes"
./nc_perf -counter-clear all 0
./nc_perf -counter-clear all 1
./nc_perf -counter-clear all 2
./nc_perf -counter-clear all 3

echo "Select them - all nodes"
./nc_perf -counter-select all 0 1
./nc_perf -counter-select all 1 1
./nc_perf -counter-select all 2 6
./nc_perf -counter-select all 3 6

echo "Mask all nodes node - This starts the counting that was stopped by clear:"
./nc_perf -counter-mask all 0 6
./nc_perf -counter-mask all 1 5
./nc_perf -counter-mask all 2 3
./nc_perf -counter-mask all 3 2

echo "read counter - all nodes"
./nc_perf -counter-read all 0 
./nc_perf -counter-read all 1         
./nc_perf -counter-read all 2 
./nc_perf -counter-read all 3 
