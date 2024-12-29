#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <mpi.h>

#include "../include/io.h"

// Prototipo de la función aplicar_mh que ahora usará MPI
extern double aplicar_mh(const double *, int, int, int, int, int *, int);

static double mseconds()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

int main(int argc, char **argv)
{
	// Inicialización de MPI
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc < 6)
	{
		if (rank == 0)
		{
			fprintf(stderr, "Ayuda:\n");
			fprintf(stderr, "  ./programa n m nGen tamPob\n");
		}
		MPI_Finalize();
		return (EXIT_FAILURE);
	}

	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
	int n_gen = atoi(argv[3]);
	int tam_pob = atoi(argv[4]);
	int ngm = atoi(argv[5]);

	// Verificar que 'm' sea menor que 'n'
	assert(m < n);

	// Generar la matriz de distancias (solo el proceso maestro)
	double *d = NULL;
	if (rank == 0)
	{
		d = read_distances(n);
#ifdef DEBUG
		// print_distances(d, n);
#endif
	}

	// Compartir la matriz de distancias con todos los procesos
	if (rank == 0)
	{
		MPI_Bcast(d, n * (n - 1) / 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}
	else
	{
		d = malloc((n * (n - 1) / 2) * sizeof(double));
		MPI_Bcast(d, n * (n - 1) / 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}

	// Asignar memoria para la solución en el proceso maestro
	int *sol = (rank == 0) ? (int *)malloc(m * sizeof(int)) : NULL;

	// Tiempo de ejecución (solo para el maestro)
	double ti = 0.0, tf = 0.0;
#ifdef TIME
	if (rank == 0)
	{
		ti = mseconds();
	}
#endif

	// Llamar a la metaheurística en paralelo
	double value = aplicar_mh(d, n, m, n_gen, tam_pob, sol, ngm);

#ifdef TIME
	if (rank == 0)
	{
		tf = mseconds();
		printf("Execution Time: %.2lf sec\n", (tf - ti) / 1000);
	}
#endif

	// Imprimir solución y fitness (solo el maestro)
#ifdef DEBUG
	if (rank == 0)
	{
		print_solution(n, m, sol, value);
	}
#endif

	// Liberar memoria
	free(d);
	if (rank == 0)
	{
		free(sol);
	}

	// Finalizar MPI
	MPI_Finalize();
	return (EXIT_SUCCESS);
}
