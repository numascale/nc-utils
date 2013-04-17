echo "Clear all counters - all nodes"
./nc_perf -counter-clear all 4
./nc_perf -counter-clear all 5
./nc_perf -counter-clear all 6
./nc_perf -counter-clear all 7

echo "Select them - all nodes"
./nc_perf -counter-select all 4 1
./nc_perf -counter-select all 5 1
./nc_perf -counter-select all 6 6
./nc_perf -counter-select all 7 6

echo "Mask all nodes node - This starts the counting that was stopped by clear:"
./nc_perf -counter-mask all 4 6
./nc_perf -counter-mask all 5 5
./nc_perf -counter-mask all 6 3
./nc_perf -counter-mask all 7 2

echo "read counter - all nodes"
./nc_perf -counter-read all 4 
./nc_perf -counter-read all 5         
./nc_perf -counter-read all 6 
./nc_perf -counter-read all 7 

echo "stop counter - all nodes"
./nc_perf -counter-stop all 4 
./nc_perf -counter-stop all 5         
./nc_perf -counter-stop all 6 
./nc_perf -counter-stop all 7 

echo "read counter - all nodes"
./nc_perf -counter-read all 4 
./nc_perf -counter-read all 5         
./nc_perf -counter-read all 6 
./nc_perf -counter-read all 7 

echo "read again - all nodes"
./nc_perf -counter-read all 4 
./nc_perf -counter-read all 5         
./nc_perf -counter-read all 6 
./nc_perf -counter-read all 7 