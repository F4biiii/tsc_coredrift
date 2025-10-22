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
#include <csignal>
#include "getCpuFrequency.h"

static constexpr size_t nb_runs = 1000000000000;
bool run = true;

// Flip flag on Ctrl + C to break loop and still print results
void signal_handler(int sig) {
    run = false;
}

int read_tsc(void* arg) {
    int coreId = *(reinterpret_cast<int *>(arg));
    std::cout << "Core " << coreId << " started" << std::endl;

    const char* name = "/shared_mem";
    const int mem_size = 8;
    uint64_t freq = getCpuFrequency();

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
    uint64_t read_tsc, now_tsc;     // Save tsc from shared memory and own tsc
    uint64_t diff = 0;              // Total tsc difference over all measures
    size_t i = 0;                   // Count number of runs
    while (i < nb_runs && run) {
        // Get own tsc and tsc from shared memory
        now_tsc = rte_get_tsc_cycles();
        read_tsc = *(reinterpret_cast<uint64_t *>(ptr));
        // Subtract higher value from lower
        if (now_tsc > read_tsc) {
            diff += now_tsc - read_tsc;
            std::cout << "Core " << coreId << " - Tsc drift: " << now_tsc - read_tsc 
                      << " (" << (now_tsc - read_tsc) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
        } else {
            diff += read_tsc - now_tsc;
            std::cout << "Core " << coreId << " - Tsc drift: " << read_tsc - now_tsc
                      << " (" << (read_tsc - now_tsc) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
        }    
        i++;
    }
    std::cout << "Core " << coreId << " - Avg. tsc difference over " << i << " runs: " << diff / i 
              << " (" << (diff / i) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;

    return 0;
}

int main(void) {
    // Let Ctrl + C flip a flag to print result before exit
    std::signal(SIGINT, signal_handler);

    // EAL arguments
    const char *rte_argv[] = {"read.cpp", "-l", "3-4", "-n", "4", "-a", "07:00.0", "--file-prefix=dpdk2", "--iova-mode=va", nullptr};
    int rte_argc = static_cast<int>(sizeof(rte_argv) / sizeof(rte_argv[0])) - 1;
    
    // Initialize EAL
    if (rte_eal_init(rte_argc, const_cast<char **>(rte_argv)) < 0) {
        rte_exit(EXIT_FAILURE,"Failed to initialize eal: %s\n", rte_strerror(rte_errno));
    }
    // Start worker cores   
    int i = 4;
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