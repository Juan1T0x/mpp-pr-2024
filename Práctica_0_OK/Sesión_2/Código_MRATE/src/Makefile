CC = gcc
CFLAGS = -O3 -Wall -std=c99 -g

N = 
M = 
N_GEN = 
T_POB = 
M_RATE = 

EXEC = sec
OUTFILE = ../output/out
C_FILES = main.c io.c mh.c

sec: $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC) -DTIME -DDEBUG

test_sec:
	./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(M_RATE) >> $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(M_RATE).txt

clean:
	rm -f $(EXEC)
