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
 * Función auxiliar:
 * Empaqueta 'count' individuos del arreglo 'poblacion[start]'... en un buffer.
 *
 * Se asume que cada Individuo tiene:
 *  - array_int[m] (MPI_INT)
 *  - fitness (MPI_DOUBLE)
 */
static void pack_individuos(Individuo *poblacion, int start, int count,
							int m, char *buffer, int buffer_size, int root)
{
	int position = 0;
	for (int i = 0; i < count; i++)
	{
		// Empaqueta array_int
		MPI_Pack(poblacion[start + i].array_int, m,
				 MPI_INT, buffer, buffer_size, &position, MPI_COMM_WORLD);
		// Empaqueta fitness
		MPI_Pack(&(poblacion[start + i].fitness), 1,
				 MPI_DOUBLE, buffer, buffer_size, &position, MPI_COMM_WORLD);
	}

	// Envía a 'root' (o a cada rank, según sea un gather o scatter).
	// Nota: El "TAG" y si se usa MPI_Send/MPI_Bcast/etc. depende del contexto.
	MPI_Send(buffer, position, MPI_PACKED, root, 0, MPI_COMM_WORLD);
}

/**
 * Función auxiliar:
 * Desempaqueta 'count' individuos en el arreglo 'dest' desde un buffer.
 */
static void unpack_individuos(Individuo *dest, int count, int m,
							  char *buffer, int buffer_size)
{
	int position = 0;
	for (int i = 0; i < count; i++)
	{
		MPI_Unpack(buffer, buffer_size, &position,
				   dest[i].array_int, m, MPI_INT, MPI_COMM_WORLD);
		MPI_Unpack(buffer, buffer_size, &position,
				   &(dest[i].fitness), 1, MPI_DOUBLE, MPI_COMM_WORLD);
	}
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

	// Calcular cuántos individuos van a cada proceso
	int NEM = tam_pob / size;
	int sobrante = tam_pob % size;

	// Reserva para la subpoblación local
	int local_count = (rank == size - 1) ? (NEM + sobrante) : NEM;
	Individuo *sub_poblacion = malloc(local_count * sizeof(Individuo));
	assert(sub_poblacion);

	// Reserva en maestro para la población total
	Individuo *poblacion = NULL;
	if (rank == 0)
	{
		poblacion = malloc(tam_pob * sizeof(Individuo));
		assert(poblacion);
		// Inicializar la población
		for (int i = 0; i < tam_pob; i++)
		{
			inicializar_individuo(&poblacion[i], n, m);
			fitness(d, &poblacion[i], n, m);
		}
	}

	// Arreglos para saber cuántos individuos van a cada proceso
	int *sendcounts = malloc(size * sizeof(int));
	int *displs = malloc(size * sizeof(int));
	for (int i = 0; i < size; i++)
	{
		sendcounts[i] = (i == size - 1) ? (NEM + sobrante) : NEM;
		displs[i] = i * NEM;
	}

	// -------------------------------------------
	// Pre-cálculo del tamaño de buffer para un individuo
	// Se empaqueta: array_int[m] (MPI_INT) + fitness (MPI_DOUBLE)
	// -------------------------------------------
	int size_int, size_double;
	MPI_Pack_size(m, MPI_INT, MPI_COMM_WORLD, &size_int);
	MPI_Pack_size(1, MPI_DOUBLE, MPI_COMM_WORLD, &size_double);
	int size_one_ind = size_int + size_double;

	// Búfer para envíos y recepciones (en maestro puede ser grande).
	// En scatter, el maestro envía a cada proceso => tamaño máx = sendcounts[i] * size_one_ind
	// En gather, cada proceso envía y maestro recibe => tamaño máx = local_count * size_one_ind
	int max_send = 0;
	for (int i = 0; i < size; i++)
	{
		int tmp = sendcounts[i] * size_one_ind;
		if (tmp > max_send)
			max_send = tmp;
	}
	// Por seguridad, hacemos uno global:
	int max_local_size = local_count * size_one_ind;
	if (max_local_size > max_send)
		max_send = max_local_size;

	char *buffer = (char *)malloc(max_send); // Búfer de uso general

	// ===========================================
	// Bucle de generaciones
	// ===========================================
	for (int g = 0; g < n_gen; g++)
	{
		// ---------------------------------
		// SCATTER manual con Pack/Unpack
		// ---------------------------------
		if (rank == 0)
		{
			// Envía a cada proceso i su parte de población
			for (int i = 0; i < size; i++)
			{
				if (i == 0)
				{
					// El maestro se "copia" su parte local sin MPI
					memcpy(sub_poblacion,
						   &poblacion[displs[i]],
						   sendcounts[i] * sizeof(Individuo));
				}
				else
				{
					int count_i = sendcounts[i];
					int start_i = displs[i];
					// Empaquetamos esos 'count_i' individuos
					int buffer_size = count_i * size_one_ind;
					// Llenamos 'buffer' y enviamos a rank i
					pack_individuos(poblacion, start_i, count_i, m,
									buffer, buffer_size, i);
				}
			}
		}
		else
		{
			// Los procesos != 0 reciben su porción
			int count_local = local_count;
			int buffer_size = count_local * size_one_ind;
			MPI_Recv(buffer, buffer_size, MPI_PACKED, 0, 0,
					 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// Desempaquetar
			unpack_individuos(sub_poblacion, count_local, m,
							  buffer, buffer_size);
		}

		// -----------------------------
		// Evolución local en cada rank
		// -----------------------------
		for (int iter = 0; iter < ngm; iter++)
		{
			// Cruce
			for (int i = 0; i < local_count / 2 - 1; i += 2)
			{
				cruzar(&sub_poblacion[i], &sub_poblacion[i + 1],
					   &sub_poblacion[local_count / 2 + i],
					   &sub_poblacion[local_count / 2 + i + 1],
					   n, m);
			}
			// Mutación y fitness
			for (int i = 0; i < local_count; i++)
			{
				mutar(&sub_poblacion[i], n, m);
				fitness(d, &sub_poblacion[i], n, m);
			}
			// Ordenar sub_población
			qsort(sub_poblacion, local_count, sizeof(Individuo), comp_fitness);
		}

		// ---------------------------------
		// GATHER manual con Pack/Unpack
		// ---------------------------------
		if (rank == 0)
		{
			// Copiamos la subpoblación local del maestro directo a 'poblacion'
			memcpy(&poblacion[displs[0]],
				   sub_poblacion,
				   sendcounts[0] * sizeof(Individuo));

			// Recibimos del resto de procesos
			for (int i = 1; i < size; i++)
			{
				int count_i = sendcounts[i];
				int start_i = displs[i];
				int buffer_size = count_i * size_one_ind;

				MPI_Recv(buffer, buffer_size, MPI_PACKED, i, 0,
						 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				// Desempaquetamos en poblacion[start_i..start_i+count_i-1]
				unpack_individuos(&poblacion[start_i], count_i, m,
								  buffer, buffer_size);
			}

			// --- En rank=0, hacemos la "migración" (ordenar, etc.)
			qsort(poblacion, tam_pob, sizeof(Individuo), comp_fitness);
			// [Ejemplo] No se implementa ninguna migración adicional,
			// se podría redistribuir los mejores, etc.
		}
		else
		{
			// Cada rank distinto de 0 empaqueta su subpoblación y la envía al maestro
			int buffer_size = local_count * size_one_ind;
			pack_individuos(sub_poblacion, 0, local_count, m,
							buffer, buffer_size, 0);
		}

		// (Aquí podría haber una barrera si se desea sincronizar)
		// MPI_Barrier(MPI_COMM_WORLD);
	} // fin de for (n_gen)

	// ---------------------------------
	// Final: obtener mejor fitness
	// ---------------------------------
	double best_fitness = 0.0;
	if (rank == 0)
	{
		// La población ya está ordenada globalmente por fitness
		memcpy(sol, poblacion[0].array_int, m * sizeof(int));
		best_fitness = poblacion[0].fitness;
		free(poblacion);
	}

	free(sub_poblacion);
	free(sendcounts);
	free(displs);
	free(buffer);

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
