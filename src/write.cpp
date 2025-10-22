/*
 *  Write tsc to shared memory
 */
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "timingServices.h"

static void write_tsc() {
    std::cout << "Core started" << std::endl;
    
    const char* name = "/shared_mem";
    const int mem_size = 8;

    // Shared memory object
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }
    // Truncate memory
    ftruncate(shm_fd, mem_size);
    
    // Map memory to ptr
    void* ptr = mmap(0, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return;
    }
    // Repeatedly put tsc in shared memory
    size_t tsc_val = 0;
    while ( 1 ) {
        tsc_val = read_tsc();
        memcpy(ptr, &tsc_val, sizeof(size_t));
    }
}

int main(int argc, char** argv) {
    write_tsc();
    return 0;
}