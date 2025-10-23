/*
 *  Read tsc from shared memory and test drift
 */
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/mman.h>
#include <fcntl.h>
#include <csignal>
#include <thread>
#include <chrono>
#include "getCpuFrequency.h"
#include "timingServices.h"

static constexpr size_t nb_runs = 1000000000000;
bool run = true;

// Flip flag on Ctrl + C to break loop and still print results
static void signal_handler(int sig) {
    run = false;
}

static void read_tsc_fun() {
    std::cout << "Core started" << std::endl;

    const char* name = "/shared_mem";
    const int mem_size = 8;
    uint64_t freq = getCpuFrequency();

    // Shared memory object
    int shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }
    // Map memory to ptr
    void* ptr = mmap(0, mem_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // Output file
    std::ofstream file;
    std::string destDir = "../test_results/";
    std::string file_name = destDir + "output.txt";
    if(!std::filesystem::exists(destDir)) { std::filesystem::create_directories(destDir); }
    file.open(file_name);

    size_t rec_tsc, now_tsc;      // Save tsc from shared memory and own tsc
    size_t diff_sum = 0;          // Sum up tsc difference over all measures
    size_t cur_diff = 0;          // Save diff of current iteration    
    size_t outliers = 0;          // Save amout of outliers over 1000
    
    // Constantly read tsc from shared memory and compare with own tsc 
    size_t i = 0;                   
    while (i < nb_runs && run) {
        // Get own tsc and tsc from shared memory
        now_tsc = read_tsc();
        rec_tsc = *(volatile size_t*)ptr;

        // Subtract higher value from lower
        if (now_tsc > rec_tsc) {
            cur_diff = now_tsc - rec_tsc;
        } else {
            cur_diff = rec_tsc - now_tsc;
        }   

        // Ignore outliers from hardware
        if (cur_diff > 1000) {
            outliers++;
            continue;
        }
        // Print diff
        std::cout << "Diff: " << cur_diff << " runs: " << cur_diff 
                  << " (" << cur_diff * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
        file << cur_diff * 1000 * 1000 * 1000 / freq << ";";        
        diff_sum += cur_diff;
        i++;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    file.close();
    std::cout << std::endl;
    std::cout << "Avg. tsc difference over " << i << " runs: " << diff_sum / i 
              << " (" << (diff_sum / i) * 1000 * 1000 * 1000 / freq << " ns)" << std::endl;
    std::cout << "Outliers: " << outliers << " ignored values over 1000" << std::endl;
}

int main(void) {
    // Let Ctrl + C flip a flag to print result before exit
    std::signal(SIGINT, signal_handler);
    
    read_tsc_fun();    
    
    return 0;
}