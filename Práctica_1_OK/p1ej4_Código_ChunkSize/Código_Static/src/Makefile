CC = gcc
CFLAGS = -O3 -Wall -std=c99 -g -fopenmp

N = 
M = 
N_GEN = 
T_POB = 
NTHREADS = 
CHUNK_SIZE = 

EXEC = sec
OUTFILE = ../output/out
C_FILES = main.c io.c mh.c

sec: $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME -DDEBUG -DCHUNK_SIZE=$(CHUNK_SIZE)

test_sec:
	OMP_NUM_THREADS=$(NTHREADS) ./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) >> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(NTHREADS)_$(CHUNK_SIZE).txt

clean:
	rm -f $(EXEC)
