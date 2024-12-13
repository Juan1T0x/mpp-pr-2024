CC = gcc
CFLAGS = -O3 -std=c99 -g -Wall

N = 
EXEC = sec

sec: main.c
	$(CC) $(CFLAGS) main.c -o $(EXEC) -DTIME

test_sec:
	./$(EXEC) $(N)

clean:
	rm -f $(EXEC)
