echo "Clear all counters"
./nc_perf -counter-clear 0 0
./nc_perf -counter-clear 0 1
./nc_perf -counter-clear 0 2
./nc_perf -counter-clear 0 3

echo "Select them - only node 0" 
./nc_perf -counter-select 0 0 1
./nc_perf -counter-select 0 1 1
./nc_perf -counter-select 0 2 6
./nc_perf -counter-select 0 3 6

echo "Mask node 0"
./nc_perf -counter-mask 0 0 6
./nc_perf -counter-mask 0 1 5
./nc_perf -counter-mask 0 2 3
./nc_perf -counter-mask 0 3 2

echo "Mask node 0 again - Should return busy"
./nc_perf -counter-mask 0 0 6
./nc_perf -counter-mask 0 1 5
./nc_perf -counter-mask 0 2 3
./nc_perf -counter-mask 0 3 2
