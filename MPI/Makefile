CC = mpicc
C_FLAGS = -g -Wall -O0
LD_FLAGS =
LIBS = -lm
OBJECTS = main.o read_data.o k_means.o random.o
DEPENDS = k_means.h
BINARY = k_means

%.o: %.c $(DEPENDS) 
	$(CC) -c -o $@ $< $(C_FLAGS)

$(BINARY): $(OBJECTS)
	$(CC) $(C_FLAGS) $(LD_FLAGS) -o $@ $^ $(LIBS)
clean:
	rm $(BINARY) $(OBJECTS)
