#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include "../include/io.h"

extern double aplicar_mh(const double *, int, int, int, int, double, int *);

static double mseconds()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

int main(int argc, char **argv)
{
	//	Check Number of Input Args
	if (argc < 6)
	{
		fprintf(stderr, "Ayuda:\n");
		fprintf(stderr, "  ./programa n m nGen tamPob m_rate\n");
		return (EXIT_FAILURE);
	}

	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
	int n_gen = atoi(argv[3]);
	int tam_pob = atoi(argv[4]);
	double m_rate = atof(argv[5]);

	//	Check that 'm' is less than 'n'
	assert(m < n);

	//	Generate matrix D with distance values among elements
	double *d = read_distances(n);

	//	Allocate memory for output data
	int *sol = (int *)malloc(m * sizeof(int));

	//	Timing code
	double ti = mseconds();

	//	Call Metaheuristic
	double value = aplicar_mh(d, n, m, n_gen, tam_pob, m_rate, sol);

	//	Timing code
	double tf = mseconds();
	printf("Execution Time: %.2lf sec\n", (tf - ti) / 1000);

	//	Print solution
	print_solution(n, m, sol, value);

	//	Free Allocated Memory
	free(sol);
	free(d);

	return (EXIT_SUCCESS);
}
