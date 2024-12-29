CC = mpicc
CFLAGS = -O3 -Wall -std=c99 -g

N = 
M = 
N_GEN = 
T_POB = 
NP = 
NGM = 

EXEC = sec
OUTFILE = ../output/out
C_FILES = main.c io.c mh.c

sec: $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME -DDEBUG

test_sec:
	mpirun -np $(NP) ./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(NGM) >> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(NP)_$(NGM).txt

clean:
	rm -f $(EXEC)
	