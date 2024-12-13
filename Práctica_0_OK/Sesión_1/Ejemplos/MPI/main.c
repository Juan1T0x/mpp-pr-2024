#include <mpi.h>
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

void matmult(int m, int n, double *mA, double *mB, double *mC)
{
	int i, j, k;
	double s;
	
	for(i=0; i<m; i++) {
		for(j=0; j<n; j++) {
			s = 0.0;
			for(k=0; k<n; k++) {
				s += mA[i*n+k] * mB[k*n+j];
			}
			mC[i*n+j] = s;
		}
	}
}

int main(int argc, char *argv[])
{
	MPI_Status status;
	
	int n, my_n;
	int my_rank, p;
	int retval = 0;
	int local_size, size;
	
  	double *local_A, *local_B, *local_C;
  	double *mA, *mB, *mC;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	if(my_rank == 0) {
		if(argc < 2) {
			printf("Usage: %s size\n", argv[0]);
			retval = -1;
		}
		else {
			n = atoi(argv[1]);
		}
	}
	MPI_Bcast(&retval, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
	if(retval == -1) {
		MPI_Finalize();
		return(EXIT_FAILURE);
	}
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	size = n*n;
	local_size = (n*n)/p;
	
	mB = (double *) malloc(sizeof(double)*size);
	local_A = (double *) malloc(sizeof(double)*local_size);
	local_C = (double *) malloc(sizeof(double)*local_size);
	
	if(my_rank == 0) {
		mA = (double *) malloc(sizeof(double)*size);
		mC = (double *) malloc(sizeof(double)*size);
		
		init_mat(n, mA, 0.0, 10.0);
		init_mat(n, mB, 0.0, 10.0);
	}
	
	#ifdef TIME
		double t1 = get_time();
	#endif
	
	/***************************************************************/
	/* Matriz B replicada a todos */
	/***************************************************************/
	MPI_Bcast(mB, size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	printf("Matriz B recibida por proceso %d\n", my_rank);
	
	/***************************************************************/  	
	/* Matriz A repartida por bloques de columnas consecutivas */
	/***************************************************************/
	MPI_Scatter(mA, local_size, MPI_DOUBLE, local_A, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	printf("Matriz local_A recibida por proceso %d\n", my_rank);
	
	/***************************************************************/
	/* Multiplicación matrices locales: mi_C = A * mi_B */
	/***************************************************************/
	matmult(n/p, n, local_A, mB, local_C);
	
	printf("Matriz local_C obtenida por proceso %d\n", my_rank);	
	/***************************************************************/	
	/* Matriz C enviada a cachos hacia el root */
	/***************************************************************/
	MPI_Gather(local_C, local_size, MPI_DOUBLE, mC, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	#ifdef TIME
		double tt = 0.0;
		double t2 = get_time();
		double time = t2 - t1;
		MPI_Reduce(&time, &tt, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	#endif
	
	if(my_rank == 0) {
		printf("Matriz C recibida en Root\n");
		
		#ifdef TIME
			printf("Time = %.6lf seconds\n", tt);
		#endif
		
		#ifdef DEBUG
			print_2D(n, mC);
		#endif

		free(mA);
		free(mC);
	}
	
	free(mB);
	free(local_A);
	free(local_C);
	MPI_Finalize();
	
	return(EXIT_SUCCESS);
}

/***************************************************************/

void init_mat(int n, double *mat, double upp, double low) {
	for(int i=0; i<n*n; i++) {
		mat[i] = ((double) rand()/RAND_MAX)*(upp - low) + low;
	}
}

/***************************************************************/ 

void print_2D(int n, double *mat)
{
 	for(int i=0; i<n; i++) {
		for(int j=0; j<n; j++) {
			printf("  %lf  ", mat[i*n+j]);
		}
		printf("\n");
	}
	printf("\n");
}
