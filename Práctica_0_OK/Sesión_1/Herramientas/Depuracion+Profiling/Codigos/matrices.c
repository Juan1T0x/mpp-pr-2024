#include <omp.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

/** Multiplicación secuencial (ver función al final) */
void multiplicar(int t, double *a, double *b, double *c);

void generar(double *m, int t)
{
	int i;
	
	for(i=0; i<t; i++) {
		while(1) {
			double valor = (20.*rand()) / RAND_MAX-10.;
			for(int j=0; j<i; j++) {
				if(m[j] == valor) { continue; }
			}
			m[i] = valor;
			break;
		}
	}
}

void escribir(double *m, int t)
{
	int i, j;
	
	for(i=0; i<t; i++) {
		for(j=0; j<t; j++)
			printf("%.4lf ", m[i*t + j]);
		printf("\n");
	}
	printf("\n");
}

long long mseconds() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec*1000 + t.tv_usec/1000;
}

int main(int argc, char *argv[]) {
	int N;
	long long ti, tf, tt=0;
	double *a, *b, *c;
	
	N = 200;
	srand(1);
	
	a = (double *) calloc(sizeof(double), N*N);
	b = (double *) calloc(sizeof(double), N*N);
	c = (double *) calloc(sizeof(double), N*N);
	
	generar(a, N*N);
	generar(b, N*N);
	
	ti = mseconds();
	multiplicar(N, a, b, c);
	tf = mseconds();  
	tt += tf - ti;
	
	free(a);
	free(b);
	free(c);
	
	printf("Tiempo: %Ld\n", tt);
	return 0;
}
  
void multiplicar(int n, double *a, double *b, double *c)
{
	int i, j, k;
	double s;

	/* Multiplicación */ 
	for(i=0; i<n; i++) {
		for(j=0; j<n; j++) {      
			s = 0.;
			for(k=0; k<n; k++)
				s += a[i*n + k] * b[k*n + j];
			c[i*n + j] = s;
		}
	}
}
