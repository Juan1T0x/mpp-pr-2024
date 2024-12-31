#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include "../include/io.h"

// Nuevo prototipo con los dos parámetros extra:
extern double aplicar_mh(
	const double *,
	int, int, int, int,
	int *,
	int, /* n_hilos_ini */
	int	 /* n_hilos_fit */
);

static double mseconds()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

int main(int argc, char **argv)
{
	// Esperamos 6 parámetros además del nombre del programa:
	//   n, m, n_gen, tam_pob, n_hilos_ini, n_hilos_fit
	if (argc < 7)
	{
		fprintf(stderr, "Uso:\n");
		fprintf(stderr, "  %s n m nGen tamPob nHilosIni nHilosFit\n", argv[0]);
		return (EXIT_FAILURE);
	}

	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
	int n_gen = atoi(argv[3]);
	int tam_pob = atoi(argv[4]);
	int n_hilos_ini = atoi(argv[5]);
	int n_hilos_fit = atoi(argv[6]);

	// Verificación opcional: 'm' < 'n'
	assert(m < n);

	// Leer las distancias (se asume read_distances está definida en io.h)
	double *d = read_distances(n);

#ifdef DEBUG
	// print_distances(d, n);
#endif

	// Reservar memoria para la solución
	int *sol = (int *)malloc(m * sizeof(int));

#ifdef TIME
	double ti = mseconds();
#endif

	// Llamada a la metaheurística con los dos nuevos parámetros
	double value = aplicar_mh(d, n, m, n_gen, tam_pob, sol, n_hilos_ini, n_hilos_fit);

#ifdef TIME
	double tf = mseconds();
	printf("Execution Time: %.2lf sec\n", (tf - ti) / 1000);
#endif

#ifdef DEBUG
	print_solution(n, m, sol, value);
#endif

	// Liberar memoria
	free(sol);
	free(d);

	return (EXIT_SUCCESS);
}
