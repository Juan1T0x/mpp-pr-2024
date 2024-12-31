#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "../include/mh.h"

#define MUTATION_RATE 0.15
#define PRINT 0

int aleatorio(int n) {
	return (rand() % n);  // genera un numero aleatorio entre 0 y n-1
}

int find_element(int *array, int end, int element)
{
	int i=0;
	int found=0;
	
	// comprueba que un elemento no está incluido en el individuo (el cual no admite enteros repetidos)
	while((i < end) && ! found) {
		if(array[i] == element) {
			found = 1;
		}
		i++;
	}
	return found;
}

int *crear_individuo(int n, int m)
{
	int i=0, value;
	int *individuo = (int *) malloc(m * sizeof(int));
	
	// inicializa array de elementos
	memset(individuo, -1, m * sizeof(int));
	
	while(i < m) {
		value = aleatorio(n);
		// si el nuevo elemento no está en el array...
		if(!find_element(individuo, i, value)) {
			individuo[i] = value;  // lo incluimos
			i++;
		}
	}
	return individuo;
}

int comp_array_int(const void *a, const void *b) {
	return (*(int *)a - *(int *)b);
}

int comp_fitness(const void *a, const void *b) {
	/* qsort pasa un puntero al elemento que está ordenando */
	return (*(Individuo **)a)->fitness - (*(Individuo **)b)->fitness;
}

double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, int *sol)
{
        // para reiniciar la secuencia pseudoaleatoria en cada ejecución
        srand(time(NULL) + getpid());
	int i, g, mutation_start;
	
	// crea poblacion inicial (array de individuos)
	Individuo **poblacion = (Individuo **) malloc(tam_pob * sizeof(Individuo *));
	assert(poblacion);
	
	// crea cada individuo (array de enteros aleatorios)
	for(i = 0; i < tam_pob; i++) {
		poblacion[i] = (Individuo *) malloc(sizeof(Individuo));
		poblacion[i]->array_int = crear_individuo(n, m);
		
		// calcula el fitness del individuo
		fitness(d, poblacion[i], n, m);
	}
	
	// ordena individuos segun la funcion de bondad (mayor "fitness" --> mas aptos)
	qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness);
	
	// evoluciona la poblacion durante un numero de generaciones
	for(g = 0; g < n_gen; g++)
	{
		// los hijos de los ascendientes mas aptos sustituyen a la ultima mitad de los individuos menos aptos
		for(i = 0; i < (tam_pob/2) - 1; i += 2) {
			cruzar(poblacion[i], poblacion[i+1], poblacion[tam_pob/2 + i], poblacion[tam_pob/2 + i + 1], n, m);
		}
		
		// inicia la mutacion a partir de 1/4 de la poblacion
		mutation_start = tam_pob/4;
		
		// muta 3/4 partes de la poblacion
		for(i = mutation_start; i < tam_pob; i++) {
			mutar(poblacion[i], n, m);
		}
		
		// recalcula el fitness del individuo
		for(i = 0; i < tam_pob; i++) {
			fitness(d, poblacion[i], n, m);
		}
		
		// ordena individuos segun la funcion de bondad (mayor "fitness" --> mas aptos)
		qsort(poblacion, tam_pob, sizeof(Individuo *), comp_fitness);
		
		if (PRINT) {
			printf("Generacion %d - ", g);
			printf("Fitness = %.0lf\n", -(poblacion[0]->fitness));
		}
	}
	
	// ordena el array solucion
	qsort(poblacion[0]->array_int, m, sizeof(int), comp_array_int);

        // y lo mueve a sol para escribirlo
	memmove(sol, poblacion[0]->array_int, m*sizeof(int));
	
	// almacena el mejor valor obtenido para el fitness
	double value = -(poblacion[0]->fitness);
	
	// se libera la memoria reservada
	free(poblacion);
	
	// devuelve el valor obtenido para el fitness
	return value;
}

void cruzar(Individuo *padre1, Individuo *padre2, Individuo *hijo1, Individuo *hijo2, int n, int m)
{
	// Elegir un "punto" de corte aleatorio a partir del que se realiza el intercambio de los genes
	
	// Los primeros genes del padre1 van al hijo1. Idem para el padre2 e hijo2.
	
	// Y los restantes son del otro padre, respectivamente.
	
	// Factibilizar: eliminar posibles repetidos de ambos hijos
	// Si encuentro alguno repetido en el hijo1, lo cambio por otro que no este en el conjunto
}

void mutar(Individuo *actual, int n, int m)
{
	// Decidir cuantos elementos mutar:
	// Si el valor es demasiado pequeño la convergencia es muy pequeña y si es demasiado alto diverge
	
	// Cambia el valor de algunos elementos de array_int de forma aleatoria
	// teniendo en cuenta que no puede haber elementos repetidos:
        // una posibilidad podría ser usar una variable, m_rate, para establecer la intensidad de la mutación 
        // (un bucle for con un número de iteraciones que dependa, por ejemplo, de m_rate*m)
}

double distancia_ij(const double *d, int i, int j, int n)
{
        // Devuelve la distancia entre dos elementos i, j de la matriz 'd'
}


void fitness(const double *d, Individuo *individuo, int n, int m)
{
	// Determina la calidad del individuo calculando la suma de la distancia entre cada par de enteros
}
