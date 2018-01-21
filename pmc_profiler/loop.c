#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define PIPE_FILENO 3

#define N 100000000
long array[N];
unsigned long start_pmc[10][10], end_pmc[10][10];

inline void RECORD_PMC()
{
write(PIPE_FILENO, "PMC_CMD: read_pmc\n", 18);
}

inline uint64_t tick()
{
    uint32_t tmp[2];
    __asm__ ("rdtsc" : "=a" (tmp[1]), "=d" (tmp[0]) : "c" (0x10) );
    return (((uint64_t)tmp[0]) << 32) | tmp[1];
}


inline uint64_t read_pmc(int c)
{
unsigned int a, d;

	__asm __volatile("rdpmc" : "=a" (a), "=d" (d) : "c" (c));
	return (uint64_t) ((unsigned long) a) | (((unsigned long) d) << 32);
}

main()
{
unsigned long i, j, sum=0;

//    for(i=0; i<N; i++)
//        array[i] = 1;

start_pmc[0][0] = read_pmc(2);
start_pmc[1][0] = tick();

/*    for(i=0;i<N;i++) sum += array[i];
*/
    for(i=0; i<N; i++)
        sum += array[i];

end_pmc[0][0] = read_pmc(2);
end_pmc[1][0] = tick();

start_pmc[0][1] = read_pmc(2);
start_pmc[1][1] = tick();

    for(i=0;i<N;i++) sum += array[i];

/*
    for(i=0; i<N; i++)
	sum += array[0];
*/

end_pmc[0][1] = read_pmc(2);
end_pmc[1][1] = tick();


printf("%lu %lu %lu %lu\n", end_pmc[0][0] - start_pmc[0][0], end_pmc[1][0] - start_pmc[1][0], end_pmc[0][1] - start_pmc[0][1], end_pmc[1][1] - start_pmc[1][1]);

printf("%f %f\n", (float)(end_pmc[0][0] - start_pmc[0][0])/(end_pmc[1][0] - start_pmc[1][0]), (float)(end_pmc[0][1] - start_pmc[0][1])/(end_pmc[1][1] - start_pmc[1][1]));
}
