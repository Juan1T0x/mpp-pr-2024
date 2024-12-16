CC = g++
CFLAGS = -O3 -Wall -g

N = 
M = 
N_GEN = 
T_POB = 

EXEC = igen
OUTFILE = ../input
C_FILES = igen.cpp

igen: $(C_FILES)
	$(CC) $(CFLAGS) $(C_FILES) -o $(EXEC)

exe_igen:
	./$(EXEC) $(N) $(M) $(N_GEN) $(T_POB) > $(OUTFILE)_$(N)_$(M)_$(N_GEN)_$(T_POB).txt

clean:
	rm -f $(EXEC)
