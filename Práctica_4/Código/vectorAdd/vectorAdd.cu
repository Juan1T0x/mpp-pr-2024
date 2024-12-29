/* This sample implements the vector addition operation */

#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <helper_string.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

#include "helper_functions.h"  // helper for shared functions common to CUDA Samples
#include "helper_cuda.h"      // helper functions for CUDA error checking and initialization

#include "vectorAdd_kernel.cu"

int main(int argc, char *argv[])
{
    cudaError_t err;
    int numElements = 50000;
	int threadsPerBlock = 256;

    float elapsed_time;
    cudaEvent_t start_event, stop_event;

    // process command line arguments
    numElements = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "nelem");
    threadsPerBlock = getCmdLineArgumentInt(argc, (const char **) argv, (const char *) "tpb");
	
    size_t size = numElements * sizeof(float);
    printf("[Vector Addition of %d Elements]\n", numElements);

    // Allocate the host input vector X
    float *h_X = (float *) malloc(size);

    // Allocate the host input vector Y
    float *h_Y = (float *) malloc(size);

    // Allocate the host output vector Z
    float *h_Z = (float *) malloc(size);

    // Verify that allocations succeeded
    if (h_X == NULL || h_Y == NULL || h_Z == NULL) {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the host input vectors
    for (int i = 0; i < numElements; ++i) {
        h_X[i] = rand() / ((float) RAND_MAX);
        h_Y[i] = rand() / ((float) RAND_MAX);
    }

	// Set the GPU to use
	err = cudaSetDevice(0);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to set device 0 (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	
    // Allocate the device input vector X
    float *d_X = NULL;
    err = cudaMalloc((void **)&d_X, size);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device vector X (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Allocate the device input vector Y
    float *d_Y = NULL;
    err = cudaMalloc((void **)&d_Y, size);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device vector Y (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Allocate the device output vector Z
    float *d_Z = NULL;
    err = cudaMalloc((void **)&d_Z, size);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device vector Z (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // create event
    checkCudaErrors(cudaEventCreate(&start_event));
    checkCudaErrors(cudaEventCreate(&stop_event));

    // using event
    cudaEventRecord(start_event, 0); // record in stream-0, to ensure that all previous CUDA calls have completed

    printf("Copy input data from the host memory to the CUDA device\n");
    err = cudaMemcpy(d_X, h_X, size, cudaMemcpyHostToDevice);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy vector X from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	
    err = cudaMemcpy(d_Y, h_Y, size, cudaMemcpyHostToDevice);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy vector Y from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Launch the Vector Add CUDA Kernel
    int blocksPerGrid = (numElements + threadsPerBlock - 1) / threadsPerBlock;	
    printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
    vectorAdd<<<blocksPerGrid, threadsPerBlock>>>(d_X, d_Y, d_Z, numElements);
	
    err = cudaGetLastError();
	
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to launch vectorAdd kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // wait for thread completion
    cudaDeviceSynchronize();

    printf("Copy output data from the CUDA device to the host memory\n");
    err = cudaMemcpy(h_Z, d_Z, size, cudaMemcpyDeviceToHost);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy vector Z from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // using event
    cudaEventRecord(stop_event, 0);    
    cudaEventSynchronize(stop_event);		// block until the event is recorded
    checkCudaErrors(cudaEventElapsedTime(&elapsed_time, start_event, stop_event));    
    printf("Elapsed Time: %.4f (ms)\n", elapsed_time);

    // Verify that the result is correct
    for (int i = 0; i < numElements; ++i) {
        if (fabs(h_X[i] + h_Y[i] - h_Z[i]) > 1E-5) {
            fprintf(stderr, "Result verification failed at element %d!\n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf("Test PASSED!!\n");

    // destroy events
    cudaEventDestroy(start_event);    
    cudaEventDestroy(stop_event);    

    // Free device global memory
    err = cudaFree(d_X);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free device vector X (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	
    err = cudaFree(d_Y);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free device vector Y (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_Z);
	
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free device vector Z (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Reset the device and exit
    err = cudaDeviceReset();

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to deinitialize the device! error=%s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
	
    // Free host memory
    free(h_X); free(h_Y); free(h_Z);
	
    printf("Done\n");
	cudaDeviceReset();
    return(0);
}
