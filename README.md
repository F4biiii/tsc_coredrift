# tsc_coredrift
Test the tsc synchronization over different cores

Usage:

# start writer, writes tsc to shared memory
./write

# start reader in other terminal, reads tsc and compares with own tsc
./read
