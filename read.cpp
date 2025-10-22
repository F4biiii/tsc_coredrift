/*
 *  Read tsc from shared memory and test drift
 */
#include <iostream>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "getCpuFrequency.h"

static constexpr size_t nb_runs = 100000000;

int read_tsc(void* arg) {
    int coreId = *(reinterpret_cast<int *>(arg));
    uint64_t freq = getCpuFrequency();

    const char* name = "/shared_mem";
    const int mem_size = 8;

    // Shared memory object
    int shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }
    
    // Map memory to ptr
    void* ptr = mmap(0, mem_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    
    uint64_t read_tsc, now_tsc;
    uint64_t diff = 0;
    for (size_t i = 0; i < nb_runs; i++) {
        now_tsc = rte_get_tsc_cycles();
        read_tsc = *(reinterpret_cast<uint64_t *>(ptr));
        if (now_tsc > read_tsc) {
            diff += now_tsc - read_tsc;
            std::cout << "Core " << coreId << " - Tsc drift: " << now_tsc - read_tsc 
                      << " (" << (now_tsc - read_tsc) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
        } else {
            diff += read_tsc - now_tsc;
            std::cout << "Core " << coreId << " - Tsc drift: " << read_tsc - now_tsc
                      << " (" << (read_tsc - now_tsc) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
        }
    }

    std::cout << "Core " << coreId << " - Avg. tsc difference over " << nb_runs << " runs: " << diff / nb_runs 
              << " (" << (diff / nb_runs) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;

    return 0;
}

int main(void) {
    // EAL arguments
    const char *rte_argv[] = {"read.cpp", "-l", "1-2", "-n", "4", "-a", "07:00.0", "--file-prefix=dpdk2", "--iova-mode=va", nullptr};
    int rte_argc = static_cast<int>(sizeof(rte_argv) / sizeof(rte_argv[0])) - 1;
    
    // Initialize EAL
    if (rte_eal_init(rte_argc, const_cast<char **>(rte_argv)) < 0) {
        rte_exit(EXIT_FAILURE,"Failed to initialize eal: %s\n", rte_strerror(rte_errno));
    }

    // Start worker cores   
    int i = 2;
    unsigned lcore_id;
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        rte_eal_remote_launch(read_tsc, &i, lcore_id);
        i++;
    }
    // Join worker cores
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        rte_eal_wait_lcore(lcore_id);
    }

    return 0;
}