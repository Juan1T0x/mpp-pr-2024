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
	./$(FILE) --gsx=4 --gsy=4 --gsz=4 --bsx=2 --bsy=2 --bsz=2
	./$(FILE) --gsx=8 --gsy=8 --gsz=4 --bsx=2 --bsy=2 --bsz=2
	./$(FILE) --gsx=8 --gsy=8 --gsz=4 --bsx=4 --bsy=4 --bsz=2
	./$(FILE) --gsx=16 --gsy=16 --gsz=4 --bsx=2 --bsy=2 --bsz=2
	./$(FILE) --gsx=16 --gsy=16 --gsz=4 --bsx=4 --bsy=4 --bsz=2
	./$(FILE) --gsx=16 --gsy=16 --gsz=4 --bsx=8 --bsy=8 --bsz=2
	./$(FILE) --gsx=32 --gsy=32 --gsz=4 --bsx=2 --bsy=2 --bsz=2
	./$(FILE) --gsx=32 --gsy=32 --gsz=4 --bsx=4 --bsy=4 --bsz=2
	./$(FILE) --gsx=32 --gsy=32 --gsz=4 --bsx=8 --bsy=8 --bsz=2
	./$(FILE) --gsx=32 --gsy=32 --gsz=4 --bsx=16 --bsy=16 --bsz=2

	 
clean:
	rm -f *.o $(FILE)

