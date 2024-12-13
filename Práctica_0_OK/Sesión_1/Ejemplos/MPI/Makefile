CC = mpicc
CFLAGS = -O3 -std=c99 -g -Wall

N = 
NP = 
EXEC = mpi

mpi: main.c
	$(CC) $(CFLAGS) main.c -o $(EXEC) -DTIME

test_mpi:
	mpirun -np $(NP) ./$(EXEC) $(N)

clean:
	rm -f $(EXEC)
