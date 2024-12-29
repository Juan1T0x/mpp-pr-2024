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
int aleatorio(int n)
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
int find_element(int *array, int end, int element)
{
	int i = 0;
	int found = 0;

	// Recorre el array desde el inicio hasta la posición 'end' o hasta que encuentre el elemento.
	while ((i < end) && !found)
	{
		// Si el elemento en la posición actual es igual al buscado, se marca como encontrado.
		if (array[i] == element)
		{
			found = 1;
		}
		i++;
	}
	// Devuelve 1 si se encontró el elemento, 0 si no.
	return found;
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
		// Si el nuevo valor no está ya en el individuo, lo añade
		if (!find_element(ind->array_int, i, value))
		{
			ind->array_int[i] = value;
			i++;
		}
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
	const Individuo *ind_a = (const Individuo *)a;
	const Individuo *ind_b = (const Individuo *)b;

	if (ind_a->fitness > ind_b->fitness)
		return -1;
	else if (ind_a->fitness < ind_b->fitness)
		return 1;
	return 0;
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

	// 1) Crear tipo derivado para Individuo
	MPI_Datatype individuo_type;
	crear_tipo_datos(m, &individuo_type);

	// 2) Calcular el reparto de individuos (sendcounts/displs)
	int NEM = tam_pob / size;	   // Número base de individuos por proceso
	int sobrante = tam_pob % size; // “resto” para repartir

	// Cada proceso i recibirá sendcounts[i] individuos.
	// Para i < sobrante => NEM + 1
	// Para i >= sobrante => NEM
	int *sendcounts = (int *)malloc(size * sizeof(int));
	int *displs = (int *)malloc(size * sizeof(int));

	for (int i = 0; i < size; i++)
	{
		sendcounts[i] = NEM; // base
		if (i < sobrante)
		{
			sendcounts[i]++; // uno extra a los primeros 'sobrante' ranks
		}
	}
	// Calcular los desplazamientos
	displs[0] = 0;
	for (int i = 1; i < size; i++)
	{
		displs[i] = displs[i - 1] + sendcounts[i - 1];
	}

	// 3) Reservar la sub_poblacion local
	int local_count = sendcounts[rank];
	Individuo *sub_poblacion = (Individuo *)malloc(local_count * sizeof(Individuo));
	assert(sub_poblacion);

	// 4) En el maestro, reservar y rellenar la población completa
	Individuo *poblacion = NULL;
	if (rank == 0)
	{
		poblacion = (Individuo *)malloc(tam_pob * sizeof(Individuo));
		assert(poblacion);

		// Inicializar todos los individuos en rank=0
		for (int i = 0; i < tam_pob; i++)
		{
			inicializar_individuo(&poblacion[i], n, m);
			fitness(d, &poblacion[i], n, m);
		}
	}

	// 5) Bucle de generaciones
	for (int g = 0; g < n_gen; g++)
	{

		// 5.1) Distribuir subpoblaciones con MPI_Scatterv
		MPI_Scatterv(
			/* sendbuf    = */ poblacion, // sólo útil en rank=0
			/* sendcounts = */ sendcounts,
			/* displs     = */ displs,
			/* sendtype   = */ individuo_type,
			/* recvbuf    = */ sub_poblacion, // buffer local
			/* recvcount  = */ local_count,
			/* recvtype   = */ individuo_type,
			/* root       = */ 0,
			/* comm       = */ MPI_COMM_WORLD);

		// 5.2) Evolución local: cruce, mutación y reordenar
		for (int iter = 0; iter < ngm; iter++)
		{

			// Cruce
			// Suponemos local_count es par, o ignoramos el último si no lo es
			int half = local_count / 2; // entero
			for (int i = 0; i < half - 1; i += 2)
			{
				cruzar(&sub_poblacion[i], &sub_poblacion[i + 1],
					   &sub_poblacion[half + i], &sub_poblacion[half + i + 1],
					   n, m);
			}

			// Mutación + Fitness
			for (int i = 0; i < local_count; i++)
			{
				mutar(&sub_poblacion[i], n, m);
				fitness(d, &sub_poblacion[i], n, m);
			}

			// Ordenar sub_población local
			qsort(sub_poblacion, local_count, sizeof(Individuo), comp_fitness);
		}

		// 5.3) Recolectar subpoblaciones con MPI_Gatherv
		MPI_Gatherv(
			/* sendbuf    = */ sub_poblacion,
			/* sendcount  = */ local_count,
			/* sendtype   = */ individuo_type,
			/* recvbuf    = */ poblacion, // sólo válido en rank=0
			/* recvcounts = */ sendcounts,
			/* displs     = */ displs,
			/* recvtype   = */ individuo_type,
			/* root       = */ 0,
			/* comm       = */ MPI_COMM_WORLD);

		// 5.4) El maestro hace migración u ordenación global
		if (rank == 0)
		{
			// Ordenar la población global
			qsort(poblacion, tam_pob, sizeof(Individuo), comp_fitness);

			// Migración: en este ejemplo, no se hace nada especial
			// (podrías volver a “insertar” los mejores en las posiciones iniciales, etc.)
		}

		// (opcional) Sincronizar
		MPI_Barrier(MPI_COMM_WORLD);
	}

	// 6) El maestro obtiene la mejor solución final
	double best_fitness = 0.0;
	if (rank == 0)
	{
		// poblacion[0] es el mejor (está ordenado descendentemente por fitness)
		best_fitness = poblacion[0].fitness;
		memcpy(sol, poblacion[0].array_int, m * sizeof(int));

		free(poblacion);
	}

	// 7) Liberar recursos
	free(sub_poblacion);
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
void cruzar(Individuo *padre1, Individuo *padre2, Individuo *hijo1, Individuo *hijo2, int n, int m)
{
	// Selecciona un punto de cruce aleatorio entre 1 y m - 1.
	int punto = aleatorio(m - 1) + 1;

	// Re-inicializamos los hijos con -1
	memset(hijo1->array_int, -1, m * sizeof(int));
	memset(hijo2->array_int, -1, m * sizeof(int));

	// Construcción del hijo1:
	// Copia los genes desde el inicio hasta el punto de cruce del padre1.
	for (int i = 0; i < punto; i++)
	{
		hijo1->array_int[i] = padre1->array_int[i];
	}

	int index = punto;
	// Completa el resto de genes con los del padre2, evitando duplicados.
	for (int i = 0; i < m && index < m; i++)
	{
		int gene = padre2->array_int[i];
		if (!find_element(hijo1->array_int, index, gene))
		{
			hijo1->array_int[index++] = gene;
		}
	}

	// Si aún faltan genes, añade genes aleatorios no repetidos.
	while (index < m)
	{
		int gene = aleatorio(n);
		if (!find_element(hijo1->array_int, index, gene))
		{
			hijo1->array_int[index++] = gene;
		}
	}

	// Construcción del hijo2 (similar al hijo1 pero intercambiando los padres):
	for (int i = 0; i < punto; i++)
	{
		hijo2->array_int[i] = padre2->array_int[i];
	}

	index = punto;
	for (int i = 0; i < m && index < m; i++)
	{
		int gene = padre1->array_int[i];
		if (!find_element(hijo2->array_int, index, gene))
		{
			hijo2->array_int[index++] = gene;
		}
	}

	while (index < m)
	{
		int gene = aleatorio(n);
		if (!find_element(hijo2->array_int, index, gene))
		{
			hijo2->array_int[index++] = gene;
		}
	}
}

/**
 * Función que aplica mutación a un individuo, modificando algunos de sus genes.
 *
 * @param actual Puntero al individuo a mutar.
 * @param n Número total de elementos posibles.
 * @param m Tamaño del individuo (número de genes).
 */
void mutar(Individuo *actual, int n, int m)
{
	// Calcula el número de mutaciones a realizar basado en la tasa de mutación.
	int num_mutations = (int)(MUTATION_RATE * m);
	if (num_mutations < 1)
		num_mutations = 1; // Asegura al menos una mutación.

	// Realiza 'num_mutations' mutaciones en el individuo.
	for (int i = 0; i < num_mutations; i++)
	{
		// Selecciona una posición aleatoria en el individuo para mutar.
		int pos = aleatorio(m);

		// Eliminamos temporalmente el gen para evitar duplicados
		actual->array_int[pos] = -1;

		int new_gene;
		do
		{
			new_gene = aleatorio(n);
		} while (find_element(actual->array_int, m, new_gene));

		// Reemplazamos con el nuevo gen
		actual->array_int[pos] = new_gene;
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
double distancia_ij(const double *d, int i, int j, int n)
{
	if (i == j)
		return 0.0; // La distancia de un elemento consigo mismo es cero.
	if (i > j)
	{
		// Asegura que i < j para el cálculo del índice.
		int temp = i;
		i = j;
		j = temp;
	}

	// Calcula el índice en el array 'd' para los elementos i y j.
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
void fitness(const double *d, Individuo *individuo, int n, int m)
{
	double sum = 0.0;
	// Recorre todos los pares de genes en el individuo.
	for (int i = 0; i < m - 1; i++)
	{
		for (int j = i + 1; j < m; j++)
		{
			sum += distancia_ij(d, individuo->array_int[i], individuo->array_int[j], n);
		}
	}
	individuo->fitness = sum;
}
