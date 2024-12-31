CC = mpicc
CFLAGS = -O3 -std=c99 -g -Wall

N      =
M      =
N_GEN  =
T_POB  =
NP     =
NGM    =

EXEC   = mpi
OUTFILE = ../output/out
C_FILES = main.c mh.c io.c

$(EXEC): $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME

test_mpi:
	mpirun -np $(NP) ./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(NGM) \
		>> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(NP)_$(NGM).txt

clean:
	rm -f $(EXEC)
