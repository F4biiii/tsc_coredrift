#include "getCpuFrequency.h"


#define DEBUG
#define TEST_RUNS 5


uint64_t getCpuFrequency (void) {

    uint64_t start, mean, results[TEST_RUNS];
    struct timespec req, rem;

    req.tv_sec = 1;        // 1 second
    req.tv_nsec = 0;       // 0 nanoseconds

    for ( int i=0 ; i<TEST_RUNS ; i++ ) {
		start = read_tsc();

		if (nanosleep(&req, &rem) == -1) {
			perror("nanosleep failed");
			return (uint64_t) -1;
		} else {
			results[i] = read_tscp() - start;
		}
    }

    mean = (uint64_t)0;
    for ( int i=0 ; i<TEST_RUNS ; i++ ) {
    	mean += results[i];
		// #ifdef DEBUG
    	// 	printf("getCpuFrequency(): value %2i: %lu\n", i, results[i]);
		// #endif
    }
    mean = mean / TEST_RUNS;
	#ifdef DEBUG
		printf ( "getCpuFrequency(): returning %lu\n", mean );
	#endif

    return mean;
}
