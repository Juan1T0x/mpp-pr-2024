/******************************************************************************
 * mh.c
 * Versión con dos nuevos parámetros:
 *   - n_hilos_ini (para paralelizar la generación inicial FOR_INI),
 *   - n_hilos_fit (para paralelizar el bucle interno en fitness, FOR_FIT).
 * Se instrumenta para medir el tiempo del bucle FOR_INI.
 *
 * Cambios:
 *  1) Uso de omp_set_nested(1) para habilitar paralelismo anidado.
 *  2) Los bucles FOR_INI y FOR_FIT se mantienen paralelizados.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <omp.h> /* <-- importante para OpenMP */
#include "../include/mh.h"

#define MUTATION_RATE 0.05 // Se reduce la tasa de mutación para mejorar el fitness
#define PRINT 0

/* --------------------------------------------------------------------------
   Funciones auxiliares (aleatorio, find_element, crear_individuo, etc.)
-------------------------------------------------------------------------- */

/**
 * Genera un número entero aleatorio en [0..n-1].
 */
int aleatorio(int n)
{
	// Simplemente se usa rand(); si deseas rand_r,
	// añade la lógica threadprivate de la semilla.
	return (rand() % n);
}

/**
 * Verifica si 'element' está en array[0..end-1].
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
 * Crea un individuo de tamaño 'm' con valores únicos [0..n-1].
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
 * Comparación para qsort (enteros ascendentes).
 */
int comp_array_int(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

/**
 * Comparación para qsort (individuos por fitness descendente).
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
 * Calcula la distancia entre i y j usando el array compactado 'd'.
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

/* --------------------------------------------------------------------------
   Cálculo de fitness en paralelo (FOR_FIT)
   Se asume que anidaremos paralelismo si es necesario
-------------------------------------------------------------------------- */
void fitness_parallel(const double *d, Individuo *ind, int n, int m, int n_hilos_fit)
{
	int *elem = ind->array_int;
	double sum = 0.0;

#pragma omp parallel for num_threads(n_hilos_fit) reduction(+ : sum) schedule(static)
	for (int i = 0; i < m - 1; i++)
	{
		for (int j = i + 1; j < m; j++)
		{
			sum += distancia_ij(d, elem[i], elem[j], n);
		}
	}
	ind->fitness = sum;
}

/* --------------------------------------------------------------------------
   Cruce y mutación
-------------------------------------------------------------------------- */
void cruzar(Individuo *padre1, Individuo *padre2,
			Individuo *hijo1, Individuo *hijo2,
			int n, int m)
{
	int punto = aleatorio(m - 1) + 1;

	if (hijo1->array_int == NULL)
		hijo1->array_int = (int *)malloc(m * sizeof(int));
	if (hijo2->array_int == NULL)
		hijo2->array_int = (int *)malloc(m * sizeof(int));

	memset(hijo1->array_int, -1, m * sizeof(int));
	memset(hijo2->array_int, -1, m * sizeof(int));

	// Hijo1
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

	// Hijo2
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

/* --------------------------------------------------------------------------
   Función principal (aplicar_mh) con 2 nuevos parámetros: n_hilos_ini y n_hilos_fit
   Se instrumenta para medir tiempo de FOR_INI.
   Se habilita paralelismo anidado con omp_set_nested(1).
-------------------------------------------------------------------------- */
double aplicar_mh(const double *d,
				  int n,
				  int m,
				  int n_gen,
				  int tam_pob,
				  int *sol,
				  int n_hilos_ini, /* nuevo */
				  int n_hilos_fit) /* nuevo */
{
	// (1) Habilitar anidado
	omp_set_nested(1);

	srand(time(NULL) + getpid());
	int i, g, mutation_start;

	// Crear población
	Individuo **poblacion = (Individuo **)malloc(tam_pob * sizeof(Individuo *));
	assert(poblacion);

	// 1) Instrumentación: medir tiempo de FOR_INI
	double t_ini_inicial = omp_get_wtime(); // <-- Captura tiempo inicial

	// FOR_INI: generar cada individuo y su fitness
#pragma omp parallel for num_threads(n_hilos_ini) schedule(static)
	for (i = 0; i < tam_pob; i++)
	{
		poblacion[i] = (Individuo *)malloc(sizeof(Individuo));
		poblacion[i]->array_int = crear_individuo(n, m);

		// Calcula fitness inicial con n_hilos_fit
		fitness_parallel(d, poblacion[i], n, m, n_hilos_fit);
	}

	double t_fin_inicial = omp_get_wtime(); // <-- Captura tiempo final de FOR_INI
	double tiempo_for_ini = t_fin_inicial - t_ini_inicial;

	// Imprimir el tiempo medido de FOR_INI
	printf("Tiempo FOR_INI con %d hilos: %.6f seg\n", n_hilos_ini, tiempo_for_ini);

	// Ordenar población (fitness desc)
	qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness);

	// 2) Evolución
	for (g = 0; g < n_gen; g++)
	{
		// Cruce
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

		// Mutación en 75% de la pob
		mutation_start = tam_pob / 4;
		for (i = mutation_start; i < tam_pob; i++)
		{
			mutar(poblacion[i], n, m);
		}

		// Recalcular fitness con n_hilos_fit (FOR_FIT)
		for (i = 0; i < tam_pob; i++)
		{
			fitness_parallel(d, poblacion[i], n, m, n_hilos_fit);
		}

		// Reordenar
		qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness);

		if (PRINT)
		{
			printf("Generación %d - Fitness = %.2lf\n", g, poblacion[0]->fitness);
		}
	}

	// Orden final de la mejor solución
	qsort(poblacion[0]->array_int, m, sizeof(int), comp_array_int);

	double best_value = poblacion[0]->fitness;
	memmove(sol, poblacion[0]->array_int, m * sizeof(int));

	// Liberar
	for (i = 0; i < tam_pob; i++)
	{
		free(poblacion[i]->array_int);
		free(poblacion[i]);
	}
	free(poblacion);

	return best_value;
}
