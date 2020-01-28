CC=g++
CFLAGS= -Wall -Ofast -std=c++11  -flto -pipe -funit-at-a-time -fopenmp -lz -Isparsepp -flto
LDFLAGS=-flto -lpthread -fopenmp -lz  -Isparsepp  -flto
EXEC=bcash



all: $(EXEC)


bcash: bcash.o 
	$(CC) -o $@ $^ $(LDFLAGS)

bcash.o: bcash.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf $(EXEC)


rebuild: clean $(EXEC)
