# tsc_coredrift
Test the tsc synchronization over different cores

# Usage (Redhawk 9.6):
Set time in run_test.py
sudo shield -a 1-2
./run_test.py

## Manual execution:
### start writer, writes tsc to shared memory
sudo shield -a 1-2
taskset -c 1 ./write

### start reader in other terminal, reads tsc and compares with own tsc
taskset -c 1 ./read
