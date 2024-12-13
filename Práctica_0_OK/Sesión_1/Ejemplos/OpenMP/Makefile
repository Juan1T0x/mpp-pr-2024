CC = gcc
CFLAGS = -O3 -std=c99 -g -Wall

N = 
EXEC = omp

omp: main.c
	$(CC) $(CFLAGS) main.c -fopenmp -o $(EXEC) -DTIME

test_omp:
	./$(EXEC) $(N)

clean:
	rm -f ./$(EXEC)
