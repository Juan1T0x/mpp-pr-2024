#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void init_mat();
void print_2D();

double get_time() {
    struct timeval t;
    gettimeofday(&t, (struct timezone *)0);
    return (((double)t.tv_sec) + ((double)t.tv_usec)*1E-6);
}

void matmult(int n, double *mA, double *mB, double *mC)
{
	int i, j, k;
	
	for(i=0; i<n; i++) {
		for(j=0; j<n; j++) {
			double s = .0;
			for(k=0; k<n; k++) {
				s += mA[i*n+k] * mB[k*n+j];
			}
			mC[i*n+j] = s;
		}
	}
}
 
int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("Usage: %s size\n", argv[0]);
		return(EXIT_FAILURE);
	}
	
	int n = atoi(argv[1]);
  	double *mA = NULL, *mB = NULL, *mC = NULL;
	
	int size = n*n;
	size_t nBytes = size*sizeof(double);
	
	mA = (double *) malloc(nBytes);
	mB = (double *) malloc(nBytes);
	mC = (double *) malloc(nBytes);
	
	init_mat(n, mA, 0.0, 10.0);
	init_mat(n, mB, 0.0, 10.0);
	
	#ifdef TIME
		double t1 = get_time();
	#endif
	
	matmult(n, mA, mB, mC);
	
	#ifdef TIME
		double t2 = get_time();
		printf("Time = %.6lf seconds\n", (t2 - t1));
	#endif
	
	#ifdef DEBUG
		print_2D(n, mC);
	#endif
	
	free(mA); free(mB); free(mC);	
	return(EXIT_SUCCESS);
}

/***************************************************************/

void init_mat(int n, double *mat, double upp, double low) {
	for(int i=0; i<n*n; i++) {
		mat[i] = ((double) rand()/RAND_MAX)*(upp - low) + low;
	}
}

/***************************************************************/ 

void print_2D(int n, double	*mat)
{
 	for(int i=0; i<n; i++) {
		for(int j=0; j<n; j++) {
			printf("  %lf  ", mat[i*n+j]);
		}
		printf("\n");
	}
	printf("\n");
}
