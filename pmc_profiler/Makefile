ALL: pmc_profiler loop

pmc_profiler: pmc_profiler.c
	gcc -O3 -o pmc_profiler pmc_profiler.c

loop: loop.c
	gcc -O3 -o loop loop.c

clean:
	rm -f pmc_profiler loop *.o

