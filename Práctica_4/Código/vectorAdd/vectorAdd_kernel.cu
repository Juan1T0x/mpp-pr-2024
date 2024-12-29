/**
 * CUDA Kernel
 *
 * Computes the vector addition: V[i] = X[i] + Y[i]
 * The three vectors have the same number of elements.
 *
 */
__global__ void vectorAdd(const float *X, const float *Y, float *V, int numElements)
{	
    // global thread ID in thread block
    int tid = blockDim.x * blockIdx.x + threadIdx.x;

	// if the thread ID corresponds to a vector position,
	// sum the corresponding elements of X and Y
    if (tid < numElements) {
        V[tid] = X[tid] + Y[tid];
    }
}
