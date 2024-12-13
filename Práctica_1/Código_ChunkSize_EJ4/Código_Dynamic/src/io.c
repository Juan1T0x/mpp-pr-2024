#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

double *read_distances(int n)
{
	int i, aux;

        //if(scanf("%d", &n)){};
        //if(scanf("%d", &aux)){};

        double *d = (double *) malloc(((n*n-n)/2)*sizeof(double));

        for(i=0;i<(n*n-n)/2;i++)
        {
 	     if(scanf("%d", &aux)){};
  	     if(scanf("%d", &aux)){};
             if(scanf("%lf", &d[i])){};
        }

	return d;
}

void print_distances(double *d, int n)
{
        int i,j,pos=0;

        printf("\nDistances: \n\n");
        for(i=0;i<=n;i++)
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
	printf("\nDistance: %.2lf\n", valor);
}
