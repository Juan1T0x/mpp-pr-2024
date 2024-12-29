CC = gcc
CFLAGS = -Wall -O3 -std=c99 -g

omp: main.c mh.c io.c
	$(CC) $(CFLAGS) main.c mh.c io.c -lm -fopenmp -o omp -DTIMEGenPobIni

run_omp:
	./omp 30000 28000 0 6 1 1 >> ../output/out
	./omp 30000 28000 0 6 2 1 >> ../output/out
	./omp 30000 28000 0 6 4 1 >> ../output/out
	./omp 30000 28000 0 6 6 1 >> ../output/out
	./omp 30000 28000 0 6 8 1 >> ../output/out
	./omp 30000 28000 0 6 10 1 >> ../output/out	
	./omp 30000 28000 0 6 12 1 >> ../output/out
	./omp 30000 28000 0 6 14 1 >> ../output/out
	./omp 30000 28000 0 6 16 1 >> ../output/out
	./omp 30000 28000 0 6 18 1 >> ../output/out
	./omp 30000 28000 0 6 20 1 >> ../output/out

clean:
	rm -f omp
