/*
 *  Write tsc to shared memory
 */
#include <iostream>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

int write_tsc(void* arg) {
    const char* name = "/shared_mem";
    const int mem_size = 8;

    // Shared memory object
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }
    
    // Truncate memory
    ftruncate(shm_fd, mem_size);
    
    // Map memory to ptr
    void* ptr = mmap(0, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    
    uint64_t tsc_val = 0;
    while ( 1 ) {
        tsc_val = rte_get_tsc_cycles();
        memcpy(ptr, &tsc_val, sizeof(uint64_t));
    }

    return 0;
}

int main(int argc, char** argv) {
    // EAL arguments
    const char *rte_argv[] = {"write.cpp", "-l", "3-4", "-n", "4", "-a", "04:00.0", "--file-prefix=dpdk1", "--iova-mode=va", nullptr};
    int rte_argc = static_cast<int>(sizeof(rte_argv) / sizeof(rte_argv[0])) - 1;

    // Initialize EAL
    if (rte_eal_init(rte_argc, const_cast<char **>(rte_argv)) < 0) {
        rte_exit(EXIT_FAILURE,"Failed to initialize eal: %s\n", rte_strerror(rte_errno));
    }

    // Start worker cores
    int i = 0;
    unsigned lcore_id;
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        rte_eal_remote_launch(write_tsc, nullptr, lcore_id);
        i++;
    }
    // Join worker cores
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        rte_eal_wait_lcore(lcore_id);
    }

    return 0;
}