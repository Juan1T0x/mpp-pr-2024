#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include <omp.h>

#include "../include/mh.h"

// #define MUTATION_RATE 0.15
#define MUTATION_RATE 0.05 // Se reduce la tasa de mutación para mejorar el fitness obtenido.
#define PRINT 0

/* Frecuencia de migración (o recolección total) */
#ifndef MIGR_FREQ
#define MIGR_FREQ 5
#endif

/* Declaración global/hilos de la semilla: rand_r necesita un 'unsigned int*' */
unsigned int seed;
#pragma omp threadprivate(seed)

/* Tamaño de chunk para bucles paralelos */
#ifndef CHUNK_SIZE
#define CHUNK_SIZE 8
#endif

int chunk_size = CHUNK_SIZE;

/* --------------------------------------------------------------------------
   Crea un nuevo tipo de datos derivado en MPI para la estructura 'Individuo'
-------------------------------------------------------------------------- */
void crear_tipo_datos(int m, MPI_Datatype *individuo_type)
{
	int blocklen[2] = {m, 1};
	MPI_Datatype dtype[2] = {MPI_INT, MPI_DOUBLE};

	MPI_Aint disp[2];
	disp[0] = offsetof(Individuo, array_int);
	disp[1] = offsetof(Individuo, fitness);

	MPI_Type_create_struct(2, blocklen, disp, dtype, individuo_type);
	MPI_Type_commit(individuo_type);
}

/* --------------------------------------------------------------------------
   Funciones auxiliares
-------------------------------------------------------------------------- */

/** Genera un número entero aleatorio [0..n-1] usando rand_r(&seed). */
static inline int aleatorio(int n)
{
	return rand_r(&seed) % n;
}

/** Devuelve 1 si val está en arr[0..len-1], 0 si no. */
static inline int find_element(const int *arr, int len, int val)
{
	for (int i = 0; i < len; i++)
		if (arr[i] == val)
			return 1;
	return 0;
}

/** Inicializa un individuo con 'm' valores únicos aleatorios en [0..n-1]. */
void inicializar_individuo(Individuo *ind, int n, int m)
{
	memset(ind->array_int, -1, m * sizeof(int));
	int idx = 0;
	while (idx < m)
	{
		int val = aleatorio(n);
		if (!find_element(ind->array_int, idx, val))
			ind->array_int[idx++] = val;
	}
}

/** Ordena enteros en orden ascendente (para qsort). */
int comp_array_int(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

/** Ordena individuos por fitness en orden descendente (para qsort). */
static int comp_fitness_desc(const void *A, const void *B)
{
	const Individuo *a = (const Individuo *)A;
	const Individuo *b = (const Individuo *)B;
	if (a->fitness > b->fitness)
		return -1;
	else if (a->fitness < b->fitness)
		return 1;
	else
		return 0;
}

/** Calcula la distancia entre i y j usando el arreglo compactado d. */
double distancia_ij(const double *d, int i, int j, int n)
{
	if (i == j)
		return 0.0;
	if (i > j)
	{
		int tmp = i;
		i = j;
		j = tmp;
	}
	int index = i * n - (i * (i + 1)) / 2 + (j - i - 1);
	return d[index];
}

/** Calcula el fitness de un individuo = suma de distancias entre pares de genes. */
void fitness(const double *d, Individuo *ind, int n, int m)
{
	double sum = 0.0;
	int *v = ind->array_int;

#pragma omp parallel for reduction(+ : sum) schedule(dynamic)
	for (int i = 0; i < m - 1; i++)
	{
		for (int j = i + 1; j < m; j++)
		{
			sum += distancia_ij(d, v[i], v[j], n);
		}
	}
	ind->fitness = sum;
}

/* --------------------------------------------------------------------------
   Función que realiza el cruce (crossover) entre dos padres para generar dos hijos.
   Cada hijo hereda parte de un padre y se rellena con genes del otro, evitando duplicados.
-------------------------------------------------------------------------- */
void cruzar(const Individuo *p1, const Individuo *p2,
			Individuo *h1, Individuo *h2,
			int n, int m)
{
	int punto = aleatorio(m - 1) + 1;

	/* Reinicializamos hijos con -1 */
	memset(h1->array_int, -1, m * sizeof(int));
	memset(h2->array_int, -1, m * sizeof(int));

#pragma omp parallel sections
	{
		/* Sección 1: Construir hijo1 */
#pragma omp section
		{
			/* Copia genes desde 0..punto-1 de p1. */
			for (int i = 0; i < punto; i++)
				h1->array_int[i] = p1->array_int[i];

			int index = punto;
			/* Rellenar con genes de p2, evitando duplicados. */
			for (int i = 0; i < m && index < m; i++)
			{
				int gene = p2->array_int[i];
				if (!find_element(h1->array_int, index, gene))
					h1->array_int[index++] = gene;
			}
			/* Si aún faltan genes, añadir genes aleatorios no repetidos. */
			while (index < m)
			{
				int gene = aleatorio(n);
				if (!find_element(h1->array_int, index, gene))
					h1->array_int[index++] = gene;
			}
		}

		/* Sección 2: Construir hijo2 */
#pragma omp section
		{
			/* Copia genes desde 0..punto-1 de p2. */
			for (int i = 0; i < punto; i++)
				h2->array_int[i] = p2->array_int[i];

			int index = punto;
			/* Rellenar con genes de p1, evitando duplicados. */
			for (int i = 0; i < m && index < m; i++)
			{
				int gene = p1->array_int[i];
				if (!find_element(h2->array_int, index, gene))
					h2->array_int[index++] = gene;
			}
			/* Si aún faltan genes, añadir genes aleatorios no repetidos. */
			while (index < m)
			{
				int gene = aleatorio(n);
				if (!find_element(h2->array_int, index, gene))
					h2->array_int[index++] = gene;
			}
		}
	} // fin parallel sections
}

/* --------------------------------------------------------------------------
   Función que aplica mutación a un individuo, modificando algunos de sus genes.
-------------------------------------------------------------------------- */
void mutar(Individuo *ind, int n, int m)
{
	int num_mut = (int)(MUTATION_RATE * m);
	if (num_mut < 1)
		num_mut = 1;

	for (int i = 0; i < num_mut; i++)
	{
		int pos = aleatorio(m);
		ind->array_int[pos] = -1;
		int new_gene;
		do
		{
			new_gene = aleatorio(n);
		} while (find_element(ind->array_int, m, new_gene));
		ind->array_int[pos] = new_gene;
	}
}

/* --------------------------------------------------------------------------
   Función principal: aplicar_mh
-------------------------------------------------------------------------- */
double aplicar_mh(const double *d, int n, int m, int n_gen,
				  int tam_pob, int *sol, int ngm)
{
	/* Inicializar MPI */
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Crear tipo derivado para 'Individuo' */
	MPI_Datatype individuo_type;
	crear_tipo_datos(m, &individuo_type);

	/* Cálculo de reparto */
	int base = tam_pob / size;
	int resto = tam_pob % size;
	int local_count = (rank == size - 1) ? (base + resto) : base;

	/* Reserva sub_población local */
	Individuo *sub_pob = (Individuo *)malloc(local_count * sizeof(Individuo));
	assert(sub_pob);

	/* Inicializa la semilla para cada hilo en cada proceso */
#pragma omp parallel
	{
		seed = (unsigned int)time(NULL) + 10000 * rank + omp_get_thread_num();
	}

	/* Reserva población global solo en maestro */
	Individuo *poblacion = NULL;
	if (rank == 0)
	{
		poblacion = (Individuo *)malloc(tam_pob * sizeof(Individuo));
		assert(poblacion);

		/* Inicializar con OpenMP */
#pragma omp parallel for schedule(dynamic, chunk_size)
		for (int i = 0; i < tam_pob; i++)
		{
			/* array_int en -1 */
			memset(poblacion[i].array_int, -1, m * sizeof(int));

			/* Rellenar con genes únicos */
			int idx = 0;
			while (idx < m)
			{
				int val = aleatorio(n);
				if (!find_element(poblacion[i].array_int, idx, val))
					poblacion[i].array_int[idx++] = val;
			}
			/* fitness local */
			fitness(d, &poblacion[i], n, m);
		}

		/* Ordenar la población global la primera vez */
		qsort(poblacion, tam_pob, sizeof(Individuo), comp_fitness_desc);
	}

	/* Preparamos sendcounts/displs para Scatter/Gather manual */
	int *sendcounts = (int *)malloc(size * sizeof(int));
	int *displs = (int *)malloc(size * sizeof(int));
	for (int i = 0; i < size; i++)
	{
		sendcounts[i] = (i == size - 1) ? (base + resto) : base;
		displs[i] = i * base;
	}

	const int TAG_SCATTER = 100;
	const int TAG_GATHER = 200;

	/* Bucle de generaciones */
	for (int g = 0; g < n_gen; g++)
	{
		/* (1) Scatter manual asíncrono */
		if (rank == 0)
		{
			int nreqs = (size > 1) ? (size - 1) : 0;
			MPI_Request *reqs = NULL;
			if (nreqs > 0)
				reqs = (MPI_Request *)malloc(nreqs * sizeof(MPI_Request));

			int idx_req = 0;
			for (int i = 1; i < size; i++)
			{
				MPI_Isend(&poblacion[displs[i]], /* origen */
						  sendcounts[i], individuo_type,
						  i, TAG_SCATTER, MPI_COMM_WORLD,
						  &reqs[idx_req++]);
			}
			/* Copiar local la parte del maestro */
			memcpy(sub_pob, &poblacion[displs[0]],
				   sendcounts[0] * sizeof(Individuo));

			if (nreqs > 0)
			{
				MPI_Waitall(nreqs, reqs, MPI_STATUSES_IGNORE);
				free(reqs);
			}
		}
		else
		{
			/* rank != 0 => recibir */
			MPI_Request req;
			MPI_Irecv(sub_pob, local_count, individuo_type,
					  0, TAG_SCATTER, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
		}

		/* (2) Evolución local: repetimos 'ngm' veces cruce+mutación+fitness */
		for (int iter = 0; iter < ngm; iter++)
		{
			int half = local_count / 2;

			/* Evitar segfault si half < 2 */
			if (half >= 2)
			{
#pragma omp parallel for schedule(dynamic, chunk_size)
				for (int i = 0; i < half - 1; i += 2)
				{
					cruzar(&sub_pob[i], &sub_pob[i + 1],
						   &sub_pob[half + i], &sub_pob[half + i + 1],
						   n, m);
				}
			}

#pragma omp parallel for schedule(static, chunk_size)
			for (int i = 0; i < local_count; i++)
			{
				mutar(&sub_pob[i], n, m);
				fitness(d, &sub_pob[i], n, m);
			}

			/* Orden local por fitness desc */
			qsort(sub_pob, local_count, sizeof(Individuo), comp_fitness_desc);
		}

		/* (3) Migrar/recolectar cada MIGR_FREQ generaciones (o en la última) */
		if ((g % MIGR_FREQ) == 0 || (g == n_gen - 1))
		{
			if (rank == 0)
			{
				memcpy(&poblacion[displs[0]], sub_pob,
					   sendcounts[0] * sizeof(Individuo));

				int nreqs = (size > 1) ? (size - 1) : 0;
				MPI_Request *reqs = NULL;
				if (nreqs > 0)
					reqs = (MPI_Request *)malloc(nreqs * sizeof(MPI_Request));

				int idx_req = 0;
				for (int i = 1; i < size; i++)
				{
					MPI_Irecv(&poblacion[displs[i]],
							  sendcounts[i], individuo_type,
							  i, TAG_GATHER, MPI_COMM_WORLD,
							  &reqs[idx_req++]);
				}
				if (nreqs > 0)
				{
					MPI_Waitall(nreqs, reqs, MPI_STATUSES_IGNORE);
					free(reqs);
				}

				/* Ordenar población global */
				qsort(poblacion, tam_pob, sizeof(Individuo), comp_fitness_desc);
				if (PRINT)
				{
					printf("[GEN=%d] Best fitness = %f\n", g, poblacion[0].fitness);
				}
			}
			else
			{
				MPI_Request req;
				MPI_Isend(sub_pob, local_count, individuo_type,
						  0, TAG_GATHER, MPI_COMM_WORLD,
						  &req);
				MPI_Wait(&req, MPI_STATUS_IGNORE);
			}
		}
	} /* fin for n_gen */

	/* rank=0 => la población global está ordenada; la mejor en poblacion[0] */
	double best_fitness = 0.0;
	if (rank == 0)
	{
		best_fitness = poblacion[0].fitness;
		memcpy(sol, poblacion[0].array_int, m * sizeof(int));
		free(poblacion);
	}

	free(sub_pob);
	free(sendcounts);
	free(displs);
	MPI_Type_free(&individuo_type);

	return best_fitness;
}
