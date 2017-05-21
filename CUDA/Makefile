NVCC = nvcc
NVCC_FLAGS = -arch=sm_30
LD_FLAGS =
LIBS = -lm
OBJECTS = main.o read_data.o k_means.o random.o
DEPENDS = k_means.h
BINARY = k_means

%.o: %.cu $(DEPENDS) 
	$(NVCC) -c -o $@ $< $(NVCC_FLAGS)

$(BINARY): $(OBJECTS)
	$(NVCC) $(NVCC_FLAGS) $(LD_FLAGS) -o $@ $^ $(LIBS)
clean:
	rm $(BINARY) $(OBJECTS)
