#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <assert.h>


inline uint64_t tick()
{
    uint32_t tmp[2];
    __asm__ ("rdtsc" : "=a" (tmp[1]), "=d" (tmp[0]) : "c" (0x10) );
    return (((uint64_t)tmp[0]) << 32) | tmp[1];
}


int main(int ac, char **av)
{
	cpu_set_t cpuset;
	unsigned int a, d, c, e, f;
	int i;
        uint64_t start, end;

	if (ac != 3) {
		fprintf(stderr, "usage: %s cpu-id pmc-num\n", av[0]);
		exit(EXIT_FAILURE);
	}
	
	i = atoi(av[1]);
	c = atoi(av[2]);
	
	CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        assert(sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0);
	
	__asm __volatile("rdpmc" : "=a" (a), "=d" (d) : "c" (c));
 	start = tick();
unsigned long MAX_LOWER = 12345678910;
unsigned long long MAX_UPPER = 5924408874716868394;

    unsigned long long num_iters = 120000000000/100;
    unsigned long long x = 0;
    unsigned long long counter[5];

    unsigned long s = 123456;    unsigned long z = 37;
    unsigned long t = 7284354168;
    unsigned long v = 35914;


    for(x = 0; x < num_iters; x++){
//        s = (s * z) ;//% MAX_LOWER;
//        t = (t * v) ;//% MAX_UPPER;
    }



    __asm __volatile("rdpmc" : "=a" (e), "=d" (f) : "c" (c));
    printf("%lx %lx\n", ((unsigned long) a) | (((unsigned long) d) << 32), ((unsigned long) e) | (((unsigned long) f) << 32));
	end = tick();
	printf("%x %x %x %x\n", d, a, f, e);
	printf("rdpmc difference = %lxh %lu\n", 
		(((unsigned long) e) | (((unsigned long) f) << 32)) - 
		(((unsigned long) a) | (((unsigned long) d) << 32)),
		(((unsigned long) e) | (((unsigned long) f) << 32)) - 
		(((unsigned long) a) | (((unsigned long) d) << 32)));

	printf("wallclock difference = %lxh %lu\n", end - start, end - start);


	return 0;
}
