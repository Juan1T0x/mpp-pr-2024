CC       = mpicc
CFLAGS   = -O3 -Wall -std=c99 -g -fopenmp

N     =
M     =
N_GEN =
T_POB =
NP    =
NGM   =
THREADS =

EXEC   = sec
OUTFILE = ../output/out

C_FILES = main.c io.c mh.c

$(EXEC): $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME -DDEBUG


test_sec:
	OMP_NUM_THREADS=$(THREADS) mpirun -np $(NP) ./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(NGM) >> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(NP)_$(NGM)_T$(THREADS).txt

clean:
	rm -f $(EXEC)
