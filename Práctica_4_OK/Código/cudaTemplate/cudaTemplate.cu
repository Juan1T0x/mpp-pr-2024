#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>

#include <timer.h>
#include <helper_string.h>

#include "helper_functions.h"  // helper for shared functions common to CUDA Samples
#include "helper_cuda.h"      // helper functions for CUDA error checking and initialization

#define SH_MEM_SIZE (16 * 1024)
#define CT_MEM_SIZE (8)

#include "cudaTemplate_kernel.cu"
const int const_h[CT_MEM_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};

////////////////////////////////
// Main Program
////////////////////////////////
int main(int argc, char *argv[])
{
    int dim_grid_x, dim_grid_y;		// grid  dimensions
    int dim_block_x, dim_block_y;	// block dimensions
	
    int *gid_h = NULL; // host data
    int *gid_d = NULL; // device data
	
    long int nPos;
    size_t nBytes, shared_mem_size;
    
    float elapsed_time;
    cudaEvent_t start_event, stop_event;
	
    // process command line arguments
    dim_grid_x  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsx");
    dim_grid_y  = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "gsy");
    dim_block_x = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsx");
    dim_block_y = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "bsy");

    nPos = dim_grid_x * dim_grid_y * dim_block_x * dim_block_y;
    nBytes = nPos * sizeof(int);

    // allocate host memory
    gid_h = (int *) malloc(nBytes);
    bzero(gid_h, nBytes);

	// Set the GPU to use
	checkCudaErrors(cudaSetDevice(0));
	
    // allocate device memory
    checkCudaErrors(cudaMalloc((void **) &gid_d, nBytes));
   
    // copy data from host memory to device memory
    checkCudaErrors(cudaMemcpy(gid_d, gid_h, nBytes, cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset((void *) gid_d, 0, nBytes));

    // initialize constant memory
    checkCudaErrors(cudaMemcpyToSymbol(const_d, const_h, CT_MEM_SIZE*sizeof(int), 0, cudaMemcpyHostToDevice));

    // create event
    checkCudaErrors(cudaEventCreate(&start_event));
    checkCudaErrors(cudaEventCreate(&stop_event));

    // using event
    cudaEventRecord(start_event, 0); // record in stream-0, to ensure that all previous CUDA calls have completed

    // setup execution parameters
    dim3 grid(dim_grid_x, dim_grid_y);
    dim3 block(dim_block_x, dim_block_y);

    // execute the kernel
    shared_mem_size = block.x * block.y * sizeof(int);
    assert(shared_mem_size <= SH_MEM_SIZE);
	
    printf("Running configuration: \t %ld threads\n\t\t\t grid of %d x %d\n"
           "\t\t\t blocks of %d x %d threads (%d threads with %lu bytes of shared memory per block)\n", 
           nPos, dim_grid_x, dim_grid_y, dim_block_x, dim_block_y, dim_block_x * dim_block_y, shared_mem_size);
    
    foo<<<grid, block, shared_mem_size>>>(gid_d);

    // wait for thread completion
    cudaDeviceSynchronize();

    // get results back from device memory
    checkCudaErrors(cudaMemcpy(gid_h, gid_d, nBytes, cudaMemcpyDeviceToHost));
    
    // using event
    cudaEventRecord(stop_event, 0);    
    cudaEventSynchronize(stop_event);		// block until the event is recorded
    checkCudaErrors(cudaEventElapsedTime(&elapsed_time, start_event, stop_event));    
    printf("Processing Time: %.4f (ms)", elapsed_time);
    
    // check results
    for(int i = 0; i < nPos; i++) {
        assert(gid_h[i] += (i + const_h[i % CT_MEM_SIZE]));
	}
	
    // destroy events
    cudaEventDestroy(start_event);    
    cudaEventDestroy(stop_event);    
    
    // free device memory
    checkCudaErrors(cudaFree((void *) gid_d));
	
    // free host memory
    free(gid_h);
	
    printf("\nPASSED\n");
    cudaDeviceReset();
    return(0);
}

