CC = g++
CFLAGS = -O3 -Wall -g

N = 
M = 
N_GEN = 
T_POB = 
M_RATE =

EXEC = igen
OUTFILE = ../input
C_FILES = igen.cpp

igen: $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC)

exe_igen:
	./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) $(M_RATE) > $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB)_$(M_RATE).txt

clean:
	rm -f $(EXEC)
