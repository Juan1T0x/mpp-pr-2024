CC = mpicc
CFLAGS = -O3 -std=c99 -g -Wall

mpi: main.c mh.c io.c derivados_mpi.c
	$(CC) $(CFLAGS) main.c mh.c io.c derivados_mpi.c -lm -o mpi -DTIME

run_mpi:
	mpirun -np 1 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 2 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 4 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 6 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 12 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 18 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 24 ./mpi 5000 2000 1 500 >> ../output/out
	mpirun -np 30 ./mpi 5000 2000 1 500 >> ../output/out

clean:
	rm -f sec
