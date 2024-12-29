GCC  = g++
NVCC = nvcc

CUDA_DIR = /usr/local/cuda
CUDA_LIB = -L$(CUDA_DIR)/lib64
CUDA_INC = -I$(CUDA_DIR)/include
CUDA_INC_COMMON = -I../Common

C_FLAGS = -O3
NVCC_FLAGS = -m64
LD_LIBS	= -lcudart -lm
GENCODE_FLAGS = -Wno-deprecated-gpu-targets

FILE = 

$(FILE): $(FILE).o
		 $(GCC) $(C_FLAGS) $< -o $(FILE) $(CUDA_LIB) $(LD_LIBS)

$(FILE).o: $(FILE).cu
		   $(NVCC) $(NVCC_FLAGS) $(CUDA_INC) $(CUDA_INC_COMMON) $(GENCODE_FLAGS) -o $@ -c $<

run: 
	./$(FILE) --nelem=10000 --tpb=16
	./$(FILE) --nelem=20000 --tpb=16
	./$(FILE) --nelem=20000 --tpb=32
	./$(FILE) --nelem=30000 --tpb=16
	./$(FILE) --nelem=30000 --tpb=32
	./$(FILE) --nelem=30000 --tpb=64
	./$(FILE) --nelem=40000 --tpb=16
	./$(FILE) --nelem=40000 --tpb=32
	./$(FILE) --nelem=40000 --tpb=64
	./$(FILE) --nelem=40000 --tpb=128
	./$(FILE) --nelem=50000 --tpb=16
	./$(FILE) --nelem=50000 --tpb=32
	./$(FILE) --nelem=50000 --tpb=64
	./$(FILE) --nelem=50000 --tpb=128
	./$(FILE) --nelem=50000 --tpb=256

clean:
	rm -f *.o $(FILE)

