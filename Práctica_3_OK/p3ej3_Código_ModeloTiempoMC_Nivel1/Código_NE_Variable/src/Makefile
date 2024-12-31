CC = gcc
CFLAGS = -O3 -Wall -std=c99 -g -lm -fopenmp

N              =
M              =
N_GEN          =
T_POB          =
N_HILOS_INI    =
N_HILOS_FIT    =

EXEC   = sec
OUTFILE = ../output/out
C_FILES = main.c io.c mh.c

sec: $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME

test_sec:
	./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(N_HILOS_INI) $(N_HILOS_FIT) \
		>> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(N_HILOS_INI)_$(N_HILOS_FIT).txt

clean:
	rm -f $(EXEC)
