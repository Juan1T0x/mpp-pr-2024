// mh.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#include "../include/mh.h"

// #define MUTATION_RATE 0.15
#define MUTATION_RATE 0.05 // Se reduce la tasa de mutación para mejorar el fitness obtenido.
#define PRINT 0

// Variables globales MPI

// Número de generaciones que cada proceso (isla) realiza el proceso evolutivo
// Proceso evolutivo: cruce, mutación, selección
// Migrar: devolver los mejores individuos al proceso maestro
// #ifndef NGM
// #define NGM 4
// #endif

/**
 *  Crea un nuevo tipo de datos derivado en MPI
 *  Necesario para el envio/recepcion de mensajes con datos de tipo Individuo
 **/
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

/**
 * Función que genera un número entero aleatorio entre 0 y n-1.
 *
 * @param n Límite superior exclusivo del número aleatorio.
 * @return Un número entero aleatorio entre 0 y n-1.
 */
static inline int aleatorio(int n)
{
	// Genera y devuelve un número aleatorio entre 0 y n-1.
	return (rand() % n);
}

/**
 * Función que verifica si un elemento está presente en un array hasta una posición dada.
 *
 * @param array El array en el que buscar.
 * @param end Índice hasta donde buscar en el array (exclusivo).
 * @param element El elemento a buscar en el array.
 * @return 1 si el elemento se encuentra en el array, 0 en caso contrario.
 */
static inline int find_element(int *array, int end, int element)
{
	for (int i = 0; i < end; i++)
		if (array[i] == element)
			return 1;
	return 0;
}

/**
 * Función que inicializa un individuo (solución) generando un array de 'm'
 * enteros únicos aleatorios entre 0 y n-1 dentro de su campo array_int.
 *
 * @param ind Puntero al individuo que se va a inicializar.
 * @param n Número total de elementos posibles.
 * @param m Tamaño del individuo (número de elementos a seleccionar).
 */
void inicializar_individuo(Individuo *ind, int n, int m)
{
	// Coloca -1 en todas las posiciones (por conveniencia)

	memset(ind->array_int, -1, m * sizeof(int));
	int i = 0;
	while (i < m)
	{
		int value = aleatorio(n);
		if (!find_element(ind->array_int, i, value))
			ind->array_int[i++] = value;
	}
}

/**
 * Función de comparación para qsort, utilizada para ordenar enteros en orden ascendente.
 *
 * @param a Puntero al primer elemento a comparar.
 * @param b Puntero al segundo elemento a comparar.
 * @return Un valor negativo si *a < *b, positivo si *a > *b, 0 si son iguales.
 */
int comp_array_int(const void *a, const void *b)
{
	// Compara los valores enteros apuntados por 'a' y 'b'.
	return (*(int *)a - *(int *)b);
}

/**
 * Función de comparación para qsort, utilizada para ordenar individuos por su valor de fitness en orden descendente.
 *
 * @param a Puntero al primer individuo a comparar.
 * @param b Puntero al segundo individuo a comparar.
 * @return -1 si el fitness de 'a' es mayor que el de 'b', 1 si es menor, 0 si son iguales.
 */
int comp_fitness(const void *a, const void *b)
{
	double fa = ((Individuo *)a)->fitness;
	double fb = ((Individuo *)b)->fitness;
	// Devuelve -1 si fa>fb (quiero el mayor primero)
	return (fa > fb) ? -1 : ((fa < fb) ? 1 : 0);
}

/**
 * Función principal que aplica la metaheurística (algoritmo genético) para resolver el problema.
 *
 * @param d Puntero al array de distancias compactado.
 * @param n Número total de elementos posibles.
 * @param m Tamaño del subconjunto a seleccionar.
 * @param n_gen Número de generaciones a simular.
 * @param tam_pob Tamaño de la población.
 * @param sol Puntero al array donde se almacenará la mejor solución encontrada.
 * @return El valor de fitness de la mejor solución encontrada.
 */
double aplicar_mh(const double *d, int n, int m, int n_gen,
				  int tam_pob, int *sol, int ngm)
{
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Crear tipo derivado
	MPI_Datatype individuo_type;
	crear_tipo_datos(m, &individuo_type);

	// Reparto base
	int NEM = tam_pob / size;
	int sobrante = tam_pob % size;
	int local_cnt = (rank == size - 1) ? (NEM + sobrante) : NEM;

	// Reservamos subpoblación local
	Individuo *sub_pop = (Individuo *)malloc(local_cnt * sizeof(Individuo));
	assert(sub_pop);

	// Solo en maestro reservamos población global
	Individuo *poblacion = NULL;
	if (rank == 0)
	{
		poblacion = (Individuo *)malloc(tam_pob * sizeof(Individuo));
		assert(poblacion);

		// Inicializamos población global
		for (int i = 0; i < tam_pob; i++)
		{
			inicializar_individuo(&poblacion[i], n, m);
			fitness(d, &poblacion[i], n, m);
		}
	}

	// Preparamos sendcounts/displs para Scatterv/Gatherv
	int *sendcounts = (int *)malloc(size * sizeof(int));
	int *displs = (int *)malloc(size * sizeof(int));
	for (int i = 0; i < size; i++)
	{
		sendcounts[i] = (i == size - 1) ? (NEM + sobrante) : NEM;
		displs[i] = i * NEM;
	}

	// Migración cada MIGR_FREQ generaciones (por ejemplo)
	const int MIGR_FREQ = 5;
	// Cantidad de individuos top a migrar
	const int MIGR_K = 2;

	// Bucle de generaciones
	for (int g = 0; g < n_gen; g++)
	{
		// SCATTERv: cada proceso recibe su parte
		MPI_Scatterv(
			poblacion, sendcounts, displs, individuo_type,
			sub_pop, local_cnt, individuo_type,
			0, MPI_COMM_WORLD);

		// Evolución local: se repite ngm veces
		for (int iter = 0; iter < ngm; iter++)
		{
			// ~ Cruce ~
			int half = local_cnt / 2;
			for (int i = 0; i + 1 < half; i += 2)
			{
				// Cruzar en-sitio: reutilizamos sub_pop[half + i..] como hijos
				cruzar(&sub_pop[i], &sub_pop[i + 1],
					   &sub_pop[half + i], &sub_pop[half + i + 1],
					   n, m);
			}
			// ~ Mutación y fitness ~
			for (int i = 0; i < local_cnt; i++)
			{
				mutar(&sub_pop[i], n, m);
				fitness(d, &sub_pop[i], n, m);
			}
			// ~ Selección local (ordenar subpoblación) ~
			qsort(sub_pop, local_cnt, sizeof(Individuo), comp_fitness);
		}

		// Migramos solo cada MIGR_FREQ generaciones (para reducir overhead)
		if ((g % MIGR_FREQ) == 0)
		{
			// Cada proceso envía top-K (no bloqueante) al maestro
			// si local_cnt >= MIGR_K
			if (local_cnt >= MIGR_K)
			{
				MPI_Request req;
				MPI_Isend(
					sub_pop, MIGR_K, individuo_type,
					0,	 // destino=maestro
					999, // tag
					MPI_COMM_WORLD,
					&req);
				// No necesitamos Wait si esperamos en el maestro.
				// Podríamos hacer MPI_Request array para todos
				// y hacer Waitsome/Waitall si se desea.
			}

			// Maestro recibe (no bloqueante) de cada rank
			if (rank == 0)
			{
				for (int src = 1; src < size; src++)
				{
					// ignoramos si local_cnt < MIGR_K en src,
					// (asumimos envía MIGR_K si puede, 0 si no).
					// Simplificamos: Recv MIGR_K desde todos de tag=999
					MPI_Request req_r;
					MPI_Irecv(
						&poblacion[src * MIGR_K], // un hueco en la población
						MIGR_K,
						individuo_type,
						src,
						999,
						MPI_COMM_WORLD,
						&req_r);
					// Esperamos (se podría Waitall luego)
					MPI_Wait(&req_r, MPI_STATUS_IGNORE);
				}
				// Maestro podría integrar esos MIGR_K a 'poblacion',
				// reordenar una parte, etc.
			}

			// Sincronizamos tras migrar
			MPI_Barrier(MPI_COMM_WORLD);
		}

		// GATHERV final de la iteración:
		// Recolectamos sub_poblaciones para poder ver el global
		// y hacer redistribución la próxima iter (si se desea).
		MPI_Gatherv(
			sub_pop, local_cnt, individuo_type,
			poblacion, sendcounts, displs, individuo_type,
			0, MPI_COMM_WORLD);

		// Maestro ajusta la población global si quiere
		if (rank == 0)
		{
			// Ordena la población global
			qsort(poblacion, tam_pob, sizeof(Individuo), comp_fitness);

			// Podríamos aquí reinyectar los MIGR_K en la parte inicial
			// para garantizar su dispersión en la próxima SCATTERv
			// o hacer lógicas más avanzadas.
		}

		// IMPORTANTE: si no necesitamos la población global en la próxima
		// iteración (o cada X iteraciones), se puede omitir este Gatherv
		// y recolección/orden global => reduce tiempo drásticamente.
	} // Fin del bucle de generaciones

	// Al terminar, el maestro tiene la población global ordenada:
	double best_fitness = 0.0;
	if (rank == 0)
	{
		best_fitness = poblacion[0].fitness;
		memcpy(sol, poblacion[0].array_int, m * sizeof(int));
		free(poblacion);
	}

	// Limpieza
	free(sub_pop);
	free(sendcounts);
	free(displs);
	MPI_Type_free(&individuo_type);

	return best_fitness;
}

/**
 * Función que realiza el cruce (crossover) entre dos padres para generar dos hijos.
 *
 * @param padre1 Puntero al primer padre.
 * @param padre2 Puntero al segundo padre.
 * @param hijo1 Puntero al primer hijo resultante.
 * @param hijo2 Puntero al segundo hijo resultante.
 * @param n Número total de elementos posibles.
 * @param m Tamaño de los individuos (número de genes).
 */
void cruzar(const Individuo *padre1, const Individuo *padre2,
			Individuo *hijo1, Individuo *hijo2,
			int n, int m)
{
	int punto = aleatorio(m - 1) + 1;
	memset(hijo1->array_int, -1, m * sizeof(int));
	memset(hijo2->array_int, -1, m * sizeof(int));

	// Copia parte del padre1 a hijo1
	for (int i = 0; i < punto; i++)
		hijo1->array_int[i] = padre1->array_int[i];

	// Rellena resto en hijo1 a partir de padre2
	int index = punto;
	for (int i = 0; i < m && index < m; i++)
	{
		int gene = padre2->array_int[i];
		if (!find_element(hijo1->array_int, index, gene))
			hijo1->array_int[index++] = gene;
	}
	// Si aun no está lleno, elige genes aleatorios
	while (index < m)
	{
		int gene = aleatorio(n);
		if (!find_element(hijo1->array_int, index, gene))
			hijo1->array_int[index++] = gene;
	}

	// Copia parte del padre2 a hijo2
	for (int i = 0; i < punto; i++)
		hijo2->array_int[i] = padre2->array_int[i];

	index = punto;
	for (int i = 0; i < m && index < m; i++)
	{
		int gene = padre1->array_int[i];
		if (!find_element(hijo2->array_int, index, gene))
			hijo2->array_int[index++] = gene;
	}
	while (index < m)
	{
		int gene = aleatorio(n);
		if (!find_element(hijo2->array_int, index, gene))
			hijo2->array_int[index++] = gene;
	}
}

/**
 * Función que aplica mutación a un individuo, modificando algunos de sus genes.
 *
 * @param actual Puntero al individuo a mutar.
 * @param n Número total de elementos posibles.
 * @param m Tamaño del individuo (número de genes).
 */
void mutar(Individuo *ind, int n, int m)
{
	int num_mutations = (int)(MUTATION_RATE * m);
	if (num_mutations < 1)
		num_mutations = 1;

	for (int i = 0; i < num_mutations; i++)
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

/**
 * Función que calcula la distancia entre dos elementos i y j utilizando el array de distancias compactado.
 *
 * @param d Puntero al array de distancias compactado.
 * @param i Índice del primer elemento.
 * @param j Índice del segundo elemento.
 * @param n Número total de elementos posibles.
 * @return La distancia entre los elementos i y j.
 */
static inline double distancia_ij(const double *d, int i, int j, int n)
{
	if (i == j)
		return 0.0;
	if (i > j)
	{
		int tmp = i;
		i = j;
		j = tmp;
	}
	// índice en d
	int index = i * n - (i * (i + 1)) / 2 + (j - i - 1);
	return d[index];
}

/**
 * Función que calcula el fitness de un individuo sumando las distancias entre cada par de sus genes.
 *
 * @param d Puntero al array de distancias compactado.
 * @param individuo Puntero al individuo cuyo fitness se va a calcular.
 * @param n Número total de elementos posibles.
 * @param m Tamaño del individuo (número de genes).
 */
void fitness(const double *d, Individuo *ind, int n, int m)
{
	double sum = 0.0;
	// (Se asume m no excesivamente grande; O(m^2) en c/u)
	for (int i = 0; i < m - 1; i++)
		for (int j = i + 1; j < m; j++)
			sum += distancia_ij(d, ind->array_int[i], ind->array_int[j], n);

	ind->fitness = sum;
}
