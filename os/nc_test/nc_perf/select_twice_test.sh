echo "Clear them"
./nc_perf -counter-clear 0 0
./nc_perf -counter-clear 0 1
./nc_perf -counter-clear 0 2
./nc_perf -counter-clear 0 3

echo "Select them - only node 0" 
./nc_perf -counter-select 0 0 1
./nc_perf -counter-select 0 1 1
./nc_perf -counter-select 0 2 6
./nc_perf -counter-select 0 3 6

echo "Try to select them again - This should fail with BUSY=2"
./nc_perf -counter-select 0 0 1
./nc_perf -counter-select 0 1 1
./nc_perf -counter-select 0 2 6
./nc_perf -counter-select 0 3 6

echo "Clear them"
./nc_perf -counter-clear 0 0
./nc_perf -counter-clear 0 1
./nc_perf -counter-clear 0 2
./nc_perf -counter-clear 0 3

echo "Now select should be nice"
./nc_perf -counter-select 0 0 1
./nc_perf -counter-select 0 1 1
./nc_perf -counter-select 0 2 6
./nc_perf -counter-select 0 3 6
