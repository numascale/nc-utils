echo "BAD Clear all counters - node 0"
echo "TESTING: ./nc_perf fabr"
./nc_perf fabr
echo "TESTING:./nc_perf  -counter"
./nc_perf  -counter
echo "TESTING:./nc_perf  -counter-clear 0 "
./nc_perf -counter-clear 0 
echo "TESTING:./nc_perf  -counter-clear 0 3"
./nc_perf -counter-clear 0 3

echo "Clear all counters - node 0"
./nc_perf -counter-clear 0 0
./nc_perf -counter-clear 0 1
./nc_perf -counter-clear 0 2
./nc_perf -counter-clear 0 3

echo "BAD Select - only node 0" 
echo "TESTING ./nc_perf  -counter-select 0 "
./nc_perf -counter-select 0 
echo "TESTING ./nc_perf  -counter-select 0 1"
./nc_perf -counter-select 0 1
echo "TESTING ./nc_perf  -counter-select 0 9"
./nc_perf -counter-select 0 1
echo "TESTING ./nc_perf  -counter-select 0 3 9"
./nc_perf -counter-select 0 3 9

echo "Select them - only node 0 - FIX" 
./nc_perf -counter-clear 0 0
./nc_perf -counter-clear 0 1
./nc_perf -counter-clear 0 2
./nc_perf -counter-clear 0 3
./nc_perf -counter-select 0 0 1
./nc_perf -counter-select 0 1 1
./nc_perf -counter-select 0 2 6
./nc_perf -counter-select 0 3 6

echo "Mask node 0"
echo "TESTING ./nc_perf  -counter-mask 0"
./nc_perf -counter-mask 0
echo "TESTING ./nc_perf  -counter-mask 0 1"
./nc_perf -counter-mask 0 1
echo "TESTING ./nc_perf  -counter-mask 9 2 3"
./nc_perf -counter-mask 9 2 3
echo "TESTING ./nc_perf  -counter-mask 0 9 3"
./nc_perf -counter-mask 0 9 3
echo "TESTING ./nc_perf  -counter-mask 0 2 9"
./nc_perf -counter-mask 0 2 9

echo "Mask them - only node 0 - FIX" 
./nc_perf -counter-mask 0 0 6
./nc_perf -counter-mask 0 1 5
./nc_perf -counter-mask 0 2 3
./nc_perf -counter-mask 0 3 2

echo "read counter - node 0"
echo "TESTING ./nc_perf  -counter-read 0 "
./nc_perf -counter-read 0 
echo "TESTING ./nc_perf  -counter-read 0 9"
./nc_perf -counter-read 0 9
