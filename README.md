# tsc_coredrift
Test the tsc synchronization over different cores

# Usage (Redhawk 9.6):
Set time in run_test.py 
### Shield cores, execute script:
sudo shield -a 1-2 <br />
./run_test.py <br />

## Manual execution:
sudo shield -a 1-2 <br />
### start writer, writes tsc to shared memory
taskset -c 1 ./write <br />

### start reader in other terminal, reads tsc and compares with own tsc
taskset -c 1 ./read
