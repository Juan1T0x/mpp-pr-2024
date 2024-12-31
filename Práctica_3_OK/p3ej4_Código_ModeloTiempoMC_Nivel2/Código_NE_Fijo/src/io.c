#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define min 0
#define max 100

void generar_d(double *d, int n)
{
  int i;
  double f;

  srand(time(NULL) + getpid());

  for(i=0;i<(n*n-n)/2;i++)
  {
 	f = (double) rand() / ((double) RAND_MAX + 1);
	*(d + i) = min + f*(max - min);
  }
}


double *read_distances(int n)
{
        double *d = (double *) malloc(((n*n-n)/2)*sizeof(double));

        generar_d(d, n);

	return d;
}

void print_distances(double *d, int n)
{
        int i,j,pos=0;

        printf("\nDistances: \n\n");
        for(i=0;i<n;i++)
        {
           for(j=i+1;j<n;j++)
           {
               printf("d %d %d %.2lf\n", i, j, d[pos]);
               pos+=1;
           }
           printf("\n");
        }
        //printf("\n");
}

void print_solution(int n, int m, const int *solucion, double valor)
{
	printf("\nSolution: ");
	for(int i = 0; i < m; i++) { printf("%d ", solucion[i]); }
	printf("\nDistance: %.0lf\n", valor);
}
