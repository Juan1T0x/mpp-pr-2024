// mh.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

#include "../include/mh.h"

// #define MUTATION_RATE 0.15
#define MUTATION_RATE 0.05 // Se reduce la tasa de mutación para mejorar el fitness obtenido.
#define PRINT 0

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4
#endif

unsigned int seed;
#pragma omp threadprivate(seed)

int chunk_size = CHUNK_SIZE;

/**
 * Función que genera un número entero aleatorio entre 0 y n-1.
 *
 * @param n Límite superior exclusivo del número aleatorio.
 * @return Un número entero aleatorio entre 0 y n-1.
 */
int aleatorio(int n)
{
	// Genera y devuelve un número aleatorio entre 0 y n-1.
	return (rand_r(&seed) % n);
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
 * Función que crea un individuo (solución) generando un array de 'm' enteros únicos aleatorios entre 0 y n-1.
 *
 * @param n Número total de elementos posibles.
 * @param m Tamaño del individuo (número de elementos a seleccionar).
 * @return Un puntero al array de enteros que representa al individuo.
 */
int *crear_individuo(int n, int m)
{
	int i = 0, value;
	int *individuo = (int *)malloc(m * sizeof(int));

	// Inicializa el array del individuo con -1 en todas las posiciones.
	memset(individuo, -1, m * sizeof(int));

	// Genera 'm' valores únicos aleatorios.
	while (i < m)
	{
		value = aleatorio(n);
		// Si el nuevo valor no está ya en el individuo, lo añade.
		if (!find_element(individuo, i, value))
		{
			individuo[i] = value;
			i++;
		}
	}
	// Devuelve el puntero al array del individuo.
	return individuo;
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
	// Obtiene los valores de fitness de los individuos.
	double fitness_a = (*(Individuo **)a)->fitness;
	double fitness_b = (*(Individuo **)b)->fitness;

	// Compara los valores de fitness para ordenar en orden descendente.
	if (fitness_a > fitness_b)
		return -1; // 'a' tiene mayor fitness, debe ir antes.
	else if (fitness_a < fitness_b)
		return 1; // 'b' tiene mayor fitness, 'a' debe ir después.
	else
		return 0; // Fitness iguales.
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
double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, int *sol)
{
	int i, g, mutation_start;
	Individuo **poblacion = (Individuo **)malloc(tam_pob * sizeof(Individuo *));
	assert(poblacion);

#pragma omp parallel private(i)
	{
		seed = (unsigned int)time(NULL) + omp_get_thread_num();

#pragma omp for schedule(dynamic, chunk_size)
		for (i = 0; i < tam_pob; i++)
		{
			poblacion[i] = (Individuo *)malloc(sizeof(Individuo));
			poblacion[i]->array_int = crear_individuo(n, m);
			fitness(d, poblacion[i], n, m);
		}
	}

	qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness);

	for (g = 0; g < n_gen; g++)
	{
#pragma omp parallel private(i)
		{
#pragma omp for schedule(dynamic, chunk_size)
			for (i = 0; i < (tam_pob / 2) - 1; i += 2)
			{
				if (poblacion[tam_pob / 2 + i]->array_int == NULL)
					poblacion[tam_pob / 2 + i]->array_int = (int *)malloc(m * sizeof(int));
				if (poblacion[tam_pob / 2 + i + 1]->array_int == NULL)
					poblacion[tam_pob / 2 + i + 1]->array_int = (int *)malloc(m * sizeof(int));

				cruzar(poblacion[i], poblacion[i + 1],
					   poblacion[tam_pob / 2 + i], poblacion[tam_pob / 2 + i + 1],
					   n, m);
			}
		}

		mutation_start = tam_pob / 4;

#pragma omp parallel for private(i) schedule(dynamic, chunk_size)
		for (i = mutation_start; i < tam_pob; i++)
		{
			seed = (unsigned int)time(NULL) + omp_get_thread_num() + i;
			mutar(poblacion[i], n, m);
		}

#pragma omp parallel for private(i) schedule(dynamic, chunk_size)
		for (i = 0; i < tam_pob; i++)
		{
			fitness(d, poblacion[i], n, m);
		}

		qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness);

		if (PRINT)
		{
			printf("Generación %d - Fitness = %.2lf\n", g, poblacion[0]->fitness);
		}
	}

	qsort(poblacion[0]->array_int, m, sizeof(int), comp_array_int);
	memmove(sol, poblacion[0]->array_int, m * sizeof(int));
	double value = poblacion[0]->fitness;

	for (i = 0; i < tam_pob; i++)
	{
		free(poblacion[i]->array_int);
		free(poblacion[i]);
	}
	free(poblacion);

	return value;
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

	// Asegura que los arrays de los hijos están asignados y inicializados.
	if (hijo1->array_int == NULL)
		hijo1->array_int = (int *)malloc(m * sizeof(int));
	if (hijo2->array_int == NULL)
		hijo2->array_int = (int *)malloc(m * sizeof(int));

	memset(hijo1->array_int, -1, m * sizeof(int));
	memset(hijo2->array_int, -1, m * sizeof(int));

	// Construcción del hijo1:
	// Copia los genes desde el inicio hasta el punto de cruce del padre1.
	for (int i = 0; i < punto; i++)
		hijo1->array_int[i] = padre1->array_int[i];

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
			hijo1->array_int[index++] = gene;
	}

	// Construcción del hijo2 (similar al hijo1 pero intercambiando los padres):
	// Copia los genes desde el inicio hasta el punto de cruce del padre2.
	for (int i = 0; i < punto; i++)
		hijo2->array_int[i] = padre2->array_int[i];

	index = punto;
	// Completa el resto de genes con los del padre1, evitando duplicados.
	for (int i = 0; i < m && index < m; i++)
	{
		int gene = padre1->array_int[i];
		if (!find_element(hijo2->array_int, index, gene))
		{
			hijo2->array_int[index++] = gene;
		}
	}

	// Si aún faltan genes, añade genes aleatorios no repetidos.
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
		int old_gene = actual->array_int[pos];

		// Elimina temporalmente el gen antiguo para evitar duplicados.
		actual->array_int[pos] = -1;

		int new_gene;
		// Busca un nuevo gen aleatorio que no esté ya en el individuo.
		do
		{
			new_gene = aleatorio(n);
		} while (find_element(actual->array_int, m, new_gene));

		// Reemplaza el gen antiguo por el nuevo gen.
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
	// Devuelve la distancia correspondiente.
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
	int *elements = individuo->array_int;
	double sum = 0.0;
	// Recorre todos los pares de genes en el individuo.
	for (int i = 0; i < m - 1; i++)
	{
		for (int j = i + 1; j < m; j++)
		{
			// Suma la distancia entre los genes i y j al total.
			sum += distancia_ij(d, elements[i], elements[j], n);
		}
	}
	// Asigna el valor de fitness calculado al individuo.
	individuo->fitness = sum;
}
