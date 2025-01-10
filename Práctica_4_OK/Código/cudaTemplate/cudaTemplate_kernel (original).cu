////////////////////////////////////////////////////////////////////////////////
// CUDA Kernel
////////////////////////////////////////////////////////////////////////////////

__constant__ int const_d[CT_MEM_SIZE];

__global__ void foo(int *gid_d)
{
    extern __shared__ int shared_mem[];

	// size of the block
    int blockSize = blockDim.x * blockDim.y;

    // global thread ID in thread block
    int tidb = (blockDim.x * threadIdx.y + threadIdx.x);

    // global thread ID in grid
    int tidg = (blockIdx.y * gridDim.x * blockSize + blockIdx.x * blockSize + tidb);

    shared_mem[tidb] = gid_d[tidg];
    
    __syncthreads();

	/* shared memory */
    shared_mem[tidb] += (tidg + const_d[tidg % CT_MEM_SIZE]);

    __syncthreads();

    gid_d[tidg] = shared_mem[tidb];
}
