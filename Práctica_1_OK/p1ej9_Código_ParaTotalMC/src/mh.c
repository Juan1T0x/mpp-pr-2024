// mh.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

#include "../include/mh.h"

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 4
#endif

int chunk_size = CHUNK_SIZE;

// MUTATION_RATE se mantiene igual
#define MUTATION_RATE 0.05
#define PRINT 0

unsigned int seed;
#pragma omp threadprivate(seed)

// Declaraciones de las nuevas funciones
void mezclar(Individuo **poblacion, int izq, int med, int der);
void mergeSort(Individuo **poblacion, int izq, int der);

/**
 * Función que genera un número entero aleatorio entre 0 y n-1.
 *
 * @param n Límite superior exclusivo del número aleatorio.
 * @return Un número entero aleatorio entre 0 y n-1.
 */
int aleatorio(int n)
{
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
	while ((i < end) && !found)
	{
		if (array[i] == element)
		{
			found = 1;
		}
		i++;
	}
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
	memset(individuo, -1, m * sizeof(int));

	while (i < m)
	{
		value = aleatorio(n);
		if (!find_element(individuo, i, value))
		{
			individuo[i] = value;
			i++;
		}
	}
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
	double fitness_a = (*(Individuo **)a)->fitness;
	double fitness_b = (*(Individuo **)b)->fitness;
	if (fitness_a > fitness_b)
		return -1;
	else if (fitness_a < fitness_b)
		return 1;
	else
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
double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, int *sol)
{
	int i, g, mutation_start;

	Individuo **poblacion = (Individuo **)malloc(tam_pob * sizeof(Individuo *));
	assert(poblacion);

	// Paralelizamos la generación de la población inicial con dinámica para balancear
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

	// Ordenar la población usando mergeSort con tareas
#pragma omp parallel
	{
#pragma omp single
		mergeSort(poblacion, 0, tam_pob);
	}

	for (g = 0; g < n_gen; g++)
	{
		// Paralelizamos el cruce: trabajo uniforme, usamos static
		// (Observación: i salta de 2 en 2, se mantiene; cada iteración procesa un par)
#pragma omp parallel for private(i) schedule(static, chunk_size)
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

		mutation_start = tam_pob / 4;

		// Mutación: puede variar el coste, usamos dynamic
#pragma omp parallel for private(i) schedule(dynamic, chunk_size)
		for (i = mutation_start; i < tam_pob; i++)
		{
			seed = (unsigned int)time(NULL) + omp_get_thread_num() + i;
			mutar(poblacion[i], n, m);
		}

		// Recalcular fitness: también dynamic por posible variabilidad
#pragma omp parallel for private(i) schedule(dynamic, chunk_size)
		for (i = 0; i < tam_pob; i++)
		{
			fitness(d, poblacion[i], n, m);
		}

#pragma omp parallel
		{
#pragma omp single
			mergeSort(poblacion, 0, tam_pob);
		}

		if (PRINT)
		{
			printf("Generación %d - ", g);
			printf("Fitness = %.2lf\n", poblacion[0]->fitness);
		}
	}

	// Ordena el mejor individuo internamente con qsort (array pequeño, secuencial)
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
	int punto = aleatorio(m - 1) + 1;

	if (hijo1->array_int == NULL)
		hijo1->array_int = (int *)malloc(m * sizeof(int));
	if (hijo2->array_int == NULL)
		hijo2->array_int = (int *)malloc(m * sizeof(int));

	memset(hijo1->array_int, -1, m * sizeof(int));
	memset(hijo2->array_int, -1, m * sizeof(int));

	// hijo1
	for (int i = 0; i < punto; i++)
		hijo1->array_int[i] = padre1->array_int[i];
	int index = punto;
	for (int i = 0; i < m && index < m; i++)
	{
		int gene = padre2->array_int[i];
		if (!find_element(hijo1->array_int, index, gene))
			hijo1->array_int[index++] = gene;
	}
	while (index < m)
	{
		int gene = aleatorio(n);
		if (!find_element(hijo1->array_int, index, gene))
			hijo1->array_int[index++] = gene;
	}

	// hijo2
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
void mutar(Individuo *actual, int n, int m)
{
	int num_mutations = (int)(MUTATION_RATE * m);
	if (num_mutations < 1)
		num_mutations = 1;

	for (int i = 0; i < num_mutations; i++)
	{
		int pos = aleatorio(m);
		actual->array_int[pos] = -1;
		int new_gene;
		do
		{
			new_gene = aleatorio(n);
		} while (find_element(actual->array_int, m, new_gene));
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
		return 0.0;
	if (i > j)
	{
		int temp = i;
		i = j;
		j = temp;
	}
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
	int *elements = individuo->array_int;
	double sum = 0.0;

	// Doble bucle: puede ser costoso.
	// Opcional: paralelizar dentro de fitness? Podría ser sobrecoste.
	// Este fitness se llama a menudo. Es mejor paralelizar a nivel más alto (aplicar_mh).
	// Dejamos secuencial para no incurrir en overhead.
	for (int i = 0; i < m - 1; i++)
	{
		for (int j = i + 1; j < m; j++)
		{
			sum += distancia_ij(d, elements[i], elements[j], n);
		}
	}
	individuo->fitness = sum;
}

void mezclar(Individuo **poblacion, int izq, int med, int der)
{
	int i = izq, j = med, k = 0;
	int n = der - izq;
	Individuo **pob = (Individuo **)malloc(n * sizeof(Individuo *));

	while (i < med && j < der)
	{
		if (poblacion[i]->fitness > poblacion[j]->fitness)
			pob[k++] = poblacion[i++];
		else
			pob[k++] = poblacion[j++];
	}
	while (i < med)
		pob[k++] = poblacion[i++];
	while (j < der)
		pob[k++] = poblacion[j++];

	memmove(&poblacion[izq], pob, n * sizeof(Individuo *));
	free(pob);
}

void mergeSort(Individuo **poblacion, int izq, int der)
{
	if ((der - izq) < 2)
		return;

	int med = (izq + der) / 2;
	int threshold = 100;

#pragma omp task shared(poblacion) if ((der - izq) > threshold)
	mergeSort(poblacion, izq, med);

#pragma omp task shared(poblacion) if ((der - izq) > threshold)
	mergeSort(poblacion, med, der);

#pragma omp taskwait
	mezclar(poblacion, izq, med, der);
}
