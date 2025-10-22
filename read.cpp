/*
 *  Read tsc from shared memory and test drift
 */
#include <iostream>
#include <fstream>
#include <filesystem>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <csignal>
#include "getCpuFrequency.h"
#include <thread>
#include <chrono>

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
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Output file
    std::ofstream file;
    std::string destDir = "../test_results/";
    std::string file_name = destDir + "output.txt";
    if(!std::filesystem::exists(destDir)) { std::filesystem::create_directories(destDir); }
    file.open(file_name);

    size_t read_tsc, now_tsc;     // Save tsc from shared memory and own tsc
    size_t diff_sum = 0;          // Sum up tsc difference over all measures
    size_t cur_diff = 0;          // Save diff of current iteration    
    size_t outliers = 0;          // Save amout of outliers over 1000
    
    size_t i = 0;                   // Count number of runs
    while (i < nb_runs && run) {
        // Get own tsc and tsc from shared memory
        now_tsc = rte_get_tsc_cycles();
        rte_smp_rmb();
        read_tsc = *(volatile size_t*)ptr;

        // Subtract higher value from lower
        if (now_tsc > read_tsc) {
            cur_diff = now_tsc - read_tsc;
        } else {
            cur_diff = read_tsc - now_tsc;
        }   

        // Ignore outliers from hardware
        if (cur_diff > 1000) {
            outliers++;
            continue;
        }
        // Print diff
        file << cur_diff * 1000 * 1000 * 1000 / freq << ";";        
        diff_sum += cur_diff;
        i++;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    file.close();
    std::cout << std::endl;
    std::cout << "Core " << coreId << " - Avg. tsc difference over " << i << " runs: " << diff_sum / i 
              << " (" << (diff_sum / i) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
    std::cout << "Core " << coreId << " - Outliers: " << outliers << " ignored values over 1000" << std::endl;
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